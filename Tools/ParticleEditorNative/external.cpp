
#include "external.h"

#include <DirectXMath.h>

float EditorExternal::mX = 0;
float EditorExternal::mY = 0;
float EditorExternal::mZ = 0;

float EditorExternal::GetParticleFidelity() {
	return 1.0f;
}

bool EditorExternal::GetObjLocation(ObjHndl obj, ::Vec3& worldPos) {
	worldPos.x = mX;
	worldPos.y = mY;
	worldPos.z = mZ;
	return true;
}

bool EditorExternal::GetObjRotation(ObjHndl obj, float& rotation) {
	rotation = 0;
	return true;
}

float EditorExternal::GetObjRadius(ObjHndl obj) {
	return 0.0f;
}

bool EditorExternal::GetBoneWorldMatrix(ObjHndl obj, const ::std::string& boneName, ::Matrix4x4& boneMatrix) {
	DirectX::XMStoreFloat4x4(&boneMatrix, DirectX::XMMatrixIdentity());
	return true;
}

int EditorExternal::GetBoneCount(ObjHndl obj) {
	return 1;
}

int EditorExternal::GetParentChildBonePos(ObjHndl obj, int boneIdx, ::Vec3& parentPos, ::Vec3& childPos) {
	return -1;
}

bool EditorExternal::GetBonePos(ObjHndl obj, int boneIdx, ::Vec3& pos) {
	pos.x = 0;
	pos.y = 0;
	pos.z = 0;
	return true;
}

void EditorExternal::WorldToScreen(const ::Vec3& worldPos, ::Vec2& screenPos) {
	screenPos.x = 0;
	screenPos.y = 0;
}

bool EditorExternal::IsPointUnfogged(const ::Vec2& point) {
	return true; // Always visible in editor
}

bool EditorExternal::IsBoxVisible(const ::Box2d& box) {
	return true; // Always visible in editor
}

EditorExternal& EditorExternal::GetInstance() {
	static EditorExternal sInstance;
	return sInstance;
}

void EditorExternal::SetObjPos(float x, float y, float z) {
	mX = x;
	mY = y;
	mZ = z;
}
