#include <particles/instances.h>
#include <particles/parser.h>
#include <particles/render.h>
#include <materials.h>
#include <textures.h>
#include <fstream>
#include <regex>

#include "external.h"
#include "renderstates.h"
#include "api.h"

using namespace particles;

class EditorMaterialManager : public gfx::MaterialManager {
public:
	explicit EditorMaterialManager(const std::string& dataPath, IDirect3DDevice9* device)
		: mDataPath(dataPath), mDevice(device) {
	}

	gfx::MaterialRef Resolve(const std::string& materialName) override;
private:
	std::string mDataPath;
	CComPtr<IDirect3DDevice9> mDevice;
};

class EditorTexture : public gfx::Texture {
public:

	EditorTexture(const std::string& name, const gfx::Size& size, IDirect3DTexture9* direct3DTexture9)
		: mName(name), mSize(size), mContentRect{0, 0, size.width, size.height}, mTexture(direct3DTexture9) {
	}

	const std::string& GetName() const override {
		return mName;
	}

	const gfx::ContentRect& GetContentRect() const override {
		return mContentRect;
	}

	const gfx::Size& GetSize() const override {
		return mSize;
	}

	IDirect3DTexture9* GetDeviceTexture() override {
		return mTexture;
	}

private:
	std::string mName;
	gfx::ContentRect mContentRect;
	gfx::Size mSize;
	CComPtr<IDirect3DTexture9> mTexture;
};

class EditorMaterial : public gfx::Material {
public:

	explicit EditorMaterial(const std::string& name, const std::string& texName, IDirect3DTexture9* texture)
		: mName(name) {

		if (texture) {
			D3DSURFACE_DESC desc;
			texture->GetLevelDesc(0, &desc);
			gfx::Size size = {(int)desc.Width, (int)desc.Height};

			mTexture = std::make_shared<EditorTexture>(texName, size, texture);
		}

	}

	std::string GetName() const override {
		return mName;
	}

	gfx::TextureRef GetPrimaryTexture() override {
		return mTexture;
	}

private:
	std::string mName;
	gfx::TextureRef mTexture;
};

gfx::MaterialRef EditorMaterialManager::Resolve(const std::string& materialName) {

	auto filename = fmt::format("{}\\art\\meshes\\Particle\\{}.mdf", mDataPath, materialName);

	std::regex r("\\s*Texture\\s+\"([^\\)]+)\"", std::regex_constants::icase);
	std::smatch match;

	std::ifstream infile(filename);
	std::string line;

	while (std::getline(infile, line)) {
		if (std::regex_search(line, match, r)) {
			auto textureFile = mDataPath + "\\" + match[1].str();

			CComPtr<IDirect3DTexture9> texture;
			HRESULT result = D3DXCreateTextureFromFileA(mDevice, textureFile.c_str(), &texture);
			if (SUCCEEDED(result)) {
				return std::make_shared<EditorMaterial>(materialName, textureFile, texture);
			}
		}
	}

	return std::make_shared<EditorMaterial>(materialName, "", nullptr);
}

struct PartSysFacade {
	PartSysFacade(const std::string& dataPath, IDirect3DDevice9* device)
		: mDataPath(dataPath),
		  mMaterials(std::make_shared<EditorMaterialManager>(mDataPath, device)),
		  mMeshes(std::make_shared<gfx::MeshesManager>()),
		  mParser(mMaterials, mMeshes) {

	}

	std::string mDataPath;
	std::shared_ptr<EditorMaterialManager> mMaterials;
	gfx::MeshesManagerPtr mMeshes;
	PartSysParser mParser;
};

static Vec3 ScreenToWorld(float screenX, float screenY) {
	if (!renderStates) {
		return Vec3(0, 0, 0);
	}
	D3DXMATRIX invProjWorld;
	D3DXMatrixTranspose(&invProjWorld, (D3DXMATRIX*)&renderStates->Get3dProjectionMatrix());
	D3DXVECTOR3 screenSpaceUnitX = *(D3DVECTOR*)&invProjWorld._11;
	D3DXVECTOR3 screenSpaceUnitY = *(D3DVECTOR *)&invProjWorld._21;
	D3DXVec3Normalize(&screenSpaceUnitX, &screenSpaceUnitX);
	D3DXVec3Normalize(&screenSpaceUnitY, &screenSpaceUnitY);

	auto pos = screenX * screenSpaceUnitX - screenY * screenSpaceUnitY;
	return Vec3(pos.x, pos.y, pos.z);
}

// This simplifies the function naming
extern "C" {

	// This is an example of an exported function.
	API PartSys* ParticleSystem_FromSpec(IDirect3DDevice9* device, const char* dataPath, const char* specTabFile) {

		// Set some required global state
		IPartSysExternal::SetCurrent(&EditorExternal::GetInstance());

		PartSysFacade facade(dataPath, device);

		auto& parser = facade.mParser;

		parser.ParseString(specTabFile);

		if (parser.begin() == parser.end()) {
			return nullptr;
		}

		auto name = parser.begin()->first;
		auto spec = parser.GetSpec(name);

		auto result = new PartSys(spec);
		// This is a dummy so the emiter is actually updated for obj position
		result->SetAttachedTo(1);
		return result;
	}

	API void ParticleSystem_Simulate(PartSys* sys, float elapsedSecs) {
		sys->Simulate(elapsedSecs);
	}

	API void ParticleSystem_SetPos(PartSys* sys, float screenX, float screenY) {
		auto worldPos = ScreenToWorld(screenX, screenY);
		EditorExternal editorExternal;
		sys->SetWorldPos(&editorExternal, worldPos.x, worldPos.y, worldPos.z);
	}

	API void ParticleSystem_SetObjPos(float screenX, float screenY) {
		auto worldPos = ScreenToWorld(screenX, screenY);
		EditorExternal::SetObjPos(worldPos.x, worldPos.y, worldPos.z);
	}

	API void ParticleSystem_Render(IDirect3DDevice9* device, PartSys* sys, float w, float h, float xTrans, float yTrans, float scale) {

		InitRenderStates(device, w, h, scale);

		if (sys->IsDead()) {
			sys->Reset();
		}

		particles::ParticleRendererManager renderManager(device);

		for (auto& emitter : *sys) {
			auto renderer = renderManager.GetRenderer(emitter->GetSpec()->GetParticleType());
			if (renderer) {
				renderer->Render(emitter.get());
			}
		}
	}

	API int ParticleSystem_GetEmitterCount(PartSys* sys) {
		return sys->GetEmitterCount();
	}

	API const PartSysEmitter* ParticleSystem_GetEmitter(PartSys* sys, int idx) {
		return sys->GetEmitter(idx);
	}

	API bool ParticleSystem_IsDead(PartSys* sys) {
		return sys->IsDead();
	}

	API void ParticleSystem_Free(PartSys* sys) {
		delete sys;
	}

	API int ParticleSystemEmitter_GetActiveParticles(const PartSysEmitter* emitter) {
		return emitter->GetActiveCount();
	}

	API int ParticleSystemEmitter_GetMaxParticles(const PartSysEmitter* emitter) {
		return emitter->GetParticles().size();
	}

}

void InitRenderStates(IDirect3DDevice9* device, float w, float h, float scale) {

	if (!renderStates) {
		renderStates.reset(new EditorRenderStates(device));
	}
	auto editorStates = (EditorRenderStates*)renderStates.get();
	editorStates->Update3dProjMatrix((float)w, (float)h, w * 0.5f, h * 0.5f, scale);

}
