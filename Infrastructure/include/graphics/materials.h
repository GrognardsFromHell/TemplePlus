#pragma once

#include "infrastructure/mdfmaterial.h"
#include "graphics/device.h"
#include "graphics/textures.h"

namespace gfx {

	inline uint8_t GetD3DColorAlpha(D3DCOLOR color) {
		return (color >> 24) & 0xFF;
	}

	inline uint8_t GetD3DColorRed(D3DCOLOR color) {
		return (color >> 16) & 0xFF;
	}

	inline uint8_t GetD3DColorGreen(D3DCOLOR color) {
		return (color >> 16) & 0xFF;
	}

	inline uint8_t GetD3DColorBlue(D3DCOLOR color) {
		return (color >> 16) & 0xFF;
	}

	inline DirectX::XMFLOAT4 D3DColorToFloat4(D3DCOLOR color) {
		return{
			GetD3DColorRed(color) / 255.f,
			GetD3DColorGreen(color) / 255.f,
			GetD3DColorBlue(color) / 255.f,
			GetD3DColorAlpha(color) / 255.f
		};
	}

	struct RasterizerState {
		// How are tesellated vertices filled?
		D3DFILLMODE fillMode = D3DFILL_SOLID;

		// Indicates whether front-facing or back-facing triangles 
		// should be culled or neither. In D3D, by default, the
		// indices need to be clock wise for the front
		D3DCULL cullMode = D3DCULL_CCW;
	};

	/**
	* Desribes the state of the render target blending stage.
	*/
	struct BlendState {
		// Enables alpha blending on the output target
		bool blendEnable = false;

		// Defines how the incoming fragment color and the one in the 
		// render target should be blended together
		D3DBLEND srcBlend = D3DBLEND_ONE;
		D3DBLEND destBlend = D3DBLEND_ZERO;

		// Write mask for writing to the render target
		bool writeRed = true;
		bool writeGreen = true;
		bool writeBlue = true;
		bool writeAlpha = true;
	};

	/**
	* Describes the state of the depth stencil stage.
	*/
	struct DepthStencilState {

		// Enables depth testing
		bool depthEnable = true;

		// Enables writing to the depth buffer
		bool depthWrite = true;

		// Comparison function used for depth test
		D3DCMPFUNC depthFunc = D3DCMP_LESS;

	};

	/**
	* Describes the state of one of the texture samplers.
	*/
	struct SamplerState {

		D3DTEXTUREFILTERTYPE minFilter = D3DTEXF_LINEAR;
		D3DTEXTUREFILTERTYPE magFilter = D3DTEXF_LINEAR;
		D3DTEXTUREFILTERTYPE mipFilter = D3DTEXF_LINEAR;

		D3DTEXTUREADDRESS addressU = D3DTADDRESS_WRAP;
		D3DTEXTUREADDRESS addressV = D3DTADDRESS_WRAP;

	};

	enum class Light3dType : uint32_t {
		Point = 1,
		Spot = 2,
		Directional = 3
	};

	struct Light3d {
		Light3dType type;
		XMFLOAT4 color;
		XMFLOAT4 pos;
		XMFLOAT4 dir;
		float range;
		float phi;
	};

	class MaterialSamplerBinding {
	public:
		MaterialSamplerBinding(const gfx::TextureRef& texture,
			const SamplerState &samplerState)
			: mTexture(texture), mSamplerState(samplerState) {
		}

		const gfx::TextureRef& GetTexture() const {
			return mTexture;
		}

		const SamplerState& GetState() const {
			return mSamplerState;
		}
	private:
		const gfx::TextureRef mTexture;
		const SamplerState mSamplerState;
	};

	class Material {
	public:

		Material(const BlendState &blendState, 
			const DepthStencilState &depthStencilState,
			const RasterizerState &rasterizerState,
			const std::vector<MaterialSamplerBinding> &samplers,
			const VertexShaderPtr &vertexShader,
			const PixelShaderPtr &pixelShader) 
			: mBlendState(blendState),
			mDepthStencilState(depthStencilState),
			mRasterizerState(rasterizerState),
			mSamplers(samplers),
			mVertexShader(vertexShader),
			mPixelShader(pixelShader)
		{
		}

		const BlendState& GetBlendState() const {
			return mBlendState;
		}

		const DepthStencilState& GetDepthStencilState() const {
			return mDepthStencilState;
		}

		const RasterizerState& GetRasterizerState() const {
			return mRasterizerState;
		}

		const std::vector<MaterialSamplerBinding>& GetSamplers() const {
			return mSamplers;
		}

		const VertexShaderPtr &GetVertexShader() const {
			return mVertexShader;
		}

		const PixelShaderPtr &GetPixelShader() const {
			return mPixelShader;
		}
		
	private:
		const BlendState mBlendState;
		const DepthStencilState mDepthStencilState;
		const RasterizerState mRasterizerState;
		const std::vector<MaterialSamplerBinding> mSamplers;
		const VertexShaderPtr mVertexShader;
		const PixelShaderPtr mPixelShader;
	};
	
}
