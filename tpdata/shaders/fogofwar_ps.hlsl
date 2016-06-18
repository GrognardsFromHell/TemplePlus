
sampler texSampler;
Texture2D<float> opacityTex : register(t0);

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float opacity = opacityTex.Sample(texSampler, input.uv);
	return float4(0, 0, 0, opacity);
}
