
#include "stdafx.h"
#include "tig_sound.h"

TigSoundAddresses::TigSoundAddresses() {
	rebase(movieVolume, 0x103010F8); // 0-127

	rebase(AllocStream, 0x101E45B0);
	rebase(SetStreamVolume, 0x101E3B60);
	rebase(PlayOnce, 0x101E3B00);
	rebase(IsStreamPlaying, 0x101E3D40);
	rebase(IsStreamActive, 0x101E3DC0);
	rebase(FreeStream, 0x101E36D0);
}

TigSoundAddresses tigSoundAddresses;


bool TigSoundAddresses::PlayInStream(int streamId, int soundId)
{
	static auto playInStream = temple::GetRef<BOOL(__cdecl)(int, int)>(0x101E38D0);
	return playInStream(streamId, soundId) != FALSE;
}

bool TigSoundStreamWrapper::Play(int soundId)
{
	assert(mStreamId != -1);
	return tigSoundAddresses.PlayInStream(mStreamId, soundId);
}

bool TigSoundStreamWrapper::Play(const string& filename, TigSoundType type)
{
	assert(mStreamId == -1);
	tigSoundAddresses.AllocStream(&mStreamId, type);
	tigSoundAddresses.PlayOnce(mStreamId, filename.c_str(), 0, true, -1);
	if (!tigSoundAddresses.IsStreamActive(mStreamId)) {
		mStreamId = -1;
	}
	return mStreamId != -1;
}
