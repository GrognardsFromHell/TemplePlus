
#pragma once

#include <gsl/gsl>

#include <memory>
#include <atlcomcli.h>

#include "textures.h"

struct ID3D11Texture2D;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;

namespace gfx {

	enum class BufferFormat;

	class DynamicTexture : public Texture {
		friend class RenderingDevice;
	public:
		DynamicTexture(ID3D11DeviceContext *context,
			ID3D11Texture2D *texture,
			ID3D11ShaderResourceView *resourceView,
			const Size &size,
			uint32_t bytesPerPixel);

		int GetId() const override;

		const std::string& GetName() const;

		const ContentRect& GetContentRect() const override {
			return mContentRect;
		}

		const Size& GetSize() const override {
			return mSize;
		}

		TextureType GetType() const override {
			return TextureType::Dynamic;
		}

		// Unloads the device texture (does't prevent it from being loaded again later)
		void FreeDeviceTexture() override;

		ID3D11ShaderResourceView* GetResourceView() override {
			return mResourceView;
		}

		uint32_t GetBytesPerPixel() const {
			return mBytesPerPixel;
		}

		template<typename T>
		void Update(gsl::span<T> data) {
			UpdateRaw(
				gsl::span(reinterpret_cast<uint8_t*>(&data[0]), data.size_bytes()), 
				mSize.width * sizeof(T)
			);
		}

	private:

		void UpdateRaw(gsl::span<uint8_t> data, size_t pitch);

		CComPtr<ID3D11DeviceContext> mContext;
		CComPtr<ID3D11Texture2D> mTexture;
		CComPtr<ID3D11ShaderResourceView> mResourceView;
		Size mSize;
		size_t mBytesPerPixel;
		ContentRect mContentRect;

	};

	using DynamicTexturePtr = std::shared_ptr<DynamicTexture>;

	class RenderTargetTexture : public Texture {
	friend class RenderingDevice;
	public:

		RenderTargetTexture(ID3D11Texture2D *texture,
			ID3D11RenderTargetView *rtView,
			ID3D11Texture2D *resolvedTexture,
			ID3D11ShaderResourceView *resourceView,
			const Size &size,
			bool multiSampled);

		int GetId() const override;

		const std::string& GetName() const;

		const ContentRect& GetContentRect() const override {
			return mContentRect;
		}

		const Size& GetSize() const override {
			return mSize;
		}

		TextureType GetType() const override {
			return TextureType::RenderTarget;
		}

		// Unloads the device texture (does't prevent it from being loaded again later)
		void FreeDeviceTexture() override;

		/**
		 * Only valid for MSAA targets. Will return the texture used to resolve the multisampling
		 * so it can be used as a shader resource.
		 */
		ID3D11Texture2D* GetResolvedTexture() const {
			return mResolvedTexture;
		}

		/**
		 * For MSAA targets, this will return the resolvedTexture, while for normal targets,
		 * this will just be the texture itself.
		 */
		ID3D11ShaderResourceView* GetResourceView() override {
			return mResourceView;
		}
		
		bool IsMultiSampled() const {
			return mMultiSampled;
		}

		gfx::BufferFormat GetFormat() const;

	private:
		CComPtr<ID3D11RenderTargetView> mRtView;
		CComPtr<ID3D11Texture2D> mTexture;
		CComPtr<ID3D11Texture2D> mResolvedTexture;
		CComPtr<ID3D11ShaderResourceView> mResourceView;
		Size mSize;
		ContentRect mContentRect;
		bool mMultiSampled;

	};

	using RenderTargetTexturePtr = std::shared_ptr<RenderTargetTexture>;

	class RenderTargetDepthStencil {
		friend class RenderingDevice;
	public:

		RenderTargetDepthStencil(ID3D11Texture2D *textureNew,
			ID3D11DepthStencilView *dsView,
			const Size &size);

		const Size& GetSize() const {
			return mSize;
		}

	private:

		CComPtr<ID3D11DepthStencilView> mDsView;
		CComPtr<ID3D11Texture2D> mTextureNew;
		Size mSize;

	};

	using RenderTargetDepthStencilPtr = std::shared_ptr<RenderTargetDepthStencil>;

}
