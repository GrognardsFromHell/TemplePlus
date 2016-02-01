
float4x4 projMat : register(c0);

struct VS_IN {
	float4 pos : POSITION;
	float4 diffuse : COLOR0;
	float2 uv : TEXCOORD0;
};

struct VS_OUT {
	float4 pos : POSITION;
	float4 diffuse : COLOR0;
	float2 uv : TEXCOORD0;
};

VS_OUT main(VS_IN input)
{
	VS_OUT result;

	float4 fixedPos = float4(input.pos.xyz, 1);
	result.pos = mul(projMat, fixedPos);
	/*result.pos.x += 1 / 1024.0f;
	result.pos.y += 1 / 768.0f;*/
	result.diffuse = input.diffuse;
	result.uv = input.uv;

	return result;
}
