
#pragma once

#include <infrastructure/meshes.h>
#include <aas/aas_renderer.h>

#include "../../common.h"

#include "../gamesystem.h"

class MapObjectRenderer;

struct GMeshFile {
	std::string skaFilename;
	std::string skmFilename;
	uint32_t usageCount = 0;
	gfx::AnimatedModelPtr model;
};

struct GMeshInstance {
	GMeshFile *file = nullptr;
	XMFLOAT3 pos3d;
	XMFLOAT3 scale;
	LocFull loc;
	float rotation = 0;
};

class GMeshSystem : public GameSystem, public MapCloseAwareGameSystem {
public:
	static constexpr auto Name = "GMesh";
	GMeshSystem(gfx::AnimatedModelFactory& modelFactory);
	~GMeshSystem();
	const std::string &GetName() const override;

	void Load(const std::string &dataDir);
	void CloseMap() override;

	const std::vector<GMeshInstance> &GetInstances() const {
		return mInstances;
	}

private:
	gfx::AnimatedModelFactory &mModelFactory;
	std::vector<GMeshFile> mFiles;
	std::vector<GMeshInstance> mInstances;
};

class GMeshRenderer {
public:
	GMeshRenderer(aas::Renderer &aasRenderer,
		MapObjectRenderer &mapObjRenderer,
		GMeshSystem &gmeshSystem);

	void Render();

private:
	aas::Renderer &mAasRenderer;
	MapObjectRenderer &mMapObjRenderer;
	GMeshSystem &mGmeshSystem;
};
