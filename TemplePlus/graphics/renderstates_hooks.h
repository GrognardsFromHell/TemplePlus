
#pragma once

struct LegacyLight {
	D3DLIGHTTYPE type;
	float colorR;
	float colorG;
	float colorB;
	D3DVECTOR pos;
	D3DVECTOR dir;
	float range;
	float phi;
};
