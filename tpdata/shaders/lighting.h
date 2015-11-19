
// float3 V = -mul(projMat, float4(0, 0, 0, 0)).xyz;
// Since we use orthogonal projection, the directional vector
// from the proverbial "eye" to the pixel in question is
// perpendicular to the screen for every pixel.
static const float3 V = float3(0, 0, 1);

struct COLOR_PAIR
{
	float4 Color : COLOR0;
	float4 ColorSpec : COLOR1;
};

#define MAX_LIGHTS 8
float3 lightPos[MAX_LIGHTS] : register(c100);
float3 lightDir[MAX_LIGHTS] : register(c108);
float4 lightAmbient[MAX_LIGHTS] : register(c116);
float4 lightDiffuse[MAX_LIGHTS] : register(c124);
float4 lightSpecular[MAX_LIGHTS] : register(c132);
float lightRange[MAX_LIGHTS] : register(c140);
float3 lightAttenuation[MAX_LIGHTS] : register(c148); //1, D, D^2;
float3 lightSpot[MAX_LIGHTS] : register(c156); //cos(theta/2), cos(phi/2), falloff

bool bSpecular : register(c164);
float fMaterialPower : register(c165);
float4 matSpecular : register(c166);
int4 lightCount: register(c167);

//---------------------------------------------------------------------
// Name: DoDirLight()
// Desc: Directional light computation
//---------------------------------------------------------------------
COLOR_PAIR DoDirLight(float3 N, float3 V, int i)
{
	COLOR_PAIR Out;
	// float3 L = mul((float3x3)matViewIT, -normalize(lightDir[i]));
	float3 L = -normalize(lightDir[i]);
	float NdotL = dot(N, L);
	Out.Color = lightAmbient[i];
	Out.ColorSpec = 0;
	if (NdotL > 0.f)
	{
		//compute diffuse color
		Out.Color += NdotL * lightDiffuse[i];
		
			//add specular component
			if (bSpecular)
			{
				float3 H = normalize(L + V); //half vector
				Out.ColorSpec = pow(max(0, dot(H, N)), fMaterialPower) *
					lightSpecular[i];
			}
	}
	return Out;
}
//---------------------------------------------------------------------
// Name: DoPointLight()
// Desc: Point light computation
//---------------------------------------------------------------------
COLOR_PAIR DoPointLight(float4 vPosition, float3 N, float3 V, int i)
{
	/*float3 L = mul((float3x3)matViewIT, normalize((lightPos[i] -
		(float3)mul(matWorld, vPosition))));*/
	float3 L = normalize(lightPos[i] - (float3)vPosition);
	COLOR_PAIR Out;
	float NdotL = dot(N, L);
	Out.Color = lightAmbient[i];
	Out.ColorSpec = 0;
	float fAtten = 1.f;
	if (NdotL >= 0.f)
	{
		//compute diffuse color
		Out.Color += NdotL * lightDiffuse[i];
		//add specular component
		if (bSpecular)
		{
			float3 H = normalize(L + V); //half vector
			Out.ColorSpec = pow(max(0, dot(H, N)), fMaterialPower) *
				lightSpecular[i];
		}
		/*float LD = length(lightPos[i] -
			(float3)mul(matWorld, vPosition));*/
		float LD = length(lightPos[i] - (float3)vPosition);
		if (LD > lightRange[i])
		{
			fAtten = 0.f;
		}
		else
		{
			fAtten *= 1.f / (lightAttenuation[i].x +
				lightAttenuation[i].y*LD +
				lightAttenuation[i].z*LD*LD);
		}
		Out.Color *= fAtten;
		Out.ColorSpec *= fAtten;
	}
	return Out;
}
//---------------------------------------------------------------------
// Name: DoSpotLight() 
// Desc: Spot light computation
//---------------------------------------------------------------------
COLOR_PAIR DoSpotLight(float4 vPosition, float3 N, float3 V, int i)
{
	/*float3 L = mul((float3x3)matViewIT, normalize((lightPos[i] -
		(float3)mul(matWorld, vPosition))));*/
	float3 L = normalize(lightPos[i] - (float3)vPosition);
	COLOR_PAIR Out;
	float NdotL = dot(N, L);
	Out.Color = lightAmbient[i];
	Out.ColorSpec = 0;
	float fAttenSpot = 1.f;
	if (NdotL >= 0.f)
	{
		//compute diffuse color
		Out.Color += NdotL * lightDiffuse[i];
		//add specular component
		if (bSpecular)
		{
			float3 H = normalize(L + V); //half vector
			Out.ColorSpec = pow(max(0, dot(H, N)), fMaterialPower) *
				lightSpecular[i];
		}
		/*float LD = length(lightPos[i] -
			(float3)mul(matWorld, vPosition));*/
		float LD = length(lightPos[i] - (float3)vPosition);
		if (LD > lightRange[i])
		{
			fAttenSpot = 0.f;
		}
		else
		{
			fAttenSpot *= 1.f / (lightAttenuation[i].x +
				lightAttenuation[i].y*LD +
				lightAttenuation[i].z*LD*LD);
		}
		//spot cone computation
		// float3 L2 = mul((float3x3)matViewIT, -normalize(lightDir[i]));
		float3 L2 = -normalize(lightDir[i]);
		float rho = dot(L, L2);
		fAttenSpot *= pow(saturate((rho - lightSpot[i].y) /
			(lightSpot[i].x - lightSpot[i].y)),
			lightSpot[i].z);
		Out.Color *= fAttenSpot;
		Out.ColorSpec *= fAttenSpot;
	}
	return Out;
}

float4 DoLights(float4 pos, float3 normal, float4 matDiffuse) {
	COLOR_PAIR lightResult;
	lightResult.ColorSpec = (float4)0;
	// Directional lights
	[loop] for (int i = 0; i < lightCount.x; i++) {
		COLOR_PAIR tmp = DoDirLight(normal, V, i);
		lightResult.Color += tmp.Color;
		lightResult.ColorSpec += tmp.ColorSpec;
	}
	// Point lights
	[loop] for (int j = lightCount.x; j < lightCount.y; j++) {
		COLOR_PAIR tmp = DoPointLight(pos, normal, V, j);
		lightResult.Color += tmp.Color;
		lightResult.ColorSpec += tmp.ColorSpec;
	}
	// Spot lights
	[loop] for (int k = lightCount.y; k < lightCount.z; k++) {
		COLOR_PAIR tmp = DoSpotLight(pos, normal, V, k);
		lightResult.Color += tmp.Color;
		lightResult.ColorSpec += tmp.ColorSpec;
	}
	return saturate(lightResult.Color) * matDiffuse 
		+ saturate(lightResult.ColorSpec) * matSpecular;
}
