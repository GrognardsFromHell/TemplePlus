
#pragma once

#include <memory>
#include <list>
#include <chrono>

#include "../platform/d3d.h"
#include "../gsl/gsl.h"

#include "buffers.h"
#include "shaders.h"
#include "camera.h"
#include "textures.h"

namespace gfx {
	
	class RenderingDevice;
	class Material;
	struct RasterizerState;
	struct BlendState;
	struct DepthStencilState;
	struct SamplerState;

	class ResourceListener {
	public:
		virtual ~ResourceListener();

		virtual void CreateResources(RenderingDevice&) = 0;
		virtual void FreeResources(RenderingDevice&) = 0;
	};

	enum class StandardSlotSemantic {
		ViewProjMatrix
	};

	// RAII class for managing resource listener registrations
	class ResourceListenerRegistration {
	public:
		explicit ResourceListenerRegistration(RenderingDevice& device, ResourceListener* listener);
		~ResourceListenerRegistration();

		NO_COPY_OR_MOVE(ResourceListenerRegistration);

	private:
		RenderingDevice& mDevice;
		ResourceListener* mListener;
	};

	using DynamicTexturePtr = std::shared_ptr<class DynamicTexture>;
	using RenderTargetTexturePtr = std::shared_ptr<class RenderTargetTexture>;

	class RenderingDevice {
	public:
		RenderingDevice(HWND mWindowHandle, int renderWidth, int renderHeight, bool antiAliasing = true);
		RenderingDevice(IDirect3DDevice9Ex *device, int renderWidth, int renderHeight);
		~RenderingDevice();

		bool BeginFrame();
		bool Present();

		using Clock = std::chrono::high_resolution_clock;
		using TimePoint = std::chrono::time_point<Clock>;

		TimePoint GetLastFrameStart() const {
			return mLastFrameStart;
		}
		TimePoint GetDeviceCreated() const {
			return mDeviceCreated;
		}

		/*
		The back buffer size of the D3D context. This is not
		necessarily the size at which ToEE is rendered:
		*/
		void SetRenderSize(int w, int h);

		int GetRenderWidth() const {
			return mRenderWidth;
		}
		int GetRenderHeight() const {
			return mRenderHeight;
		}		
		int GetScreenWidth() const {
			return mBackBufferDesc.Width;
		}
		int GetScreenHeight() const {
			return mBackBufferDesc.Height;
		}
		float GetScreenWidthF() const {
			return (float) mBackBufferDesc.Width;
		}
		float GetScreenHeightF() const {
			return (float) mBackBufferDesc.Height;
		}

		float GetSceneScale() const {
			return mSceneScale;
		}
		const XMFLOAT4 &GetSceneRect() const {
			return mSceneRect;
		}
		
		IDirect3DSurface9* GetRenderSurface() {
			return mSceneSurface;
		}
		IDirect3DSurface9* GetRenderDepthStencilSurface() {
			return mSceneDepthSurface;
		}
		IDirect3DSurface9* GetBackBuffer() {
			return mBackBuffer;
		}
		IDirect3DSurface9* GetBackBufferDepthStencil() {
			return mBackBufferDepth;
		}
				
		std::shared_ptr<class IndexBuffer> CreateEmptyIndexBuffer(size_t count);
		std::shared_ptr<class VertexBuffer> CreateEmptyVertexBuffer(size_t count, bool forPoints = false);
		DynamicTexturePtr CreateDynamicTexture(D3DFORMAT format, int width, int height);
		RenderTargetTexturePtr CreateRenderTargetTexture(D3DFORMAT format, int width, int height);

		template<typename T>
		VertexBufferPtr CreateVertexBuffer(gsl::span<T> data);
		VertexBufferPtr CreateVertexBufferRaw(gsl::span<const uint8_t> data);
		IndexBufferPtr CreateIndexBuffer(gsl::span<const uint16_t> data);

		void SetMaterial(const Material &material);
		void SetVertexShaderConstant(uint32_t startRegister, StandardSlotSemantic semantic);
		void SetPixelShaderConstant(uint32_t startRegister, StandardSlotSemantic semantic);

		void SetRasterizerState(const RasterizerState &state);
		void SetBlendState(const BlendState &state);
		void SetDepthStencilState(const DepthStencilState &state);
		void SetSamplerState(int samplerIdx, const SamplerState &state);

		/*
		  Changes the currently used cursor to the given surface.
		 */
		void SetCursor(int hotspotX, int hotspotY, const gfx::TextureRef &texture);

		/*
			Take a screenshot with the given size. The image will be stretched 
			to the given size.
		*/
		void TakeScaledScreenshot(const std::string& filename, int width, int height, int quality = 90);

		IDirect3DDevice9Ex *GetDevice() const {
			return mDevice;
		}

		Shaders& GetShaders() {
			return mShaders;
		}

		Textures& GetTextures() {
			return mTextures;
		}

		WorldCamera& GetCamera() {
			return mCamera;
		}

		void SetAntiAliasing(bool enable);

	private:
		friend class ResourceListenerRegistration;
		void AddResourceListener(ResourceListener* resourceListener);
		void RemoveResourceListener(ResourceListener* resourceListener);

		void AfterDeviceResetOrCreated();
		void CreatePresentParams();
		void ReadCaps();
		void ResetDevice();

		int mBeginSceneDepth = 0;
		int mRenderWidth;
		int mRenderHeight;
		float mSceneScale; // Scale of the scene when drawn on the backbuffer
		
		// The rectangle (x,y,w,h) where the scene is Drawn on the backbuffer
		XMFLOAT4 mSceneRect;
		bool mAntiAliasing;
		
		HWND mWindowHandle;
		
		D3DPRESENT_PARAMETERS mPresentParams;
		D3DCAPS9 mCaps;
		size_t mVideoMemory = 0;
		D3DSURFACE_DESC mBackBufferDesc;
		std::vector<D3DMULTISAMPLE_TYPE> mSupportedAaSamples;

		CComPtr<IDirect3D9Ex> mDirect3d9;
		CComPtr<IDirect3DDevice9Ex> mDevice;
		CComPtr<IDirect3DSurface9> mBackBuffer;
		CComPtr<IDirect3DSurface9> mBackBufferDepth;
		// Render targets used to draw the ToEE scene at a different resolution
		// than the screen resolution
		CComPtr<IDirect3DSurface9> mSceneSurface;
		CComPtr<IDirect3DSurface9> mSceneDepthSurface;
		
		gfx::TextureRef mCursor;
		XMINT2 mCursorHotspot;

		std::list<ResourceListener*> mResourcesListeners;
		bool mResourcesCreated = false;
		
		TimePoint mLastFrameStart = Clock::now();
		TimePoint mDeviceCreated = Clock::now();

		size_t mUsedSamplers = 0;

		Shaders mShaders;
		Textures mTextures;
		WorldCamera mCamera;
	};
	
	template <typename T>
	VertexBufferPtr RenderingDevice::CreateVertexBuffer(gsl::span<T> data) {
		return CreateVertexBufferRaw(gsl::as_span(reinterpret_cast<const uint8_t*>(&data[0]), data.size_bytes()));
	}

	extern RenderingDevice *renderingDevice;

}
