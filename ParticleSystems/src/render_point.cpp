
#include <platform/d3d.h>
#include <infrastructure/logging.h>

#include <graphics/bufferbinding.h>

#include "render_general.h"
#include "particles/render_point.h"
#include "particles/instances.h"

namespace particles {

using namespace gfx;

// Structure used for vertices
struct PointVertex {
  float x;
  float y;
  float z;
  float size;
  D3DCOLOR diffuse;
};

/**
* The rendering state associated with a point emitter.
*/
struct PointEmitterRenderState : GeneralEmitterRenderState {
	PointEmitterRenderState(RenderingDevice &device, PartSysEmitter &emitter);

	VertexBufferPtr vertexBuffer;
	BufferBinding bufferBinding;
};

PointEmitterRenderState::PointEmitterRenderState(RenderingDevice &device,
	PartSysEmitter &emitter)
	: GeneralEmitterRenderState(device, emitter, true) {

	auto maxCount = emitter.GetSpec()->GetMaxParticles();

	vertexBuffer =
		device.CreateEmptyVertexBuffer(sizeof(PointVertex) * maxCount, true);

	bufferBinding.AddBuffer(vertexBuffer, 0, sizeof(PointVertex))
		.AddElement(VertexElementType::Float3, VertexElementSemantic::Position)
		.AddElement(VertexElementType::Float1, VertexElementSemantic::PointSize)
		.AddElement(VertexElementType::Float2, VertexElementSemantic::TexCoord);
}

PointParticleRenderer::PointParticleRenderer(RenderingDevice &device)
    : ParticleRenderer(device), mDevice(device) {}

void PointParticleRenderer::RenderParticles(PartSysEmitter &emitter) {
  auto it = emitter.NewIterator();
  auto totalCount = emitter.GetActiveCount();

  // Lazily initialize render state
  if (!emitter.HasRenderState()) {
    emitter.SetRenderState(
        std::make_unique<PointEmitterRenderState>(mDevice, emitter));
  }

  auto renderState =
      static_cast<PointEmitterRenderState &>(emitter.GetRenderState());

  auto lock(renderState.vertexBuffer->Lock<PointVertex>(totalCount));
  int i = 0;
  while (it.HasNext()) {
    auto particleIdx = it.Next();
    FillPointVertex(emitter, particleIdx, lock[i]);
    i++;
  }
  lock.Unlock();

  mDevice.SetMaterial(renderState.material);
  renderState.bufferBinding.Bind();

  // Draw the batch
  auto device = mDevice.GetDevice();
  auto result = device->DrawPrimitive(D3DPT_POINTLIST, 0, totalCount);
  if (!SUCCEEDED(result)) {
    logger->error("Unable to draw the point particles.");
    return;
  }
}

void PointParticleRenderer::Render(PartSysEmitter &emitter) {

  XMFLOAT4X4 worldMatrix;
  if (!GetEmitterWorldMatrix(emitter, worldMatrix)) {
    return;
  }

  mDevice.GetDevice()->SetVertexShaderConstantF(0, &worldMatrix._11, 4);

  EnablePointStates();

  RenderParticles(emitter);

  DisablePointStates();
}

void PointParticleRenderer::FillPointVertex(const PartSysEmitter &emitter,
                                            int particleIdx,
                                            PointVertex &vertex) {

  vertex.diffuse = GetParticleColor(emitter, particleIdx);
  vertex.x = emitter.GetParticleState().GetState(PSF_POS_VAR_X, particleIdx);
  vertex.y = emitter.GetParticleState().GetState(PSF_POS_VAR_Y, particleIdx);
  vertex.z = emitter.GetParticleState().GetState(PSF_POS_VAR_Z, particleIdx);

  auto scaleParam = emitter.GetParamState(part_scale_X);
  if (scaleParam) {
    vertex.size = scaleParam->GetValue(&emitter, particleIdx,
                                       emitter.GetParticleAge(particleIdx));
  } else {
    vertex.size = 1.0f;
  }
}

void PointParticleRenderer::EnablePointStates() {
  auto device = mDevice.GetDevice();
  device->SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE);
  device->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
  device->SetRenderState(D3DRS_POINTSIZE, CoerceToInteger(30.08f));
  device->SetRenderState(D3DRS_POINTSIZE_MIN, CoerceToInteger(10.0f));
  device->SetRenderState(D3DRS_POINTSIZE_MAX, CoerceToInteger(64.0f));
  device->SetRenderState(D3DRS_POINTSCALE_A, CoerceToInteger(10.0f));
  device->SetRenderState(D3DRS_POINTSCALE_B, CoerceToInteger(10.0f));
  device->SetRenderState(D3DRS_POINTSCALE_C, CoerceToInteger(10.0f));
}

void PointParticleRenderer::DisablePointStates() {
  auto device = mDevice.GetDevice();
  device->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
  device->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
}

}
