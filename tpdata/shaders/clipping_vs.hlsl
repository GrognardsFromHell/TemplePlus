
float objRotCos;
float objRotSin;
float3 objPos;
float3 objScale;
float4x4 projMat;

float4 main( float4 pos : POSITION ) : SV_POSITION
{
	/*
		This performs two rotations. First it rotates
		using the obj specific rotation, then it applies
		the game's perspective.
	*/
	float rot1 = (-(objRotCos * pos.y) - -(objRotSin * pos.x));
	float rot2 = - (objRotSin * pos.y + objRotCos * pos.x);
	float rot3 = (rot2 + rot1) * objScale.y * 0.70710599 * 0.70710599;
	float rot4 = (rot1 - rot2) * objScale.x * 0.70710599 * 0.70710599;
	pos.x = objPos.x + rot3 - rot4;
	pos.y = objPos.y + pos.z * objScale.z;
	pos.z = objPos.z + rot4 + rot3;
	return mul(projMat, pos);
}
