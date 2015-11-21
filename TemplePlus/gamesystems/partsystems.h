
#pragma once

#include "gamesystem.h"

#include <graphics/math.h>

struct GameSystemConf;

namespace particles {
	using PartSysSpecPtr = std::shared_ptr<class PartSysSpec>;
	using PartSysPtr = std::shared_ptr<class PartSys>;
}

class ParticleSysSystem : public GameSystem, public TimeAwareGameSystem {
public:
	using Handle = int;

	static constexpr auto Name = "ParticleSys";
	ParticleSysSystem();
	~ParticleSysSystem();

	void AdvanceTime(uint32_t time) override;

	const std::string &GetName() const override;
		
	Handle CreateAt(uint32_t nameHash, XMFLOAT3 pos);

private:
	std::unordered_map<uint32_t, particles::PartSysSpecPtr> mPartSysByHash;
	std::unordered_map<std::string, particles::PartSysSpecPtr> mPartSysByName;

	int mNextId = 1;
	std::unordered_map<Handle, particles::PartSysPtr> mActiveSys;
	std::unique_ptr<class PartSysExternal> mExternal;
};
