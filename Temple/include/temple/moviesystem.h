
#pragma once

#include <string>
#include <memory>

#include <infrastructure/macros.h>

namespace temple {
	
	struct BinkMovie;

	class MovieSystem;
	class SoundSystem;

	class MovieFile {
	public:
		MovieFile(MovieSystem &system, BinkMovie* movie);
		~MovieFile();

		/**
		 * Sets the volume of this movie. Range is 0 - 1.
		 */
		void SetVolume(float volume);

		/**
		 * Gets the width of the movie file in pixels.
		 */
		int GetWidth() const;

		/**
		 * Gets the height of the movie file in pixels.
		 */
		int GetHeight() const;

		/**
		 * Wait until it is time to render the next frame of this movie.
		 */
		bool WaitForNextFrame();

		/**
		 * Go to the next frame in this movie.
		 */
		void NextFrame();

		/**
		 * Decode the frame.
		 */
		void DecodeFrame();
		
		/**
		 * Returns true if the last frame of this movie has been reached.
		 */
		bool AtEnd() const;

		/**
		 * Copies the pixel data of the current frame to a destination buffer, after
		 * the frame has been decoded. Pixels are copied as 32-bit ARGB.
		 */
		void CopyFramePixels(void* dest, int destpitch, int destheight);

		NO_COPY_OR_MOVE(MovieFile);
	private:
		MovieSystem &mSystem;
		BinkMovie *mMovie;
	};

	class MovieSystem {
		friend class MovieFile;
	public:
		MovieSystem(SoundSystem &soundSystem);
		~MovieSystem();

		std::unique_ptr<MovieFile> OpenMovie(const std::string &filename,
			uint32_t soundtrackId = 0);

	private:
		SoundSystem &mSoundSystem;

		struct Impl;
		std::unique_ptr<Impl> mImpl;		
	};

}
