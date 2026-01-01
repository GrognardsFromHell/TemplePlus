
#pragma once

#include <gsl/gsl>

#include <memory>
#include <list>
#include <chrono>

#include <EASTL/fixed_vector.h>
#include <EASTL/map.h>

#include "../platform/d3d.h"

#include "buffers.h"
#include "shaders.h"
#include "camera.h"
#include "textures.h"

struct ID3D11Texture2D;

namespace gfx {
	
	class RenderingDevice;
	class Material;
	struct RasterizerSpec;
	class RasterizerState;
	struct BlendSpec;
	class BlendState;
	struct DepthStencilSpec;
	class DepthStencilState;
	struct SamplerSpec;
	class SamplerState;
	struct MaterialSamplerSpec;
	class BufferBinding;
	class TextEngine;
	
	using SamplerStatePtr = std::shared_ptr<SamplerState>;
	using DepthStencilStatePtr = std::shared_ptr<DepthStencilState>;
	using BlendStatePtr = std::shared_ptr<BlendState>;
	using RasterizerStatePtr = std::shared_ptr<RasterizerState>;
	using WorldCameraPtr = std::shared_ptr<WorldCamera>;

	using ResizeListener = std::function<void(int w, int h)>;

	// RAII style listener registration
	class ResizeListenerRegistration {
		friend class RenderingDevice;
	public:		
		~ResizeListenerRegistration();

		NO_COPY(ResizeListenerRegistration);

		ResizeListenerRegistration(ResizeListenerRegistration &&o) : mDevice(o.mDevice), mKey(o.mKey) {
			o.mKey = 0;
		}
		ResizeListenerRegistration &operator =(ResizeListenerRegistration &&o) = delete;
	private:
		ResizeListenerRegistration(gfx::RenderingDevice &device, uint32_t key) : mDevice(device), mKey(key) {}

		gfx::RenderingDevice &mDevice;
		uint32_t mKey;		
	};

	class ResourceListener {
	public:
		virtual ~ResourceListener();

		virtual void CreateResources(RenderingDevice&) = 0;
		virtual void FreeResources(RenderingDevice&) = 0;
	};

	enum class StandardSlotSemantic {
		ViewProjMatrix,
		UiProjMatrix
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
	using RenderTargetDepthStencilPtr = std::shared_ptr<class RenderTargetDepthStencil>;

	// An output of a display device (think: monitor)
	struct DisplayDeviceOutput {
		// Technical id for use in a configuration or log file
		std::string id;
		// Name to display to the end user
		std::string name;
	};

	// A display device that can be used to render the game (think: GPU)
	struct DisplayDevice {
		// Technical id for use in a configuration or log file
		size_t id;
		// Name to display to the end user
		std::string name;
		// Outputs associated with this display device
		eastl::fixed_vector<DisplayDeviceOutput, 4> outputs;
	};

	enum class PrimitiveType {
		TriangleList,
		TriangleStrip,
		LineStrip,
		LineList,
		PointList,
	};

	enum class BufferFormat {
		A8,
		A8R8G8B8,
		X8R8G8B8
	};

	enum class MapMode {
		Read,
		Discard,
		NoOverwrite
	};
	
	template<typename TBuffer, typename TElement>
	class MappedBuffer {
	public:

		using View = gsl::span<TElement>;
		using Iterator = gsl::contiguous_span_iterator<View>;

		MappedBuffer(TBuffer &buffer,
			RenderingDevice &device,
			View data,
			uint32_t rowPitch) : mBuffer(buffer), mDevice(device), mData(data), mRowPitch(rowPitch) {}

		MappedBuffer(MappedBuffer&& o) : mBuffer(o.mBuffer), mDevice(o.mDevice), mData(o.mData), mRowPitch(o.rowPitch) {
			o.mMoved = true;
		}

		~MappedBuffer();

		Iterator begin() {
			return mData.begin();
		}

		Iterator end() {
			return mData.end();
		}

		TElement &operator[](size_t idx) {
			assert(!mMoved);
			return mData[idx];
		}

		size_t size() const {
			return mData.size();
		}

		TElement *GetData() const {
			return mData.data();
		}

		// Only relevant for mapped texture buffers
		size_t GetRowPitch() const {
			return mRowPitch;
		}

		void Unmap();

	private:
		TBuffer& mBuffer;
		RenderingDevice &mDevice;
		View mData;
		size_t mRowPitch;
		bool mMoved = false;
	};

	template<typename TElement>
	using MappedVertexBuffer = MappedBuffer<VertexBuffer, TElement>;
	using MappedIndexBuffer = MappedBuffer<IndexBuffer, uint16_t>;
	using MappedTexture = MappedBuffer<DynamicTexture, uint8_t>;

	class RenderingDevice {
	template<typename T>
	friend class Shader;
	friend class BufferBinding;
	friend class TextureLoader;
	friend class DebugUI;
	public:
		RenderingDevice(HWND mWindowHandle, uint32_t adapterIdx = 0, bool debugDevice = false);
		~RenderingDevice();

		bool BeginFrame();
		bool Present();
		void PresentForce();
		void Flush();

		void ClearCurrentColorTarget(XMCOLOR color);

		void ClearCurrentDepthTarget(bool clearDepth = true, 
			bool clearStencil = true, 
			float depthValue = 1.0f, 
			uint8_t stencilValue = 0);

		using Clock = std::chrono::high_resolution_clock;
		using TimePoint = std::chrono::time_point<Clock>;

		TimePoint GetLastFrameStart() const {
			return mLastFrameStart;
		}
		TimePoint GetDeviceCreated() const {
			return mDeviceCreated;
		}

		const eastl::vector<DisplayDevice> &GetDisplayDevices();

		// Resize the back buffer
		void ResizeBuffers(int w, int h);

		Material CreateMaterial(
			const BlendSpec &blendSpec,
			const DepthStencilSpec &depthStencilSpec,
			const RasterizerSpec &rasterizerSpec,
			const std::vector<MaterialSamplerSpec> &samplerSpecs,
			const VertexShaderPtr &vs,
			const PixelShaderPtr &ps
		);

		BlendStatePtr CreateBlendState(const BlendSpec &spec);
		DepthStencilStatePtr CreateDepthStencilState(const DepthStencilSpec &spec);
		RasterizerStatePtr CreateRasterizerState(const RasterizerSpec &spec);
		SamplerStatePtr CreateSamplerState(const SamplerSpec &spec);

		// Changes the current scissor rect to the given rectangle
		void SetScissorRect(int x, int y, int width, int height);

		// Resets the scissor rect to the current render target's size
		void ResetScissorRect();
				
		std::shared_ptr<class IndexBuffer> CreateEmptyIndexBuffer(size_t count);
		std::shared_ptr<class VertexBuffer> CreateEmptyVertexBuffer(size_t count, bool forPoints = false);
		DynamicTexturePtr CreateDynamicTexture(gfx::BufferFormat format, int width, int height);
		DynamicTexturePtr CreateDynamicStagingTexture(gfx::BufferFormat format, int width, int height);
		void CopyRenderTarget(gfx::RenderTargetTexture &renderTarget, gfx::DynamicTexture &stagingTexture);
		RenderTargetTexturePtr CreateRenderTargetTexture(gfx::BufferFormat format, int width, int height, bool multiSampled = false);
		RenderTargetTexturePtr CreateRenderTargetForNativeSurface(ID3D11Texture2D *surface);
		RenderTargetTexturePtr CreateRenderTargetForSharedSurface(IUnknown *surface);
		RenderTargetDepthStencilPtr CreateRenderTargetDepthStencil(int width, int height, bool multiSampled = false);

		template<typename T>
		VertexBufferPtr CreateVertexBuffer(gsl::span<T> data, bool immutable = true);
		VertexBufferPtr CreateVertexBufferRaw(gsl::span<const uint8_t> data, bool immutable = true);
		IndexBufferPtr CreateIndexBuffer(gsl::span<const uint16_t> data, bool immutable = true);

		void SetMaterial(const Material &material);
		void SetVertexShaderConstant(uint32_t startRegister, StandardSlotSemantic semantic);
		void SetPixelShaderConstant(uint32_t startRegister, StandardSlotSemantic semantic);

		void SetRasterizerState(const RasterizerState &state);
		void SetBlendState(const BlendState &state);
		void SetDepthStencilState(const DepthStencilState &state);
		void SetSamplerState(int samplerIdx, const SamplerState &state);

		void SetTexture(uint32_t slot, gfx::Texture &texture);

		void SetIndexBuffer(const gfx::IndexBuffer &indexBuffer);

		void Draw(PrimitiveType type, uint32_t vertexCount, uint32_t startVertex = 0);
		
		void DrawIndexed(PrimitiveType type, uint32_t vertexCount, uint32_t indexCount, uint32_t startVertex = 0, uint32_t vertexBase = 0);

		/*
		  Changes the currently used cursor to the given surface.
		 */
		void SetCursor(int hotspotX, int hotspotY, const gfx::TextureRef &texture);
		void ShowCursor();
		void HideCursor();

		/*
			Take a screenshot with the given size. The image will be stretched 
			to the given size.
		*/
		void TakeScaledScreenshot(const std::string& filename, int width, int height, int quality = 90);

		// Creates a buffer binding for a MDF material that
		// is preinitialized with the correct shader
		BufferBinding CreateMdfBufferBinding(bool perVertexColor = false);

		Shaders& GetShaders() {
			return mShaders;
		}

		Textures& GetTextures() {
			return mTextures;
		}

		/**
		 * Returns the current camera used to render.
		 */
		WorldCamera &GetCurrentCamera() {
			return *mCurrentCamera;
		}

		/**
		 * Returns the camera that will be used for rendering.
		 */
		void SetCurrentCamera(WorldCameraPtr camera);

		void SetAntiAliasing(bool enable, uint32_t samples, uint32_t quality);
		void SetVSync(bool enable);
		
		template<typename T>
		void UpdateBuffer(VertexBuffer &buffer, gsl::span<T> data) {
			UpdateBuffer(buffer, data.data(), data.size_bytes());
		}

		void UpdateBuffer(VertexBuffer &buffer, const void *data, size_t size);
		
		void UpdateBuffer(IndexBuffer &buffer, gsl::span<uint16_t> data);
				
		template<typename TElement>
		MappedVertexBuffer<TElement> Map(VertexBuffer &buffer, gfx::MapMode mode = gfx::MapMode::Discard) {
			auto data = MapVertexBufferRaw(buffer, mode);
			auto castData = gsl::span<TElement>((TElement*)data.data(), data.size() / sizeof(TElement));
			return MappedVertexBuffer<TElement>(buffer, *this, castData, 0);
		}
		void Unmap(VertexBuffer &buffer);

		// Index buffer memory mapping techniques
		MappedIndexBuffer Map(IndexBuffer &buffer, gfx::MapMode mode = gfx::MapMode::Discard);
		void Unmap(IndexBuffer &buffer);

		MappedTexture Map(DynamicTexture &texture, gfx::MapMode mode = gfx::MapMode::Discard);
		void Unmap(DynamicTexture &texture);

		static constexpr uint32_t MaxVsConstantBufferSize = 2048;

		template<typename T>
		void SetVertexShaderConstants(uint32_t slot, const T &buffer) {
			static_assert(sizeof(T) <= MaxVsConstantBufferSize, "Constant buffer exceeds maximum size");
			UpdateResource(mVsConstantBuffer, &buffer, sizeof(T));
			VSSetConstantBuffer(slot, mVsConstantBuffer);
		}

		static constexpr uint32_t MaxPsConstantBufferSize = 512;

		template<typename T>
		void SetPixelShaderConstants(uint32_t slot, const T &buffer) {
			static_assert(sizeof(T) <= MaxPsConstantBufferSize, "Constant buffer exceeds maximum size");
			UpdateResource(mPsConstantBuffer, &buffer, sizeof(T));
			PSSetConstantBuffer(slot, mPsConstantBuffer);
		}

		const gfx::Size &GetBackBufferSize() const;

		// Pushes the back buffer and it's depth buffer as the current render target
		void PushBackBufferRenderTarget();
		void PushRenderTarget(
			const gfx::RenderTargetTexturePtr &colorBuffer,
			const gfx::RenderTargetDepthStencilPtr &depthStencilBuffer
		);
		void PopRenderTarget();
		const gfx::RenderTargetTexturePtr &GetCurrentRederTargetColorBuffer() const {
			return mRenderTargetStack.back().colorBuffer;
		}
		const gfx::RenderTargetDepthStencilPtr &GetCurrentRenderTargetDepthStencilBuffer() const {
			return mRenderTargetStack.back().depthStencilBuffer;
		}

		ResizeListenerRegistration AddResizeListener(ResizeListener listener);

		bool IsDebugDevice() const;

		/**
		 * Emits the start of a rendering call group if the debug device is being used. 
		 * This information can be used in the graphic debugger.
		 */
		template<typename... T>
		void BeginPerfGroup(const char *format, const T &... args) const {
			if (IsDebugDevice()) {
				BeginPerfGroupInternal(fmt::format(format, args...).c_str());
			}
		}
		/**
		 * Ends a previously started performance group.
		 */
		void EndPerfGroup() const;

		TextEngine& GetTextEngine() const;

	private:
		friend class ResourceListenerRegistration;
		friend class ResizeListenerRegistration;

		void BeginPerfGroupInternal(const char *msg) const;

		void RemoveResizeListener(uint32_t key);

		void AddResourceListener(ResourceListener* resourceListener);
		void RemoveResourceListener(ResourceListener* resourceListener);

		void UpdateResource(ID3D11Resource *resource, const void *data, size_t size);
		CComPtr<ID3D11Buffer> CreateConstantBuffer(const void *initialData, size_t initialDataSize);
		void VSSetConstantBuffer(uint32_t slot, ID3D11Buffer *buffer);
		void PSSetConstantBuffer(uint32_t slot, ID3D11Buffer *buffer);

		gsl::span<uint8_t> MapVertexBufferRaw(VertexBuffer &buffer, MapMode mode);
		
		CComPtr<IDXGIAdapter1> GetAdapter(size_t index);

		void UpdateDefaultCameraScreenSize();
		
		int mBeginSceneDepth = 0;
		
		HWND mWindowHandle;

		CComPtr<IDXGIFactory1> mDxgiFactory;

		// The DXGI adapter we use
		CComPtr<IDXGIAdapter1> mAdapter;
				
		// D3D11 device and related
		CComPtr<ID3D11Device> mD3d11Device;
		CComPtr<ID3D11Device1> mD3d11Device1;
		DXGI_SWAP_CHAIN_DESC mSwapChainDesc;
		CComPtr<IDXGISwapChain> mSwapChain;
		CComPtr<ID3D11DeviceContext> mContext;
		gfx::RenderTargetTexturePtr mBackBufferNew;
		gfx::RenderTargetDepthStencilPtr mBackBufferDepthStencil;
		
		struct RenderTarget {
			gfx::RenderTargetTexturePtr colorBuffer;
			gfx::RenderTargetDepthStencilPtr depthStencilBuffer;
		};
		eastl::fixed_vector<RenderTarget, 16> mRenderTargetStack;

		D3D_FEATURE_LEVEL mFeatureLevel = D3D_FEATURE_LEVEL_9_1;

		eastl::vector<DisplayDevice> mDisplayDevices;
		
		CComPtr<ID3D11Buffer> mVsConstantBuffer;
		CComPtr<ID3D11Buffer> mPsConstantBuffer;
		
		eastl::map<uint32_t, ResizeListener> mResizeListeners;
		uint32_t mResizeListenersKey = 0;

		std::list<ResourceListener*> mResourcesListeners;
		bool mResourcesCreated = false;
		
		
		TimePoint mLastFrameStart = Clock::now();
		TimePoint mDeviceCreated = Clock::now();

		size_t mUsedSamplers = 0;

		Shaders mShaders;
		Textures mTextures;
		WorldCameraPtr mDefaultCamera;
		WorldCameraPtr mCurrentCamera;

		struct Impl;
		std::unique_ptr<Impl> mImpl;
	};
	
	template <typename T>
	VertexBufferPtr RenderingDevice::CreateVertexBuffer(gsl::span<T> data, bool immutable) {
		return CreateVertexBufferRaw(gsl::span(reinterpret_cast<const uint8_t*>(&data[0]), data.size_bytes()), immutable);
	}

	extern RenderingDevice *renderingDevice;

	template<typename TBuffer, typename TElement>
	inline MappedBuffer<TBuffer, TElement>::~MappedBuffer()
	{
		if (!mMoved) {
			mDevice.Unmap(mBuffer);
		}
	}

	template<typename TBuffer, typename TElement>
	inline void MappedBuffer<TBuffer, TElement>::Unmap()
	{
		assert(!mMoved);
		mMoved = true;
		mDevice.Unmap(mBuffer);
	}

	inline ResizeListenerRegistration::~ResizeListenerRegistration() {
		mDevice.RemoveResizeListener(mKey);
	}


	// RAII style demarcation of render call groups for performance debugging
	class PerfGroup {
	public:
		template<typename... T>
		PerfGroup(RenderingDevice &device, const char *format, T... args) : mDevice(device) {
			device.BeginPerfGroup(format, args...);
		}
		~PerfGroup() {
			mDevice.EndPerfGroup();
		}
	private:
		const RenderingDevice &mDevice;
	};

}
