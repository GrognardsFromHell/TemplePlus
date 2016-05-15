
#include "stdafx.h"

#include "util/fixes.h"
#include "gamesystems.h"
#include "particlesystems.h"

#include <particles/instances.h>

#include <graphics/math.h>
#include <obj.h>

using PartSysHandle = int;

static class PartSysHooks : public TempleFix {
public:
	void apply() override {
		replaceFunction(0x101E6E30, GetNameHashByHandle);
		replaceFunction(0x101E77F0, DoesHashExist);
		replaceFunction(0x101E78A0, KillAll);
		replaceFunction(0x101E7BB0, CreateAt);
		replaceFunction(0x101E7C80, Kill);
		replaceFunction(0x101E7D10, End);
		replaceFunction(0x101E7D50, SetObj);
		replaceFunction(0x101E7E00, InvalidateObj);
	}

	static uint32_t GetNameHashByHandle(PartSysHandle handle);
	static BOOL DoesHashExist(uint32_t hash);
	static void KillAll();
	static PartSysHandle CreateAt(uint32_t hash, XMFLOAT3 pos);
	static void Kill(PartSysHandle handle);
	static void End(PartSysHandle handle);
	static void SetObj(PartSysHandle handle, objHndl obj);
	static void InvalidateObj(objHndl obj);

} hooks;

uint32_t PartSysHooks::GetNameHashByHandle(PartSysHandle handle) {
	if (!handle) {
		return 0;
	}

	auto &particles = gameSystems->GetParticleSys();
	auto sys = particles.GetByHandle(handle);
	if (!sys) {
		logger->error("Trying to get name hash for invalid particle system handle: {}", handle);
		return 0;
	}
	return sys->GetSpec()->GetNameHash();
}

BOOL PartSysHooks::DoesHashExist(uint32_t hash) {
	auto &particles = gameSystems->GetParticleSys();
	return particles.DoesNameHashExist(hash) ? TRUE : FALSE;
}

void PartSysHooks::KillAll() {
	if (!gameSystems) {
		return; // Already all destroyed
	}
	gameSystems->GetParticleSys().RemoveAll();
}

PartSysHandle PartSysHooks::CreateAt(uint32_t hash, XMFLOAT3 pos) {
	auto &particles = gameSystems->GetParticleSys();
	return particles.CreateAt(hash, pos);
}

void PartSysHooks::Kill(PartSysHandle handle) {
	if (!gameSystems) {
		return; // Already all destroyed
	}
	auto &particles = gameSystems->GetParticleSys();
	particles.Remove(handle);
}

void PartSysHooks::End(PartSysHandle handle) {
	if (!gameSystems) {
		return; // Already all destroyed
	}
	auto &particles = gameSystems->GetParticleSys();
	auto sys = particles.GetByHandle(handle);
	if (!sys) {
		logger->error("Trying to set object for invalid particle system handle: {}", handle);
		return;
	}
	sys->EndPrematurely();
}

void PartSysHooks::SetObj(PartSysHandle handle, objHndl obj) {
	auto &particles = gameSystems->GetParticleSys();
	auto sys = particles.GetByHandle(handle);
	if (!sys) {
		logger->error("Trying to set object for invalid particle system handle: {}", handle);
		return;
	}
	sys->SetAttachedTo(obj.handle);
}

void PartSysHooks::InvalidateObj(objHndl obj) {
	if (!gameSystems) {
		return; // Already all destroyed
	}
	auto &particles = gameSystems->GetParticleSys();
	for (auto &sys : particles) {
		if (sys.second->GetAttachedTo() == obj.handle) {
			sys.second->SetAttachedTo(0);
			sys.second->EndPrematurely();
		}
	}
}
