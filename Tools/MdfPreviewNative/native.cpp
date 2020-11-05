
#include "api.h"

#include <infrastructure/vfs.h>
#include <infrastructure/logging.h>
#include "spdlog/spdlog.h"

#include <temple/vfs.h>
#include <temple/dll.h>
#include <temple/meshes.h>
#include <aas/aas_renderer.h>
#include <graphics/buffers.h>
#include <graphics/textures.h>
#include <graphics/shaperenderer2d.h>
#include <graphics/shaperenderer3d.h>
#include <graphics/bufferbinding.h>
#include <graphics/mdfmaterials.h>
#include <particles/parser.h>
#include <particles/instances.h>
#include <particles/render.h>

#include <regex>
#include <aas\aas_model_factory.h>

using namespace DirectX;
using namespace gfx;
using namespace temple;
struct MdfPreviewNative;

/*
Interface for external functionality required by the particle systems.
*/
class PartSysExternal : public particles::IPartSysExternal {
public:

	PartSysExternal(MdfPreviewNative& native) : mNative(native) {}

	float GetParticleFidelity() override {
		return 0.25f;
	}

	bool GetObjLocation(ObjHndl obj, Vec3& worldPos) override {
		worldPos.x = 0;
		worldPos.y = 0;
		worldPos.z = 0;
		return true;
	}

	bool GetObjRotation(ObjHndl obj, float& rotation) override;

	float GetObjRadius(ObjHndl obj) override;

	bool GetBoneWorldMatrix(ObjHndl obj, const std::string& boneName, XMFLOAT4X4& boneMatrix) override;

	int GetBoneCount(ObjHndl obj) override;

	int GetParentChildBonePos(ObjHndl obj, int boneIdx, Vec3& parentPos, Vec3& childPos) override;

	bool GetBonePos(ObjHndl obj, int boneIdx, Vec3& pos) override;

	void WorldToScreen(const Vec3& worldPos, Vec2& screenPos) override;

	bool IsBoxVisible(const Vec2& screenPos, const Box2d& box) override {
		return true;
	}

private:
	MdfPreviewNative &mNative;
};

struct LogAppender : spdlog::sinks::base_sink<std::mutex>
{
	std::string logLines;

	void _sink_it(const spdlog::details::log_msg& msg) override
	{
		if (!logLines.empty()) {
			logLines.append("\n");
		}
		logLines.append(msg.formatted.str());
	}

	void _flush() override {}

};

struct MdfPreviewNative {
	std::unique_ptr<RenderingDevice> device;
	VertexBufferPtr vertexBuffer;
	IndexBufferPtr indexBuffer;
	std::unique_ptr<BufferBinding> bufferBinding;	
	std::unique_ptr<MdfMaterialFactory> materialFactory;
	MdfRenderMaterialPtr material;
	std::unique_ptr<AnimatedModelFactory> aasFactory;
	std::unique_ptr<ShapeRenderer2d> shapeRenderer2d;
	std::unique_ptr<ShapeRenderer3d> shapeRenderer3d;
	std::unique_ptr<aas::Renderer> aasRenderer;
	std::unique_ptr<particles::PartSysParser> parser;
	std::unique_ptr<particles::ParticleRendererManager> partSysRenderers;
	std::unique_ptr<PartSysExternal> partSysExternal;

	AnimatedModelParams aasParams;
	AnimatedModelPtr model;
	gfx::EncodedAnimId curAnimId = EncodedAnimId(2);
	bool loopAnim = false;
	bool pauseAnim = false;

	VertexBufferPtr sphereVertices;
	IndexBufferPtr sphereIndices;
	int spherePrimCount = 0;
	int sphereVertCount = 0;

	std::string error;
	std::shared_ptr<LogAppender> logAppender;

	std::vector<particles::PartSysPtr> partSys;

	void MakeSphere();

	void SpawnParticles(const std::string &name);
	void SimulateAndRenderParticles();
};

struct MdfVertex {
	XMFLOAT4 pos;
	XMFLOAT2 uv;
	XMFLOAT4 normal;
	XMCOLOR color;
};

API MdfPreviewNative* MdfPreviewNative_Load(const wchar_t* installPath,
	const wchar_t* tpDataPath) {
	
	spdlog::set_level(spdlog::level::debug);

	auto debugSink = std::make_shared<LogAppender>();
	spdlog::drop_all(); // Reset all previous loggers
	logger = spdlog::create("core", { debugSink });

	temple::Dll::GetInstance().Load(installPath);

	vfs = std::make_unique<temple::TioVfs>();

	auto& tioVfs = static_cast<temple::TioVfs&>(*vfs);
	tioVfs.AddPath("./ToEE1.dat");
	tioVfs.AddPath("./ToEE2.dat");
	tioVfs.AddPath("./ToEE3.dat");
	tioVfs.AddPath("./ToEE4.dat");
	tioVfs.AddPath("./data");
	
	auto localTpDataPath(ucs2_to_local(tpDataPath));
	tioVfs.AddPath(localTpDataPath.c_str());

	if (!tioVfs.FileExists("templeplus/data_present")) {
		MessageBox(nullptr, L"TemplePlus data could not be found.\nPlease copy tpdata to your ToEE installation directory.", 
			L"Data not found", MB_OK | MB_ICONERROR);
	}
	
	auto result(new MdfPreviewNative);
	result->logAppender = debugSink;
	return result;
}

API void MdfPreviewNative_Unload(MdfPreviewNative *native) {
	delete native;
	Dll::GetInstance().Unload();
}

API void MdfPreviewNative_InitDevice(MdfPreviewNative *native,
	HWND handle,
	int renderWidth, int renderHeight) {
	native->device = std::make_unique<RenderingDevice>(handle);

	std::array<MdfVertex, 4> corners;
	corners[0].pos = XMFLOAT4(-1, 1, 0.5f, 1);
	corners[0].normal = XMFLOAT4(0, 0, 1, 0);
	corners[0].uv = XMFLOAT2(0, 0);
	corners[0].color = 0xFFFFFFFF;
	corners[1].pos = XMFLOAT4(1, 1, 0.5f, 1);
	corners[1].normal = XMFLOAT4(0, 0, 1, 0);
	corners[1].uv = XMFLOAT2(1, 0);
	corners[1].color = 0xFFFFFFFF;
	corners[2].pos = XMFLOAT4(1, -1, 0.5f, 1);
	corners[2].normal = XMFLOAT4(0, 0, 1, 0);
	corners[2].uv = XMFLOAT2(1, 1);
	corners[2].color = 0xFFFFFFFF;
	corners[3].pos = XMFLOAT4(-1, -1, 0.5f, 1);
	corners[3].normal = XMFLOAT4(0, 0, 1, 0);
	corners[3].uv = XMFLOAT2(0, 1);
	corners[3].color = 0xFFFFFFFF;
	native->vertexBuffer = native->device->CreateVertexBuffer<MdfVertex>(corners);

	std::array<uint16_t, 6> indices{ 
		0, 1, 2,
		2, 3, 0
	};
	native->indexBuffer = native->device->CreateIndexBuffer(indices);

	native->bufferBinding = std::make_unique<BufferBinding>(native->device->CreateMdfBufferBinding());
	native->bufferBinding->AddBuffer(native->vertexBuffer, 0, sizeof(MdfVertex))
		.AddElement(VertexElementType::Float4, VertexElementSemantic::Position)
		.AddElement(VertexElementType::Float2, VertexElementSemantic::TexCoord)
		.AddElement(VertexElementType::Float4, VertexElementSemantic::Normal)
		.AddElement(VertexElementType::Color, VertexElementSemantic::Color);

	native->materialFactory = std::make_unique<MdfMaterialFactory>(*native->device);

	native->MakeSphere();

	AasConfig aasConfig;
	aasConfig.resolveMaterial = [=](const std::string &name) -> int {
		auto material = native->materialFactory->LoadMaterial(name);
		if (!material) {
			return 0;
		}
		return material->GetId();
	};

	// Example: "game.particles( 'Mon-Balor-Smokebody60', anim_obj )"
	std::regex particleRegex("\\s*game\\.particles\\(\\s*'([^']+)'\\s*,\\s*anim_obj\\s*\\)\\s*");

	aasConfig.runScript = [=](const std::string &command) {
		logger->info("Running Script: {}", command);
		std::smatch match;
		if (std::regex_match(command, match, particleRegex)) {
			auto partSys = match[1].str();
			logger->info("Spawning particle system {}", partSys);
			native->SpawnParticles(partSys);
		}
	};
	
	native->aasParams.rotation = XMConvertToRadians(135);
	
	native->aasFactory = std::make_unique<aas::AnimatedModelFactory>(
		aasConfig.resolveSkaFile,
		aasConfig.resolveSkmFile,
		aasConfig.runScript,
		aasConfig.resolveMaterial);
	native->shapeRenderer2d = std::make_unique<ShapeRenderer2d>(*native->device);
	native->shapeRenderer3d = std::make_unique<ShapeRenderer3d>(*native->device);

	native->aasRenderer = std::make_unique<aas::Renderer>(*native->aasFactory, 
		*native->device, 
		*native->shapeRenderer2d,
		*native->shapeRenderer3d,
		*native->materialFactory);

	native->parser = std::make_unique<particles::PartSysParser>();
	native->parser->ParseFile("rules/partsys0.tab");
	native->parser->ParseFile("rules/partsys1.tab");
	native->parser->ParseFile("rules/partsys2.tab");

	native->partSysRenderers = std::make_unique<particles::ParticleRendererManager>(*native->device,
		*native->aasFactory,
		*native->aasRenderer);
	native->partSysExternal = std::make_unique<PartSysExternal>(*native);
	particles::IPartSysExternal::SetCurrent(native->partSysExternal.get());
	
}

API void MdfPreviewNative_FreeDevice(MdfPreviewNative *native) {
	native->model.reset();
	native->sphereIndices.reset();
	native->sphereVertices.reset();
	native->parser.reset();
	native->aasRenderer.reset();
	native->aasFactory.reset();
	native->material.reset();
	native->materialFactory.reset();
	native->bufferBinding.reset();
	native->indexBuffer.reset();
	native->vertexBuffer.reset();
	native->device.reset();
}

API void MdfPreviewNative_SetCameraPos(MdfPreviewNative *native, float x, float y) {
	native->device->GetCamera().SetTranslation(x, y);
}
API void MdfPreviewNative_GetCameraPos(MdfPreviewNative *native, float* x, float* y) {
	
	auto translation = native->device->GetCamera().GetTranslation();
	*x = translation.x;
	*y = translation.y;

}

API void MdfPreviewNative_Render(MdfPreviewNative *native) {
	
	native->device->BeginFrame();

	// device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 0, 0, 0), 1.0f, 0);

	Light3d globalLight;
	globalLight.type = Light3dType::Directional;
	globalLight.color = XMFLOAT4(1, 1, 1, 1);
	globalLight.dir = XMFLOAT4(0, 0, -1, 0);
	globalLight.pos = XMFLOAT4(0, 0, 0, 1);
	globalLight.range = 800;
		
	if (native->material) {
		native->device->GetCamera().SetIdentityTransform(true);

		std::vector<Light3d> lights{ globalLight };
		native->material->Bind(*native->device, lights);
		native->bufferBinding->Bind();
		native->device->SetIndexBuffer(*native->indexBuffer);
		native->device->DrawIndexed(gfx::PrimitiveType::TriangleList, 4, 6);
	}

	if (native->model) {
		native->device->GetCamera().SetIdentityTransform(false);

		globalLight.color = XMFLOAT4(1, 1, 1, 1);
		globalLight.dir = XMFLOAT4(-0.6324094f, -0.7746344f, 0, 0);
		std::vector<Light3d> lights{ globalLight };
		
		if (!native->pauseAnim) {
			auto events = native->model->Advance(0.033f, 3.0f, 0.063f, native->aasParams);
			if (events.IsEnd() && native->loopAnim) {
				native->model->SetAnimId(native->curAnimId);
			}
		}
		
		native->aasRenderer->Render(native->model.get(), native->aasParams, lights);

		native->SimulateAndRenderParticles();
	}

	native->device->Present();

}

API bool MdfPreviewNative_SetModel(MdfPreviewNative *native, 
	const char *skmFilename,
	const char *skaFilename) {

	native->material.reset();
	native->device->GetCamera().CenterOn(0, 0, 0);

	try {
		EncodedAnimId idleId(WeaponAnim::Idle);
		native->model = native->aasFactory->FromFilenames(
			skmFilename,
			skaFilename,
			idleId,
			native->aasParams
			);

		auto headMdf(native->materialFactory->LoadMaterial("art\\meshes\\PCs\\PC_Human_Male\\head.mdf"));
		native->model->AddReplacementMaterial(gfx::MaterialPlaceholderSlot::HEAD, headMdf);
		auto handsMdf(native->materialFactory->LoadMaterial("art\\meshes\\PCs\\PC_Human_Male\\hands.mdf"));
		native->model->AddReplacementMaterial(gfx::MaterialPlaceholderSlot::GLOVES, handsMdf);
		auto chestMdf(native->materialFactory->LoadMaterial("art\\meshes\\PCs\\PC_Human_Male\\chest.mdf"));
		native->model->AddReplacementMaterial(gfx::MaterialPlaceholderSlot::CHEST, chestMdf);
		auto feetMdf(native->materialFactory->LoadMaterial("art\\meshes\\PCs\\PC_Human_Male\\feet.mdf"));
		native->model->AddReplacementMaterial(gfx::MaterialPlaceholderSlot::BOOTS, feetMdf);

		MdfPreviewNative_SetAnimation(native, (int)WeaponAnim::LeftAttack, true);
		
		native->device->GetCamera().SetCameraAngle(0.f);

		return true;
	} catch (std::exception &e) {
		native->error = e.what();
		return false;
	}

}

API void MdfPreviewNative_SetAnimation(MdfPreviewNative* native, int animId, bool combatAnimation)
{
	auto newAnimId = combatAnimation ? EncodedAnimId( (WeaponAnim)animId )  : EncodedAnimId(animId);
	native->curAnimId = newAnimId;
	if (native->model) {
		native->model->SetAnimId(native->curAnimId);
	}
}

API bool MdfPreviewNative_SetMaterial(MdfPreviewNative *native, const char *name) {

	native->model.reset();

	try {
		native->material = native->materialFactory->LoadMaterial(name);
		return true;
	} catch (std::exception &e) {
		native->error = e.what();
		return false;
	}

}

void MdfPreviewNative::MakeSphere()
{
	int nRings = 32;
	int nSegments = 32;
	float r = 1;

	std::vector<MdfVertex> vertices;

	// allocate the vertex buffer
	int vertexCount = (nRings + 1) * (nSegments + 1);
	vertices.reserve(vertexCount);
	
	// allocate index buffer
	int primCount = 6 * nRings * (nSegments + 1);
	std::vector<uint16_t> indices;
	indices.reserve(primCount * 3);
	
	float fDeltaRingAngle = (XM_PI / nRings);
	float fDeltaSegAngle = (XM_2PI / nSegments);
	unsigned short wVerticeIndex = 0;

	// Generate the group of rings for the sphere
	for (int ring = 0; ring <= nRings; ring++) {
		float r0 = r * sinf(ring * fDeltaRingAngle);
		float y0 = r * cosf(ring * fDeltaRingAngle);

		// Generate the group of segments for the current ring
		for (int seg = 0; seg <= nSegments; seg++) {
			float x0 = r0 * sinf(seg * fDeltaSegAngle);
			float z0 = r0 * cosf(seg * fDeltaSegAngle);

			// Add one vertex to the strip which makes up the sphere
			MdfVertex vertex;
			vertex.pos.x = x0;
			vertex.pos.y = y0;
			vertex.pos.z = z0;

			XMFLOAT4 normal;
			XMStoreFloat4(&normal, XMVector3Normalize(XMVectorSet(x0, y0, z0, 0)));
			vertex.normal = normal;

			vertex.uv.x = (float)seg / (float)nSegments;
			vertex.uv.y = (float)ring / (float)nRings;

			vertex.color = 0xFFFFFFFF;

			vertices.push_back(vertex);

			if (ring != nRings) {
				// each vertex (except the last) has six indices pointing to it
				
				indices.push_back(wVerticeIndex + nSegments);
				indices.push_back(wVerticeIndex);
				indices.push_back(wVerticeIndex + nSegments + 1);
				
				indices.push_back(wVerticeIndex);
				indices.push_back(wVerticeIndex + 1);				
				indices.push_back(wVerticeIndex + nSegments + 1);
				
				wVerticeIndex++;
			}
		}; // end for seg
	} // end for ring
	
	spherePrimCount = primCount;
	sphereVertices = device->CreateVertexBuffer<MdfVertex>(vertices);
	sphereVertCount = vertices.size();
	sphereIndices = device->CreateIndexBuffer(indices);

}

static char* AllocFromStr(const std::string &str) {
	char* result = (char*)CoTaskMemAlloc(str.length() + 1);
	memcpy(result, str.c_str(), str.length());
	result[str.length()] = 0;
	return result;
}

API char *MdfPreviewNative_GetError(MdfPreviewNative *native) {
	return AllocFromStr(native->error);
}

API char *MdfPreviewNative_GetAndClearLog(MdfPreviewNative *native) {
	auto result = AllocFromStr(native->logAppender->logLines);
	native->logAppender->logLines.clear();
	return result;
}

API void MdfPreviewNative_SetRotation(MdfPreviewNative *native, float rotation) {
	native->aasParams.rotation = rotation;
}

API void MdfPreviewNative_SetScale(MdfPreviewNative *native, float scale) {
	native->device->GetCamera().SetScale(scale);
}

API void MdfPreviewNative_SetOffsetZ(MdfPreviewNative* native, float offz)
{
	native->aasParams.offsetZ= offz;
}

API void MdfPreviewNative_SetLoopAnimation(MdfPreviewNative* native, bool loopEn)
{
	native->loopAnim = loopEn;
}

API void MdfPreviewNative_SetPauseAnimation(MdfPreviewNative* native, bool en)
{
	native->pauseAnim = en;
}

API void MdfPreviewNative_SetCombatAnimation(MdfPreviewNative* native, bool en)
{
	auto curAnum = native->curAnimId;
	if (curAnum.IsWeaponAnim() == en) {
		return;
	}

}

void MdfPreviewNative::SpawnParticles(const std::string &name) {
	auto partSysSpec = parser->GetSpec(name);
	if (!partSysSpec) {
		logger->warn("Unknown particle system: {}", name);
		return;
	}

	auto sys(std::make_shared<particles::PartSys>(partSysSpec));
	sys->SetAttachedTo(1);
	partSys.push_back(sys);
}

void MdfPreviewNative::SimulateAndRenderParticles() {
	auto it = partSys.begin();
	while (it != partSys.end()) {
		auto sys = it->get();
		sys->Simulate(0.033f);

		if (sys->IsDead()) {
			it = partSys.erase(it);
			continue;
		}

		for (auto &emitter : *sys) {
			auto& renderer = partSysRenderers->GetRenderer(emitter->GetSpec()->GetParticleType());
			renderer.Render(*emitter);
		}		

		++it;
	}
}


bool PartSysExternal::GetObjRotation(ObjHndl obj, float& rotation) {
	rotation = mNative.aasParams.rotation;
	return true;
}

float PartSysExternal::GetObjRadius(ObjHndl obj) {
	return 100; // TODO: Calculate based on what, exactly?
}

bool PartSysExternal::GetBoneWorldMatrix(ObjHndl obj, 
	                                     const std::string& boneName, 
	                                     XMFLOAT4X4& boneMatrix) {
	return mNative.model->GetBoneWorldMatrixByName(mNative.aasParams, boneName, &boneMatrix);
}

int PartSysExternal::GetBoneCount(ObjHndl obj) {
	return mNative.model->GetBoneCount();
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

	auto parentId(mNative.model->GetBoneParentId(boneIdx));
	if (parentId < 0) {
		return parentId;
	}

	auto boneName(mNative.model->GetBoneName(boneIdx));
	if (boneName.empty()) {
		return -1;
	}

	if (IsIgnoredBone(boneName)) {
		return -1;
	}

	auto parentName(mNative.model->GetBoneName(parentId));
	if (parentName.empty()) {
		return -1;
	}

	DirectX::XMFLOAT4X4	worldMatrix;
	if (!mNative.model->GetBoneWorldMatrixByName(mNative.aasParams, parentName, &worldMatrix)) {
		return -1;
	}
	
	parentPos.x = worldMatrix._41;
	parentPos.y = worldMatrix._42;
	parentPos.z = worldMatrix._43;

	if (!mNative.model->GetBoneWorldMatrixByName(mNative.aasParams, boneName, &worldMatrix)) {
		return -1;
	}

	childPos.x = worldMatrix._41;
	childPos.y = worldMatrix._42;
	childPos.z = worldMatrix._43;
	return parentId;
}

bool PartSysExternal::GetBonePos(ObjHndl obj, int boneIdx, Vec3& pos) {
	
	auto boneName(mNative.model->GetBoneName(boneIdx));
	if (boneName.empty()) {
		return false;
	}
	
	DirectX::XMFLOAT4X4	worldMatrix;
	if (!mNative.model->GetBoneWorldMatrixByName(mNative.aasParams, boneName, &worldMatrix)) {
		return false;
	}
	
	pos.x = worldMatrix._41;
	pos.y = worldMatrix._42;
	pos.z = worldMatrix._43;
	return true;

}

void PartSysExternal::WorldToScreen(const Vec3& worldPos, Vec2& screenPos) {

	screenPos = mNative.device->GetCamera().WorldToScreen(worldPos);

}

API void MdfPreviewNative_SetRenderSize(MdfPreviewNative *native, int w, int h) {
	native->device->ResizeBuffers(w, h);
}

API void MdfPreviewNative_ScreenToWorld(MdfPreviewNative *native, float x, float y, float* xOut, float *yOut, float *zOut) {
	auto world = native->device->GetCamera().ScreenToWorld(x, y);
	*xOut = world.x;
	*yOut = world.y;
	*zOut = world.z;
}
