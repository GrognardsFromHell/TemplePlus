
#pragma once

#include "addresses.h"

struct SubtitleLine {
	uint32_t startMs;
	uint32_t durationMs;
	D3DCOLOR color;
	char *fontname;
	char *text;
	SubtitleLine *nextLine;
};

struct MovieFuncs : AddressTable {
	void(__cdecl *PlayLegalMovies)() = nullptr;
	void(__cdecl *PlayMovie)(char* filename, int, int, int);
	void(__cdecl *PlayMovieBink)(const char *filename, const SubtitleLine *subtitles, int flags, int soundtrackId);

	GlobalPrimitive<uint32_t, 0x103010F8> MovieVolume; // 0-127

	// Seems to suppress input event processing if true
	GlobalBool<0x10EF32A0> MovieIsPlaying;

	void rebase(Rebaser rebase) override {
		rebase(PlayMovie, 0x10034100);
		rebase(PlayLegalMovies, 0x10003AC0);
		rebase(PlayMovieBink, 0x101F1BE0);
	}

};

extern MovieFuncs movieFuncs;

void hook_movies();
