
float4x4 projMat : register(c0);

struct VS_IN {
	float4 pos : POSITION;
	float pointsize : PSIZE0;
	float4 diffuse : COLOR0;
};

struct VS_OUT {
	float4 pos : SV_POSITION;
	float pointsize : PSIZE0;
	float4 diffuse : COLOR0;
};

VS_OUT main(VS_IN input)
{
	VS_OUT result = (VS_OUT)0;
	result.pos = mul(projMat, input.pos);
	result.diffuse = input.diffuse;
	result.pointsize = input.pointsize;
	return result;
}
