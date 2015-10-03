#include "stdafx.h"
#include <util/fixes.h>
#include <obj.h>
#include <hashtable.h>
#include <limits>
#include "tig/tig_partsys.h"

static class PartSysLagFix : public TempleFix {
public:
	const char* name() override {
		return "Particle System Simulation Spam Fix";
	}

	void apply() override {
		// OrgSpawnPartSysAtObj = replaceFunction(0x10049B70, SpawnPartSysAtObj);
		// OrgSpawnPartSysAtLoc = replaceFunction(0x10049BD0, SpawnPartSysAtLoc);
		OrgSimulateAllPartSys = replaceFunction(0x10049AF0, SimulateAllPartSys);
		OrgRenderAllPartSys = replaceFunction(0x101E7B80, RenderAllPartSys);
		OrgSimulatePartSys = replaceFunction(0x101E7130, SimulatePartSys);
		sPartSysTable = temple::GetPointer<ToEEHashtable<legacypartsys::PartSysSpec>*>(0x10EEEA04);
	}

	static int CountPartSys();

	static ToEEHashtable<legacypartsys::PartSysSpec>** sPartSysTable;

	static int (*OrgSpawnPartSysAtObj)(uint32_t nameHash, objHndl obj);
	static int SpawnPartSysAtObj(uint32_t nameHash, objHndl obj);

	static int (*OrgSpawnPartSysAtLoc)(uint32_t nameHash, float x, float y, float z);
	static int SpawnPartSysAtLoc(uint32_t nameHash, float x, float y, float z);

	static void (*OrgSimulateAllPartSys)(int gameTimeMs);
	static void SimulateAllPartSys(int gameTimeMs);

	static void (*OrgRenderAllPartSys)();
	static void RenderAllPartSys();

	static void (*OrgSimulatePartSys)(legacypartsys::PartSys* sys, float simulTimeSecs);
	static void SimulatePartSys(legacypartsys::PartSys* sys, float simulTimeSecs);
	
	// Caches max particle system lifetime for each particle system spec
	static std::unordered_map<string, float> sMaxLifetimeCache;
	static float GetPartSysMaxLifetime(legacypartsys::PartSys* sys);

} fix;

ToEEHashtable<legacypartsys::PartSysSpec>** PartSysLagFix::sPartSysTable;
int (*PartSysLagFix::OrgSpawnPartSysAtObj)(uint32_t nameHash, objHndl obj);
int (*PartSysLagFix::OrgSpawnPartSysAtLoc)(uint32_t nameHash, float x, float y, float z);
void (*PartSysLagFix::OrgSimulateAllPartSys)(int gameTimeMs);
void (*PartSysLagFix::OrgRenderAllPartSys)();
void (*PartSysLagFix::OrgSimulatePartSys)(legacypartsys::PartSys* sys, float simulTimeSecs);
std::unordered_map<string, float> PartSysLagFix::sMaxLifetimeCache;

int PartSysLagFix::CountPartSys() {

	auto sys = reinterpret_cast<legacypartsys::PartSys*>((*sPartSysTable)->pad);

	auto active = 0;

	while (sys) {
		active++;
		sys = sys->next;
	}

	return active;

}

int PartSysLagFix::SpawnPartSysAtObj(uint32_t nameHash, objHndl obj) {

	auto name = objects.GetDisplayName(obj, obj);

	auto count = CountPartSys() + 1;
	logger->info("Spawning particle system {:x} @ obj {} ({:x}) (Active: {})",
	             nameHash, name, obj, count);
	return OrgSpawnPartSysAtObj(nameHash, obj);
}

int PartSysLagFix::SpawnPartSysAtLoc(uint32_t nameHash, float x, float y, float z) {
	auto count = CountPartSys() + 1;
	logger->info("Spawning particle system {:x} @ {},{},{} (Active: {})", nameHash, x, y, z, count);
	return OrgSpawnPartSysAtLoc(nameHash, x, y, z);
}

void PartSysLagFix::SimulateAllPartSys(int gameTimeMs) {

	Stopwatch sw;

	OrgSimulateAllPartSys(gameTimeMs);

	auto elapsed = sw.GetElapsedMs();
	if (elapsed > 250) {
		logger->info("Slow part sys simulation: {}ms", elapsed);
	}

}

void PartSysLagFix::RenderAllPartSys() {

	Stopwatch sw;

	OrgRenderAllPartSys();

	auto elapsed = sw.GetElapsedMs();
	if (elapsed > 250) {
		logger->info("Slow part sys rendering: {}ms", elapsed);
	}

}

void PartSysLagFix::SimulatePartSys(legacypartsys::PartSys* sys, float simulTimeSecs) {

	auto lifetime = GetPartSysMaxLifetime(sys);

	/*
		If a particle system is out of view, it's emitters dont advance and the system overall
		will never be killed even if it's long long dead. To alleviate this, we manually
		kill a system's emitters, when the entire system is dead.

		A lifetime of ininity means one of the system's emitters has a permanent lifespan. This
		if is always false in that case.
	*/
	if (sys->aliveInSecs + simulTimeSecs >= lifetime) {
		// Kill it immediately without even trying to simulate it by
		// setting the emitter alive time to infinity
		for (auto i = 0; i < sys->spec->emitterCount; ++i) {
			sys->emitters[i]->aliveInSecs = std::numeric_limits<float>::infinity();
		}
	}

	OrgSimulatePartSys(sys, simulTimeSecs);
}

float PartSysLagFix::GetPartSysMaxLifetime(legacypartsys::PartSys* sys) {

	auto it = sMaxLifetimeCache.find(sys->spec->name);

	if (it != sMaxLifetimeCache.end()) {
		return it->second;
	}

	// Calculate the real max lifetime of the system
	auto lifetime = 0.0f;

	auto spec = sys->spec;
	Guide::array_view<legacypartsys::PartSysEmitterSpec*> emitters(spec->emitters, spec->emitterCount);

	for (auto emitter : emitters) {
		if (emitter->flags & legacypartsys::PSEF_PERM) {
			lifetime = std::numeric_limits<float>::infinity();
			break;
		}

		auto emitterDeathTime = emitter->delay + emitter->emitterLifespan + emitter->particleLifespan;
		if (emitterDeathTime > lifetime) {
			lifetime = emitterDeathTime;
		}
	}

	sMaxLifetimeCache[sys->spec->name] = lifetime;
	return lifetime;
}
