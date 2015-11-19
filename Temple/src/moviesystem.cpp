
#include <platform/windows.h>
#include <infrastructure/exception.h>
#include <infrastructure/logging.h>

#include "temple/moviesystem.h"
#include "temple/soundsystem.h"

namespace temple {

	/*
		This is an incomplete description of the bink structure that is returned by
		BinkOpen. It represents what is currently loaded for a given movie.
	*/
	struct BinkMovie {
		uint32_t width;
		uint32_t height;
		uint32_t frameCount;
		uint32_t currentFrame;
	};

	struct MovieSystem::Impl {
		void* BinkOpenMiles; // Only passed to BinkSetSoundSystem
		using FNBinkSetSoundSystem = int(__stdcall *)(void*, void*);
		using FNBinkSetSoundTrack = void(__stdcall *)(uint32_t trackCount, uint32_t* trackIds);
		using FNBinkOpen = BinkMovie*(__stdcall *)(const char* name, uint32_t flags);
		using FNBinkSetVolume = void(__stdcall *)(BinkMovie* movie, uint32_t trackId, int volume);
		using FNBinkNextFrame = void(__stdcall *)(BinkMovie* movie);
		using FNBinkWait = int(__stdcall *)(BinkMovie* movie);
		using FNBinkDoFrame = int(__stdcall *)(BinkMovie* movie);
		using FNBinkCopyToBuffer = int(__stdcall *)(BinkMovie* movie,
			void* dest, int destpitch, int destheight, int destx, int desty, int flags);
		using FNBinkClose = void(__stdcall *)(BinkMovie* movie);

		FNBinkSetSoundSystem BinkSetSoundSystem;
		FNBinkSetSoundTrack BinkSetSoundTrack;
		FNBinkOpen BinkOpen;
		FNBinkSetVolume BinkSetVolume;
		FNBinkNextFrame BinkNextFrame;
		FNBinkWait BinkWait;
		FNBinkDoFrame BinkDoFrame;
		FNBinkCopyToBuffer BinkCopyToBuffer;
		FNBinkClose BinkClose;

		Impl() {
			Resolve("_BinkOpenMiles@4", BinkOpenMiles);
			Resolve("_BinkSetSoundSystem@8", BinkSetSoundSystem);
			Resolve("_BinkSetSoundTrack@8", BinkSetSoundTrack);
			Resolve("_BinkOpen@8", BinkOpen);
			Resolve("_BinkSetVolume@12", BinkSetVolume);
			Resolve("_BinkNextFrame@4", BinkNextFrame);
			Resolve("_BinkWait@4", BinkWait);
			Resolve("_BinkDoFrame@4", BinkDoFrame);
			Resolve("_BinkCopyToBuffer@28", BinkCopyToBuffer);
			Resolve("_BinkClose@4", BinkClose);
		}

	private:
		template <typename T>
		void Resolve(const char* entryPoint, T* & funcPtr) {
			auto binkHandle = GetModuleHandle(L"binkw32");
			if (!binkHandle) {
				throw TempleException("binkw32.dll is not loaded");
			}

			funcPtr = (T*)GetProcAddress(binkHandle, entryPoint);

			if (!funcPtr) {
				throw TempleException("Entry point {} was not found in binkw32.dll",
					entryPoint);
			}
		}
	};

	MovieFile::MovieFile(MovieSystem& system, BinkMovie* movie)
	 : mSystem(system), mMovie(movie) {
	}

	MovieFile::~MovieFile() {
		mSystem.mImpl->BinkClose(mMovie);
	}

	void MovieFile::SetVolume(float volume) {

		// Strange conversion factor used by BINK here
		auto binkVolume = (int)(volume * 127 * 258);

		mSystem.mImpl->BinkSetVolume(mMovie, 0, binkVolume);

	}

	int MovieFile::GetWidth() const {
		return mMovie->width;
	}

	int MovieFile::GetHeight() const {
		return mMovie->height;
	}

	bool MovieFile::WaitForNextFrame() {
		return mSystem.mImpl->BinkWait(mMovie) != 0;
	}

	void MovieFile::NextFrame() {
		mSystem.mImpl->BinkNextFrame(mMovie);
	}

	void MovieFile::DecodeFrame() {
		mSystem.mImpl->BinkDoFrame(mMovie);
	}

	bool MovieFile::AtEnd() const {
		return mMovie->currentFrame >= mMovie->frameCount;
	}

	void MovieFile::CopyFramePixels(void* dest, int destpitch, int destheight) {

		// 0x80000000 stands for copy all, not just changed
		// 0x70000000 stands for no scaling
		// 0x00000003 stands for 32-bit pixels
		constexpr auto flags = 0xF0000003;
		mSystem.mImpl->BinkCopyToBuffer(mMovie, dest, destpitch, destheight, 0, 0, flags);

	}

	MovieSystem::MovieSystem(SoundSystem &soundSystem) 
		: mSoundSystem(soundSystem), mImpl(std::make_unique<Impl>()) {

		// Set the sound system to be used by Bink
		auto driverHandle = soundSystem.GetSoundDriverHandle();
		mImpl->BinkSetSoundSystem(mImpl->BinkOpenMiles, driverHandle);
	}

	MovieSystem::~MovieSystem() {
	}

	std::unique_ptr<MovieFile> MovieSystem::OpenMovie(const std::string& filename,
		uint32_t soundtrackId) {

		uint32_t openFlags = 0;
		if (soundtrackId) {
			mImpl->BinkSetSoundTrack(1, &soundtrackId);
			openFlags |= 0x4000; // Apparently tells BinkOpen a soundtrack was used
		}

		auto movie = mImpl->BinkOpen(filename.c_str(), openFlags);
		if (!movie) {
			logger->error("Unable to open movie file {} with flags {}",
				filename, openFlags);
			return nullptr;
		}

		return std::make_unique<MovieFile>(*this, movie);

	}

}
