
#include <particles/instances.h>
#include <particles/parser.h>
#include <particles/render.h>
#include <materials.h>
#include <textures.h>
#include <d3dx9math.h>

#include "external.h"
#include "renderstates.h"
#include "api.h"

class EditorMaterialManager : public gfx::MaterialManager {
public:
	gfx::MaterialRef Resolve(const std::string& materialName) override;
};

class EditorMaterial : public gfx::Material {
public:

	explicit EditorMaterial(const std::string& name)
		: mName(name) {
	}

	std::string GetName() const override {
		return mName;
	}

	gfx::TextureRef GetPrimaryTexture() override {
		return gfx::TextureRef(); // NYI
	}

private:
	std::string mName;
};

gfx::MaterialRef EditorMaterialManager::Resolve(const std::string& materialName) {
	return std::make_shared<EditorMaterial>(materialName);
}

struct PartSysFacade {
	PartSysFacade(const std::string& dataPath) : mDataPath(dataPath), mParser(mMaterials) {

	}

	std::string mDataPath;
	EditorMaterialManager mMaterials;
	PartSysParser mParser;
};

// This simplifies the function naming
extern "C" {

	// This is an example of an exported function.
	API PartSys* ParticleSystem_FromSpec(const char* dataPath, const char* specTabFile) {
		
		// Set some required global state
		IPartSysExternal::SetCurrent(&EditorExternal::GetInstance());
		
		PartSysFacade facade(dataPath);

		auto& parser = facade.mParser;

		parser.ParseString(specTabFile);

		if (parser.begin() == parser.end()) {
			return nullptr;
		}

		auto name = parser.begin()->first;
		auto spec = parser.GetSpec(name);

		return new PartSys(spec);
	}

	API void ParticleSystem_Simulate(PartSys* sys, float elapsedSecs) {
		sys->Simulate(elapsedSecs);
	}
	
	API void ParticleSystem_Render(IDirect3DDevice9 *device, PartSys* sys, float w, float h, float xTrans, float yTrans, float scale) {
		
		InitRenderStates(device, w, h, scale);

		if (sys->GetAliveInSecs() > 5) {
			sys->Reset();
		}

		particles::ParticleRendererManager renderManager(device);
		
		for (auto &emitter : *sys) {
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

void InitRenderStates(IDirect3DDevice9 *device, float w, float h, float scale) {

	if (!renderStates) {
		renderStates.reset(new EditorRenderStates(device));
	}
	auto editorStates = (EditorRenderStates*)renderStates.get();
	editorStates->Update3dProjMatrix((float)w, (float)h, w * 0.5f, h * 0.5f, scale);
	
}
