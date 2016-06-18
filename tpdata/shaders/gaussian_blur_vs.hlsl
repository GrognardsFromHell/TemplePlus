
struct VS_IN {
	float4 pos : POSITION;
	float2 uv : TEXCOORD0;
};

struct VS_OUT {
	float4 pos : SV_POSITION;
	float2 uv1 : TEXCOORD0;
};

VS_OUT main(VS_IN input)
{
	VS_OUT result = (VS_OUT)0;
	float4 fixedPos = float4(input.pos.xyz, 1);
	result.pos = fixedPos;
	result.uv1 = input.uv;
	return result;
}
