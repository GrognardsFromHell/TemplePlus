
#pragma once

#include "util/addresses.h"

struct SubtitleLine {
	uint32_t startMs;
	uint32_t durationMs;
	D3DCOLOR color;
	char *fontname;
	char *text;
	SubtitleLine *nextLine;
};

struct MovieFuncs : AddressTable {	

	/*
		Plays a movie from movies.mes, which could either be a slide or binkw movie.
		The soundtrack id is used for BinkW movies with multiple soundtracks.
		As far as we know, this is not used at all in ToEE.
	*/
	void (__cdecl *PlayMovieId)(int movieId, int flags, int soundtrackId);

	void (__cdecl *PlayLegalMovies)() = nullptr; // No longer used (see gamesystems.cpp)
	void (__cdecl *PlayMovie)(char* filename, int, int, int);
	void (__cdecl *PlayMovieSlide)(uint32_t, uint32_t, const SubtitleLine *subtitles, uint32_t, uint32_t);
	void (__cdecl *PlayMovieBink)(const char *filename, const SubtitleLine *subtitles, int flags, int soundtrackId);

	// Should be replaced / removed since it seems unnecessarily complex
	// Previously it would try to only change the vido mode once for several movies
	// but this is no longer needed
	void (__cdecl *MovieQueueAdd)(int movieId);
	void (__cdecl *MovieQueuePlay)();

	// Seems to suppress input event processing if true
	GlobalBool<0x10EF32A0> MovieIsPlaying;

	MovieFuncs() {
		rebase(PlayMovie, 0x10034100);
		rebase(PlayLegalMovies, 0x10003AC0);
		rebase(PlayMovieBink, 0x101F1BE0);
		rebase(PlayMovieSlide, 0x101F10F0);
		rebase(PlayMovieId, 0x100341F0);

		rebase(MovieQueueAdd, 0x10033DE0);
		rebase(MovieQueuePlay, 0x100345A0);
	}

};

extern MovieFuncs movieFuncs;

void hook_movies();
