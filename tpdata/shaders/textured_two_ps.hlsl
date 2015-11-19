
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
