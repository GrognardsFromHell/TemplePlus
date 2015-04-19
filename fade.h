#pragma once
#include "obj.h"

enum FadeAndTeleportFlags : uint32_t {
	ftf_play_movie = 1,
	ftf_advance_time = 8,
	ftf_play_sound = 0x10
};

#pragma pack(push, 1)
struct FadeAndTeleportArgs {
	int flags; // FadeAndTeleportFlags
	int field4;
	objHndl somehandle;
	locXY destLoc;
	int destMap;
	int movieId;
	int field20;
	int field24;
	int field28;
	int field2c;
	D3DCOLOR color = D3DCOLOR_ARGB(0xff, 0, 0, 0);
	int field34;
	float somefloat;
	int field3c;
	int field40;
	int field44;
	int field48;
	int field4c;
	int field50;
	float somefloat2;
	int field58;
	int field5c;
	int field60;
	int soundId;
	int timeToAdvance;
	int field6c;
	int field70;
	int field74;
};

struct FadeArgs {
	int field0;
	D3DCOLOR color;
	int field8;
	float transitionTime;
	int field10;
	int field14;
	int field18;
};
#pragma pack(pop)

class Fade {
public:
	bool FadeAndTeleport(const FadeAndTeleportArgs &args);

	void PerformFade(const FadeArgs &args);
};

extern Fade fade;
