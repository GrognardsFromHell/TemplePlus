
#pragma once
#include <util/addresses.h>

enum class TigSoundType : uint32_t {
	Effects = 0,
	Music = 1,
	Voice = 2,
	ThreeD = 3 // Positional
};

struct TigSoundAddresses : AddressTable {

	int *movieVolume;

	int (__cdecl *AllocStream)(int *streamId, TigSoundType type);
	int (__cdecl *SetStreamVolume)(int streamId, int volume);
	void (__cdecl *LoadSound)(int streamId, const char *filename, int a4, bool allowReverb, int a5);
	bool (__cdecl *IsStreamPlaying)(int streamId);
	bool (__cdecl *IsStreamActive)(int streamId);
	void (__cdecl *FreeStream)(int streamId);

	TigSoundAddresses() {
		rebase(movieVolume, 0x103010F8); // 0-127

		rebase(AllocStream, 0x101E45B0);
		rebase(SetStreamVolume, 0x101E3B60);
		rebase(LoadSound, 0x101E3B00);
		rebase(IsStreamPlaying, 0x101E3D40);
		rebase(IsStreamActive, 0x101E3DC0);
		rebase(FreeStream, 0x101E36D0);
	}
	
};

extern TigSoundAddresses tigSoundAddresses;

class TigSoundStreamWrapper {
public:

	~TigSoundStreamWrapper() {
		if (mStreamId != -1) {
			tigSoundAddresses.FreeStream(mStreamId);
		}
	}

	bool Play(const string &filename, TigSoundType type) {
		assert(mStreamId == -1);
		tigSoundAddresses.AllocStream(&mStreamId, type);
		tigSoundAddresses.LoadSound(mStreamId, filename.c_str(), 0, true, -1);
		if (!tigSoundAddresses.IsStreamActive(mStreamId)) {
			mStreamId = -1;
		}
		return mStreamId != -1;
	}

	void SetVolume(int volume) {
		tigSoundAddresses.SetStreamVolume(mStreamId, volume);
	}

	bool IsValid() {
		return mStreamId != -1;
	}

	bool IsPlaying() {
		return tigSoundAddresses.IsStreamPlaying(mStreamId);
	}

private:
	int mStreamId = -1;

};
