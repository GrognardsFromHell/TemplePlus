
float4x4 projMat : register(c0);
float3 lightDir : register(c4);
float offsetZ : register(c5);

struct VS_IN {
	float4 pos : POSITION;
	float4 normal : NORMAL;
};

struct VS_OUT {
	float4 pos : POSITION;
	float4 diffuse : COLOR0;
};

VS_OUT main(VS_IN input)
{
	VS_OUT result = (VS_OUT)0;
	float4 fixedPos = float4(input.pos.xyz, 1);
	float3 normal = normalize(input.normal.xyz);

	float alpha = 1;
	float y = fixedPos.y;
	if (y <= 160.0f) {
		if (y < 0) {
			alpha = 1;
		} else {
			float shadowIntensity = abs(dot(lightDir, normal));
			alpha = (1 - (y - offsetZ) / 160.0f) * shadowIntensity;
		}
	} else {
		alpha = 0;
	}
	result.diffuse = float4(0, 0, 0, alpha * 0.5f);

	fixedPos.x = fixedPos.x - (y - offsetZ) * lightDir.x / lightDir.y;
	fixedPos.y = offsetZ;
	fixedPos.z = fixedPos.z - (y - offsetZ) * lightDir.z / lightDir.y;

	result.pos = mul(projMat, fixedPos);

	return result;
}
