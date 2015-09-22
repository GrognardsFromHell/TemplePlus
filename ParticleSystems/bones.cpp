
#include "particles/bones.h"
#include "particles/external.h"

namespace particles {

	BonesState::BonesState(ObjHndl attachedTo) : mObject(attachedTo) {

		auto external = IPartSysExternal::GetCurrent();
		auto boneCount = external->GetBoneCount(attachedTo);

		if (boneCount > 0) {
			mIds.reserve(boneCount);
			mParentIds.reserve(boneCount);
			mDistFromParent.reserve(boneCount);
			mPos.resize(boneCount);
			mPrevPos.resize(boneCount);
			
			for (auto boneId = 0; boneId < boneCount; ++boneId) {
				DirectX::XMVECTOR parentPos, bonePos;

				auto parentId = external->GetParentChildBonePos(attachedTo, boneId, (Vec3&)parentPos, (Vec3&)bonePos);
				if (parentId >= 0) {
					mChildBoneCount++;
					auto length = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(bonePos, parentPos)));
					
					mIds.push_back(boneId);
					mParentIds.push_back(parentId);
					mDistFromParent.push_back(length);
					mDistFromParentSum += length;
				}
			}
			
			mBoneCount = boneCount;
		}

		UpdatePos();
	}

	void BonesState::UpdatePos() {

		auto external = IPartSysExternal::GetCurrent();

		for (auto i = 0; i < mBoneCount; ++i) {

			// Update the position of each bone, but store the previous position
			auto &pos = mPos[i];
			mPrevPos[i] = pos;
			
			external->GetBonePos(mObject, i, pos);
		}

	}
}
