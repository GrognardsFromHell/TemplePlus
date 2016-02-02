
float4x4 projMat : register(c0);

struct VS_IN {
	float4 pos : POSITION;
	float2 uv : TEXCOORD0;
};

struct VS_OUT {
	float4 pos : POSITION;
	float2 uv : TEXCOORD0;
};

VS_OUT main(VS_IN input)
{
	VS_OUT result;

	float4 fixedPos = float4(input.pos.xyz, 1);
	result.pos = mul(projMat, fixedPos);
	result.uv = input.uv;

	return result;
}
