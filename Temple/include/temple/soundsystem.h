
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

	private:
		struct Impl;
		std::unique_ptr<Impl> mImpl;
	};

}
