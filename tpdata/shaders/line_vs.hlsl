
float4x4 projMat : register(c0);
float4 matDiffuse : register(c4);

struct VS_IN {
	float3 pos : POSITION;
};

struct VS_OUT {
	float4 pos : SV_POSITION;
	float4 diffuse : COLOR0;
};

VS_OUT main(VS_IN input)
{
	VS_OUT result = (VS_OUT)0;
	float4 fixedPos = float4(input.pos, 1);
	result.pos = mul(projMat, fixedPos);
	result.diffuse = matDiffuse;
	return result;
}
