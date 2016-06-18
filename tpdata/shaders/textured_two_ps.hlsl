
#ifdef D3D11

sampler sampler1;
sampler sampler2;
Texture2D texture1 : register(t0);
Texture2D texture2 : register(t1);

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float4 diffuse : COLOR0;
	float2 uv : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	return texture1.Sample(sampler1, input.uv) * 
		texture2.Sample(sampler2, input.uv) *
		input.diffuse;
}

#else

sampler tex0 : register(s0);
sampler tex1 : register(s1);

struct PS_INPUT
{
	float4 diffuse : COLOR0;
	float2 uv : TEXCOORD0;
};

float4 main(PS_INPUT input) : COLOR0
{
	return tex2D(tex0, input.uv) * 
		   tex2D(tex1, input.uv) * 
		   input.diffuse;
}

#endif