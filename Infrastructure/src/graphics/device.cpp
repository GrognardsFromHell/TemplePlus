
#include "infrastructure/infrastructure.h"
#include "infrastructure/images.h"
#include "infrastructure/vfs.h"
#include "graphics/device.h"
#include "graphics/materials.h"

namespace gfx {

	static constexpr int minTexWidth = 1024;
	static constexpr int minTexHeight = 1024;

	RenderingDevice *renderingDevice = nullptr;
	
	ResourceListener::~ResourceListener() {
	}

	ResourceListenerRegistration::ResourceListenerRegistration(RenderingDevice& device, 
		ResourceListener* listener) : mDevice(device), mListener(listener) {
		mDevice.AddResourceListener(listener);
	}

	ResourceListenerRegistration::~ResourceListenerRegistration() {
		mDevice.RemoveResourceListener(mListener);
	}
	
	RenderingDevice::RenderingDevice(HWND windowHandle, int renderWidth, int renderHeight) 
		  : mWindowHandle(windowHandle), 
			mRenderWidth(renderWidth),
			mRenderHeight(renderHeight),
			mShaders(*this),
			mTextures(*this, 128 * 1024 * 1024) {
		Expects(!renderingDevice);
		renderingDevice = this;
		
		HRESULT status;

		status = D3DLOG(Direct3DCreate9Ex(D3D_SDK_VERSION, &mDirect3d9));
		if (status != D3D_OK) {
			throw TempleException("Unable to create Direct3D9Ex interface.");
		}

		// At this point we only do a GetDisplayMode to check the resolution. We could also do this elsewhere
		D3DDISPLAYMODE displayMode;
		status = D3DLOG(mDirect3d9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode));
		if (status != D3D_OK) {
			throw TempleException("Unable to query display mode for primary adapter.");
		}

		// We need at least 1024x768
		if (displayMode.Width < 1024 || displayMode.Height < 768) {
			throw TempleException("You need at least a display resolution of 1024x768.");
		}

		CreatePresentParams();

		status = D3DLOG(mDirect3d9->CreateDeviceEx(
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			mWindowHandle,
			D3DCREATE_HARDWARE_VERTEXPROCESSING,
			&mPresentParams,
			nullptr,
			&mDevice));

		if (status != D3D_OK) {
			throw TempleException("Unable to create Direct3D9 device!");
		}

		// TODO: color bullshit is not yet done (tig_d3d_init_handleformat et al)

		// Get the device caps for real this time.
		ReadCaps();
		
		// Get the currently attached backbuffer
		if (D3DLOG(mDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &mBackBuffer)) != D3D_OK) {
			throw TempleException("Unable to retrieve the back buffer");
		}
		if (D3DLOG(mDevice->GetDepthStencilSurface(&mBackBufferDepth)) != D3D_OK) {
			throw TempleException("Unable to retrieve depth/stencil surface from device");
		}

		memset(&mBackBufferDesc, 0, sizeof(mBackBufferDesc));
		if (D3DLOG(mBackBuffer->GetDesc(&mBackBufferDesc)) != D3D_OK) {
			throw TempleException("Unable to retrieve back buffer description");
		}

		SetRenderSize(renderWidth, renderHeight);
		
		for (auto &listener : mResourcesListeners) {
			listener->CreateResources(*this);
		}
		mResourcesCreated = true;
	}

	RenderingDevice::RenderingDevice(IDirect3DDevice9Ex *device, int renderWidth, int renderHeight)
		: mDevice(device),
		mRenderWidth(renderWidth),
		mRenderHeight(renderHeight),
		mShaders(*this),
		mTextures(*this, 128 * 1024 * 1024) {
		Expects(!renderingDevice);
		renderingDevice = this;

		mCamera.SetScreenWidth((float)renderWidth, (float)renderHeight);

		// TODO: color bullshit is not yet done (tig_d3d_init_handleformat et al)

		// Get the device caps for real this time.
		ReadCaps();

		// Get the currently attached backbuffer
		if (D3DLOG(mDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &mBackBuffer)) != D3D_OK) {
			throw TempleException("Unable to retrieve the back buffer");
		}
		if (D3DLOG(mDevice->GetDepthStencilSurface(&mBackBufferDepth)) != D3D_OK) {
			throw TempleException("Unable to retrieve depth/stencil surface from device");
		}

		memset(&mBackBufferDesc, 0, sizeof(mBackBufferDesc));
		if (D3DLOG(mBackBuffer->GetDesc(&mBackBufferDesc)) != D3D_OK) {
			throw TempleException("Unable to retrieve back buffer description");
		}

		// Create surfaces for the scene
		D3DLOG(mDevice->CreateRenderTarget(
			mRenderWidth,
			mRenderHeight,
			mBackBufferDesc.Format,
			mBackBufferDesc.MultiSampleType,
			mBackBufferDesc.MultiSampleQuality,
			FALSE,
			&mSceneSurface,
			nullptr));

		D3DLOG(mDevice->CreateDepthStencilSurface(
			mRenderWidth,
			mRenderHeight,
			D3DFMT_D16,
			mBackBufferDesc.MultiSampleType,
			mBackBufferDesc.MultiSampleQuality,
			TRUE,
			&mSceneDepthSurface,
			nullptr));

		for (auto &listener : mResourcesListeners) {
			listener->CreateResources(*this);
		}
		mResourcesCreated = true;
	}

	RenderingDevice::~RenderingDevice() {
		renderingDevice = nullptr;
	}

	void RenderingDevice::CreatePresentParams()
	{
		memset(&mPresentParams, 0, sizeof(mPresentParams));

		mPresentParams.BackBufferFormat = D3DFMT_X8R8G8B8;
		// Using discard here allows us to do multisampling.
		mPresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
		mPresentParams.hDeviceWindow = mWindowHandle;
		mPresentParams.Windowed = true;
		mPresentParams.EnableAutoDepthStencil = true;
		mPresentParams.AutoDepthStencilFormat = D3DFMT_D16;
		mPresentParams.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		mPresentParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	}

	void RenderingDevice::ReadCaps()
	{

		mVideoMemory = mDevice->GetAvailableTextureMem();

		if (D3DLOG(mDevice->GetDeviceCaps(&mCaps)) != D3D_OK) {
			throw TempleException("Unable to retrieve Direct3D device mCaps");
		}

		/*
		Several sanity checks follow
		*/
		if (!(mCaps.SrcBlendCaps & D3DPBLENDCAPS_SRCALPHA)) {
			logger->error("source D3DPBLENDCAPS_SRCALPHA is missing");
		}
		if (!(mCaps.SrcBlendCaps & D3DPBLENDCAPS_ONE)) {
			logger->error("source D3DPBLENDCAPS_ONE is missing");
		}
		if (!(mCaps.SrcBlendCaps & D3DPBLENDCAPS_ZERO)) {
			logger->error("source D3DPBLENDCAPS_ZERO is missing");
		}
		if (!(mCaps.DestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA)) {
			logger->error("destination D3DPBLENDCAPS_INVSRCALPHA is missing");
		}
		if (!(mCaps.DestBlendCaps & D3DPBLENDCAPS_ONE)) {
			logger->error("destination D3DPBLENDCAPS_ONE is missing");
		}
		if (!(mCaps.DestBlendCaps & D3DPBLENDCAPS_ZERO)) {
			logger->error("destination D3DPBLENDCAPS_ZERO is missing");
		}

		if (mCaps.MaxSimultaneousTextures < 4) {
			logger->error("less than 4 active textures possible: {}", mCaps.MaxSimultaneousTextures);
		}
		if (mCaps.MaxTextureBlendStages < 4) {
			logger->error("less than 4 texture blend stages possible: {}", mCaps.MaxTextureBlendStages);
		}

		if (!(mCaps.TextureOpCaps & D3DTOP_DISABLE)) {
			logger->error("texture op D3DTOP_DISABLE is missing");
		}
		if (!(mCaps.TextureOpCaps & D3DTOP_SELECTARG1)) {
			logger->error("texture op D3DTOP_SELECTARG1 is missing");
		}
		if (!(mCaps.TextureOpCaps & D3DTOP_SELECTARG2)) {
			logger->error("texture op D3DTOP_SELECTARG2 is missing");
		}
		if (!(mCaps.TextureOpCaps & D3DTOP_BLENDTEXTUREALPHA)) {
			logger->error("texture op D3DTOP_BLENDTEXTUREALPHA is missing");
		}
		if (!(mCaps.TextureOpCaps & D3DTOP_BLENDCURRENTALPHA)) {
			logger->error("texture op D3DTOP_BLENDCURRENTALPHA is missing");
		}
		if (!(mCaps.TextureOpCaps & D3DTOP_MODULATE)) {
			logger->error("texture op D3DTOP_MODULATE is missing");
		}
		if (!(mCaps.TextureOpCaps & D3DTOP_ADD)) {
			logger->error("texture op D3DTOP_ADD is missing");
		}
		if (!(mCaps.TextureOpCaps & D3DTOP_MODULATEALPHA_ADDCOLOR)) {
			logger->error("texture op D3DTOP_MODULATEALPHA_ADDCOLOR is missing");
		}
		if (mCaps.MaxTextureWidth < minTexWidth || mCaps.MaxTextureHeight < minTexHeight) {
			auto msg = fmt::format("minimum texture resolution of {}x{} is not supported. Supported: {}x{}",
				minTexWidth, minTexHeight, mCaps.MaxTextureWidth, mCaps.MaxTextureHeight);
			throw TempleException(msg);
		}

		if ((mCaps.TextureCaps & D3DPTEXTURECAPS_POW2) != 0) {
			logger->error("Textures must be power of two");
		}
		if ((mCaps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) != 0) {
			logger->error("Textures must be square");
		}
	}

	void RenderingDevice::AddResourceListener(ResourceListener* listener) {
		mResourcesListeners.push_back(listener);
		if (mResourcesCreated) {
			listener->CreateResources(*this);
		}
	}

	void RenderingDevice::RemoveResourceListener(ResourceListener* listener) {
		mResourcesListeners.remove(listener);
		if (mResourcesCreated) {
			listener->FreeResources(*this);
		}
	}

	bool RenderingDevice::BeginFrame() {

		if (mBeginSceneDepth++ > 0) {
			return true;
		}

		auto clearColor = D3DCOLOR_ARGB(0, 0, 0, 0);

		auto result = D3DLOG(mDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clearColor, 1.0f, 0));

		if (result != D3D_OK) {
			return false;
		}

		result = D3DLOG(mDevice->BeginScene());

		if (result != D3D_OK) {
			return false;
		}

		mLastFrameStart = Clock::now();

		return true;
	}

	bool RenderingDevice::Present() {
		if (--mBeginSceneDepth > 0) {
			return true;
		}

		// TODO mTextureManager->FreeUnusedTextures();

		if (D3DLOG(mDevice->EndScene()) != D3D_OK) {
			return false;
		}

		auto result = mDevice->Present(nullptr, nullptr, nullptr, nullptr);

		if (result != S_OK && result != S_PRESENT_OCCLUDED) {
			LogD3dError("Present()", result);
			if (result == D3DERR_DEVICELOST) {
				ResetDevice();
			}
			return false;
		}

		return true;
	}

	void RenderingDevice::ResetDevice() {
		

		CreatePresentParams();
		auto result = mDevice->Reset(&mPresentParams);
		if (result != D3D_OK) {
			logger->warn("Device reset failed.");
		}

	}

	void RenderingDevice::SetMaterial(const Material &material) {
		
		SetRasterizerState(material.GetRasterizerState());
		SetBlendState(material.GetBlendState());
		SetDepthStencilState(material.GetDepthStencilState());

		for (size_t i = 0; i < material.GetSamplers().size(); ++i) {
			auto& sampler = material.GetSamplers()[i];
			if (sampler.GetTexture()) {
				D3DLOG(mDevice->SetTexture(i, sampler.GetTexture()->GetDeviceTexture()));
			} else {
				D3DLOG(mDevice->SetTexture(i, nullptr));
			}
			SetSamplerState(i, sampler.GetState());
		}

		// Free up the texture bindings of the samplers currently being used
		for (size_t i = material.GetSamplers().size(); i < mUsedSamplers; ++i) {
			D3DLOG(mDevice->SetTexture(i, nullptr));
		}

		mUsedSamplers = material.GetSamplers().size();
		
		material.GetVertexShader()->Bind();
		material.GetPixelShader()->Bind();
	}

	IndexBufferPtr RenderingDevice::CreateEmptyIndexBuffer(size_t count) {
		CComPtr<IDirect3DIndexBuffer9> buffer;

		auto length = sizeof(uint16_t) * count;
		if (D3DLOG(mDevice->CreateIndexBuffer(length,
			D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX16,
			D3DPOOL_DEFAULT,
			&buffer,
			nullptr)) != D3D_OK) {
			throw TempleException("Unable to create index buffer.");
		}

		return std::make_shared<IndexBuffer>(buffer, count);
	}

	VertexBufferPtr RenderingDevice::CreateEmptyVertexBuffer(size_t size, bool forPoints) {
		CComPtr<IDirect3DVertexBuffer9> buffer;

		DWORD usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;

		if (forPoints) {
			usage |= D3DUSAGE_POINTS;
		}

		if (mDevice->CreateVertexBuffer(size,
			usage,
			0,
			D3DPOOL_DEFAULT,
			&buffer,
			nullptr) != D3D_OK) {
			throw TempleException("Unable to create index buffer.");
		}

		return std::make_shared<VertexBuffer>(buffer, size);
	}

	void RenderingDevice::SetRasterizerState(const RasterizerState &state) {
		mDevice->SetRenderState(D3DRS_FILLMODE, state.fillMode);
		mDevice->SetRenderState(D3DRS_CULLMODE, state.cullMode);
	}

	void RenderingDevice::SetBlendState(const BlendState &state) {
		mDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, state.blendEnable ? TRUE : FALSE);
		mDevice->SetRenderState(D3DRS_SRCBLEND, state.srcBlend);
		mDevice->SetRenderState(D3DRS_DESTBLEND, state.destBlend);

		DWORD mask = 0;
		if (state.writeRed) {
			mask |= D3DCOLORWRITEENABLE_RED;
		}
		if (state.writeGreen) {
			mask |= D3DCOLORWRITEENABLE_GREEN;
		}
		if (state.writeBlue) {
			mask |= D3DCOLORWRITEENABLE_BLUE;
		}
		if (state.writeAlpha) {
			mask |= D3DCOLORWRITEENABLE_ALPHA;
		}
		mDevice->SetRenderState(D3DRS_COLORWRITEENABLE, mask);
	}

	void RenderingDevice::SetDepthStencilState(const DepthStencilState &state) {
		mDevice->SetRenderState(D3DRS_ZENABLE, state.depthEnable ? TRUE : FALSE);
		mDevice->SetRenderState(D3DRS_ZFUNC, state.depthFunc);
		mDevice->SetRenderState(D3DRS_ZWRITEENABLE, state.depthWrite ? TRUE : FALSE);
	}

	void RenderingDevice::SetSamplerState(int samplerIdx, const SamplerState &state) {
		mDevice->SetSamplerState(samplerIdx, D3DSAMP_MINFILTER, state.minFilter);
		mDevice->SetSamplerState(samplerIdx, D3DSAMP_MAGFILTER, state.magFilter);
		mDevice->SetSamplerState(samplerIdx, D3DSAMP_MIPFILTER, state.mipFilter);

		mDevice->SetSamplerState(samplerIdx, D3DSAMP_ADDRESSU, state.addressU);
		mDevice->SetSamplerState(samplerIdx, D3DSAMP_ADDRESSV, state.addressV);
	}

	VertexBufferPtr RenderingDevice::CreateVertexBufferRaw(gsl::array_view<uint8_t> data) {
		CComPtr<IDirect3DVertexBuffer9> result;

		D3DLOG(mDevice->CreateVertexBuffer(data.size(), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &result, nullptr));

		void* dataOut;
		D3DLOG(result->Lock(0, data.size(), &dataOut, D3DLOCK_DISCARD));

		memcpy(dataOut, &data[0], data.size());

		D3DLOG(result->Unlock());

		return std::make_shared<VertexBuffer>(result, data.size());
	}

	IndexBufferPtr RenderingDevice::CreateIndexBuffer(gsl::array_view<uint16_t> data) {
		CComPtr<IDirect3DIndexBuffer9> result;

		D3DLOG(mDevice->CreateIndexBuffer(data.size() * sizeof(uint16_t),
			D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
			D3DFMT_INDEX16,
			D3DPOOL_DEFAULT,
			&result, nullptr));

		void* dataOut;
		D3DLOG(result->Lock(0, data.size(), &dataOut, D3DLOCK_DISCARD));

		memcpy(dataOut, &data[0], data.size() * sizeof(uint16_t));

		D3DLOG(result->Unlock());

		return std::make_shared<IndexBuffer>(result, data.size());
	}

	void RenderingDevice::SetBackBufferSize(int w, int h) {
		
	}

	void RenderingDevice::SetRenderSize(int w, int h) {

		mRenderWidth = w;
		mRenderHeight = h;

		mCamera.SetScreenWidth((float) mRenderWidth, (float) mRenderHeight);

		auto widthFactor = mBackBufferDesc.Width / (float)mRenderWidth;
		auto heightFactor = mBackBufferDesc.Height / (float)mRenderHeight;
		mSceneScale = std::min<float>(widthFactor, heightFactor);

		// Calculate the rectangle on the back buffer where the scene will
		// be stretched to
		auto drawWidth = mSceneScale * mRenderWidth;
		auto drawHeight = mSceneScale * mRenderHeight;
		auto drawX = (GetScreenWidthF() - drawWidth) / 2;
		auto drawY = (GetScreenHeightF() - drawHeight) / 2;
		mSceneRect = XMFLOAT4(drawX, drawY, drawWidth, drawHeight);

		mSceneSurface.Release();
		mSceneDepthSurface.Release();
		
		// Create surfaces for the scene
		D3DLOG(mDevice->CreateRenderTarget(
			mRenderWidth,
			mRenderHeight,
			mBackBufferDesc.Format,
			mBackBufferDesc.MultiSampleType,
			mBackBufferDesc.MultiSampleQuality,
			FALSE,
			&mSceneSurface,
			nullptr));

		D3DLOG(mDevice->CreateDepthStencilSurface(
			mRenderWidth,
			mRenderHeight,
			D3DFMT_D16,
			mBackBufferDesc.MultiSampleType,
			mBackBufferDesc.MultiSampleQuality,
			TRUE,
			&mSceneDepthSurface,
			nullptr));

	}

	void RenderingDevice::TakeScaledScreenshot(const std::string &filename, int width, int height, int quality) {
		logger->debug("Creating screenshot with size {}x{} in {}", width, height, filename);
		
		D3DSURFACE_DESC desc;
		mSceneSurface->GetDesc(&desc);

		// Support taking unscaled screenshots
		auto stretch = true;
		if (width == 0 || height == 0) {
			width = desc.Width;
			height = desc.Height;
			stretch = false;
		}

		// Create system memory surface to copy the screenshot to
		CComPtr<IDirect3DSurface9> sysMemSurface;
		if (D3DLOG(mDevice->CreateOffscreenPlainSurface(width, height, desc.Format, D3DPOOL_SYSTEMMEM, &sysMemSurface, nullptr)) != D3D_OK) {
			logger->error("Unable to create offscreen surface for copying the screenshot");
			return;
		}

		if (stretch) {
			CComPtr<IDirect3DSurface9> stretchedScene;
			if (D3DLOG(mDevice->CreateRenderTarget(width, height, desc.Format, desc.MultiSampleType, desc.MultiSampleQuality, false, &stretchedScene, NULL)) != D3D_OK) {
				return;
			}

			if (D3DLOG(mDevice->StretchRect(mSceneSurface, nullptr, stretchedScene, nullptr, D3DTEXF_LINEAR)) != D3D_OK) {
				logger->error("Unable to copy front buffer to target surface for screenshot");
				return;
			}

			if (D3DLOG(mDevice->GetRenderTargetData(stretchedScene, sysMemSurface))) {
				logger->error("Unable to copy stretched render target to system memory.");
				return;
			}
		} else {
			if (D3DLOG(mDevice->GetRenderTargetData(mSceneSurface, sysMemSurface))) {
				logger->error("Unable to copy scene render target to system memory.");
				return;
			}
		}

		/*
		Get access to the pixel data for the surface and encode it to a JPEG.
		*/
		D3DLOCKED_RECT locked;
		if (D3DLOG(sysMemSurface->LockRect(&locked, nullptr, 0))) {
			logger->error("Unable to lock screenshot surface.");
			return;
		}

		// Clamp quality to [1, 100]
		quality = std::min(100, std::max(1, quality));

		auto jpegData(gfx::EncodeJpeg(reinterpret_cast<uint8_t*>(locked.pBits),
			gfx::JpegPixelFormat::BGRX,
			width,
			height,
			quality,
			locked.Pitch));

		if (D3DLOG(sysMemSurface->UnlockRect())) {
			logger->error("Unable to unlock screenshot surface.");
			return;
		}

		// We have to write using tio or else it goes god knows where
		try {
			vfs->WriteBinaryFile(filename, jpegData);
		} catch (std::exception &e) {
			logger->error("Unable to save screenshot due to an IO error: {}", e.what());

		}
	}

}
