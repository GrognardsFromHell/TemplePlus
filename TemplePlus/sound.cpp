
#include "stdafx.h"
#include "sound.h"
#include "util/fixes.h"

Sound sound;

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


class SoundHooks: TempleFix
{
public: 
	const char* name() override { 
		return "Sound hooks";
	} 
	

	static int FindSound(int soundId, char* filename);

	void apply() override {
		

		// Find Sound
		replaceFunction(0x1003B9E0, FindSound);
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


	// try tpSounds.mes first

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
