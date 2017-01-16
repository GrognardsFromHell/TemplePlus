#pragma once

#include "infrastructure/mdfmaterial.h"
#include "graphics/device.h"
#include "graphics/textures.h"

struct ID3D11RasterizerState;

namespace gfx {

	enum class CullMode {
		None,
		Back,
		Front
	};

	struct RasterizerSpec {
		// How are tesellated vertices filled? Default is solid.
		bool wireframe = false;

		// Indicates whether front-facing or back-facing triangles 
		// should be culled or neither. In D3D, by default, the
		// indices need to be clock wise for the front
		CullMode cullMode = CullMode::Back;

		// Enable or disable scissor culling
		bool scissor = true;

		bool operator <(const RasterizerSpec &o) const {
			return std::tie(wireframe, cullMode, scissor) < std::tie(o.wireframe, o.cullMode, scissor);
		}

	};
		
	enum class BlendOperand {
		Zero,
		One,
		SrcColor,
		InvSrcColor,
		SrcAlpha,
		InvSrcAlpha,
		DestAlpha,
		InvDestAlpha,
		DestColor,
		InvDestColor
	};

	/**
	* Desribes the state of the render target blending stage.
	*/
	struct BlendSpec {
		// Enables alpha blending on the output target
		bool blendEnable = false;

		// Defines how the incoming fragment color and the one in the 
		// render target should be blended together
		BlendOperand srcBlend = BlendOperand::One;
		BlendOperand destBlend = BlendOperand::Zero;

		// By default use the destination alpha and keep it
		BlendOperand srcAlphaBlend = BlendOperand::Zero;
		BlendOperand destAlphaBlend = BlendOperand::One;

		// Write mask for writing to the render target
		bool writeRed = true;
		bool writeGreen = true;
		bool writeBlue = true;
		bool writeAlpha = true;

		bool operator <(const BlendSpec &o) const {
			return std::tie(blendEnable, srcBlend, destBlend, writeRed, writeGreen, writeBlue, writeAlpha)
				< std::tie(o.blendEnable, o.srcBlend, o.destBlend, o.writeRed, o.writeGreen, o.writeBlue, o.writeAlpha);
		}

	};

	enum class ComparisonFunc {
		Never,
		Less,
		Equal,
		LessEqual,
		Greater,
		NotEqual,
		GreaterEqual,
		Always
	};
	
	/**
	* Describes the state of the depth stencil stage.
	*/
	struct DepthStencilSpec {

		// Enables depth testing
		bool depthEnable = true;

		// Enables writing to the depth buffer
		bool depthWrite = true;

		// Comparison function used for depth test
		ComparisonFunc depthFunc = ComparisonFunc::Less;

		bool operator <(const DepthStencilSpec &o) const {
			return std::tie(depthEnable, depthWrite, depthFunc)
				< std::tie(o.depthEnable, o.depthWrite, o.depthFunc);
		}

	};

	enum class TextureFilterType {
		NearestNeighbor,
		Linear
	};

	enum class TextureAddress {
		Wrap,
		Clamp
	};

	/**
	* Describes the state of one of the texture samplers.
	*/
	struct SamplerSpec {

		TextureFilterType minFilter = TextureFilterType::Linear;
		TextureFilterType magFilter = TextureFilterType::Linear;
		TextureFilterType mipFilter = TextureFilterType::Linear;

		TextureAddress addressU = TextureAddress::Wrap;
		TextureAddress addressV = TextureAddress::Wrap;

		bool operator <(const SamplerSpec &o) const {
			return std::tie(minFilter, magFilter, mipFilter, addressU, addressV)
				< std::tie(o.minFilter, o.magFilter, o.mipFilter, o.addressU, o.addressV);
		}

	};

	// The GPU resource for the state associated with the rasterizer
	class RasterizerState {
		friend class RenderingDevice;
	public:
		RasterizerState(const RasterizerSpec &spec, ID3D11RasterizerState *gpuState)
			: mSpec(spec), mGpuState(gpuState) {}

		const RasterizerSpec &GetSpec() const {
			return mSpec;
		}

		NO_COPY_OR_MOVE(RasterizerState);
	private:
		const RasterizerSpec mSpec;
		CComPtr<ID3D11RasterizerState> mGpuState;
	};

	class BlendState {
		friend class RenderingDevice;
	public:
		BlendState(const BlendSpec &spec, ID3D11BlendState *gpuState)
			: mSpec(spec), mGpuState(gpuState) {}

		const BlendSpec &GetSpec() const {
			return mSpec;
		}

		NO_COPY_OR_MOVE(BlendState);
	private:
		const BlendSpec mSpec;
		CComPtr<ID3D11BlendState> mGpuState;
	};

	class DepthStencilState {
		friend class RenderingDevice;
	public:
		DepthStencilState(const DepthStencilSpec &spec, ID3D11DepthStencilState *gpuState)
			: mSpec(spec), mGpuState(gpuState) {}

		const DepthStencilSpec &GetSpec() const {
			return mSpec;
		}

		NO_COPY_OR_MOVE(DepthStencilState);
	private:
		const DepthStencilSpec mSpec;
		CComPtr<ID3D11DepthStencilState> mGpuState;
	};

	class SamplerState {
		friend class RenderingDevice;
	public:
		SamplerState(const SamplerSpec &spec, ID3D11SamplerState *gpuState)
			: mSpec(spec), mGpuState(gpuState) {}

		const SamplerSpec &GetSpec() const {
			return mSpec;
		}

		NO_COPY_OR_MOVE(SamplerState);
	private:
		const SamplerSpec mSpec;
		CComPtr<ID3D11SamplerState> mGpuState;
	};

	enum class Light3dType : uint32_t {
		Point = 1,
		Spot = 2,
		Directional = 3
	};

	struct Light3d {
		Light3d() : ambient(0, 0, 0, 0) {
		}

		Light3dType type;
		XMFLOAT4 ambient;
		XMFLOAT4 color;
		XMFLOAT4 pos;
		XMFLOAT4 dir;
		float range;
		float phi;
	};

	struct MaterialSamplerSpec {
		gfx::TextureRef texture;
		SamplerSpec samplerSpec;
	};

	class MaterialSamplerBinding {
	public:
		MaterialSamplerBinding(const gfx::TextureRef& texture,
			const SamplerStatePtr &samplerState)
			: mTexture(texture), mSamplerState(samplerState) {
		}

		const gfx::TextureRef& GetTexture() const {
			return mTexture;
		}

		const SamplerState& GetState() const {
			return *mSamplerState;
		}
	private:
		const gfx::TextureRef mTexture;
		const SamplerStatePtr mSamplerState;
	};

	class Material {
	public:

		Material(const BlendStatePtr &blendState, 
			const DepthStencilStatePtr &depthStencilState,
			const RasterizerStatePtr &rasterizerState,
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
			return *mBlendState;
		}

		const DepthStencilState& GetDepthStencilState() const {
			return *mDepthStencilState;
		}

		const RasterizerState& GetRasterizerState() const {
			return *mRasterizerState;
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
		const BlendStatePtr mBlendState;
		const DepthStencilStatePtr mDepthStencilState;
		const RasterizerStatePtr mRasterizerState;
		const std::vector<MaterialSamplerBinding> mSamplers;
		const VertexShaderPtr mVertexShader;
		const PixelShaderPtr mPixelShader;
	};
	
}
