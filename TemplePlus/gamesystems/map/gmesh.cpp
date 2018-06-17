
#include "stdafx.h"
#include "gmesh.h"
#include "util/streams.h"
#include "../mapobjrender.h"
#include "tig/tig_startup.h"


//*****************************************************************************
//* GMesh

GMeshSystem::GMeshSystem(gfx::AnimatedModelFactory &modelFactory) : mModelFactory(modelFactory) {
}

GMeshSystem::~GMeshSystem() {
}

const std::string &GMeshSystem::GetName() const {
	static std::string name("GMesh");
	return name;
}

static gfx::AnimatedModelParams GetAnimParams(const GMeshInstance &instance) {
	gfx::AnimatedModelParams params;
	params.rotation = instance.rotation;
	auto& loc = instance.loc.location;
	params.x = loc.location.locx;
	params.y = loc.location.locy;
	params.offsetX = loc.off_x;
	params.offsetY = loc.off_y;
	params.offsetZ = instance.loc.off_z;
	params.scale = 1;
	return params;
}

void GMeshSystem::Load(const std::string & dataDir)
{
	auto filesFilename = fmt::format("{}\\gmesh.gmf", dataDir);

	mFiles.clear();
	mInstances.clear();

	VfsInputStream filesIn(filesFilename);

	auto filesCount = filesIn.ReadUInt32();
	mFiles.reserve(filesCount);
	for (size_t i = 0; i < filesCount; i++) {
		GMeshFile file;
		file.skaFilename = filesIn.ReadStringFixed(260);
		file.skmFilename = filesIn.ReadStringFixed(260);
		mFiles.emplace_back(std::move(file));
	}

	auto instancesFilename = fmt::format("{}\\gmesh.gmi", dataDir);
	VfsInputStream instancesIn(instancesFilename);
	auto instancesCount = instancesIn.ReadUInt32();
	mInstances.reserve(instancesCount);
	for (size_t i = 0; i < instancesCount; ++i) {
		auto fileId = instancesIn.ReadUInt32();
		GMeshInstance instance;
		instance.file = &mFiles[fileId];
		instance.pos3d = instancesIn.ReadXMFLOAT3();
		instance.loc.location = LocAndOffsets::FromInches(instance.pos3d.x, instance.pos3d.z);
		instance.loc.off_z = instance.pos3d.y;
		instance.scale = instancesIn.ReadXMFLOAT3();
		instance.rotation = instancesIn.ReadFloat();

		// Initialize the model when the first object that uses it appears
		if (instance.file->usageCount == 0) {
			auto params = GetAnimParams(instance);
			instance.file->model = mModelFactory.FromFilenames(instance.file->skmFilename,
				instance.file->skaFilename,
				gfx::EncodedAnimId(0),
				params);
		}
		instance.file->usageCount++;

		mInstances.emplace_back(std::move(instance));
	}
	
}

void GMeshSystem::CloseMap() {
	mInstances.clear();
	mFiles.clear();
}

GMeshRenderer::GMeshRenderer(aas::Renderer& aasRenderer, MapObjectRenderer& mapObjRenderer, GMeshSystem& gmeshSystem)
	: mAasRenderer(aasRenderer), mMapObjRenderer(mapObjRenderer), mGmeshSystem(gmeshSystem) {
}

void GMeshRenderer::Render() {

	gfx::PerfGroup perfGroup(tig->GetRenderingDevice(), "GMesh");

	auto& instances = mGmeshSystem.GetInstances();

	for (auto &instance : instances) {
		auto file = instance.file;

		// Find lights that might affect the gmesh at it's position
		auto lights = mMapObjRenderer.FindLights(instance.loc.location, 0);

		auto params = GetAnimParams(instance);
		mAasRenderer.Render(file->model.get(), params, lights);
	}

}
