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
