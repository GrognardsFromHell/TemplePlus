
float4 shadowMapWorldPos : register(c0);
float3 lightDir : register(c1);
float offsetZ : register(c2);
float4 matDiffuse : register(c4);

struct VS_IN {
	float4 pos : POSITION;
};

struct VS_OUT {
	float4 pos : POSITION;
	float4 diffuse : COLOR0;
};

VS_OUT main(VS_IN input)
{
	VS_OUT result = (VS_OUT)0;
	float4 fixedPos = float4(input.pos.xyz, 1);

	result.pos.x = input.pos.x - (input.pos.y - offsetZ) * lightDir.x / lightDir.y;
	result.pos.y = input.pos.z - (input.pos.y - offsetZ) * lightDir.z / lightDir.y;
	result.pos.x = (result.pos.x - shadowMapWorldPos.x) / shadowMapWorldPos.z;
	result.pos.y = (result.pos.y - shadowMapWorldPos.y) / shadowMapWorldPos.w;
	result.pos.z = 0;
	result.pos.w = 1;

	result.pos.x = result.pos.x * 2 - 1;
	result.pos.y = 1 - result.pos.y * 2;

	result.diffuse = matDiffuse;

	return result;
}
