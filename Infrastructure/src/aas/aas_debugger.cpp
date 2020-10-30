#include "aas/aas_debugger.h"
#include "aas/aas_math.h"

bool isForcedFrame = false;
float forcedFrame = 0.0f;
bool isForcedWorldMat = false;
DirectX::XMFLOAT4X4 forcedWorldMat;

bool AasDebugger::IsForcedFrame()
{
	return isForcedFrame;
}

void AasDebugger::SetForcedFrame(bool en, float frame)
{
	isForcedFrame = en;
	if (en) {
		forcedFrame = frame;
	}
}

float AasDebugger::GetForcedFrame()
{
	return forcedFrame;
}

bool AasDebugger::IsForcedWorldMat()
{
	return isForcedWorldMat;
}



void AasDebugger::GetForcedWorldMatrix(DirectX::XMFLOAT4X4& worldMat)
{	
	worldMat = forcedWorldMat;	
}

void AasDebugger::SetForcedWorldMatrix(bool en, const DirectX::XMFLOAT4X4& worldMat)
{
	isForcedFrame = en;
	if (en) {
		forcedWorldMat = worldMat;
	}
}
