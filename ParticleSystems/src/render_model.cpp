
#include "particles/render_point.h"
#include "render_general.h"

#include <infrastructure/vfs.h>
#include <infrastructure/logging.h>
#include <graphics/mdfmaterials.h>

namespace particles {

	using namespace gfx;

	/**
	 * The rendering state associated with a point emitter.
	 */
	struct ModelEmitterRenderState : PartSysEmitterRenderState {
		ModelEmitterRenderState(const AnimatedModelPtr &model) : model(model) {}

		AnimatedModelPtr model;
	};
	
	ModelParticleRenderer::ModelParticleRenderer(RenderingDevice& device,
		AnimatedModelFactory &aasFactory,
		AnimatedModelRenderer &aasRenderer)
		: ParticleRenderer(device), 
		mDevice(device),
		mModelFactory(aasFactory),
		mModelRenderer(aasRenderer) {}

	void ModelParticleRenderer::Render(PartSysEmitter &emitter) {

		auto it = emitter.NewIterator();
		auto totalCount = emitter.GetActiveCount();

		AnimatedModelParams animParams;

		// Lazily initialize render state
		if (!emitter.HasRenderState()) {

			// Resolve the mesh filename
			auto baseName = ResolveBasename(emitter.GetSpec()->GetMeshName());
			auto skmName = baseName + ".skm";
			auto skaName = baseName + ".ska";
			
			try {
				EncodedAnimId animId(0); // This seems to be item_idle
				auto model(mModelFactory.FromFilenames(skmName, skaName, animId, animParams));

				emitter.SetRenderState(
					std::make_unique<ModelEmitterRenderState>(model)
					);
			} catch (const TempleException &e) {
				logger->error("Unable to load model {} for particle system {}: {}",
					baseName, emitter.GetSpec()->GetParent().GetName(), e.what());

				emitter.SetRenderState(std::make_unique<ModelEmitterRenderState>(nullptr));
			}
		}

		auto renderState = static_cast<ModelEmitterRenderState &>(emitter.GetRenderState());

		if (!renderState.model) {
			return; // The loader above was unable to load the model for this emitter
		}
		
		MdfRenderOverrides overrides;
		overrides.ignoreLighting = true;
		overrides.overrideDiffuse = true;
		
		while (it.HasNext()) {
			auto particleIdx = it.Next();
			auto age = emitter.GetParticleAge(particleIdx);

			overrides.overrideColor = GetParticleColor(emitter, particleIdx);
			
			// Yes, this is *actually* swapped for Y / Z
			auto& particleState = emitter.GetParticleState();
			animParams.offsetX = particleState.GetState(PSF_POS_VAR_X, particleIdx);
			animParams.offsetY = particleState.GetState(PSF_POS_VAR_Z, particleIdx);
			animParams.offsetZ = particleState.GetState(PSF_POS_VAR_Y, particleIdx);

			renderState.model->SetTime(animParams, age);

			mModelRenderer.Render(renderState.model.get(), animParams, {}, &overrides);
		}
	}

	static const char *sSearchPath[] = {
		"art\\meshes\\Particle\\",
		"art\\meshes\\Scenery\\Containers\\",
		"art\\meshes\\Scenery\\Misc\\Main Menu\\",
		"art\\meshes\\Weapons\\"
	};

	std::string ModelParticleRenderer::ResolveBasename(const std::string& modelName) {

		// A real, existing basename...
		if (vfs->FileExists(modelName + ".skm") && vfs->FileExists(modelName + ".ska")) {
			return modelName;
		}

		for (auto searchPath : sSearchPath) {
			auto baseName = std::string(searchPath) + modelName;
			if (vfs->FileExists(baseName + ".skm") && vfs->FileExists(baseName + ".ska")) {
				return baseName;
			}
		}

		// Probably invalid -> will throw
		return modelName;

	}

}
