
#include <graphics/device.h>
#include <graphics/textures.h>

#include "render_general.h"

namespace particles {
using namespace gfx;

GeneralEmitterRenderState::GeneralEmitterRenderState(RenderingDevice &device,
                                                     PartSysEmitter &emitter,
                                                     bool pointSprites)
    : material(CreateMaterial(device, emitter, pointSprites)) {}

Material GeneralEmitterRenderState::CreateMaterial(RenderingDevice &device,
                                                   PartSysEmitter &emitter,
                                                   bool pointSprites) {
  BlendState blendState;
  blendState.blendEnable = true;

  switch (emitter.GetSpec()->GetBlendMode()) {
  case PartSysBlendMode::Add:
    blendState.srcBlend = D3DBLEND_SRCALPHA;
    blendState.destBlend = D3DBLEND_ONE;
    break;
  case PartSysBlendMode::Subtract:
    blendState.srcBlend = D3DBLEND_ZERO;
    blendState.destBlend = D3DBLEND_INVSRCALPHA;
    break;
  case PartSysBlendMode::Blend:
    blendState.srcBlend = D3DBLEND_SRCALPHA;
    blendState.destBlend = D3DBLEND_INVSRCALPHA;
    break;
  case PartSysBlendMode::Multiply:
    blendState.srcBlend = D3DBLEND_ZERO;
    blendState.destBlend = D3DBLEND_SRCCOLOR;
    break;
  default:
    break;
  }

  // Particles respect the depth buffer, but do not modify it
  DepthStencilState depthStencilState;
  depthStencilState.depthEnable = true;
  depthStencilState.depthWrite = false;
  RasterizerState rasterizerState;
  PixelShaderPtr pixelShader;
  std::vector<MaterialSamplerBinding> samplers;

  auto shaderName("diffuse_only_ps");
  auto& textureName(emitter.GetSpec()->GetTextureName());  
  if (!textureName.empty()) {
	auto texture(device.GetTextures().Resolve(textureName, true));
	samplers.push_back({ texture, {} });
	shaderName = "textured_simple_ps";
  }
  pixelShader = device.GetShaders().LoadPixelShader(shaderName);
  
  auto vsName = pointSprites ? "particles_points_vs" : "particles_quads_vs";

  auto vertexShader(device.GetShaders().LoadVertexShader(vsName));

  return Material(blendState, depthStencilState, rasterizerState, samplers,
                  vertexShader, pixelShader);
}

static uint8_t GetParticleColorComponent(ParticleStateField stateField,
                                         PartSysParamId paramId,
                                         const PartSysEmitter &emitter,
                                         int particleIdx) {
  auto colorParam = emitter.GetParamState(paramId);
  uint8_t value;
  if (colorParam) {
    auto partAge = emitter.GetParticleAge(particleIdx);
    auto partColor = colorParam->GetValue(&emitter, particleIdx, partAge);
    partColor += emitter.GetParticleState().GetState(stateField, particleIdx);
    if (partColor >= 255) {
      value = 255;
    } else if (partColor < 0) {
      value = 0;
    } else {
      value = (uint8_t)partColor;
    }
  } else {
    value =
        (uint8_t)emitter.GetParticleState().GetState(stateField, particleIdx);
  }
  return value;
}

D3DCOLOR GetParticleColor(const PartSysEmitter &emitter, int particleIdx) {
  auto red = GetParticleColorComponent(PSF_RED, part_red, emitter, particleIdx);
  auto green =
      GetParticleColorComponent(PSF_GREEN, part_green, emitter, particleIdx);
  auto blue =
      GetParticleColorComponent(PSF_BLUE, part_blue, emitter, particleIdx);
  auto alpha =
      GetParticleColorComponent(PSF_ALPHA, part_alpha, emitter, particleIdx);

  return D3DCOLOR_ARGB(alpha, red, green, blue);
}
}
