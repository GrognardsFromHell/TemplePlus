
#include "graphics/dynamictexture.h"
#include "infrastructure/exception.h"
#include "platform/d3d.h"

namespace gfx {

	DynamicTexture::DynamicTexture(IDirect3DTexture9* texture, const Size &size) : mTexture(texture), mSize(size) {
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
	}

	void DynamicTexture::UpdateRaw(gsl::array_view<uint8_t> data)
	{
		D3DLOCKED_RECT locked;
		mTexture->LockRect(0, &locked, nullptr, D3DLOCK_DISCARD);
		memcpy(locked.pBits, &data[0], data.size());
		mTexture->UnlockRect(0);
	}

	RenderTargetTexture::RenderTargetTexture(IDirect3DTexture9* texture, const Size &size) : mTexture(texture), mSize(size) {
		mContentRect = { 0, 0, size.width, size.height };
		D3DLOG(mTexture->GetSurfaceLevel(0, &mSurface));
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
		mSurface.Release();
		mTexture.Release();
	}

}
