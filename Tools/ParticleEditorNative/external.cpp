#include "external.h"

#include <DirectXMath.h>
#include "api.h"
#include <infrastructure/stringutil.h>

float EditorExternal::mX = 0;
float EditorExternal::mY = 0;
float EditorExternal::mZ = 0;

float EditorExternal::GetParticleFidelity() {
	return 1.0f;
}

bool EditorExternal::GetObjLocation(ObjHndl obj, XMFLOAT3& worldPos) {
	worldPos.x = mDll.animParams.offsetX;
	worldPos.y = mDll.animParams.offsetY;
	worldPos.z = mDll.animParams.offsetZ;
	return true;
}

bool EditorExternal::GetObjRotation(ObjHndl obj, float& rotation) {
	rotation = mDll.animParams.rotation;
	return true;
}

float EditorExternal::GetObjRadius(ObjHndl obj) {
	return 60.0f;
}

bool EditorExternal::GetBoneWorldMatrix(ObjHndl obj, const ::std::string& boneName, ::Matrix4x4& boneMatrix) {
	return mDll.currentModel->GetBoneWorldMatrixByName(mDll.animParams, boneName, &boneMatrix);
}

int EditorExternal::GetBoneCount(ObjHndl obj) {
	return mDll.currentModel->GetBoneCount();
}

static bool StrContains(const std::string &str, const char *otherStr) {
	return str.find(otherStr) != std::string::npos;
}

static bool IsIgnoredBone(const std::string &name) {

	if (name[0] == '#') {
		return true; // Cloth bone
	}
	if (tolower(name) == "bip01") {
		return true;
	}

	return StrContains(name, "Pony")
		|| StrContains(name, "Footstep")
		|| StrContains(name, "Origin")
		|| StrContains(name, "Casting_ref")
		|| StrContains(name, "EarthElemental_reg")
		|| StrContains(name, "Casting_ref")
		|| StrContains(name, "origin")
		|| StrContains(name, "Bip01 Footsteps")
		|| StrContains(name, "FootL_ref")
		|| StrContains(name, "FootR_ref")
		|| StrContains(name, "Head_ref")
		|| StrContains(name, "HandL_ref")
		|| StrContains(name, "HandR_ref")
		|| StrContains(name, "Chest_ref")
		|| StrContains(name, "groundParticleRef")
		|| StrContains(name, "effects_ref")
		|| StrContains(name, "trap_ref");

}

int EditorExternal::GetParentChildBonePos(ObjHndl obj, int boneIdx, ::Vec3& parentPos, ::Vec3& childPos) {

	auto model = mDll.currentModel;
	auto aasParams = mDll.animParams;

	auto parentId(model->GetBoneParentId(boneIdx));
	if (parentId < 0) {
		return parentId;
	}

	auto boneName(model->GetBoneName(boneIdx));
	if (boneName.empty()) {
		return -1;
	}

	if (IsIgnoredBone(boneName)) {
		return -1;
	}

	auto parentName(model->GetBoneName(parentId));
	if (parentName.empty()) {
		return -1;
	}

	DirectX::XMFLOAT4X4	worldMatrix;
	if (!model->GetBoneWorldMatrixByName(aasParams, parentName, &worldMatrix)) {
		return -1;
	}

	parentPos.x = worldMatrix._41;
	parentPos.y = worldMatrix._42;
	parentPos.z = worldMatrix._43;

	if (!model->GetBoneWorldMatrixByName(aasParams, boneName, &worldMatrix)) {
		return -1;
	}

	childPos.x = worldMatrix._41;
	childPos.y = worldMatrix._42;
	childPos.z = worldMatrix._43;
	return parentId;
}

bool EditorExternal::GetBonePos(ObjHndl obj, int boneIdx, ::Vec3& pos) {
	auto model = mDll.currentModel;
	auto aasParams = mDll.animParams;

	auto boneName(model->GetBoneName(boneIdx));
	if (boneName.empty()) {
		return false;
	}

	DirectX::XMFLOAT4X4	worldMatrix;
	if (!model->GetBoneWorldMatrixByName(aasParams, boneName, &worldMatrix)) {
		return false;
	}

	pos.x = worldMatrix._41;
	pos.y = worldMatrix._42;
	pos.z = worldMatrix._43;
	return true;
}

void EditorExternal::WorldToScreen(const ::Vec3& worldPos, ::Vec2& screenPos) {
	screenPos.x = 0;
	screenPos.y = 0;
}

bool EditorExternal::IsBoxVisible(const ::Vec2& screenPos, const ::Box2d& bounds) {
	return true; // Always visible in editor
}

void EditorExternal::SetObjPos(float x, float y, float z) {
	mX = x;
	mY = y;
	mZ = z;
}
