
#pragma once

#include <atlcomcli.h>
#include <vector>
#include "graphics.h"

struct IDirect3DVertexShader9;
struct IDirect3DPixelShader9;
struct IDirect3DDevice9;

template<typename T>
class Shader {
public:
	Shader(const std::string &name, const std::vector<uint8_t> &compiledShader)
		: mName(name), mCompiledShader(compiledShader) {		
	}

	~Shader() {
	}

	void CreateShader(Graphics &g);
	void FreeShader() {
		mDeviceShader.Release();
	}

	void Bind(Graphics &g);
	void Unbind(Graphics &g);

private:
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

	Shaders(Graphics &g);
	~Shaders();

	VertexShaderPtr LoadVertexShader(const std::string &name);
	PixelShaderPtr LoadPixelShader(const std::string &name);
	
private:
	class Impl;
	std::unique_ptr<Impl> mImpl;
};
