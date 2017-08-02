#pragma once
#include "obj.h"

enum FadeAndTeleportFlags : uint32_t {
	ftf_play_movie = 1,
	ftf_advance_time = 8,
	ftf_play_sound = 0x10,
	ftf_unk20 = 0x20,
	ftf_unk80000000 = 0x80000000
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
	XMCOLOR color = XMCOLOR(0, 0, 0, 1);
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
	int flags;
	XMCOLOR color;
	int countSthgUsually48;
	float transitionTime;
	int field10;
	int field14;
	int hoursToPass;
};
#pragma pack(pop)

class Fade {
public:
	bool FadeAndTeleport(const FadeAndTeleportArgs &args);

	void PerformFade(const FadeArgs &args);
};

extern Fade fade;
