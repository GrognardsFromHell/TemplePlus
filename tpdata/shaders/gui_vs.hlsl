
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

VS_OUT main( VS_IN input )
{
	VS_OUT output;
	float4 pos = float4(input.pos, 1);
#ifndef DRAW_LINES
	pos.x -= 0.5;
	pos.y -= 0.5;
#endif
	output.pos = mul(projMat, pos);
	output.diffuse = input.diffuse;
	output.uv = input.uv;
	return output;
}
