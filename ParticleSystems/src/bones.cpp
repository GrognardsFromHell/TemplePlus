
#include "particles/bones.h"
#include "particles/external.h"

namespace particles {

	using namespace DirectX;

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
				DirectX::XMFLOAT3 parentPos, bonePos;

				int parentId = external->GetParentChildBonePos(attachedTo, boneId, parentPos, bonePos);
				if (parentId >= 0) {
					mChildBoneCount++;
					auto length = XMVectorGetX(
						XMVector3Length(XMLoadFloat3(&bonePos) - XMLoadFloat3(&parentPos))
					);
					
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

	bool BonesState::GetRandomPos(float timeScale, XMFLOAT3 *result) const
	{
		if (mChildBoneCount == 0) {
			return false;
		}

		/*
			ToEE uses a specific particle system specific randomness sequence here,
			but since this is purely graphical, we will not.
		*/
		auto randFactor = rand() / (float) RAND_MAX;
		auto randDist = randFactor * mDistFromParentSum;

		/*
			Instead of just randomly selecting a bone, each bone is 
			effectively weighted by the distance between its parent
			and the bone. This makes sense, since a bigger space means
			it's probably a bigger bone and it shoud be used more often
			to spawn particles.
		*/
		int i;
		for (i = 0; i < mChildBoneCount; ++i) {
			if (randDist < mDistFromParent[i]) {
				break;
			}
			randDist -= mDistFromParent[i];
		}
		// If we couldn't find any (how does this happen?)
		// use the last  one
		if (i == mChildBoneCount) {
			i = mChildBoneCount - 1;
		}
		
		auto boneId = mIds[i];
		auto boneParentId = mParentIds[i];
		
		if (boneParentId >= 0) {
			auto distFactor = randDist / mDistFromParent[i];
			if (distFactor > 1 || distFactor < 0) {
				distFactor = 0.5f;
			}

			auto bonePos(XMLoadFloat3(&mPos[boneId]));
			auto parentPos(XMLoadFloat3(&mPos[boneParentId]));
			auto posNow = parentPos + (bonePos - parentPos) * distFactor;

			auto bonePrevPos(XMLoadFloat3(&mPrevPos[boneId]));
			auto parentPrevPos(XMLoadFloat3(&mPrevPos[boneParentId]));
			auto posPrev = parentPrevPos + (bonePrevPos - parentPrevPos) * distFactor;

			// Lerp between the last simulation point and now using the time factor
			auto finalPos(posPrev + (posNow - posPrev) * timeScale);
			XMStoreFloat3(result, finalPos);
			return true;
		}

		return false;
	}
}
