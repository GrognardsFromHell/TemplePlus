#include "graphics/device.h"
#include "graphics/bufferbinding.h"
#include "graphics/dynamictexture.h"
#include "graphics/materials.h"
#include "infrastructure/images.h"
#include "infrastructure/infrastructure.h"
#include "infrastructure/vfs.h"
#include "graphics/shaperenderer2d.h"
#include "graphics/textengine.h"

#include <VersionHelpers.h>

namespace gfx {
	
RenderingDevice *renderingDevice = nullptr;

ResourceListener::~ResourceListener() {}

ResourceListenerRegistration::ResourceListenerRegistration(
    RenderingDevice &device, ResourceListener *listener)
    : mDevice(device), mListener(listener) {
  mDevice.AddResourceListener(listener);
}

ResourceListenerRegistration::~ResourceListenerRegistration() {
  mDevice.RemoveResourceListener(mListener);
}

void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const BufferFormat &format) {
	switch (format) {
	case BufferFormat::A8:
		f.writer() << "A8";
		break;
	case BufferFormat::A8R8G8B8:
		f.writer() << "A8R8G8B8";
		break;
	case BufferFormat::X8R8G8B8:
		f.writer() << "X8R8G8B8";
		break;
	}
}

void format_arg(fmt::BasicFormatter<char> &f, const char *&format_str, const D3D_FEATURE_LEVEL &level) {
	switch (level) {
	case D3D_FEATURE_LEVEL_9_1:
		f.writer() << "D3D_FEATURE_LEVEL_9_1";
		break;
	case D3D_FEATURE_LEVEL_9_2:
		f.writer() << "D3D_FEATURE_LEVEL_9_2";
		break;
	case D3D_FEATURE_LEVEL_9_3:
		f.writer() << "D3D_FEATURE_LEVEL_9_3";
		break;
	case D3D_FEATURE_LEVEL_10_0:
		f.writer() << "D3D_FEATURE_LEVEL_10_0";
		break;
	case D3D_FEATURE_LEVEL_10_1:
		f.writer() << "D3D_FEATURE_LEVEL_10_1";
		break;
	case D3D_FEATURE_LEVEL_11_0:
		f.writer() << "D3D_FEATURE_LEVEL_11_0";
		break;
	case D3D_FEATURE_LEVEL_11_1:
		f.writer() << "D3D_FEATURE_LEVEL_11_1";
		break;
	}
}

struct RenderingDevice::Impl {

	Impl() {
		for (auto &state : currentSamplerState) {
			state = nullptr;
		}
	}

	// Anti Aliasing Settings
	bool antiAliasing = false;
	bool vsync = true;
	uint32_t msaaSamples = 4;
	uint32_t msaaQuality = 0;

	// Caches for cursors
	eastl::map<std::string, HCURSOR> cursorCache;
	HCURSOR currentCursor = nullptr;

	// Caches for created device states
	std::array<const SamplerState*, 4> currentSamplerState;
	eastl::map<SamplerSpec, SamplerStatePtr> samplerStates;

	const DepthStencilState *currentDepthStencilState = nullptr;
	eastl::map<DepthStencilSpec, DepthStencilStatePtr> depthStencilStates;

	const BlendState* currentBlendState = nullptr;
	eastl::map<BlendSpec, BlendStatePtr> blendStates;

	const RasterizerState *currentRasterizerState = nullptr;
	eastl::map<RasterizerSpec, RasterizerStatePtr> rasterizerStates;

	// Debugging related
	bool debugDevice = false;
	CComPtr<ID3DUserDefinedAnnotation> annotation;

	// Text rendering (Direct2D integration)
	std::unique_ptr<TextEngine> textEngine;
};

RenderingDevice::RenderingDevice(HWND windowHandle, uint32_t adapterIdx, bool debugDevice)
    : mWindowHandle(windowHandle), mShaders(*this),
      mTextures(*this, 128 * 1024 * 1024), mImpl(std::make_unique<Impl>()) {
  Expects(!renderingDevice);
  renderingDevice = this;
  
  mImpl->debugDevice = debugDevice;

  mDefaultCamera = std::make_shared<WorldCamera>();
  mCurrentCamera = mDefaultCamera;

  HRESULT status;

  status = D3DLOG(
      CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **)(&mDxgiFactory)));
  if (!SUCCEEDED(status)) {
    throw TempleException("Cannot create DXGI factory.");
  }

  auto displayDevices = GetDisplayDevices();

  // Find the adapter selected by the user, although we might fall back to the
  // default one
  // if the user didn't select one or the adapter selection changed
  mAdapter = GetAdapter(adapterIdx);
  if (!mAdapter) {
    // Fall back to default
    logger->error("Couldn't retrieve adapter #{}. Falling back to default", 0);
    mAdapter = GetAdapter(displayDevices[0].id);
    if (!mAdapter) {
      throw TempleException(
          R"(Couldn't retrieve your configured graphics adapter, 
					but also couldn't fall back to the default adapter.)");
    }
  }

  // Required for the Direct2D support
  uint32_t deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
  if (debugDevice) {
    deviceFlags |=
        D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT;
  }

  // Try creating a D3D11.1 device first (won't work on Vista and Win7 without
  // SP2)
  eastl::fixed_vector<D3D_FEATURE_LEVEL, 7> requestedLevels{
      D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_3,  D3D_FEATURE_LEVEL_9_2,
      D3D_FEATURE_LEVEL_9_1};

  status = D3D11CreateDevice(mAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL,
                             deviceFlags, requestedLevels.data(),
                             requestedLevels.size(), D3D11_SDK_VERSION,
                             &mD3d11Device, &mFeatureLevel, &mContext);

  if (status == DXGI_ERROR_INVALID_CALL) {
    logger->info("D3D11.1 doesn't seem to be available on this system.");

    status = D3D11CreateDevice(mAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL,
                               deviceFlags, &requestedLevels[1],
                               requestedLevels.size() - 1, D3D11_SDK_VERSION,
                               &mD3d11Device, &mFeatureLevel, &mContext);
  }

  // Handle some special codes
  if (debugDevice && status == DXGI_ERROR_SDK_COMPONENT_MISSING) {
    throw TempleException("To use the D3D debugging feature, you need to "
                          "install the corresponding Windows SDK component.");
  }

  if (!SUCCEEDED(D3DLOG(status))) {
    throw TempleException("Unable to create a Direct3D 11 device.");
  }

  logger->info("Created D3D11 device with feature level {}", mFeatureLevel);

  // Retrieve the interface used to emit event groupings for debugging
  if (debugDevice) {
	  mContext.QueryInterface(&mImpl->annotation);
  }

  // Retrieve DXGI device
  CComPtr<IDXGIDevice> dxgiDevice;
  if (!SUCCEEDED(mD3d11Device.QueryInterface(&dxgiDevice))) {
    throw TempleException("Couldn't retrieve DXGI device from D3D11 device.");
  }
  CComPtr<IDXGIAdapter> dxgiAdapter;
  if (!SUCCEEDED(dxgiDevice->GetParent(__uuidof(IDXGIAdapter),
                                       (void **)&dxgiAdapter))) {
    throw TempleException("Couldn't retrieve DXGI adapter from DXGI device.");
  }
  CComPtr<IDXGIFactory> dxgiFactory;
  if (!SUCCEEDED(dxgiAdapter->GetParent(__uuidof(IDXGIFactory),
                                        (void **)&dxgiFactory))) {
    throw TempleException("Couldn't retrieve DXGI factory from DXGI adapter.");
  }
  mDxgiFactory = dxgiFactory; // Hang on to the DXGI factory used here

  // Create 2D rendering
  mImpl->textEngine = std::make_unique<TextEngine>(mD3d11Device, debugDevice);

  if (windowHandle) {
	  memset(&mSwapChainDesc, 0, sizeof(mSwapChainDesc));
	  mSwapChainDesc.BufferCount = 2;
	  mSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	  mSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	  mSwapChainDesc.OutputWindow = windowHandle;
	  mSwapChainDesc.SampleDesc.Count = 1;
	  mSwapChainDesc.Windowed =
		  TRUE; // As per the recommendation, we always create windowed

	  if (!SUCCEEDED(D3DLOG(dxgiFactory->CreateSwapChain(
		  mD3d11Device, &mSwapChainDesc, &mSwapChain)))) {
		  throw TempleException("Unable to create swap chain");
	  }

	  // Get the backbuffer from the swap chain
	  CComPtr<ID3D11Texture2D> backBufferTexture;
	  D3DVERIFY(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		  (void **)&backBufferTexture));

	  mBackBufferNew = CreateRenderTargetForNativeSurface(backBufferTexture);
	  auto &backBufferSize = mBackBufferNew->GetSize();
	  mBackBufferDepthStencil = CreateRenderTargetDepthStencil(backBufferSize.width, backBufferSize.height);

	  // Push back the initial render target that should never be removed
	  PushBackBufferRenderTarget();
  }

  // Create centralized constant buffers for the vertex and pixel shader stages
  mVsConstantBuffer = CreateConstantBuffer(nullptr, MaxVsConstantBufferSize);
  mPsConstantBuffer = CreateConstantBuffer(nullptr, MaxPsConstantBufferSize);
  
  // TODO: color bullshit is not yet done (tig_d3d_init_handleformat et al)
  
  for (auto &listener : mResourcesListeners) {
    listener->CreateResources(*this);
  }
  mResourcesCreated = true;
}

RenderingDevice::~RenderingDevice() { renderingDevice = nullptr; }

void RenderingDevice::SetCurrentCamera(WorldCameraPtr camera)
{
	mCurrentCamera = camera ? camera : mDefaultCamera;
}

void RenderingDevice::SetAntiAliasing(bool enable, uint32_t samples, uint32_t quality) {

	mImpl->msaaQuality = quality;
	mImpl->msaaSamples = samples;

  if (mImpl->antiAliasing != enable) {
	  mImpl->antiAliasing = enable;
	
	// Recreate all rasterizer states to set the multisampling flag accordingly
	for (auto &entry : mImpl->rasterizerStates) {
		auto gpuState = entry.second->mGpuState;
		D3D11_RASTERIZER_DESC gpuDesc;
		gpuState->GetDesc(&gpuDesc);

		gpuDesc.MultisampleEnable = enable ? TRUE : FALSE;

		entry.second->mGpuState.Release();
		mD3d11Device->CreateRasterizerState(&gpuDesc, &entry.second->mGpuState);
	}
  }
}

void RenderingDevice::SetVSync(bool enable) {
  mImpl->vsync = enable;
}

void RenderingDevice::UpdateResource(ID3D11Resource *resource, const void *data,
                                     size_t size) {
  D3D11_MAPPED_SUBRESOURCE mapped;
  D3DVERIFY(mContext->Map(resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));

  memcpy(mapped.pData, data, size);

  mContext->Unmap(resource, 0);
}

void RenderingDevice::UpdateBuffer(VertexBuffer &buffer, const void *data,
                                   size_t size) {
  UpdateResource(buffer.mBuffer, data, size);
}

void RenderingDevice::UpdateBuffer(IndexBuffer &buffer,
                                   gsl::span<uint16_t> data) {
  UpdateResource(buffer.mBuffer, data.data(), data.size_bytes());
}

static D3D11_MAP ConvertMapMode(gfx::MapMode mapMode) {
  switch (mapMode) {
  case gfx::MapMode::Read:
	  return D3D11_MAP_READ;
  case gfx::MapMode::Discard:
    return D3D11_MAP_WRITE_DISCARD;
  case gfx::MapMode::NoOverwrite:
    return D3D11_MAP_WRITE_NO_OVERWRITE;
  default:
    throw TempleException("Unknown map type");
  }
}

MappedIndexBuffer RenderingDevice::Map(IndexBuffer &buffer, gfx::MapMode mode) {
  auto mapMode = ConvertMapMode(mode);

  D3D11_MAPPED_SUBRESOURCE mapped;
  D3DVERIFY(mContext->Map(buffer.mBuffer, 0, mapMode, 0, &mapped));
  auto data = gsl::span((uint16_t *)mapped.pData, buffer.mCount);

  return MappedIndexBuffer(buffer, *this, data, 0);
}

void RenderingDevice::Unmap(IndexBuffer &buffer) {
  mContext->Unmap(buffer.mBuffer, 0);
}

MappedTexture RenderingDevice::Map(DynamicTexture &texture, gfx::MapMode mode) {
  auto mapMode = ConvertMapMode(mode);

  D3D11_MAPPED_SUBRESOURCE mapped;
  D3DVERIFY(mContext->Map(texture.mTexture, 0, mapMode, 0, &mapped));
  auto size = texture.GetSize().width * texture.GetSize().height *
              texture.GetBytesPerPixel();
  auto data = gsl::span((uint8_t *)mapped.pData, size);
  auto rowPitch = mapped.RowPitch;

  return MappedTexture(texture, *this, data, rowPitch);
}

void RenderingDevice::Unmap(DynamicTexture &texture) {
  mContext->Unmap(texture.mTexture, 0);
}

gsl::span<uint8_t> RenderingDevice::MapVertexBufferRaw(VertexBuffer &buffer,
                                                       MapMode mode) {
  auto mapMode = ConvertMapMode(mode);

  D3D11_MAPPED_SUBRESOURCE mapped;
  D3DVERIFY(mContext->Map(buffer.mBuffer, 0, mapMode, 0, &mapped));

  return gsl::span((uint8_t *)mapped.pData, buffer.mSize);
}

void RenderingDevice::Unmap(VertexBuffer &buffer) {
  mContext->Unmap(buffer.mBuffer, 0);
}

const gfx::Size &RenderingDevice::GetBackBufferSize() const
{
	return mBackBufferNew->GetSize();
}

void RenderingDevice::PushBackBufferRenderTarget() {
  PushRenderTarget(mBackBufferNew, mBackBufferDepthStencil);
}

void RenderingDevice::PushRenderTarget(
    const gfx::RenderTargetTexturePtr &colorBuffer,
    const gfx::RenderTargetDepthStencilPtr &depthStencilBuffer) {
  // If a depth stencil surface is to be used, it HAS to be the same size
  assert(!depthStencilBuffer ||
         colorBuffer->GetSize() == depthStencilBuffer->GetSize());

  // Activate the render target on the device
  auto rtv = colorBuffer->mRtView;
  ID3D11DepthStencilView *depthStencilView = nullptr; // Optional!
  if (depthStencilBuffer) {
    depthStencilView = depthStencilBuffer->mDsView;
  }

  mContext->OMSetRenderTargets(1, &rtv.p, depthStencilView);
  mImpl->textEngine->SetRenderTarget(colorBuffer->mTexture);

  // Set the viewport accordingly
  auto& size = colorBuffer->GetSize();
  CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(size.width), static_cast<float>(size.height));
  mContext->RSSetViewports(1, &viewport);

  mRenderTargetStack.push_back({colorBuffer, depthStencilBuffer});

  ResetScissorRect();

  UpdateDefaultCameraScreenSize();

}

void RenderingDevice::PopRenderTarget() {

  // The last targt should NOT be popped, if the backbuffer was auto-pushed
	if (mBackBufferNew) {
		assert(mRenderTargetStack.size() > 1);
	}

  mRenderTargetStack.pop_back(); // Remove ref to last target

  if (mRenderTargetStack.empty()) {
	  mContext->OMSetRenderTargets(0, nullptr, nullptr);
	  mImpl->textEngine->SetRenderTarget(nullptr);
	  return;	  
  }

  auto &newTarget = mRenderTargetStack.back();

  // Activate the render target on the device
  auto rtv = newTarget.colorBuffer->mRtView;
  ID3D11DepthStencilView *depthStencilView = nullptr; // Optional!
  if (newTarget.depthStencilBuffer) {
    depthStencilView = newTarget.depthStencilBuffer->mDsView;
  }

  mContext->OMSetRenderTargets(1, &rtv.p, depthStencilView);
  mImpl->textEngine->SetRenderTarget(newTarget.colorBuffer->mTexture);

  // Set the viewport accordingly
  auto& size = newTarget.colorBuffer->GetSize();
  CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(size.width), static_cast<float>(size.height));
  mContext->RSSetViewports(1, &viewport);

  ResetScissorRect();
  
  UpdateDefaultCameraScreenSize();

}

ResizeListenerRegistration RenderingDevice::AddResizeListener(ResizeListener listener)
{
	auto newKey = ++mResizeListenersKey;	
	mResizeListeners[newKey] = listener;
	return ResizeListenerRegistration(*this, newKey);
}

bool RenderingDevice::IsDebugDevice() const
{
	return mImpl->debugDevice;
}

void RenderingDevice::BeginPerfGroupInternal(const char * msg) const
{
	if (mImpl->annotation) {
		mImpl->annotation->BeginEvent(local_to_ucs2(msg).c_str());
	}
}

void RenderingDevice::EndPerfGroup() const {
	if (mImpl->debugDevice && mImpl->annotation) {
		mImpl->annotation->EndEvent();
	}
}

TextEngine & RenderingDevice::GetTextEngine() const
{
	return *mImpl->textEngine;
}

void RenderingDevice::RemoveResizeListener(uint32_t key)
{
	mResizeListeners.erase(key);
}

void RenderingDevice::AddResourceListener(ResourceListener *listener) {
  mResourcesListeners.push_back(listener);
  if (mResourcesCreated) {
    listener->CreateResources(*this);
  }
}

void RenderingDevice::RemoveResourceListener(ResourceListener *listener) {
  mResourcesListeners.remove(listener);
  if (mResourcesCreated) {
    listener->FreeResources(*this);
  }
}

CComPtr<ID3D11Buffer>
RenderingDevice::CreateConstantBuffer(const void *initialData,
                                      size_t initialDataSize) {

  CD3D11_BUFFER_DESC bufferDesc(initialDataSize, D3D11_BIND_CONSTANT_BUFFER,
                                D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

  D3D11_SUBRESOURCE_DATA *ptrSubresourceData = nullptr;
  D3D11_SUBRESOURCE_DATA subresourceData;

  if (initialData) {
    ptrSubresourceData = &subresourceData;
    subresourceData.pSysMem = initialData;
  }

  CComPtr<ID3D11Buffer> buffer;
  D3DVERIFY(
      mD3d11Device->CreateBuffer(&bufferDesc, ptrSubresourceData, &buffer));
  return buffer;
}

void RenderingDevice::PSSetConstantBuffer(uint32_t slot, ID3D11Buffer *buffer) {
  mContext->PSSetConstantBuffers(slot, 1, &buffer);
}

void RenderingDevice::VSSetConstantBuffer(uint32_t slot, ID3D11Buffer *buffer) {
  mContext->VSSetConstantBuffers(slot, 1, &buffer);
}

bool RenderingDevice::BeginFrame() {

  if (mBeginSceneDepth++ > 0) {
    return true;
  }

  ClearCurrentColorTarget(XMCOLOR(0, 0, 0, 1));
  ClearCurrentDepthTarget();

  mLastFrameStart = Clock::now();

  return true;
}

bool RenderingDevice::Present() {
  if (--mBeginSceneDepth > 0) {
    return true;
  }

  PresentForce();

  return true;
}

void RenderingDevice::PresentForce() {
  mTextures.FreeUnusedTextures();

  /*auto result = mDevice->Present(nullptr, nullptr, nullptr, nullptr);

  if (result != S_OK && result != S_PRESENT_OCCLUDED) {
  LogD3dError("Present()", result);
  if (result == D3DERR_DEVICELOST || result == S_PRESENT_MODE_CHANGED) {
  ResetDevice();
  }
  return false;
  }*/

  D3DLOG(mSwapChain->Present(mImpl->vsync, 0));
}

void RenderingDevice::Flush()
{
	mContext->Flush();
}

void RenderingDevice::ClearCurrentColorTarget(XMCOLOR color) {

  auto &target = GetCurrentRederTargetColorBuffer();

  // Clear the current render target view
  XMFLOAT4 clearColorVec;
  XMStoreFloat4(&clearColorVec, XMLoadColor(&color));

  mContext->ClearRenderTargetView(target->mRtView, &clearColorVec.x);
}

void RenderingDevice::ClearCurrentDepthTarget(bool clearDepth,
                                              bool clearStencil,
                                              float depthValue,
                                              uint8_t stencilValue) {
  if (!clearDepth && !clearStencil) {
    return;
  }

  int flags = 0;
  if (clearDepth) {
    flags |= D3D11_CLEAR_DEPTH;
  }
  if (clearStencil) {
    flags |= D3D11_CLEAR_STENCIL;
  }

  auto &depthStencil = GetCurrentRenderTargetDepthStencilBuffer();

  if (!depthStencil) {
    logger->warn(
        "Trying to clear current depthstencil view, but none is bound.");
    return;
  }

  mContext->ClearDepthStencilView(depthStencil->mDsView, flags, depthValue,
                                  stencilValue);
}

CComPtr<IDXGIAdapter1> RenderingDevice::GetAdapter(size_t index) {
  CComPtr<IDXGIAdapter1> adapter;
  mDxgiFactory->EnumAdapters1(index, &adapter);
  return adapter;
}

void RenderingDevice::UpdateDefaultCameraScreenSize()
{
	auto& currentTarget = mRenderTargetStack.back();
	auto& currentSize = currentTarget.colorBuffer->GetSize();
	mDefaultCamera->SetScreenSize((float) currentSize.width, (float) currentSize.height);
}

const eastl::vector<DisplayDevice> &RenderingDevice::GetDisplayDevices() {
  // Recreate the DXGI factory if we want to enumerate a new list of devices
  if (!mDisplayDevices.empty() && mDxgiFactory->IsCurrent()) {
    return mDisplayDevices;
  }

  // Enumerate devices
  logger->info("Enumerating DXGI display devices...");

  mDisplayDevices.clear();

  for (uint32_t adapterIdx = 0;; adapterIdx++) {
    CComPtr<IDXGIAdapter1> adapter;
    if (mDxgiFactory->EnumAdapters1(adapterIdx, &adapter) ==
        DXGI_ERROR_NOT_FOUND) {
      break;
    }

    // Get an adapter descriptor
    DXGI_ADAPTER_DESC1 adapterDesc;
    if (!SUCCEEDED(adapter->GetDesc1(&adapterDesc))) {
      logger->warn("Unable to retrieve DXGI description of adapter #{}",
                   adapterIdx);
      continue;
    }

    DisplayDevice displayDevice;

    displayDevice.name = ucs2_to_local(&adapterDesc.Description[0]);
    logger->info("Adapter #{} '{}'", adapterIdx, displayDevice.name);

    // Enumerate all outputs of the adapter
    for (uint32_t outputIdx = 0;; outputIdx++) {
      CComPtr<IDXGIOutput> output;
      if (!SUCCEEDED(adapter->EnumOutputs(outputIdx, &output))) {
        break;
      }

      DXGI_OUTPUT_DESC outputDesc;
      if (!SUCCEEDED(D3DLOG(output->GetDesc(&outputDesc)))) {
        continue;
      }

      auto deviceName = ucs2_to_local(&outputDesc.DeviceName[0]);

      MONITORINFOEXW monInfoEx;
      monInfoEx.cbSize = sizeof(MONITORINFOEXW);
      if (!GetMonitorInfoW(outputDesc.Monitor, &monInfoEx)) {
        logger->warn("Could not get monitor info.");
        continue;
      }

      DISPLAY_DEVICEW dispDev;
      dispDev.cb = sizeof(DISPLAY_DEVICEW);
      if (!EnumDisplayDevicesW(monInfoEx.szDevice, 0, &dispDev, 0)) {
        logger->warn("Could not enumerate display devices for monitor.");
        continue;
      }

      DisplayDeviceOutput displayOutput;
      displayOutput.id = deviceName;
      displayOutput.name = ucs2_to_local(&dispDev.DeviceString[0]);
      logger->info("  Output #{} Device '{}' Monitor '{}'", outputIdx,
                   deviceName, displayOutput.name);
      displayDevice.outputs.emplace_back(std::move(displayOutput));
    }

    if (!displayDevice.outputs.empty()) {
      mDisplayDevices.emplace_back(std::move(displayDevice));
    } else {
      logger->info("Skipping device because it has no outputs.");
    }
  }

  return mDisplayDevices;
}

void RenderingDevice::SetMaterial(const Material &material) {

  SetRasterizerState(material.GetRasterizerState());
  SetBlendState(material.GetBlendState());
  SetDepthStencilState(material.GetDepthStencilState());

  for (size_t i = 0; i < material.GetSamplers().size(); ++i) {
    auto &sampler = material.GetSamplers()[i];
    if (sampler.GetTexture()) {
      SetTexture(i, *sampler.GetTexture());
    } else {
      SetTexture(i, *gfx::Texture::GetInvalidTexture());
    }
    SetSamplerState(i, sampler.GetState());
  }

  // Free up the texture bindings of the samplers currently being used
  for (size_t i = material.GetSamplers().size(); i < mUsedSamplers; ++i) {
    SetTexture(i, *gfx::Texture::GetInvalidTexture());
  }

  mUsedSamplers = material.GetSamplers().size();

  material.GetVertexShader()->Bind();
  material.GetPixelShader()->Bind();
}

void RenderingDevice::SetVertexShaderConstant(uint32_t startRegister,
                                              StandardSlotSemantic semantic) {
  switch (semantic) {
  case StandardSlotSemantic::ViewProjMatrix:
    SetVertexShaderConstants(startRegister, mCurrentCamera->GetViewProj());
    break;
  case StandardSlotSemantic::UiProjMatrix:
    SetVertexShaderConstants(startRegister, mCurrentCamera->GetUiProjection());
	break;
  default:
    break;
  }
}

void RenderingDevice::SetPixelShaderConstant(uint32_t startRegister,
                                             StandardSlotSemantic semantic) {
  switch (semantic) {
  case StandardSlotSemantic::ViewProjMatrix:
    SetPixelShaderConstants(startRegister, mCurrentCamera->GetViewProj());
    break;
  case StandardSlotSemantic::UiProjMatrix:
    SetPixelShaderConstants(startRegister, mCurrentCamera->GetUiProjection());
    break;
  default:
    break;
  }
}

Material RenderingDevice::CreateMaterial(
    const BlendSpec &blendSpec, const DepthStencilSpec &depthStencilSpec,
    const RasterizerSpec &rasterizerSpec,
    const std::vector<MaterialSamplerSpec> &samplerSpecs,
    const VertexShaderPtr &vs, const PixelShaderPtr &ps) {

  auto blendState = CreateBlendState(blendSpec);
  auto depthStencilState = CreateDepthStencilState(depthStencilSpec);
  auto rasterizerState = CreateRasterizerState(rasterizerSpec);

  std::vector<MaterialSamplerBinding> samplerBindings;
  samplerBindings.reserve(samplerSpecs.size());
  for (auto &samplerSpec : samplerSpecs) {
    samplerBindings.push_back(
        {samplerSpec.texture, CreateSamplerState(samplerSpec.samplerSpec)});
  }

  return Material(blendState, depthStencilState, rasterizerState,
                  samplerBindings, vs, ps);
}

static D3D11_BLEND ConvertBlendOperand(BlendOperand op) {
	switch (op) {
	case BlendOperand::Zero:
		return D3D11_BLEND_ZERO;
	case BlendOperand::One:
		return D3D11_BLEND_ONE;
	case BlendOperand::SrcColor:
		return D3D11_BLEND_SRC_COLOR;
	case BlendOperand::InvSrcColor:
		return D3D11_BLEND_INV_SRC_COLOR;
	case BlendOperand::SrcAlpha:
		return D3D11_BLEND_SRC_ALPHA;
	case BlendOperand::InvSrcAlpha:
		return D3D11_BLEND_INV_SRC_ALPHA;
	case BlendOperand::DestAlpha:
		return D3D11_BLEND_DEST_ALPHA;
	case BlendOperand::InvDestAlpha:
		return D3D11_BLEND_INV_DEST_ALPHA;
	case BlendOperand::DestColor:
		return D3D11_BLEND_DEST_COLOR;
	case BlendOperand::InvDestColor:
		return D3D11_BLEND_INV_DEST_COLOR;
	default:
		throw TempleException("Unknown blend operand.");
	}
}

BlendStatePtr RenderingDevice::CreateBlendState(const BlendSpec &spec) {
  
	// Check if we have a matching state already
	auto it = mImpl->blendStates.find(spec);

	if (it != mImpl->blendStates.end()) {
		return it->second;
	}

  CD3D11_BLEND_DESC blendDesc{CD3D11_DEFAULT()};

  auto &targetDesc = blendDesc.RenderTarget[0];
  targetDesc.BlendEnable = spec.blendEnable ? TRUE : FALSE;
  targetDesc.SrcBlend = ConvertBlendOperand(spec.srcBlend);
  targetDesc.DestBlend = ConvertBlendOperand(spec.destBlend);
  targetDesc.SrcBlendAlpha = ConvertBlendOperand(spec.srcAlphaBlend);
  targetDesc.DestBlendAlpha = ConvertBlendOperand(spec.destAlphaBlend);

  uint8_t writeMask = 0;
  // Never overwrite the alpha channel with random stuff when blending is disabled
  if (spec.writeAlpha && targetDesc.BlendEnable) {
    writeMask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
  }
  if (spec.writeRed) {
    writeMask |= D3D11_COLOR_WRITE_ENABLE_RED;
  }
  if (spec.writeGreen) {
    writeMask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
  }
  if (spec.writeBlue) {
    writeMask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
  }
  targetDesc.RenderTargetWriteMask = writeMask;

  CComPtr<ID3D11BlendState> gpuState;
  D3DVERIFY(mD3d11Device->CreateBlendState(&blendDesc, &gpuState));

  auto state = std::make_shared<BlendState>(spec, gpuState);
  mImpl->blendStates.insert({ spec, state });
  return state;
}

static D3D11_COMPARISON_FUNC ConvertComparisonFunc(ComparisonFunc func) {
  switch (func) {
  case ComparisonFunc::Never:
    return D3D11_COMPARISON_NEVER;
  case ComparisonFunc::Less:
    return D3D11_COMPARISON_LESS;
  case ComparisonFunc::Equal:
    return D3D11_COMPARISON_EQUAL;
  case ComparisonFunc::LessEqual:
    return D3D11_COMPARISON_LESS_EQUAL;
  case ComparisonFunc::Greater:
    return D3D11_COMPARISON_GREATER;
  case ComparisonFunc::NotEqual:
    return D3D11_COMPARISON_NOT_EQUAL;
  case ComparisonFunc::GreaterEqual:
    return D3D11_COMPARISON_GREATER_EQUAL;
  case ComparisonFunc::Always:
    return D3D11_COMPARISON_ALWAYS;
  default:
	  throw TempleException("Unknown comparison func.");
  }
}

DepthStencilStatePtr
RenderingDevice::CreateDepthStencilState(const DepthStencilSpec &spec) {
	
	// Check if we have a matching state already
	auto it = mImpl->depthStencilStates.find(spec);

	if (it != mImpl->depthStencilStates.end()) {
		return it->second;
	}

  CD3D11_DEPTH_STENCIL_DESC depthStencilDesc{CD3D11_DEFAULT()};
  depthStencilDesc.DepthEnable = spec.depthEnable ? TRUE : FALSE;
  depthStencilDesc.DepthWriteMask = spec.depthWrite
                                        ? D3D11_DEPTH_WRITE_MASK_ALL
                                        : D3D11_DEPTH_WRITE_MASK_ZERO;

  depthStencilDesc.DepthFunc = ConvertComparisonFunc(spec.depthFunc);

  CComPtr<ID3D11DepthStencilState> gpuState;

  D3DVERIFY(
      mD3d11Device->CreateDepthStencilState(&depthStencilDesc, &gpuState));

  auto state = std::make_shared<DepthStencilState>(spec, gpuState);
  mImpl->depthStencilStates.insert({ spec, state });
  return state;
}

RasterizerStatePtr
RenderingDevice::CreateRasterizerState(const RasterizerSpec &spec) {

 // Check if we have a matching state already
	auto it = mImpl->rasterizerStates.find(spec);

	if (it != mImpl->rasterizerStates.end()) {
		return it->second;
	}

  CD3D11_RASTERIZER_DESC rasterizerDesc{CD3D11_DEFAULT()};
  if (spec.wireframe) {
    rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
  }
  switch (spec.cullMode) {
  case CullMode::Back:
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    break;
  case CullMode::Front:
    rasterizerDesc.CullMode = D3D11_CULL_FRONT;
    break;
  case CullMode::None:
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    break;
  }

  rasterizerDesc.ScissorEnable = spec.scissor ? TRUE : FALSE;

  rasterizerDesc.MultisampleEnable = mImpl->antiAliasing ? TRUE : FALSE;

  CComPtr<ID3D11RasterizerState> gpuState;

  D3DVERIFY(mD3d11Device->CreateRasterizerState(&rasterizerDesc, &gpuState));

  auto state = std::make_shared<RasterizerState>(spec, gpuState);
  mImpl->rasterizerStates.insert({spec, state});
  return state;

}

static D3D11_TEXTURE_ADDRESS_MODE ConvertTextureAddress(TextureAddress address) {
	switch (address) {
	case TextureAddress::Clamp:
		return D3D11_TEXTURE_ADDRESS_CLAMP;
	case TextureAddress::Wrap:
		return D3D11_TEXTURE_ADDRESS_WRAP;
	default:
		throw TempleException("Unknown texture address mode.");
	}
}

SamplerStatePtr RenderingDevice::CreateSamplerState(const SamplerSpec &spec) {

  CD3D11_SAMPLER_DESC samplerDesc{CD3D11_DEFAULT()};

  // we only support mapping point + linear
  bool minPoint = (spec.minFilter == TextureFilterType::NearestNeighbor);
  bool magPoint = (spec.magFilter == TextureFilterType::NearestNeighbor);
  bool mipPoint = (spec.mipFilter == TextureFilterType::NearestNeighbor);

  // This is a truth table for all possible values represented above
  if (!minPoint && !magPoint && !mipPoint) {
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  } else if (!minPoint && !magPoint && mipPoint) {
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
  } else if (!minPoint && magPoint && !mipPoint) {
    samplerDesc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
  } else if (!minPoint && magPoint && mipPoint) {
    samplerDesc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
  } else if (minPoint && !magPoint && !mipPoint) {
    samplerDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
  } else if (minPoint && !magPoint && mipPoint) {
    samplerDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
  } else if (minPoint && magPoint && !mipPoint) {
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
  } else if (minPoint && magPoint && mipPoint) {
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
  }

  samplerDesc.AddressU = ConvertTextureAddress(spec.addressU);
  samplerDesc.AddressV = ConvertTextureAddress(spec.addressV);

  CComPtr<ID3D11SamplerState> gpuState;

  D3DVERIFY(mD3d11Device->CreateSamplerState(&samplerDesc, &gpuState));

  auto state = std::make_shared<SamplerState>(spec, gpuState);
  mImpl->samplerStates.insert({ spec, state });
  return state;
}

IndexBufferPtr RenderingDevice::CreateEmptyIndexBuffer(size_t count) {
	CD3D11_BUFFER_DESC bufferDesc(
		count * sizeof(uint16_t),
		D3D11_BIND_INDEX_BUFFER,
		D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_WRITE
	);

  CComPtr<ID3D11Buffer> buffer;
  D3DVERIFY(mD3d11Device->CreateBuffer(&bufferDesc, nullptr, &buffer));

  return std::make_shared<IndexBuffer>(buffer, count);
}

VertexBufferPtr RenderingDevice::CreateEmptyVertexBuffer(size_t size,
                                                         bool forPoints) {
  // Create a dynamic vertex buffer since it'll be updated (probably a lot)
	CD3D11_BUFFER_DESC bufferDesc(
		size,
		D3D11_BIND_VERTEX_BUFFER,
		D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_WRITE
	);

  CComPtr<ID3D11Buffer> buffer;
  D3DVERIFY(mD3d11Device->CreateBuffer(&bufferDesc, nullptr, &buffer));

  return std::make_shared<VertexBuffer>(buffer, size);
}

void RenderingDevice::SetRasterizerState(const RasterizerState &state) {
	if (mImpl->currentRasterizerState == &state) {
		return; // Already set
	}
	mImpl->currentRasterizerState = &state;
	mContext->RSSetState(state.mGpuState);
}

void RenderingDevice::SetBlendState(const BlendState &state) {
	if (mImpl->currentBlendState == &state) {
		return; // Already set
	}
	mImpl->currentBlendState = &state;
	mContext->OMSetBlendState(state.mGpuState, nullptr, 0xFFFFFFFF);
}

void RenderingDevice::SetDepthStencilState(const DepthStencilState &state) {
	if (mImpl->currentDepthStencilState == &state) {
		return; // Already set
	}
	mImpl->currentDepthStencilState = &state;
  mContext->OMSetDepthStencilState(state.mGpuState, 0);
}

void RenderingDevice::SetSamplerState(int samplerIdx,
                                      const SamplerState &state) {

	auto &curSampler = mImpl->currentSamplerState[samplerIdx];
	if (curSampler == &state) {
		return; // Already set
	}
	curSampler = &state;

  ID3D11SamplerState *sampler = state.mGpuState;
  mContext->PSSetSamplers(samplerIdx, 1, &sampler);
}

void RenderingDevice::SetTexture(uint32_t slot, gfx::Texture &texture) {

	// If we are binding a multisample render target, we automatically resolve the MSAA to use
	// a non-MSAA texture like a normal texture
	if (texture.GetType() == TextureType::RenderTarget) {
		auto &rt = static_cast<RenderTargetTexture&>(texture);

		if (rt.IsMultiSampled()) {
			D3D11_TEXTURE2D_DESC mDesc;
			rt.mTexture->GetDesc(&mDesc);

			mContext->ResolveSubresource(rt.mResolvedTexture,
				0,
				rt.mTexture,
				0,
				mDesc.Format
			);
		}
	}

  // D3D11
  auto resourceView = texture.GetResourceView();
  mContext->PSSetShaderResources(slot, 1, &resourceView);
}

void RenderingDevice::SetIndexBuffer(const gfx::IndexBuffer &indexBuffer) {
  mContext->IASetIndexBuffer(indexBuffer.mBuffer, DXGI_FORMAT_R16_UINT, 0);
}

void RenderingDevice::Draw(PrimitiveType type, uint32_t vertexCount,
                           uint32_t startVertex) {
  D3D11_PRIMITIVE_TOPOLOGY primTopology;

  switch (type) {
  case PrimitiveType::TriangleStrip:
    primTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    break;
  case PrimitiveType::TriangleList:
    primTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    break;
  case PrimitiveType::LineStrip:
    primTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
    break;
  case PrimitiveType::LineList:
    primTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
    break;
  case PrimitiveType::PointList:
    primTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    break;
  default:
    throw TempleException("Unsupported primitive type");
  }

  mContext->IASetPrimitiveTopology(primTopology);
  mContext->Draw(vertexCount, startVertex);
}

void RenderingDevice::DrawIndexed(PrimitiveType type, uint32_t vertexCount,
                                  uint32_t indexCount, uint32_t startVertex,
                                  uint32_t vertexBase) {
  D3D11_PRIMITIVE_TOPOLOGY primTopology;

  switch (type) {
  case PrimitiveType::TriangleStrip:
    primTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    break;
  case PrimitiveType::TriangleList:
    primTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    break;
  case PrimitiveType::LineStrip:
    primTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
    break;
  case PrimitiveType::LineList:
    primTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
    break;
  case PrimitiveType::PointList:
    primTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    break;
  default:
    throw TempleException("Unsupported primitive type");
  }

  mContext->IASetPrimitiveTopology(primTopology);
  mContext->DrawIndexed(indexCount, startVertex, vertexBase);
}

void RenderingDevice::SetCursor(int hotspotX, int hotspotY,
                                const gfx::TextureRef &texture) {

	HCURSOR cursor;
	auto it = mImpl->cursorCache.find(texture->GetName());
	if (it == mImpl->cursorCache.end()) {
		auto textureData = vfs->ReadAsBinary(texture->GetName());
		cursor = gfx::LoadImageToCursor(textureData, hotspotX, hotspotY);
		mImpl->cursorCache[texture->GetName()] = cursor;
	} else {
		cursor = it->second;
	}
		
  SetClassLong(mWindowHandle, GCL_HCURSOR, (LONG)cursor);
  ::SetCursor(cursor);
  mImpl->currentCursor = cursor;
}

void RenderingDevice::ShowCursor() {
	if (mImpl->currentCursor) {
		SetClassLong(mWindowHandle, GCL_HCURSOR, (LONG)mImpl->currentCursor);
		::SetCursor(mImpl->currentCursor);
	}
}

void RenderingDevice::HideCursor() {
	SetClassLong(mWindowHandle, GCL_HCURSOR, (LONG)nullptr);
	::SetCursor(nullptr);
}

VertexBufferPtr
RenderingDevice::CreateVertexBufferRaw(gsl::span<const uint8_t> data,
                                       bool immutable) {
  // Create a dynamic or immutable vertex buffer depending on the immutable flag
	CD3D11_BUFFER_DESC bufferDesc(
		data.size_bytes(),
		D3D11_BIND_VERTEX_BUFFER,
		immutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DYNAMIC,
		immutable ? 0 : D3D11_CPU_ACCESS_WRITE
	);

  D3D11_SUBRESOURCE_DATA initData;
  initData.pSysMem = data.data();

  CComPtr<ID3D11Buffer> buffer;
  D3DVERIFY(mD3d11Device->CreateBuffer(&bufferDesc, &initData, &buffer));

  return std::make_shared<VertexBuffer>(buffer, data.size());
}

IndexBufferPtr
RenderingDevice::CreateIndexBuffer(gsl::span<const uint16_t> data,
                                   bool immutable) {
  CD3D11_BUFFER_DESC bufferDesc(
	  data.size_bytes(),
	  D3D11_BIND_INDEX_BUFFER,
	  immutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DYNAMIC,
	  immutable ? 0 : D3D11_CPU_ACCESS_WRITE
  );

  D3D11_SUBRESOURCE_DATA subresourceData;
  subresourceData.pSysMem = data.data();

  CComPtr<ID3D11Buffer> buffer;
  D3DVERIFY(mD3d11Device->CreateBuffer(&bufferDesc, &subresourceData, &buffer));

  return std::make_shared<IndexBuffer>(buffer, data.size());
}

void RenderingDevice::ResizeBuffers(int w, int h) {

	if (!mSwapChain) {
		return;
	}

	if (GetCurrentRederTargetColorBuffer() == mBackBufferNew) {
		mImpl->textEngine->SetRenderTarget(nullptr);
	}
	
	// Somewhat annoyingly, we have to release all existing buffers
	mBackBufferNew->mRtView.Release();
	mBackBufferNew->mTexture.Release();
	mBackBufferDepthStencil->mDsView.Release();
	mBackBufferDepthStencil->mTextureNew.Release();
	
	D3DVERIFY(mSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0));

	// Get the backbuffer from the swap chain
	CComPtr<ID3D11Texture2D> backBufferTexture;
	D3DVERIFY(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		(void **)&backBufferTexture));

	// Create a render target view for rendering to the real backbuffer
	CComPtr<ID3D11RenderTargetView> backBufferView;
	CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(backBufferTexture.p,
		D3D11_RTV_DIMENSION_TEXTURE2D);
	D3DVERIFY(mD3d11Device->CreateRenderTargetView(backBufferTexture, &rtvDesc,
		&backBufferView));

	D3D11_TEXTURE2D_DESC backBufferDesc;
	backBufferTexture->GetDesc(&backBufferDesc);
	gfx::Size backBufferSize{ (int)backBufferDesc.Width,
		(int)backBufferDesc.Height };
	
	// Update the back buffer render target
	mBackBufferNew->mTexture = backBufferTexture;
	mBackBufferNew->mSize = backBufferSize;
	mBackBufferNew->mContentRect = { 0, 0, backBufferSize.width, backBufferSize.height };
	mBackBufferNew->mRtView = backBufferView;
	
	// This works because the actual dx11 surfaces are independently reference counted
	auto newDs = CreateRenderTargetDepthStencil(backBufferSize.width, backBufferSize.height);
	mBackBufferDepthStencil->mDsView = newDs->mDsView;
	mBackBufferDepthStencil->mSize = newDs->mSize;
	mBackBufferDepthStencil->mTextureNew = newDs->mTextureNew;

	// Is the back buffer currently the active RT?
	if (GetCurrentRederTargetColorBuffer() == mBackBufferNew) {
		mRenderTargetStack.pop_back();
		PushBackBufferRenderTarget();
	}

	// Notice listeners about changed backbuffer size
	for (auto &entry : mResizeListeners) {
		entry.second(w, h);
	}

}

void RenderingDevice::TakeScaledScreenshot(const std::string &filename,
                                           int width, int height, int quality) {
  logger->debug("Creating screenshot with size {}x{} in {}", width, height,
                filename);

  auto &currentTarget = GetCurrentRederTargetColorBuffer();
  auto targetSize = currentTarget->GetSize();

  // Support taking unscaled screenshots
  auto stretch = true;
  if (width == 0 || height == 0) {
    width = targetSize.width;
    height = targetSize.height;
    stretch = false;
  }

  // Retrieve the backbuffer format...
  D3D11_TEXTURE2D_DESC currentTargetDesc;
  currentTarget->mTexture->GetDesc(&currentTargetDesc);

  // Create a staging surface for copying pixels back from the backbuffer
  // texture
  D3D11_TEXTURE2D_DESC stagingDesc = currentTargetDesc;
  stagingDesc.Width = width;
  stagingDesc.Height = height;
  stagingDesc.Usage = D3D11_USAGE_STAGING;
  stagingDesc.BindFlags = 0; // Not going to bind it at all
  stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  stagingDesc.MipLevels = 1;
  stagingDesc.ArraySize = 1;
  // Never use multi sampling for the screenshot
  stagingDesc.SampleDesc.Count = 1;
  stagingDesc.SampleDesc.Quality = 0;

  CComPtr<ID3D11Texture2D> stagingTex;
  D3DVERIFY(mD3d11Device->CreateTexture2D(&stagingDesc, nullptr, &stagingTex));
  
  if (stretch) {
	  // Create a default texture to copy the current RT to that we can use as a src for the blitting
	  CD3D11_TEXTURE2D_DESC tmpDesc(currentTargetDesc);
	  // Force MSAA off
	  tmpDesc.SampleDesc.Count = 1;
	  tmpDesc.SampleDesc.Quality = 0;
	  // Make it a default texture with binding as Shader Resource
	  tmpDesc.Usage = D3D11_USAGE_DEFAULT;
	  tmpDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	  CComPtr<ID3D11Texture2D> tmpTexture;
	  D3DVERIFY(mD3d11Device->CreateTexture2D(&tmpDesc, nullptr, &tmpTexture));

	  // Copy/resolve the current RT into the temp texture
	  if (currentTargetDesc.SampleDesc.Count > 1) {
		  mContext->ResolveSubresource(tmpTexture, 0, currentTarget->mTexture, 0, tmpDesc.Format);
	  } else {
		  mContext->CopyResource(tmpTexture, currentTarget->mTexture);
	  }

	  // Create the Shader Resource View that we can use to use the tmp texture for sampling in a shader
	  CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D);
	  CComPtr<ID3D11ShaderResourceView> srv;
	  D3DVERIFY(mD3d11Device->CreateShaderResourceView(tmpTexture, &srvDesc, &srv));

	  // Create our own wrapper so we can use the standard rendering functions
	  Size tmpSize { (int) currentTargetDesc.Width, (int) currentTargetDesc.Height };
	  DynamicTexture tmpTexWrapper{ mContext,
		  tmpTexture,
		  srv,
		  tmpSize,
		  4 };

	  // Create a texture the size of the target and stretch into it via a blt
	  // the target also needs to be a render target for that to work
	  auto stretchedRt = CreateRenderTargetTexture(currentTarget->GetFormat(), width, height);
	  
	  PushRenderTarget(stretchedRt, nullptr);
	  ShapeRenderer2d renderer(*this);
	  
	  auto w = mCurrentCamera->GetScreenWidth();
	  auto h = mCurrentCamera->GetScreenHeight();
	
	  renderer.DrawRectangle(0, 0, w, h, tmpTexWrapper);

	  PopRenderTarget();

	  // Copy our stretchted RT to the staging resource
	  mContext->CopyResource(stagingTex, stretchedRt->mTexture);

  } else {
	  // Resolve multi sampling if necessary
	  if (currentTargetDesc.SampleDesc.Count > 1) {
		  mContext->ResolveSubresource(stagingTex, 0, currentTarget->mTexture, 0, stagingDesc.Format);
	  }
	  else {
		  mContext->CopyResource(stagingTex, currentTarget->mTexture);
	  }
  }

  // Lock the resource and retrieve it
  D3D11_MAPPED_SUBRESOURCE mapped;
  D3DVERIFY(mContext->Map(stagingTex, 0, D3D11_MAP_READ, 0, &mapped));

  // Clamp quality to [1, 100]
  quality = std::min(100, std::max(1, quality));

  auto jpegData(gfx::EncodeJpeg(reinterpret_cast<uint8_t *>(mapped.pData),
                                gfx::JpegPixelFormat::BGRX, width, height,
                                quality, mapped.RowPitch));

  mContext->Unmap(stagingTex, 0);

  // We have to write using tio or else it goes god knows where
  try {
    vfs->WriteBinaryFile(filename, jpegData);
  } catch (std::exception &e) {
    logger->error("Unable to save screenshot due to an IO error: {}", e.what());
  }
}

BufferBinding RenderingDevice::CreateMdfBufferBinding(bool perVertexColor) {

  auto &vs = GetShaders().LoadVertexShader(
      "mdf_vs",
      {
          {"TEXTURE_STAGES", "1"}, // Necessary so the input struct gets the UVs
		  {"PER_VERTEX_COLOR", perVertexColor ? "1":"0"} // Enable per-vertex 
      });

  return BufferBinding(vs);
}


static DXGI_FORMAT ConvertFormat(gfx::BufferFormat format, uint32_t *bytesPerPixel) {
	DXGI_FORMAT formatNew;
	switch (format) {
	case BufferFormat::A8:
		formatNew = DXGI_FORMAT_R8_UNORM;
		*bytesPerPixel = 1;
		break;
	case BufferFormat::A8R8G8B8:
		formatNew = DXGI_FORMAT_B8G8R8A8_UNORM;
		*bytesPerPixel = 4;
		break;
	case BufferFormat::X8R8G8B8:
		formatNew = DXGI_FORMAT_B8G8R8X8_UNORM;
		*bytesPerPixel = 4;
		break;
	default:
		throw TempleException("Unsupported format: {}", format);
	}
	return formatNew;
}

gfx::DynamicTexturePtr
RenderingDevice::CreateDynamicTexture(gfx::BufferFormat format, int width,
                                      int height) {

  Size size{width, height};
    
  uint32_t bytesPerPixel;
  auto formatNew = ConvertFormat(format, &bytesPerPixel);

  CD3D11_TEXTURE2D_DESC textureDesc(
      formatNew, width, height, 1, 1, D3D11_BIND_SHADER_RESOURCE,
      D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

  CComPtr<ID3D11Texture2D> textureNew;
  D3DVERIFY(mD3d11Device->CreateTexture2D(&textureDesc, nullptr, &textureNew));

  CD3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc(
      textureNew, D3D11_SRV_DIMENSION_TEXTURE2D);
  CComPtr<ID3D11ShaderResourceView> resourceView;

  D3DVERIFY(mD3d11Device->CreateShaderResourceView(
      textureNew, &resourceViewDesc, &resourceView));

  return std::make_shared<DynamicTexture>(mContext, textureNew, resourceView,
                                          size, bytesPerPixel);
}

DynamicTexturePtr RenderingDevice::CreateDynamicStagingTexture(gfx::BufferFormat format, int width, int height)
{
	Size size{ width, height };

	uint32_t bytesPerPixel;
	auto formatNew = ConvertFormat(format, &bytesPerPixel);

	CD3D11_TEXTURE2D_DESC textureDesc(
		formatNew, width, height, 1, 1, 0,
		D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ);

	CComPtr<ID3D11Texture2D> textureNew;
	D3DVERIFY(mD3d11Device->CreateTexture2D(&textureDesc, nullptr, &textureNew));

	return std::make_shared<DynamicTexture>(mContext, textureNew, nullptr,
		size, bytesPerPixel);
}

void RenderingDevice::CopyRenderTarget(gfx::RenderTargetTexture & renderTarget, gfx::DynamicTexture & stagingTexture)
{
	mContext->CopyResource(stagingTexture.mTexture, renderTarget.mTexture);
}

RenderTargetTexturePtr
RenderingDevice::CreateRenderTargetTexture(gfx::BufferFormat format, int width, int height, bool multiSample) {

  Size size{width, height};

  uint32_t bpp;
  auto formatDx = ConvertFormat(format, &bpp);

  auto bindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
  auto sampleCount = 1;
  auto sampleQuality = 0;

  if (multiSample) {
	  logger->info("using multisampling");
	  // If this is a multi sample render target, we cannot use it as a texture, or at least, we shouldn't
	  bindFlags = D3D11_BIND_RENDER_TARGET;
	  sampleCount = mImpl->msaaSamples;
	  sampleQuality = mImpl->msaaQuality;
  } else 
	  logger->info("not using multisampling");

  logger->info("width {} height {}", width, height);
  CD3D11_TEXTURE2D_DESC textureDesc(formatDx, width, height, 1, 1, bindFlags, D3D11_USAGE_DEFAULT, 0, sampleCount, sampleQuality);

  CComPtr<ID3D11Texture2D> texture;
  D3DVERIFY(mD3d11Device->CreateTexture2D(&textureDesc, nullptr, &texture));

  // Create the render target view of the backing buffer
  CComPtr<ID3D11RenderTargetView> rtView;
  CD3D11_RENDER_TARGET_VIEW_DESC rtViewDesc(texture, D3D11_RTV_DIMENSION_TEXTURE2D);

  if (multiSample) {
	  rtViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
  }

  D3DVERIFY(mD3d11Device->CreateRenderTargetView(texture, &rtViewDesc, &rtView));

  ID3D11Texture2D *srvTexture = texture;
  CComPtr<ID3D11Texture2D> resolvedTexture;
  if (multiSample) {
	  // We have to create another non-multisampled texture and use it for the SRV instead

	  // Adapt the existing texture Desc to be a non-MSAA texture with otherwise identical properties
	  textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	  textureDesc.SampleDesc.Count = 1;
	  textureDesc.SampleDesc.Quality = 0;

	  D3DVERIFY(mD3d11Device->CreateTexture2D(&textureDesc, nullptr, &resolvedTexture));
	  srvTexture = resolvedTexture;
  }

  CD3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc(texture, D3D11_SRV_DIMENSION_TEXTURE2D);
  CComPtr<ID3D11ShaderResourceView> resourceView;
  D3DVERIFY(mD3d11Device->CreateShaderResourceView(srvTexture, &resourceViewDesc, &resourceView));

  return std::make_shared<RenderTargetTexture>(texture, rtView, resolvedTexture, resourceView, size, multiSample);
}

RenderTargetTexturePtr RenderingDevice::CreateRenderTargetForNativeSurface(ID3D11Texture2D * surface)
{
	// Create a render target view for rendering to the real backbuffer
	CComPtr<ID3D11RenderTargetView> backBufferView;
	CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(surface, D3D11_RTV_DIMENSION_TEXTURE2D);
	D3DVERIFY(mD3d11Device->CreateRenderTargetView(surface, &rtvDesc, &backBufferView));

	D3D11_TEXTURE2D_DESC backBufferDesc;
	surface->GetDesc(&backBufferDesc);

	gfx::Size size{ (int)backBufferDesc.Width, (int)backBufferDesc.Height };
	return std::make_shared<RenderTargetTexture>(surface,
		backBufferView,
		nullptr,
		nullptr,
		size,
		false);
}

RenderTargetTexturePtr RenderingDevice::CreateRenderTargetForSharedSurface(IUnknown * surface)
{	
	CComPtr<IDXGIResource> dxgiResource;

	if (FAILED(CComPtr<IUnknown>(surface).QueryInterface(&dxgiResource))) {
		return nullptr;
	}

	HANDLE sharedHandle;
	if (FAILED(dxgiResource->GetSharedHandle(&sharedHandle))) {
		return nullptr;
	}

	dxgiResource.Release();

	CComPtr<ID3D11Resource> sharedResource;
	D3DVERIFY(mD3d11Device->OpenSharedResource(sharedHandle, __uuidof(ID3D11Resource), (void**)(&sharedResource)));

	CComPtr<ID3D11Texture2D> sharedTexture;
	D3DVERIFY(sharedResource.QueryInterface(&sharedTexture));

	return CreateRenderTargetForNativeSurface(sharedTexture);
}

RenderTargetDepthStencilPtr
RenderingDevice::CreateRenderTargetDepthStencil(int width, int height, bool multiSample) {

  CD3D11_TEXTURE2D_DESC descDepth(DXGI_FORMAT_D24_UNORM_S8_UINT, width, height,
                                  1U,
                                  1U, // Disable Mip Map generation
                                  D3D11_BIND_DEPTH_STENCIL);

  // Enable multi sampling
  if (multiSample) {
	  descDepth.SampleDesc.Count = mImpl->msaaSamples;
	  descDepth.SampleDesc.Quality = mImpl->msaaQuality;
  }

  CComPtr<ID3D11Texture2D> texture;
  D3DVERIFY(mD3d11Device->CreateTexture2D(&descDepth, nullptr, &texture));

  // Create the depth stencil view
  CD3D11_DEPTH_STENCIL_VIEW_DESC descDSV(D3D11_DSV_DIMENSION_TEXTURE2D, descDepth.Format);

  if (multiSample) {
	  descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
  }

  CComPtr<ID3D11DepthStencilView> depthStencilView;
  D3DVERIFY(mD3d11Device->CreateDepthStencilView(texture, &descDSV, &depthStencilView));

  Size size{width, height};
  return std::make_shared<RenderTargetDepthStencil>(texture, depthStencilView, size);
}

void RenderingDevice::SetScissorRect(int x, int y, int width, int height) {
	D3D11_RECT rect{ x, y, x + width, y + height };
	mContext->RSSetScissorRects(1, &rect);

	mImpl->textEngine->SetScissorRect({ x, y, width, height });
}

void RenderingDevice::ResetScissorRect() {
	auto &size = mRenderTargetStack.back().colorBuffer->GetSize();
	D3D11_RECT rect{ 0, 0, size.width, size.height };
	mContext->RSSetScissorRects(1, &rect);

	mImpl->textEngine->ResetScissorRect();
}

}
