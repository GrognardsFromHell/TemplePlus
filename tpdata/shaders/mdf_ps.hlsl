
// Different types of texture blending (per stage)

/*
	Equivalent to fixed function settings:
	colorOp = D3DTOP_MODULATE arg1 = D3DTA_TEXTURE arg2 = D3DTA_CURRENT
	alphaOp = D3DTOP_MODULATE arg1 = D3DTA_TEXTURE arg2 = D3DTA_CURRENT
*/
#define TEXBLEND_MODULATE 1
/*
	Equivalent to fixed function settings:
	colorOp = D3DTOP_ADD         arg1 = D3DTA_TEXTURE arg2 = D3DTA_CURRENT
	alphaOp = D3DTOP_SELECTARG2  arg1 = D3DTA_TEXTURE arg2 = D3DTA_CURRENT
*/
#define TEXBLEND_ADD 2
/*
	Equivalent to fixed function settings:
	colorOp = D3DTOP_BLENDTEXTUREALPHA  arg1 = D3DTA_TEXTURE arg2 = D3DTA_CURRENT
	alphaOp = D3DTOP_SELECTARG2         arg1 = D3DTA_TEXTURE arg2 = D3DTA_CURRENT
*/
#define TEXBLEND_TEXTURE_ALPHA 3
/*
	Equivalent to fixed function settings:
	colorOp = D3DTOP_BLENDCURRENTALPHA  arg1 = D3DTA_TEXTURE arg2 = D3DTA_CURRENT
	alphaOp = D3DTOP_SELECTARG2         arg1 = D3DTA_TEXTURE arg2 = D3DTA_DIFFUSE
*/
#define TEXBLEND_CURRENT_ALPHA 4
/*
	Equivalent to fixed function settings:
	colorOp = D3DTOP_MODULATEALPHA_ADDCOLOR  arg1 = D3DTA_CURRENT arg2 = D3DTA_TEXTURE
	alphaOp = D3DTOP_SELECTARG2              arg1 = D3DTA_CURRENT arg2 = D3DTA_DIFFUSE
*/
#define TEXBLEND_CURRENT_ALPHA_ADD 5

#if TEXTURE_STAGES >= 1
sampler texSampler1 : register(s0);
#endif
#if TEXTURE_STAGES >= 2
sampler texSampler2 : register(s1);
#endif
#if TEXTURE_STAGES >= 3
sampler texSampler3 : register(s2);
#endif

struct PS_INPUT
{
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

float4 DoTexStage(float4 current, float4 diffuse, sampler sampl, float2 uv, int mode) {
	if (mode == TEXBLEND_MODULATE) {
		return current * tex2D(sampl, uv);
	} else if (mode == TEXBLEND_ADD) {
		return float4(current.rgb + tex2D(sampl, uv).rgb, current.a);
	} else if (mode == TEXBLEND_TEXTURE_ALPHA) {
		float4 texel = tex2D(sampl, uv);
		current.rgb = current.rgb * (1 - texel.a) + texel.rgb * texel.a;
		return current;
	} else if (mode == TEXBLEND_CURRENT_ALPHA) {
		float4 texel = tex2D(sampl, uv);
		current.rgb = current.rgb * (1 - current.a) + texel.rgb * current.a;
		current.a = diffuse.a;
		return current;
	}
}

float4 main(PS_INPUT input) : COLOR0
{
	float4 result = input.diffuse;

#if TEXTURE_STAGES >= 1
	result = DoTexStage(result, input.diffuse, texSampler1, input.uv1, TEXTURE_STAGE1_MODE);
#endif
#if TEXTURE_STAGES >= 2
	result = DoTexStage(result, input.diffuse, texSampler2, input.uv2, TEXTURE_STAGE2_MODE);
#endif
#if TEXTURE_STAGES >= 3
	result = DoTexStage(result, input.diffuse, texSampler3, input.uv3, TEXTURE_STAGE3_MODE);
#endif

	if (result.a == 0) {
		discard;
	}

	return result;
}
