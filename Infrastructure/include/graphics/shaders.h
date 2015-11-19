
#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>

#include "../platform/d3d.h"

namespace gfx {

	class RenderingDevice;

	template<typename T>
	class Shader {
	public:
		Shader(RenderingDevice &device, const std::string &name, const std::vector<uint8_t> &compiledShader)
			: mDevice(device), mName(name), mCompiledShader(compiledShader) {
		}

		~Shader() {
		}

		void CreateShader();
		void FreeShader() {
			mDeviceShader.Release();
		}

		void Bind();
		void Unbind();

	private:
		RenderingDevice &mDevice;
		CComPtr<T> mDeviceShader;
		std::string mName;
		std::vector<uint8_t> mCompiledShader;
	};

	using VertexShader = Shader<IDirect3DVertexShader9>;
	using VertexShaderPtr = std::shared_ptr<VertexShader>;
	using PixelShader = Shader<IDirect3DPixelShader9>;
	using PixelShaderPtr = std::shared_ptr<PixelShader>;

	/**
	 * Manages built-in shaders.
	 */
	class Shaders {
	public:
		Shaders(RenderingDevice &device);
		~Shaders();

		using ShaderDefines = std::map<std::string, std::string>;

		VertexShaderPtr LoadVertexShader(const std::string &name,
			const ShaderDefines& defines);
		VertexShaderPtr LoadVertexShader(const std::string &name);
		PixelShaderPtr LoadPixelShader(const std::string &name,
			const ShaderDefines& defines);
		PixelShaderPtr LoadPixelShader(const std::string &name);

	private:
		class Impl;
		std::unique_ptr<Impl> mImpl;
	};

}
