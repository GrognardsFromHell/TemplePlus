
#include "stdafx.h"
#include "particlesystems.h"

#include <graphics/device.h>
#include <graphics/camera.h>
#include <graphics/shaperenderer2d.h>

#include <particles/parser.h>
#include <particles/instances.h>
#include "../obj.h"
#include "../config/config.h"

#include "ui/ui_render.h"

using namespace gfx;
using namespace particles;

class PartSysExternal : public IPartSysExternal {
public:
	PartSysExternal(ParticleSysSystem &system, WorldCamera &camera) 
		: mSystem(system), mCamera(camera) {}

	float GetParticleFidelity() override;
	bool GetObjLocation(ObjHndl obj, Vec3& worldPos) override;
	bool GetObjRotation(ObjHndl obj, float& rotation) override;
	float GetObjRadius(ObjHndl obj) override;
	bool GetBoneWorldMatrix(ObjHndl obj, const std::string& boneName, Matrix4x4& boneMatrix) override;
	int GetBoneCount(ObjHndl obj) override;
	int GetParentChildBonePos(ObjHndl obj, int boneIdx, Vec3& parentPos, Vec3& childPos) override;
	bool GetBonePos(ObjHndl obj, int boneIdx, Vec3& pos) override;
	void WorldToScreen(const Vec3& worldPos, Vec2& screenPos) override;
	bool IsBoxVisible(const Vec2& screenPos, const Box2d& box) override;
private:
	ParticleSysSystem &mSystem;
	WorldCamera &mCamera;	
};

ParticleSysSystem::ParticleSysSystem(WorldCamera& camera) {
	
	PartSysParser parser;
	parser.ParseFile("rules\\partsys0.tab");
	parser.ParseFile("rules\\partsys1.tab");
	parser.ParseFile("rules\\partsys2.tab");

	for (auto &spec : parser) {
		mPartSysByName[spec.first] = spec.second;
		mPartSysByHash[spec.second->GetNameHash()] = spec.second;
	}

	mExternal = std::make_unique<PartSysExternal>(*this, camera);
	IPartSysExternal::SetCurrent(mExternal.get());

	// Register a config for the partsys fidelity
	config.AddVanillaSetting("partsys_fidelity", "100", [=]() {
		SetFidelity(config.GetVanillaInt("partsys_fidelity") / 100.0f);
	});
	SetFidelity(config.GetVanillaInt("partsys_fidelity") / 100.0f);
}

ParticleSysSystem::~ParticleSysSystem() {
	IPartSysExternal::SetCurrent(nullptr);
}

void ParticleSysSystem::AdvanceTime(uint32_t time) {

	// First call
	if (mLastSimTime == 0) {
		mLastSimTime = time;
		return;
	}

	auto sinceLastSim = time - mLastSimTime;
	mLastSimTime = time;

	auto timeInSecs = sinceLastSim / 1000.0f;

	if (timeInSecs > 0.5f) {
		timeInSecs = 0.5f;
	}

	auto it = mActiveSys.begin();
	while (it != mActiveSys.end()) {
		auto& sys = *it->second;

		sys.Simulate(timeInSecs);

		// Remove dead systems
		if (sys.IsDead()) {
			it = mActiveSys.erase(it);
		} else {
			it++;
		}
	}

}

const std::string &ParticleSysSystem::GetName() const {
	static std::string name("ParticleSys");
	return name;
}

int ParticleSysSystem::CreateAt(uint32_t nameHash, XMFLOAT3 pos) {
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

ParticleSysSystem::Handle ParticleSysSystem::CreateAtObj(const std::string &name, objHndl obj) {
	auto it = mPartSysByName.find(tolower(name));

	if (it == mPartSysByName.end()) {
		logger->warn("Unable to spawn unknown particle system: {}", name);
		return -1;
	}

	auto& spec = it->second;

	auto loc = objects.GetLocationFull(obj);
	auto absLoc = loc.ToCenterOfTileAbs3D(objects.GetOffsetZ(obj));

	auto sys(std::make_shared<PartSys>(spec));
	sys->SetWorldPos(mExternal.get(), absLoc.x, absLoc.y, absLoc.z);
	sys->SetAttachedTo(obj);

	auto assignedId = mNextId++;

	mActiveSys[assignedId] = sys;
	return assignedId;
}

ParticleSysSystem::Handle ParticleSysSystem::CreateAtPos(const std::string &name, XMFLOAT3 pos) {
	auto it = mPartSysByName.find(tolower(name));

	if (it == mPartSysByName.end()) {
		logger->warn("Unable to spawn unknown particle system: {}", name);
		return -1;
	}

	auto& spec = it->second;

	auto sys(std::make_shared<PartSys>(spec));
	sys->SetWorldPos(mExternal.get(), pos.x, pos.y, pos.z);

	auto assignedId = mNextId++;

	mActiveSys[assignedId] = sys;
	return assignedId;
}

bool ParticleSysSystem::DoesNameExist(const std::string & name)
{
	return mPartSysByName.find(tolower(name)) != mPartSysByName.end();
}

bool ParticleSysSystem::DoesNameHashExist(uint32_t nameHash)
{
	return mPartSysByHash.find(nameHash) != mPartSysByHash.end();
}

particles::PartSysPtr ParticleSysSystem::GetByHandle(Handle handle) {
	auto it = mActiveSys.find(handle);
	if (it != mActiveSys.end()) {
		return it->second;
	}
	return nullptr;
}

void ParticleSysSystem::Remove(Handle handle)
{
	mActiveSys.erase(handle);
}

void ParticleSysSystem::End(Handle partsysId) {
	auto partSys = GetByHandle(partsysId);
	if (partSys) {
		partSys->EndPrematurely();
	}
}

void ParticleSysSystem::RemoveAll() {
	mActiveSys.clear();
}

float PartSysExternal::GetParticleFidelity() {
	return mSystem.GetFidelity();
}

bool PartSysExternal::GetObjLocation(ObjHndl obj, Vec3& worldPos) {
	auto locWithOffsets = objects.GetLocationFull(obj);
	auto offsetZ = objects.GetOffsetZ(obj);
	auto center3d(locWithOffsets.ToInches3D(offsetZ));
	worldPos.x = center3d.x;
	worldPos.y = center3d.y;
	worldPos.z = center3d.z;
	return true;
}

bool PartSysExternal::GetObjRotation(ObjHndl obj, float& rotation) {
	rotation = objects.GetRotation(obj);
	return true;
}

float PartSysExternal::GetObjRadius(ObjHndl obj) {
	return objects.GetRadius(obj);
}

bool PartSysExternal::GetBoneWorldMatrix(ObjHndl obj, const std::string& boneName, Matrix4x4& boneMatrix) {
	auto model = objects.GetAnimHandle(obj);
	if (!model) {
		return false;
	}

	auto animParams = objects.GetAnimParams(obj);

	auto objType = objects.GetType(obj);
	if (objType >= obj_t_weapon && objType <= obj_t_generic || objType == obj_t_bag) {
		auto parent = inventory.GetParent(obj);
		if (parent) {
			auto parentModel = objects.GetAnimHandle(parent);
			return parentModel->GetBoneWorldMatrixByNameForChild(
				model, animParams, boneName, &boneMatrix
			);
		}
	}
	
	return model->GetBoneWorldMatrixByName(animParams, boneName, &boneMatrix);
}

int PartSysExternal::GetBoneCount(ObjHndl obj) {
	auto model = objects.GetAnimHandle(obj);
	if (model) {
		return model->GetBoneCount();
	} else {
		return 0;
	}
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

int PartSysExternal::GetParentChildBonePos(ObjHndl obj, int boneIdx, Vec3& parentPos, Vec3& childPos) {

	auto model = objects.GetAnimHandle(obj);
	auto aasParams = objects.GetAnimParams(obj);

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

bool PartSysExternal::GetBonePos(ObjHndl obj, int boneIdx, Vec3& pos) {
	auto model = objects.GetAnimHandle(obj);
	auto aasParams = objects.GetAnimParams(obj);

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

void PartSysExternal::WorldToScreen(const Vec3& worldPos, Vec2& screenPos) {
	screenPos = mCamera.WorldToScreen(worldPos);
	auto offset2d = mCamera.Get2dTranslation();
	screenPos.x += offset2d.x;
	screenPos.y += offset2d.y;
}

bool PartSysExternal::IsBoxVisible(const Vec2& screenPos, const Box2d& box) {

	return mCamera.IsBoxOnScreen(screenPos,
		box.left, box.top,
		box.right, box.bottom);

}
