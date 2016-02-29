
#pragma once

struct LegacyLight {
	D3DLIGHTTYPE type;
	float colorR;
	float colorG;
	float colorB;
	XMFLOAT3 pos;
	XMFLOAT3 dir;
	float range;
	float phi;
};
