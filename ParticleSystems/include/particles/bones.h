
#pragma once

#include <vector>
#include <DirectXMath.h>

#include "types.h"

namespace particles {
	
	class BonesState {
	public:
		
		explicit BonesState(ObjHndl handle);

		void UpdatePos();

	private:

		ObjHndl mObject;
		int mChildBoneCount = 0;
		int mBoneCount = 0;
		float mDistFromParentSum = 0;
		std::vector<int> mIds;
		std::vector<int> mParentIds;
		std::vector<float> mDistFromParent;
		std::vector<DirectX::XMFLOAT3> mPos;
		std::vector<DirectX::XMFLOAT3> mPrevPos;
	};
	
}
