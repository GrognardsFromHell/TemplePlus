
#include "stdafx.h"

#include <particles/parser.h>
#include <particles/instances.h>
#include <infrastructure/vfs.h>

using namespace particles;

class PartSysExternalMock : public IPartSysExternal {
public:
	float GetParticleFidelity() override {
		return 1.0f;
	}
	bool GetObjLocation(ObjHndl obj, Vec3& worldPos) override {
		return false;
	}
	bool GetObjRotation(ObjHndl obj, float& rotation) override {
		return false;
	}
	float GetObjRadius(ObjHndl obj) override {
		return 0;
	}
	bool GetBoneWorldMatrix(ObjHndl obj, const std::string& boneName, Matrix4x4& boneMatrix) override {
		return false;
	}
	int GetBoneCount(ObjHndl obj) override {
		return 0;
	}
	int GetParentChildBonePos(ObjHndl obj, int boneIdx, Vec3& parentPos, Vec3& childPos) override {
		return -1;
	}
	bool GetBonePos(ObjHndl obj, int boneIdx, Vec3& pos) override {
		return false;
	}
	void WorldToScreen(const Vec3& worldPos, Vec2& screenPos) override {
	}
	bool IsPointUnfogged(const Vec2& point) override {
		return true;
	}
	bool IsBoxVisible(const Box2d& box) override {
		return true;
	}
};

/*
This is a test fixture for particle system parser tests that only parses all particle systems once
to speed up the tests.
*/


class PartSysSimulationTest : public testing::Test {
protected:	
	static PartSysParser &GetParser() {
		static PartSysParser sParser;
		return sParser;
	}

	static void SetUpTestCase() {
		// Init VFS with mock/dummy code
		vfs.reset(Vfs::CreateStdIoVfs());
				
		GetParser().ParseFile("data\\minimal.tab");
	}
};

TEST_F(PartSysSimulationTest, TestMovement) {

	IPartSysExternal::SetCurrent(new PartSysExternalMock);

	auto spec = GetParser().GetSpec("Brasier");
	auto partSys = std::make_shared<PartSys>(spec);

	partSys->Simulate(1.0f);

	printf("\n");

}
