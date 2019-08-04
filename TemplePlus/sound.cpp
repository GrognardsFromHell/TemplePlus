
#include "stdafx.h"
#include "sound.h"
#include "util/fixes.h"
#include <tig/tig_sound.h>

Sound sound;
std::map<int, std::string> Sound::mUserSounds;

static struct SoundAddresses : temple::AddressTable {
	// Seems to return the sound stream id or -1 on failure
	int(__cdecl *Sound)(int soundId, int loopCount);
	int(__cdecl *SoundAtObj)(int soundId, int loopCount, objHndl obj);
	int(__cdecl *SoundAtLoc)(int soundId, int loopCount, locXY loc);
	int(*MssSoundPlay)(int soundId);

	int * soundInited;
	char ** soundFolder;
	SoundAddresses() {
		
		rebase(Sound, 0x1003BDB0);
		rebase(SoundAtObj, 0x1003D090);
		rebase(SoundAtLoc, 0x1003DCB0);
		rebase(MssSoundPlay, 0x101E46A0);
		rebase(soundInited, 0x108F270C);
		rebase(soundFolder, 0x102AF784);
	}

	
} addresses;

int Sound::PlaySound(int soundId, int loopCount) {
	return addresses.Sound(soundId, loopCount);
}

int Sound::PlaySoundAtObj(int soundId, objHndl obj, int loopCount) {
	if (obj)
		return addresses.SoundAtObj(soundId, loopCount, obj);
	return 0;
}

int Sound::PlaySoundAtLoc(int soundId, locXY loc, int loopCount) {
	return addresses.SoundAtLoc(soundId, loopCount, loc);
}

int Sound::MssPlaySound(int soundId)
{
	return addresses.MssSoundPlay(soundId);
}

void Sound::Init(){

	// rules\spell_enums.mes extension
	TioFileList soundMesFiles;
	tio_filelist_create(&soundMesFiles, "sound\\user_sounds\\*.mes");

	for (auto i = 0; i< soundMesFiles.count; i++) {
		std::string combinedFname(fmt::format("sound\\user_sounds\\{}", soundMesFiles.files[i].name));
		mesFuncs.AddToMap(combinedFname, mUserSounds);
	}
	tio_filelist_destroy(&soundMesFiles);
}

class SoundHooks: TempleFix
{
public: 
	static int FindSound(int soundId, char* filename);

	void apply() override {
		

		// Find Sound
		replaceFunction(0x1003B9E0, FindSound);

		// Alloc Stream - hooked for feedback on errors
		static int (__cdecl*orgAllocStream)(int*, int) = replaceFunction<int(__cdecl)(int*, int)>(0x101E45B0, [](int* streamId, int soundType){
		
			auto soundInited = temple::GetRef<int>(0x10EE7570);
			if (!soundInited){
				*streamId = -1;
				return 1;
			}

			if (soundType != 1)	{
				auto result = orgAllocStream(streamId, soundType);
				if (result != 0) {
					logger->debug("Failed to allocate stream! Stream ID {}, sound type {}", *streamId, soundType);
				}
				auto &mss = temple::GetRef<MilesSoundSthg[]>(0x10EE7578)[*streamId];
				if (mss.streamSthg) {
					logger->debug("Non-null mss_stream caught");
				};
				return result;
			}

			// Fix for allocating music
			auto &streamSlots = temple::GetRef<MilesSoundSthg[]>(0x10EE7578);
			auto newStreamId = -1;
			
			for (auto i=2; i <= 5; i++){
				if (!streamSlots[i].isUsed){
					newStreamId = i;
					break;
				}
			}
			
			if (newStreamId == -1){
				auto &ringBufferStreamId = temple::GetRef<int>(0x10EED5A0);
				tigSoundAddresses.FreeStream(ringBufferStreamId);
				newStreamId = ringBufferStreamId++;
				if (ringBufferStreamId > 5)
					ringBufferStreamId = 2;
			}


			if (newStreamId != -1){
				memset(&streamSlots[newStreamId], 0, sizeof(MilesSoundSthg));
				streamSlots[newStreamId].field20 = -1;
				streamSlots[newStreamId].isUsed = 1;
				streamSlots[newStreamId].flags = 0x100;
				streamSlots[newStreamId].field10 = 1;
				streamSlots[newStreamId].curVolume = 127;
				streamSlots[newStreamId].field130 = 64;
				*streamId = newStreamId;
			}

			if (newStreamId == -1){
				logger->error("No Stream ID given for music track!");
				*streamId = -1;
				return 3;
				
			}

			if (soundType == 1 && *streamId == 1){
				logger->warn("Music stream given ID 1!");
			}

			auto &mss = temple::GetRef<MilesSoundSthg[]>(0x10EE7578)[*streamId];
			if (mss.streamSthg){
				logger->debug("Non-null mss_stream caught");
			};

			return 0;
		});

		static void(__cdecl*orgSetStreamVolume)(int, int) = replaceFunction<void(__cdecl)(int, int)>(0x101E3B60, [](int streamId, int volume){
			auto unk = temple::GetRef<int>(0x10EE7570);
			if (!unk)
				return;
				
			if (streamId < 0 || streamId >= 70){
				return;
			}
				
			auto &mss = temple::GetRef<MilesSoundSthg[]>(0x10EE7578)[streamId];
			auto flags = mss.flags;
			if (flags & 1){
				auto asdf = mss.streamSthg;
				logger->debug("Setting stream (ID {}) volume to {}, pointer to miles is {}", streamId, volume, asdf);
			}
			orgSetStreamVolume(streamId, volume);
		});
	}
} soundHooks;

int SoundHooks::FindSound(int soundId, char* filename){

	if (!*addresses.soundInited) {
		*filename = 0;
		return 17;
	}

	// weapon hit sounds (encoded data)
	if (soundId & 0xC0000000) {
		static auto getWeaponHitSound = temple::GetRef<int(__cdecl)(int, char*)>(0x1006E440);
		return getWeaponHitSound(soundId, filename);
	}


	// First try the Temple+ user sounds
	auto findUserSnd = sound.mUserSounds.find(soundId);
	if (findUserSnd != sound.mUserSounds.end()){
		sprintf(filename, "%s%s", *addresses.soundFolder, findUserSnd->second.c_str());
		return 0;
	}

	// try tpmes\sounds.mes

	MesLine mesline;
	mesline.key = soundId;

	if (sound.tpSounds != -1 && mesFuncs.GetLine(sound.tpSounds, &mesline)) {
		sprintf(filename, "%s%s", *addresses.soundFolder, mesline.value);
		return 0;
	};


	// search the master list (auto generated from snd_00index.mes)

	auto soundMasterListNum = temple::GetRef<int>(0x108F2708);
	auto soundMasterList = temple::GetRef<MesHandle*>(0x108F2738);

	for (int i = 0; i < soundMasterListNum; i++){

		if (mesFuncs.GetLine(soundMasterList[i], &mesline)){
			sprintf(filename, "%s%s", *addresses.soundFolder, mesline.value);
			return 0;
		}
	}

	// not found in those; check snd_user.mes

	auto soundUser = temple::GetRef<MesHandle>(0x108F28D0);
	if (soundUser != -1 && mesFuncs.GetLine(soundUser, &mesline)){
		sprintf(filename, "%s%s", *addresses.soundFolder, mesline.value);
		return 0;
	}

	*filename = 0;
	return 17;
}
