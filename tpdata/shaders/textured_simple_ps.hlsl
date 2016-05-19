
#ifdef D3D11

sampler texSampler;
Texture2D mainTexture : register(t0);

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float4 diffuse : COLOR0;
	float2 uv : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	return mainTexture.Sample(texSampler, input.uv) * input.diffuse;
}

#else

sampler texSampler : register(s0);

struct PS_INPUT
{
	float4 diffuse : COLOR0;
	float2 uv : TEXCOORD0;
};

float4 main(PS_INPUT input) : COLOR0
{
	return tex2D(texSampler, input.uv) * input.diffuse;
}

#endif