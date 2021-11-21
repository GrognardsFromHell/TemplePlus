
#if LIGHTING
#include "lighting.fxh"
#endif

// Texture animation types
#define TEXUV_ENV 1
#define TEXUV_DRIFT 2
#define TEXUV_SWIRL 3
#define TEXUV_WAVEY 4

float4x4 projMat : register(c0);
float4 matDiffuse : register(c4);

struct VS_IN {
	float4 pos : POSITION;
	float4 normal : NORMAL0;
#if TEXTURE_STAGES > 0
	float2 uv : TEXCOORD0;
#endif
#if PER_VERTEX_COLOR
	float4 diffuse : COLOR0;
#endif
};

struct VS_OUT {
	float4 pos : SV_POSITION;
	float4 diffuse : COLOR0;
#if TEXTURE_STAGES >= 1
	float2 uv1 : TEXCOORD0;
#endif
#if TEXTURE_STAGES >= 2
	float2 uv2 : TEXCOORD1;
#endif
#if TEXTURE_STAGES >= 3
	float2 uv3 : TEXCOORD2;
#endif
};

float uvAnimTime : register(c5);
float4 uvSwirlFactors[4] : register(c6);
static const float PI_2 = 2 * 3.14159265f;
static const float SIN_45_DEG = 0.70710599f;

float2 ApplyUvAnim(int stage, float2 uv, float3 normal, int mode, float speedU, float speedV) {
	if (mode == TEXUV_ENV) {
		return float2(
			0.5f + (normal.x - normal.z) * SIN_45_DEG * 0.5f,
			0.5f - normal.y * 0.5f
		);
	} else if (mode == TEXUV_DRIFT) {
		return float2(
			uv.x + speedU * uvAnimTime,
			uv.y + speedV * uvAnimTime
		);
	} else if (mode == TEXUV_SWIRL) {
		return uv + uvSwirlFactors[stage].xy;
	} else if (mode == TEXUV_WAVEY) {
		return float2(
			uv.x + cos((speedU * uvAnimTime + uv.x) * PI_2) * 0.1f,
			uv.y + sin((speedV * uvAnimTime + uv.y) * PI_2) * 0.1f
			);
	} else {
		return uv;
	}
}

VS_OUT main(VS_IN input)
{
	VS_OUT result = (VS_OUT) 0;
	float4 fixedPos = float4(input.pos.xyz, 1);
	float3 normal = normalize(input.normal.xyz);

	float4 diffuse = matDiffuse;
#if PER_VERTEX_COLOR
	diffuse *= input.diffuse;
#endif

#if HIGHLIGHT
	float alpha = abs((normal.x + normal.z) * 0.504798f
		+ normal.y * 0.7f);
	diffuse.a = alpha * alpha;

	// Extend the object "outwards" a bit
	fixedPos.x += 2 * normal.x;
	fixedPos.y += 2 * normal.y;
	fixedPos.z += 2 * normal.z;
	normal = -normal; // Simulate light from "behind"
#endif

	result.pos = mul(projMat, fixedPos);

#if LIGHTING	
	result.diffuse = DoLights(fixedPos, normal, diffuse);
	result.diffuse.a = diffuse.a;
#else
	result.diffuse = diffuse;
#endif

#if TEXTURE_STAGES >= 1	
#ifdef TEXTURE_STAGE1_UVANIM
	result.uv1 = ApplyUvAnim(0, input.uv, normal, TEXTURE_STAGE1_UVANIM, TEXTURE_STAGE1_SPEEDU, TEXTURE_STAGE1_SPEEDV);
#else
	result.uv1 = input.uv;
#endif
#endif
#if TEXTURE_STAGES >= 2
#ifdef TEXTURE_STAGE2_UVANIM
	result.uv2 = ApplyUvAnim(1, input.uv, normal, TEXTURE_STAGE2_UVANIM, TEXTURE_STAGE2_SPEEDU, TEXTURE_STAGE2_SPEEDV);
#else
	result.uv2 = input.uv;
#endif
#endif
#if TEXTURE_STAGES >= 3
#ifdef TEXTURE_STAGE3_UVANIM
	result.uv3 = ApplyUvAnim(2, input.uv, normal, TEXTURE_STAGE3_UVANIM, TEXTURE_STAGE3_SPEEDU, TEXTURE_STAGE3_SPEEDV);
#else
	result.uv3 = input.uv;
#endif
#endif
	return result;
}
