#include "stdafx.h"
#include "shaders.h"
#include "graphics.h"
#include "../shader/shadercode.h"

template <>
void Shader<IDirect3DPixelShader9>::CreateShader(Graphics& g) {
	mDeviceShader.Release();

	auto device = g.device();

	if (D3DLOG(device->CreatePixelShader((DWORD*)&mCompiledShader[0], &mDeviceShader)) != D3D_OK) {
		throw TempleException("Unable to create pixel shader.");
	}
}

template <>
void Shader<IDirect3DPixelShader9>::Bind(Graphics& g) {
	auto device = g.device();
	D3DLOG(device->SetPixelShader(mDeviceShader));
}

template <>
void Shader<IDirect3DPixelShader9>::Unbind(Graphics& g) {
	auto device = g.device();
	D3DLOG(device->SetPixelShader(nullptr));
}

template <>
void Shader<IDirect3DVertexShader9>::CreateShader(Graphics& g) {
	mDeviceShader.Release();

	auto device = g.device();

	if (D3DLOG(device->CreateVertexShader((DWORD*)&mCompiledShader[0], &mDeviceShader)) != D3D_OK) {
		throw TempleException("Unable to create vertex shader.");
	}
}

template <>
void Shader<IDirect3DVertexShader9>::Bind(Graphics& g) {
	auto device = g.device();
	D3DLOG(device->SetVertexShader(mDeviceShader));
}

template <>
void Shader<IDirect3DVertexShader9>::Unbind(Graphics& g) {
	auto device = g.device();
	D3DLOG(device->SetVertexShader(nullptr));
}

class Shaders::Impl : public ResourceListener {
public:
	explicit Impl(Graphics &g) : mGraphics(g), mRegistration(g, this) {}

	void CreateResources(Graphics&) override;
	void FreeResources(Graphics&) override;

	Graphics& mGraphics;

	std::unordered_map<std::string, VertexShaderPtr> mVertexShaders;
	std::unordered_map<std::string, PixelShaderPtr> mPixelShaders;

	ResourceListenerRegistration mRegistration;
};

void Shaders::Impl::CreateResources(Graphics&) {

	for (auto& pair : mVertexShaders) {
		pair.second->CreateShader(mGraphics);
	}

	for (auto& pair : mPixelShaders) {
		pair.second->CreateShader(mGraphics);
	}

}

void Shaders::Impl::FreeResources(Graphics&) {

	for (auto& pair : mVertexShaders) {
		pair.second->FreeShader();
	}

	for (auto& pair : mPixelShaders) {
		pair.second->FreeShader();
	}

}

Shaders::Shaders(Graphics& g) : mImpl(std::make_unique<Impl>(g)) {
	std::vector<uint8_t> vsCode(clipping_vs, clipping_vs + clipping_vs_size);
	mImpl->mVertexShaders["clipping_vs"] = std::make_shared<VertexShader>("clipping_vs", vsCode);

	std::vector<uint8_t> psCode(clipping_ps, clipping_ps + clipping_ps_size);
	mImpl->mPixelShaders["clipping_ps"] = std::make_shared<PixelShader>("clipping_ps", psCode);

	mImpl->CreateResources(g);
}

Shaders::~Shaders() = default;

VertexShaderPtr Shaders::LoadVertexShader(const std::string& name) {

	auto it = mImpl->mVertexShaders.find(name);

	if (it == mImpl->mVertexShaders.end()) {
		throw TempleException("Unknown vertex shader: {}", name);
	}

	return it->second;

}

PixelShaderPtr Shaders::LoadPixelShader(const std::string& name) {
	
	auto it = mImpl->mPixelShaders.find(name);

	if (it == mImpl->mPixelShaders.end()) {
		throw TempleException("Unknown pixel shader: {}", name);
	}

	return it->second;

}
