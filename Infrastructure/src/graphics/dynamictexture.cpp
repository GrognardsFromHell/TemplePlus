
#include "graphics/dynamictexture.h"
#include "graphics/device.h"
#include "infrastructure/exception.h"
#include "platform/d3d.h"

namespace gfx {

	DynamicTexture::DynamicTexture(ID3D11DeviceContext *context, 
		ID3D11Texture2D *texture, 
		ID3D11ShaderResourceView *resourceView,
		const Size &size,
		uint32_t bytesPerPixel) 
		: mTexture(texture), mContext(context), mResourceView(resourceView), mSize(size), mBytesPerPixel(bytesPerPixel) {
		mContentRect = { 0, 0, size.width, size.height };
	}

	int DynamicTexture::GetId() const {
		throw TempleException("Unsupported operation for dynamic textures.");
	}

	const std::string & gfx::DynamicTexture::GetName() const
	{
		static std::string sDynamicName("<dynamic>");
		return sDynamicName;
	}

	void DynamicTexture::FreeDeviceTexture()
	{
		mTexture.Release();
		mResourceView.Release();
	}

	void DynamicTexture::UpdateRaw(gsl::span<uint8_t> data, size_t pitch)
	{
		auto mapped = renderingDevice->Map(*this);

		if (mapped.GetRowPitch() == pitch) { 
			memcpy(mapped.GetData(), &data[0], data.size());
		} else {
			auto dest = (uint8_t*)mapped.GetData();
			auto src = data.data();
			for (size_t y = 0; y < (size_t)mSize.height; ++y) {
				memcpy(dest, src, pitch);
				dest += mapped.GetRowPitch();
				src += pitch;
			}
		}
	}

	RenderTargetTexture::RenderTargetTexture(ID3D11Texture2D *texture,
		ID3D11RenderTargetView *rtView,
		ID3D11Texture2D *resolvedTexture,
		ID3D11ShaderResourceView *resourceView,
		const Size &size,
		bool multiSampled,
		bool shareable) : mTexture(texture), mRtView(rtView), mResolvedTexture(resolvedTexture), 
			mResourceView(resourceView), mSize(size), mMultiSampled(multiSampled), mShareable(shareable) {
		mContentRect = { 0, 0, size.width, size.height };
	}

	int RenderTargetTexture::GetId() const {
		throw TempleException("Unsupported operation for render target textures.");
	}

	const std::string & gfx::RenderTargetTexture::GetName() const
	{
		static std::string sDynamicName("<rt>");
		return sDynamicName;
	}

	void RenderTargetTexture::FreeDeviceTexture()
	{
		mTexture.Release();
		mRtView.Release();
		mResourceView.Release();
	}

	void * RenderTargetTexture::GetShareHandle()
	{
		CComPtr<IDXGIResource> dxgiResource;
		if (!SUCCEEDED(mTexture.QueryInterface(&dxgiResource))) {
			throw TempleException("Unable to retrieve DXGI resource underlying texture.");
		}

		HANDLE sharedHandle;
		if (!SUCCEEDED(dxgiResource->GetSharedHandle(&sharedHandle))) {
			throw TempleException("Unable to retrieve shared handle from underlying resource.");
		}

		return sharedHandle;
	}

	gfx::BufferFormat RenderTargetTexture::GetFormat() const
	{
		D3D11_TEXTURE2D_DESC desc;
		mTexture->GetDesc(&desc);
		switch (desc.Format) {
		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return gfx::BufferFormat::A8R8G8B8;
		case DXGI_FORMAT_B8G8R8X8_UNORM:
			return gfx::BufferFormat::X8R8G8B8;
		default:
			throw TempleException("Unsupported buffer format.");
		}
	}

	RenderTargetDepthStencil::RenderTargetDepthStencil(ID3D11Texture2D *textureNew,
		ID3D11DepthStencilView *dsView,
		const Size &size) : mTextureNew(textureNew), mDsView(dsView), mSize(size) {
	}

}
