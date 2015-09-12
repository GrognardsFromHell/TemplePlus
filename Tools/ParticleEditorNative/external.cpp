
#include "external.h"

float EditorExternal::GetParticleFidelity() {
	return 1.0f;
}

bool EditorExternal::GetObjLocation(ObjHndl obj, ::Vec3& worldPos) {
	return false;
}

bool EditorExternal::GetObjRotation(ObjHndl obj, float& rotation) {
	return false;
}

float EditorExternal::GetObjRadius(ObjHndl obj) {
	return 0.0f;
}

bool EditorExternal::GetBoneWorldMatrix(ObjHndl obj, const ::std::string& boneName, ::Matrix4x4& boneMatrix) {
	return false;
}

int EditorExternal::GetBoneCount(ObjHndl obj) {
	return 0;
}

int EditorExternal::GetParentChildBonePos(ObjHndl obj, int boneIdx, ::Vec3& parentPos, ::Vec3& childPos) {
	return -1;
}

bool EditorExternal::GetBonePos(ObjHndl obj, int boneIdx, ::Vec3& pos) {
	return false;
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
