
#include "stdafx.h"
#include "tig_sound.h"

TigSoundAddresses::TigSoundAddresses() {
	rebase(movieVolume, 0x103010F8); // 0-127

	rebase(AllocStream, 0x101E45B0);
	rebase(SetStreamVolume, 0x101E3B60);
	rebase(LoadSound, 0x101E3B00);
	rebase(IsStreamPlaying, 0x101E3D40);
	rebase(IsStreamActive, 0x101E3DC0);
	rebase(FreeStream, 0x101E36D0);
}

TigSoundAddresses tigSoundAddresses;

kiuuhgjkh
int TigSoundAddresses::StreamInit(int& streamIdOut, int streamType)
{
	static auto streamInit = temple::GetRef<int(__cdecl)(int&, int)>(0x101E45B0);
	auto result = streamInit(streamIdOut, streamType);
	return result;
}

void TigSoundAddresses::PlayInStream(int streamId, int soundId)
{
	static auto playInStream = temple::GetRef<void(__cdecl)(int, int)>(0x101E38D0);
	playInStream(streamId, soundId);
}
