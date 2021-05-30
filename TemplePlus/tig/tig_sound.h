
#pragma once
#include <temple/dll.h>

enum class TigSoundType : uint32_t {
	Effects = 0,
	Music = 1,
	Voice = 2,
	ThreeD = 3 // Positional
};

struct TigSoundAddresses : temple::AddressTable {

	int *movieVolume;

	int (__cdecl *AllocStream)(int *streamId, TigSoundType type);
	int (__cdecl *SetStreamVolume)(int streamId, int volume);
	void (__cdecl *LoadSound)(int streamId, const char *filename, int a4, bool allowReverb, int a5);
	bool (__cdecl *IsStreamPlaying)(int streamId);
	bool (__cdecl *IsStreamActive)(int streamId);
	void (__cdecl *FreeStream)(int streamId);

	int StreamInit(int& streamIdOut, int streamType);
	void PlayInStream(int streamId, int soundId);
	void StreamEnd(int streamId);

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
