
#pragma once

#include <particles/external.h>

class EditorExternal : public IPartSysExternal {
public:
	float GetParticleFidelity() override;
	bool GetObjLocation(ObjHndl obj, ::Vec3& worldPos) override;
	bool GetObjRotation(ObjHndl obj, float& rotation) override;
	float GetObjRadius(ObjHndl obj) override;
	bool GetBoneWorldMatrix(ObjHndl obj, const ::std::string& boneName, ::Matrix4x4& boneMatrix) override;
	int GetBoneCount(ObjHndl obj) override;
	int GetParentChildBonePos(ObjHndl obj, int boneIdx, ::Vec3& parentPos, ::Vec3& childPos) override;
	bool GetBonePos(ObjHndl obj, int boneIdx, ::Vec3& pos) override;
	void WorldToScreen(const ::Vec3& worldPos, ::Vec2& screenPos) override;
	bool IsPointUnfogged(const ::Vec2& point) override;
	bool IsBoxVisible(const ::Box2d& box) override;	
	static EditorExternal &GetInstance();

	static void SetObjPos(float x, float y, float z);
private:
	static float mX, mY, mZ;
};
