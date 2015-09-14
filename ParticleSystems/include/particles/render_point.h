
#pragma once

#include "render.h"

#include <d3d9.h>
#include <atlcomcli.h>

namespace particles {
	struct PointVertex;
	struct SpriteVertex;

	class PointParticleRenderer : public ParticleRenderer {
	public:
		explicit PointParticleRenderer(IDirect3DDevice9 *device);
				
		void Render(const PartSysEmitter* emitter) override;

	private:

		void FillPointVertex(const PartSysEmitter* emitter, int particleIdx, PointVertex* vertex);
		void RenderParticles(const PartSysEmitter* emitter);

		CComPtr<IDirect3DVertexBuffer9> mBuffer;
		CComPtr<IDirect3DDevice9> mDevice;
		
	};

	class SpriteParticleRenderer : public ParticleRenderer {
	public:
		explicit SpriteParticleRenderer(IDirect3DDevice9 *device);

		void Render(const PartSysEmitter* emitter) override;

	private:

		void FillSpriteVertex(const PartSysEmitter* emitter, int particleIdx, SpriteVertex* vertex);
		void RenderParticles(const PartSysEmitter* emitter);

		CComPtr<IDirect3DVertexBuffer9> mBuffer;
		CComPtr<IDirect3DIndexBuffer9> mIndices;
		CComPtr<IDirect3DDevice9> mDevice;

	};

}
