
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
  BlendSpec blendState;
  blendState.blendEnable = true;

  switch (emitter.GetSpec()->GetBlendMode()) {
  case PartSysBlendMode::Add:
    blendState.srcBlend = BlendOperand::SrcAlpha;
    blendState.destBlend = BlendOperand::One;
    break;
  case PartSysBlendMode::Subtract:
    blendState.srcBlend = BlendOperand::Zero;
    blendState.destBlend = BlendOperand::InvSrcAlpha;
    break;
  case PartSysBlendMode::Blend:
    blendState.srcBlend = BlendOperand::SrcAlpha;
    blendState.destBlend = BlendOperand::InvSrcAlpha;
    break;
  case PartSysBlendMode::Multiply:
    blendState.srcBlend = BlendOperand::Zero;
    blendState.destBlend = BlendOperand::SrcColor;
    break;
  default:
    break;
  }

  // Particles respect the depth buffer, but do not modify it
  DepthStencilSpec depthStencilState;
  depthStencilState.depthEnable = true;
  depthStencilState.depthWrite = false;
  RasterizerSpec rasterizerState;
  rasterizerState.cullMode = CullMode::None;
  PixelShaderPtr pixelShader;
  std::vector<MaterialSamplerSpec> samplers;

  auto shaderName("diffuse_only_ps");
  auto& textureName(emitter.GetSpec()->GetTextureName());  
  if (!textureName.empty()) {
	SamplerSpec samplerState;
	samplerState.addressU = TextureAddress::Clamp;
	samplerState.addressV = TextureAddress::Clamp;
	samplerState.minFilter = TextureFilterType::Linear;
	samplerState.magFilter = TextureFilterType::Linear;
	samplerState.mipFilter = TextureFilterType::Linear;

	auto texture(device.GetTextures().Resolve(textureName, true));
	samplers.push_back({ texture, samplerState });
	shaderName = "textured_simple_ps";
  }
  pixelShader = device.GetShaders().LoadPixelShader(shaderName);
  
  auto vsName = pointSprites ? "particles_points_vs" : "particles_quads_vs";

  auto vertexShader(device.GetShaders().LoadVertexShader(vsName));

  return device.CreateMaterial(blendState, depthStencilState, rasterizerState, samplers,
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

XMCOLOR GetParticleColor(const PartSysEmitter &emitter, int particleIdx) {
  auto red = GetParticleColorComponent(PSF_RED, part_red, emitter, particleIdx);
  auto green =
      GetParticleColorComponent(PSF_GREEN, part_green, emitter, particleIdx);
  auto blue =
      GetParticleColorComponent(PSF_BLUE, part_blue, emitter, particleIdx);
  auto alpha =
      GetParticleColorComponent(PSF_ALPHA, part_alpha, emitter, particleIdx);

  return XMCOLOR_ARGB(alpha, red, green, blue);
}
}
