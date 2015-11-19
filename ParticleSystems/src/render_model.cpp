
#include "particles/render_point.h"
#include "render_general.h"

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

		XMFLOAT4X4 worldMatrix;
		if (!GetEmitterWorldMatrix(emitter, worldMatrix)) {
			return;
		}

		mDevice.GetDevice()->SetVertexShaderConstantF(0, &worldMatrix._11, 4);

		auto it = emitter.NewIterator();
		auto totalCount = emitter.GetActiveCount();

		AnimatedModelParams animParams;

		// Lazily initialize render state
		if (!emitter.HasRenderState()) {

			// Resolve the mesh filename
			std::string skmName(fmt::format("art/meshes/particle/{}.skm", 
				emitter.GetSpec()->GetMeshName()));
			std::string skaName(fmt::format("art/meshes/particle/{}.ska",
				emitter.GetSpec()->GetMeshName()));
			
			EncodedAnimId animId(0); // This seems to be item_idle
			auto model(mModelFactory.FromFilenames(skmName, skaName, animId, animParams));

			emitter.SetRenderState(
				std::make_unique<ModelEmitterRenderState>(model)
			);
		}

		auto renderState =
			static_cast<ModelEmitterRenderState &>(emitter.GetRenderState());
		
		MdfRenderOverrides overrides;
		overrides.ignoreLighting = true;
		overrides.overrideDiffuse = true;
		
		while (it.HasNext()) {
			auto particleIdx = it.Next();
			auto age = emitter.GetParticleAge(particleIdx);

			overrides.overrideColor = GetParticleColor(emitter, particleIdx);
			
			auto& particleState = emitter.GetParticleState();
			animParams.offsetX = particleState.GetState(PSF_POS_VAR_X, particleIdx);
			animParams.offsetY = particleState.GetState(PSF_POS_VAR_Y, particleIdx);
			animParams.offsetZ = particleState.GetState(PSF_POS_VAR_Z, particleIdx);

			renderState.model->SetTime(animParams, age);

			mModelRenderer.Render(renderState.model.get(), animParams, {}, &overrides);
		}
	}

}
