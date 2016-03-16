
#include "stdafx.h"
#include "sound.h"

Sound sound;

static struct SoundAddresses : temple::AddressTable {
	// Seems to return the sound stream id or -1 on failure
	int(__cdecl *Sound)(int soundId, int loopCount);
	int(__cdecl *SoundAtObj)(int soundId, int loopCount, objHndl obj);
	int(__cdecl *SoundAtLoc)(int soundId, int loopCount, locXY loc);
	int(*MssSoundPlay)(int soundId);

	SoundAddresses() {
		
		rebase(Sound, 0x1003BDB0);
		rebase(SoundAtObj, 0x1003D090);
		rebase(SoundAtLoc, 0x1003DCB0);
		rebase(MssSoundPlay, 0x101E46A0);
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
