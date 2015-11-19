
struct VS_IN {
	float3 pos : POSITION;
	float4 diffuse : COLOR0;
	float2 uv : TEXCOORD0;
};

struct VS_OUT {
	float4 pos : POSITION;
	float4 diffuse : COLOR0;
	float2 uv : TEXCOORD0;
};

float4x4 projMat : register(c0);

VS_OUT main(VS_IN input)
{
	VS_OUT output;
	output.pos = mul(projMat, float4(input.pos, 1));
	output.diffuse = input.diffuse;
	output.uv = input.uv;
	return output;
}
