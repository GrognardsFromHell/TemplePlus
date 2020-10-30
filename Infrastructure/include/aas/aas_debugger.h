#pragma once
#include <DirectXMath.h>


class AasDebugger {
public:
	static bool IsForcedFrame();
	static bool IsForcedWorldMat();

	static void SetForcedFrame(bool en, float frame);
	static float GetForcedFrame();
	static void GetForcedWorldMatrix(DirectX::XMFLOAT4X4& worldMat);
	static void SetForcedWorldMatrix(bool en, const DirectX::XMFLOAT4X4& worldMat);
};
