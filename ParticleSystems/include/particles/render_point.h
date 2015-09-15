#pragma once

#include "render.h"

#include <d3d9.h>
#include <atlcomcli.h>

namespace particles {
	struct PointVertex;
	struct SpriteVertex;

	class PointParticleRenderer : public ParticleRenderer {
	public:
		explicit PointParticleRenderer(IDirect3DDevice9* device);

		void Render(const PartSysEmitter* emitter) override;

	private:

		void FillPointVertex(const PartSysEmitter* emitter, int particleIdx, PointVertex* vertex);
		void RenderParticles(const PartSysEmitter* emitter);

		CComPtr<IDirect3DVertexBuffer9> mBuffer;
		CComPtr<IDirect3DDevice9> mDevice;

	};

	class QuadParticleRenderer : public ParticleRenderer {
	public:
		explicit QuadParticleRenderer(IDirect3DDevice9* device);

		void Render(const PartSysEmitter* emitter) override;

	protected:
		CComPtr<IDirect3DVertexBuffer9> mBuffer;
		CComPtr<IDirect3DIndexBuffer9> mIndices;
		CComPtr<IDirect3DDevice9> mDevice;

	private:
		void RenderParticles(const PartSysEmitter* emitter);

		virtual void FillVertex(const PartSysEmitter* emitter, int particleIdx, SpriteVertex* vertex) = 0;

	};

	class SpriteParticleRenderer : public QuadParticleRenderer {
	public:
		explicit SpriteParticleRenderer(IDirect3DDevice9* device)
			: QuadParticleRenderer(device) {
		}

	protected:

		void FillVertex(const PartSysEmitter* emitter, int particleIdx, SpriteVertex* vertex) override;

	};

	class DiscParticleRenderer : public QuadParticleRenderer {
	public:
		explicit DiscParticleRenderer(IDirect3DDevice9* device)
			: QuadParticleRenderer(device) {
		}

	protected:

		void FillVertex(const PartSysEmitter* emitter, int particleIdx, SpriteVertex* vertex) override;

	};

}
