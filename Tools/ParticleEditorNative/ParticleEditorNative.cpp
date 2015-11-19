#include <particles/instances.h>
#include <particles/parser.h>
#include <particles/render.h>
#include <graphics/mdfmaterials.h>
#include <graphics/textures.h>
#include <temple/aasrenderer.h>
#include <temple/meshes.h>
#include <temple/vfs.h>
#include <fstream>
#include <regex>

#include "external.h"
#include "renderstates.h"
#include "api.h"

using namespace particles;
using namespace gfx;
using namespace temple;

extern std::string lastError;

// This simplifies the function naming
extern "C" {

	// This is an example of an exported function.
	API bool ParticleSystem_FromSpec(TempleDll* dll, const char* specTabFile) {
		dll->renderingDevice.GetCamera().CenterOn(0, 0, 0);

		try {
			PartSysParser parser(dll->mdfFactory);
			parser.ParseString(specTabFile);

			if (parser.begin() == parser.end()) {
				lastError = "Unable to parse the spec.";
				return false;
			}

			auto name = parser.begin()->first;
			auto spec = parser.GetSpec(name);

			dll->partSys = std::make_unique<PartSys>(spec);
			// This is a dummy so the emiter is actually updated for obj position
			dll->partSys->SetAttachedTo(1);
			return true;
		} catch (std::exception &e) {
			lastError = e.what();
			return false;
		}
	}

	API void ParticleSystem_Simulate(TempleDll* dll, float elapsedSecs) {
		dll->partSys->Simulate(elapsedSecs);
	}

	API void ParticleSystem_SetPos(TempleDll* dll, float screenX, float screenY) {
		auto worldPos = dll->renderingDevice.GetCamera().ScreenToWorld(screenX, screenY);
		EditorExternal editorExternal;
		//dll->partSys->SetWorldPos(&editorExternal, worldPos.x, worldPos.y, worldPos.z);
	}

	API void ParticleSystem_SetObjPos(TempleDll* dll, float screenX, float screenY) {
		auto worldPos = dll->renderingDevice.GetCamera().ScreenToWorld(screenX, screenY);
		//EditorExternal::SetObjPos(worldPos.x, worldPos.y, worldPos.z);
	}

	API void ParticleSystem_SetScale(TempleDll* dll, float scale) {
		dll->renderingDevice.GetCamera().SetScale(scale);
	}

	API void ParticleSystem_Resize(TempleDll* dll, float w, float h) {
		dll->renderingDevice.GetCamera().SetScreenWidth(w, h);
	}

	API void ParticleSystem_Render(TempleDll* dll) {
		dll->renderingDevice.GetCamera().SetScale(1);

		if (dll->partSys->IsDead()) {
			dll->partSys->Reset();
		}

		for (auto& emitter : *dll->partSys) {
			auto& renderer = dll->renderManager.GetRenderer(emitter->GetSpec()->GetParticleType());
			renderer.Render(*emitter);
		}
	}

	API int ParticleSystem_GetEmitterCount(TempleDll* dll) {
		return dll->partSys->GetEmitterCount();
	}

	API const PartSysEmitter* ParticleSystem_GetEmitter(TempleDll* dll, int idx) {
		return dll->partSys->GetEmitter(idx);
	}

	API bool ParticleSystem_IsDead(TempleDll* dll) {
		return dll->partSys->IsDead();
	}

	API void ParticleSystem_Free(TempleDll* dll) {
		dll->partSys.reset();
	}

	API int ParticleSystemEmitter_GetActiveParticles(const PartSysEmitter* emitter) {
		return emitter->GetActiveCount();
	}

	API int ParticleSystemEmitter_GetMaxParticles(const PartSysEmitter* emitter) {
		return emitter->GetParticles().size();
	}

}
