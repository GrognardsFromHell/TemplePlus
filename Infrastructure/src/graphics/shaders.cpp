
#include <unordered_map>

#include "graphics/shaders.h"
#include "graphics/device.h"
#include "infrastructure/exception.h"
#include "infrastructure/logging.h"
#include "infrastructure/vfs.h"
#include "shaders_compiler.h"

namespace gfx {

	template<typename T>
	void Shader<T>::PrintConstantBuffers()
	{
		CComPtr<ID3D11ShaderReflection> reflector;
		D3DVERIFY(D3D11Reflect(code.data(), code.size(), &reflector));

		D3D11_SHADER_DESC shaderDesc;
		D3DVERIFY(reflector->GetDesc(&shaderDesc));

		logger->info("Vertex Shader '{}' has {} constant buffers:", mName, shaderDesc.ConstantBuffers);

		for (auto i = 0u; i < shaderDesc.ConstantBuffers; i++) {
			auto cbufferDesc = reflector->GetConstantBufferByIndex(i);
			D3D11_SHADER_BUFFER_DESC bufferDesc;
			D3DVERIFY(cbufferDesc->GetDesc(&bufferDesc));

			logger->info("  Constant Buffer #{} '{}'", i, bufferDesc.Name);

			for (auto j = 0u; j < bufferDesc.Variables; j++) {
				auto variable = cbufferDesc->GetVariableByIndex(j);
				D3D11_SHADER_VARIABLE_DESC variableDesc;
				D3DVERIFY(variable->GetDesc(&variableDesc));

				logger->info("    {} @ {}", variableDesc.Name, variableDesc.StartOffset);
			}
		}
	}

	template <>
	void Shader<ID3D11PixelShader>::CreateShader() {
		mDeviceShader.Release();

		D3DVERIFY(mDevice.mD3d11Device->CreatePixelShader(&mCompiledShader[0], mCompiledShader.size(), nullptr, &mDeviceShader));
	}

	template <>
	void Shader<ID3D11PixelShader>::Bind() {
		mDevice.mContext->PSSetShader(mDeviceShader, nullptr, 0);
	}

	template <>
	void Shader<ID3D11PixelShader>::Unbind() {
		mDevice.mContext->PSSetShader(nullptr, nullptr, 0);
	}

	template <>
	void Shader<ID3D11VertexShader>::CreateShader() {
		D3DVERIFY(mDevice.mD3d11Device->CreateVertexShader(&mCompiledShader[0], mCompiledShader.size(), nullptr, &mDeviceShader));
	}

	template <>
	void Shader<ID3D11VertexShader>::Bind() {
		mDevice.mContext->VSSetShader(mDeviceShader, nullptr, 0);
	}

	template <>
	void Shader<ID3D11VertexShader>::Unbind() {
		mDevice.mContext->PSSetShader(nullptr, nullptr, 0);
	}

	template<typename T>
	struct ShaderCode {
		// The HLSL shader source code
		std::string source;

		// The compiled variants based on the defines used to compile them
		std::list<std::pair<Shaders::ShaderDefines, T>> compiledVariants;
	};

	class Shaders::Impl : public ResourceListener {
	public:
		Impl(RenderingDevice &g) : mDevice(g), mRegistration(g, this) {}

		void CreateResources(RenderingDevice&) override;
		void FreeResources(RenderingDevice&) override;

		RenderingDevice& mDevice;

		// For each shader file, we may have multiple compiled 
		// variants depending on the defines used
		template<typename T>
		using ShaderVariants = std::list<std::pair<Shaders::ShaderDefines, T>>;

		std::unordered_map<std::string, ShaderCode<VertexShaderPtr>> mVertexShaders;
		std::unordered_map<std::string, ShaderCode<PixelShaderPtr>> mPixelShaders;

		ResourceListenerRegistration mRegistration;
	};

	void Shaders::Impl::CreateResources(RenderingDevice&) {

		for (auto& pair : mVertexShaders) {
			for (auto& variant : pair.second.compiledVariants) {
				variant.second->CreateShader();
			}
		}

		for (auto& pair : mPixelShaders) {
			for (auto& variant : pair.second.compiledVariants) {
				variant.second->CreateShader();
			}
		}

	}

	void Shaders::Impl::FreeResources(RenderingDevice&) {

		for (auto& pair : mVertexShaders) {
			for (auto& variant : pair.second.compiledVariants) {
				variant.second->FreeShader();
			}
		}

		for (auto& pair : mPixelShaders) {
			for (auto& variant : pair.second.compiledVariants) {
				variant.second->FreeShader();
			}
		}

	}

	Shaders::Shaders(RenderingDevice& g) : mImpl(std::make_unique<Impl>(g)) {
	}

	Shaders::~Shaders() = default;

	VertexShaderPtr Shaders::LoadVertexShader(const std::string& name) {
		static ShaderDefines sEmptyDefines;
		return LoadVertexShader(name, sEmptyDefines);
	}

	VertexShaderPtr Shaders::LoadVertexShader(const std::string& name,
		const ShaderDefines& defines) {

		auto it = mImpl->mVertexShaders.find(name);

		if (it == mImpl->mVertexShaders.end()) {
			auto content(vfs->ReadAsString(fmt::format("shaders/{}.hlsl", name)));
			mImpl->mVertexShaders[name].source = content;
			it = mImpl->mVertexShaders.find(name);
		} else {
			// Search for a variant that matches the defines that are required
			for (auto &variant : it->second.compiledVariants) {
				if (variant.first == defines) {
					return variant.second;
				}
			}
		}

		// No variant was available for the requested defines, so 
		// we compile it now
		ShaderCompiler compiler;
		compiler.SetDefines(defines);
		compiler.SetName(name);
		compiler.SetSource(it->second.source);
		auto shader = compiler.CompileVertexShader(mImpl->mDevice);
		shader->CreateShader();

		// Insert the newly created shader into the cache
		it->second.compiledVariants.push_back({ defines, shader });

		return shader;

	}

	PixelShaderPtr Shaders::LoadPixelShader(const std::string& name) {
		static ShaderDefines sEmptyDefines;
		return LoadPixelShader(name, sEmptyDefines);
	}

	PixelShaderPtr Shaders::LoadPixelShader(const std::string& name,
		const ShaderDefines& defines) {

		auto it = mImpl->mPixelShaders.find(name);

		if (it == mImpl->mPixelShaders.end()) {
			auto content(vfs->ReadAsString(fmt::format("shaders/{}.hlsl", name)));
			mImpl->mPixelShaders[name].source = content;
			it = mImpl->mPixelShaders.find(name);
		} else {
			// Search for a variant that matches the defines that are required
			for (auto &variant : it->second.compiledVariants) {
				if (variant.first == defines) {
					return variant.second;
				}
			}
		}		

		// No variant was available for the requested defines, so 
		// we compile it now
		ShaderCompiler compiler;
		compiler.SetDefines(defines);
		compiler.SetName(name);
		compiler.SetSource(it->second.source);
		auto shader = compiler.CompilePixelShader(mImpl->mDevice);
		shader->CreateShader();

		// Insert the newly created shader into the cache
		it->second.compiledVariants.push_back({ defines, shader });

		return shader;
	}
	

}
