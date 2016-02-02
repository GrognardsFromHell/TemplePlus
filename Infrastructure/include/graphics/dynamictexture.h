
#pragma once

#include <memory>
#include <atlcomcli.h>
#include "gsl/array_view.h"

#include "textures.h"

struct IDirect3DTexture9;
struct IDirect3DSurface9;

namespace gfx {


	class DynamicTexture : public Texture {
	public:

		DynamicTexture(IDirect3DTexture9* texture, const Size &size);

		int GetId() const override;

		const std::string& GetName() const;

		const ContentRect& GetContentRect() const override {
			return mContentRect;
		}

		const Size& GetSize() const override {
			return mSize;
		}

		// Unloads the device texture (does't prevent it from being loaded again later)
		void FreeDeviceTexture() override;

		IDirect3DTexture9* GetDeviceTexture() override {
			return mTexture;
		}

		template<typename T>
		void Update(gsl::array_view<T> data) {
			UpdateRaw(
				{ reinterpret_cast<uint8_t*>(&data[0]), data.size() * sizeof(T) }, 
				mSize.width * sizeof(T)
			);
		}

	private:

		void UpdateRaw(gsl::array_view<uint8_t> data, size_t pitch);

		CComPtr<IDirect3DTexture9> mTexture;
		Size mSize;
		ContentRect mContentRect;

	};

	using DynamicTexturePtr = std::shared_ptr<DynamicTexture>;

	class RenderTargetTexture : public Texture {
	public:

		RenderTargetTexture(IDirect3DTexture9* texture, const Size &size);

		int GetId() const override;

		const std::string& GetName() const;

		const ContentRect& GetContentRect() const override {
			return mContentRect;
		}

		const Size& GetSize() const override {
			return mSize;
		}

		// Unloads the device texture (does't prevent it from being loaded again later)
		void FreeDeviceTexture() override;

		IDirect3DTexture9* GetDeviceTexture() override {
			return mTexture;
		}

		IDirect3DSurface9* GetSurface() const {
			return mSurface;
		}

	private:

		CComPtr<IDirect3DTexture9> mTexture;
		CComPtr<IDirect3DSurface9> mSurface;
		Size mSize;
		ContentRect mContentRect;

	};

	using RenderTargetTexturePtr = std::shared_ptr<RenderTargetTexture>;

}
