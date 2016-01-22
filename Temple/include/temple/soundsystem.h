
#pragma once

#include <memory>

namespace temple {

	// Internal structure of MSS
	struct SoundDriver;
	using SoundDriverHandle = SoundDriver*;

	class SoundSystem {
	public:
		SoundSystem();
		~SoundSystem();

		SoundDriverHandle GetSoundDriverHandle();

		void ProcessEvents();

		void SetReverb(int roomType, int reverbDry, int reverbWet);

	private:
		struct Impl;
		std::unique_ptr<Impl> mImpl;
	};

}
