
#pragma once

#include <graphics/materials.h>

#include "particles/instances.h"

namespace gfx {
	class RenderingDevice;
}

namespace particles {
	
	struct GeneralEmitterRenderState : public PartSysEmitterRenderState {
		GeneralEmitterRenderState(gfx::RenderingDevice &device, PartSysEmitter &emitter, bool pointSprites);

		static gfx::Material CreateMaterial(gfx::RenderingDevice &device,
			PartSysEmitter &emitter,
			bool pointSprites);

		gfx::Material material;
	};

	inline static DWORD CoerceToInteger(float value) { return *(DWORD *)&value; }
	
	D3DCOLOR GetParticleColor(const PartSysEmitter &emitter, int particleIdx);

}
