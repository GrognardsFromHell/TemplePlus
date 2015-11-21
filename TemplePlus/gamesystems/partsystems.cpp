
#include "stdafx.h"
#include "partsystems.h"

#include <particles/parser.h>
#include <particles/instances.h>

using namespace particles;

class PartSysExternal : public IPartSysExternal {
public:
	float GetParticleFidelity() override;
	bool GetObjLocation(ObjHndl obj, Vec3& worldPos) override;
	bool GetObjRotation(ObjHndl obj, float& rotation) override;
	float GetObjRadius(ObjHndl obj) override;
	bool GetBoneWorldMatrix(ObjHndl obj, const std::string& boneName, Matrix4x4& boneMatrix) override;
	int GetBoneCount(ObjHndl obj) override;
	int GetParentChildBonePos(ObjHndl obj, int boneIdx, Vec3& parentPos, Vec3& childPos) override;
	bool GetBonePos(ObjHndl obj, int boneIdx, Vec3& pos) override;
	void WorldToScreen(const Vec3& worldPos, Vec2& screenPos) override;
	bool IsPointUnfogged(const Vec2& point) override;
	bool IsBoxVisible(const Box2d& box) override;
};

ParticleSysSystem::ParticleSysSystem() {

	PartSysParser parser;
	parser.ParseFile("rules\\partsys0.tab");
	parser.ParseFile("rules\\partsys1.tab");
	parser.ParseFile("rules\\partsys2.tab");

	for (auto &spec : parser) {
		mPartSysByName[spec.first] = spec.second;
		mPartSysByHash[spec.second->GetNameHash()] = spec.second;
	}

	mExternal = std::make_unique<PartSysExternal>();

	/*config_add_default(&config, "partsys_fidelity", "100", sub_10049EC0);
	a1 = (long double)config_get_int(&config, "partsys_fidelity") * 0.0099999998;
	set_particle_fidelity(a1);*/

}

ParticleSysSystem::~ParticleSysSystem() {
}

void ParticleSysSystem::AdvanceTime(uint32_t time) {

	float timeInSecs = time / 1000.0f;

	for (auto it = mActiveSys.begin(); it != mActiveSys.end(); ++it) {
		auto& sys = *it->second;

		sys.Simulate(timeInSecs);
	}

}

const std::string &ParticleSysSystem::GetName() const {
	static std::string name("ParticleSys");
	return name;
}

int ParticleSysSystem::CreateAt(uint32_t nameHash, XMFLOAT3 pos) {

	static std::string sEmptyName;

	auto it = mPartSysByHash.find(nameHash);

	if (it == mPartSysByHash.end()) {
		logger->warn("Unable to spawn unknown particle system: {}", nameHash);
		return -1;
	}

	auto& spec = it->second;

	auto sys(std::make_shared<PartSys>(spec));
	sys->SetWorldPos(mExternal.get(), pos.x, pos.y, pos.z);

	auto assignedId = mNextId++;

	mActiveSys[assignedId] = sys;
	return assignedId;

}

float PartSysExternal::GetParticleFidelity() {
	return 1.0f;
}

bool PartSysExternal::GetObjLocation(ObjHndl obj, Vec3& worldPos) {
	return false; // TODO
}

bool PartSysExternal::GetObjRotation(ObjHndl obj, float& rotation) {
	return false; // TODO
}

float PartSysExternal::GetObjRadius(ObjHndl obj) {
	return 0; // TODO
}

bool PartSysExternal::GetBoneWorldMatrix(ObjHndl obj, const std::string& boneName, Matrix4x4& boneMatrix) {
	return false; // TODO
}

int PartSysExternal::GetBoneCount(ObjHndl obj) {
	return -1; // TODO
}

int PartSysExternal::GetParentChildBonePos(ObjHndl obj, int boneIdx, Vec3& parentPos, Vec3& childPos) {
	return -1; // TODO
}

bool PartSysExternal::GetBonePos(ObjHndl obj, int boneIdx, Vec3& pos) {
	return false; // TODO
}

void PartSysExternal::WorldToScreen(const Vec3& worldPos, Vec2& screenPos) {
	// TODO
}

bool PartSysExternal::IsPointUnfogged(const Vec2& point) {
	return true; // TODO
}

bool PartSysExternal::IsBoxVisible(const Box2d& box) {
	return true; // TODO
}
