
#pragma once

enum LegacyLightType : uint32_t {
	LLT_POINT = 1,
	LLT_SPOT = 2,
	LLT_DIRECTIONAL = 3
};

struct LegacyLight {
	LegacyLightType type;
	float colorR;
	float colorG;
	float colorB;
	XMFLOAT3 pos;
	XMFLOAT3 dir;
	float range;
	float phi;
};
