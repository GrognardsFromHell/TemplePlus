
#include <unordered_map>

#include "graphics/shaders.h"
#include "graphics/device.h"
#include "infrastructure/exception.h"
#include "infrastructure/logging.h"
#include "infrastructure/vfs.h"
#include "shaders_compiler.h"

namespace gfx {
	
	template <>
	void Shader<IDirect3DPixelShader9>::CreateShader() {
		mDeviceShader.Release();

		auto device = mDevice.GetDevice();

		if (D3DLOG(device->CreatePixelShader((DWORD*)&mCompiledShader[0], &mDeviceShader)) != D3D_OK) {
			throw TempleException("Unable to create pixel shader.");
		}
	}

	template <>
	void Shader<IDirect3DPixelShader9>::Bind() {
		auto device = mDevice.GetDevice();
		D3DLOG(device->SetPixelShader(mDeviceShader));
	}

	template <>
	void Shader<IDirect3DPixelShader9>::Unbind() {
		auto device = mDevice.GetDevice();
		D3DLOG(device->SetPixelShader(nullptr));
	}

	template <>
	void Shader<IDirect3DVertexShader9>::CreateShader() {
		mDeviceShader.Release();

		auto device = mDevice.GetDevice();

		if (D3DLOG(device->CreateVertexShader((DWORD*)&mCompiledShader[0], &mDeviceShader)) != D3D_OK) {
			throw TempleException("Unable to create vertex shader.");
		}
	}

	template <>
	void Shader<IDirect3DVertexShader9>::Bind() {
		auto device = mDevice.GetDevice();
		D3DLOG(device->SetVertexShader(mDeviceShader));
	}

	template <>
	void Shader<IDirect3DVertexShader9>::Unbind() {
		auto device = mDevice.GetDevice();
		D3DLOG(device->SetVertexShader(nullptr));
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
