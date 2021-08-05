
#pragma once
#include <temple/dll.h>

enum class TigSoundType : uint32_t {
	Effects = 0,
	Music = 1,
	Voice = 2,
	ThreeD = 3 // Positional
};

struct TigSoundAddresses : temple::AddressTable {
	TigSoundAddresses();
	int *movieVolume;

	int (__cdecl *AllocStream)(int *streamId, TigSoundType type);
	int (__cdecl *SetStreamVolume)(int streamId, int volume);
	void (__cdecl *PlayOnce)(int streamId, const char *filename, int fadeoutTime, BOOL allowReverb, int nextStreamId);
	bool (__cdecl *IsStreamPlaying)(int streamId);
	bool (__cdecl *IsStreamActive)(int streamId);
	void (__cdecl *FreeStream)(int streamId);

	bool PlayInStream(int streamId, int soundId);

};

extern TigSoundAddresses tigSoundAddresses;

class TigSoundStreamWrapper {
public:

	~TigSoundStreamWrapper() {
		if (mStreamId != -1) {
			tigSoundAddresses.FreeStream(mStreamId);
		}
	}

	
	bool Play(int soundId);
	bool Play(const std::string& filename, TigSoundType type);

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
