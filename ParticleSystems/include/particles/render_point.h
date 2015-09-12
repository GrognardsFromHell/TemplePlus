
#pragma once

#include "render.h"

#include <d3d9.h>
#include <atlcomcli.h>

namespace particles {

	class PointParticleRenderer : public ParticleRenderer {
	public:
		explicit PointParticleRenderer(IDirect3DDevice9 *device);
				
		void Render(const PartSysEmitter* emitter) override;

	private:
		
		void RenderParticles(const PartSysEmitter* emitter);

		CComPtr<IDirect3DVertexBuffer9> mBuffer;
		CComPtr<IDirect3DDevice9> mDevice;
		
	};

}
