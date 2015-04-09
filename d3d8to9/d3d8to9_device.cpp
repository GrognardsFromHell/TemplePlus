
#include "stdafx.h"
#include "d3d8to9_device.h"
#include "d3d8to9_convert.h"
#include "d3d8to9_texture.h"
#include "d3d8to9_vertexbuffer.h"
#include "d3d8to9_indexbuffer.h"
#include "d3d8to9_surface.h"

HRESULT Direct3DDevice8Adapter::QueryInterface(THIS_ REFIID /*riid*/, void** /*ppvObj*/)
{
	return E_NOINTERFACE;
}

ULONG Direct3DDevice8Adapter::AddRef(THIS)
{
	return delegate->AddRef();
}

ULONG Direct3DDevice8Adapter::Release(THIS)
{
	// TODO: Free once Release == 0
	return delegate->Release();
}

/*** IDirect3DDevice8 methods ***/
HRESULT Direct3DDevice8Adapter::TestCooperativeLevel(THIS)
{
	return delegate->TestCooperativeLevel();
}

UINT Direct3DDevice8Adapter::GetAvailableTextureMem(THIS)
{
	// ToEE apparently seems to incorrectly use a 32-bit int
	return min<UINT>(0x7FFFFFFF, delegate->GetAvailableTextureMem());
}

HRESULT Direct3DDevice8Adapter::ResourceManagerDiscardBytes(THIS_ DWORD Bytes)
{
	logger->info("Unsupported D3D method called: ResourceManagerDiscardBytes");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::GetDirect3D(THIS_ d3d8::IDirect3D8** ppD3D8)
{
	logger->info("Unsupported D3D method called: GetDirect3D");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::GetDeviceCaps(THIS_ d3d8::D3DCAPS8* pCaps)
{
	D3DCAPS9 caps;
	HRESULT result = delegate->GetDeviceCaps(&caps);
	handleD3dError("GetDeviceCaps", result);

	convert(caps, pCaps);

	return result;
}

HRESULT Direct3DDevice8Adapter::GetDisplayMode(THIS_ d3d8::D3DDISPLAYMODE* pMode)
{
	// TODO Swap chain number???
	// TODO: D3DFormat is actually incompatible
	HRESULT result = delegate->GetDisplayMode(0, (D3DDISPLAYMODE*)(pMode));
	pMode->Format = convert((D3DFORMAT)pMode->Format);
	handleD3dError("GetDisplayMode", result);
	return result;
}

HRESULT Direct3DDevice8Adapter::GetCreationParameters(THIS_ d3d8::D3DDEVICE_CREATION_PARAMETERS* pParameters)
{
	logger->info("Unsupported D3D method called: GetCreationParameters");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::SetCursorProperties(THIS_ UINT XHotSpot, UINT YHotSpot, d3d8::IDirect3DSurface8* pCursorBitmap)
{
	// TODO: Gotta use the IDirect3DSurface8ADAPTER and get the *real* d3d9 surface
	return delegate->SetCursorProperties(XHotSpot, YHotSpot, NULL);
}

void Direct3DDevice8Adapter::SetCursorPosition(THIS_ int X, int Y, DWORD Flags)
{
	// TODO: Check Flags
	return delegate->SetCursorPosition(X, Y, Flags);
}

BOOL Direct3DDevice8Adapter::ShowCursor(THIS_ BOOL bShow)
{
	return delegate->ShowCursor(bShow);
}

HRESULT Direct3DDevice8Adapter::CreateAdditionalSwapChain(THIS_ d3d8::D3DPRESENT_PARAMETERS* pPresentationParameters, d3d8::IDirect3DSwapChain8** pSwapChain)
{
	logger->info("Unsupported D3D method called: CreateAdditionalSwapChain");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::Reset(THIS_ d3d8::D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	D3DPRESENT_PARAMETERS presentParams;
	memset(&presentParams, 0, sizeof(presentParams));
	
	// presentParams.MultiSampleType = (D3DMULTISAMPLE_TYPE)pPresentationParameters->MultiSampleType;
	// presentParams.MultiSampleQuality = 0;

	// We always use DISCARD
	presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParams.hDeviceWindow = pPresentationParameters->hDeviceWindow;
	presentParams.Windowed = true; // pPresentationParameters->Windowed;
	presentParams.EnableAutoDepthStencil = pPresentationParameters->EnableAutoDepthStencil;
	presentParams.AutoDepthStencilFormat = convert(pPresentationParameters->AutoDepthStencilFormat);
	presentParams.Flags = pPresentationParameters->Flags;
	presentParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	/*
	presentParams.BackBufferWidth = pPresentationParameters->BackBufferWidth;
	presentParams.BackBufferHeight = pPresentationParameters->BackBufferHeight;
	presentParams.BackBufferFormat = convert(pPresentationParameters->BackBufferFormat);
	presentParams.BackBufferCount = pPresentationParameters->BackBufferCount;
	presentParams.FullScreen_RefreshRateInHz = pPresentationParameters->FullScreen_RefreshRateInHz;
	*/

	auto result = delegate->Reset(&presentParams);
	handleD3dError("Reset", result);
	return result;
}

HRESULT Direct3DDevice8Adapter::Present(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{
	return delegate->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT Direct3DDevice8Adapter::GetBackBuffer(THIS_ UINT BackBuffer, d3d8::D3DBACKBUFFER_TYPE Type, d3d8::IDirect3DSurface8** ppBackBuffer)
{
	auto adapter = new Direct3DSurface8Adapter;
	HRESULT result = handleD3dError("GetBackBuffer", delegate->GetBackBuffer(0, BackBuffer, D3DBACKBUFFER_TYPE_MONO, &adapter->delegate));
	*ppBackBuffer = adapter;
	return result;
}

HRESULT Direct3DDevice8Adapter::GetRasterStatus(THIS_ d3d8::D3DRASTER_STATUS* pRasterStatus)
{
	// TODO: Swap Chain
	// Structures are identical
	return delegate->GetRasterStatus(0, (D3DRASTER_STATUS*)pRasterStatus);
}

void Direct3DDevice8Adapter::SetGammaRamp(THIS_ DWORD Flags, CONST d3d8::D3DGAMMARAMP* pRamp)
{
	// TODO: Swap Chain
	return delegate->SetGammaRamp(0, Flags, (D3DGAMMARAMP*)pRamp);
}

void Direct3DDevice8Adapter::GetGammaRamp(THIS_ d3d8::D3DGAMMARAMP* pRamp)
{
	// TODO: Swap Chain
	delegate->GetGammaRamp(0, (D3DGAMMARAMP*)pRamp);
}

HRESULT Direct3DDevice8Adapter::CreateTexture(THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, d3d8::D3DFORMAT Format, d3d8::D3DPOOL Pool, d3d8::IDirect3DTexture8** ppTexture)
{
	auto adapter = new Direct3DTexture8Adapter;

	auto d3d9format = convert(Format);
	auto d3d9pool = (D3DPOOL)Pool;
	if (config.useDirect3d9Ex && d3d9pool == D3DPOOL_MANAGED)
	{
		d3d9pool = D3DPOOL_DEFAULT;
		Usage |= D3DUSAGE_DYNAMIC;
	}
	auto result = delegate->CreateTexture(Width, Height, Levels, Usage, d3d9format, d3d9pool, &adapter->delegate, NULL);
	handleD3dError("CreateTexture", result);
	*ppTexture = adapter;
	return result;
}

HRESULT Direct3DDevice8Adapter::CreateVolumeTexture(THIS_ UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, d3d8::D3DFORMAT Format, d3d8::D3DPOOL Pool, d3d8::IDirect3DVolumeTexture8** ppVolumeTexture)
{
	logger->info("Unsupported D3D method called: CreateVolumeTexture");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::CreateCubeTexture(THIS_ UINT EdgeLength, UINT Levels, DWORD Usage, d3d8::D3DFORMAT Format, d3d8::D3DPOOL Pool, d3d8::IDirect3DCubeTexture8** ppCubeTexture)
{
	logger->info("Unsupported D3D method called: CreateCubeTexture");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::CreateVertexBuffer(THIS_ UINT Length, DWORD Usage, DWORD FVF, d3d8::D3DPOOL Pool, d3d8::IDirect3DVertexBuffer8** ppVertexBuffer)
{
	// FVF in d3d8 seems to be a subset of d3d9
	auto adapter = new Direct3DVertexBuffer8Adapter;
	// Pool is compatible
	// TODO: Check this. for some reason particle systems are flickering if this is the wrong pool or the wrong usage
	Usage = D3DUSAGE_DYNAMIC;
	Pool = d3d8::D3DPOOL_SYSTEMMEM;
	HRESULT result = delegate->CreateVertexBuffer(Length, Usage, FVF, (D3DPOOL)Pool, &adapter->delegate, 0);
	handleD3dError("CreateVertexBuffer", result);
	*ppVertexBuffer = adapter;
	return result;
}

HRESULT Direct3DDevice8Adapter::CreateIndexBuffer(THIS_ UINT Length, DWORD Usage, d3d8::D3DFORMAT Format, d3d8::D3DPOOL Pool, d3d8::IDirect3DIndexBuffer8** ppIndexBuffer)
{
	auto adapter = new Direct3DIndexBuffer8Adapter;

	auto result = delegate->CreateIndexBuffer(Length, Usage, convert(Format), (D3DPOOL)Pool, &adapter->delegate, NULL);
	handleD3dError("CreateIndexBuffer", result);
	*ppIndexBuffer = adapter;
	return result;
}

HRESULT Direct3DDevice8Adapter::CreateRenderTarget(THIS_ UINT Width, UINT Height, d3d8::D3DFORMAT Format, d3d8::D3DMULTISAMPLE_TYPE MultiSample, BOOL Lockable, d3d8::IDirect3DSurface8** ppSurface)
{
	logger->info("Unsupported D3D method called: CreateRenderTarget");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::CreateDepthStencilSurface(THIS_ UINT Width, UINT Height, d3d8::D3DFORMAT Format, d3d8::D3DMULTISAMPLE_TYPE MultiSample, d3d8::IDirect3DSurface8** ppSurface)
{
	logger->info("Unsupported D3D method called: CreateDepthStencilSurface");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::CreateImageSurface(THIS_ UINT Width, UINT Height, d3d8::D3DFORMAT Format, d3d8::IDirect3DSurface8** ppSurface)
{
	auto adapter = new Direct3DSurface8Adapter;
	*ppSurface = adapter;
	HRESULT result = delegate->CreateOffscreenPlainSurface(Width, Height, convert(Format), D3DPOOL_SYSTEMMEM, &adapter->delegate, NULL);
	return handleD3dError("CreateImageSurface", result);
}

HRESULT Direct3DDevice8Adapter::CopyRects(THIS_ d3d8::IDirect3DSurface8* pSourceSurface, CONST RECT* pSourceRectsArray, UINT cRects, d3d8::IDirect3DSurface8* pDestinationSurface, CONST POINT* pDestPointsArray)
{
	logger->info("Unsupported D3D method called: CopyRects");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::UpdateTexture(THIS_ d3d8::IDirect3DBaseTexture8* pSourceTexture, d3d8::IDirect3DBaseTexture8* pDestinationTexture)
{
	logger->info("Unsupported D3D method called: UpdateTexture");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::GetFrontBuffer(THIS_ d3d8::IDirect3DSurface8* pDestSurface)
{
	auto adapter = (Direct3DSurface8Adapter*)pDestSurface;
	return handleD3dError("GetFrontBuffer", delegate->GetFrontBufferData(0, adapter->delegate));
}

HRESULT Direct3DDevice8Adapter::SetRenderTarget(THIS_ d3d8::IDirect3DSurface8* pRenderTarget, d3d8::IDirect3DSurface8* pNewZStencil)
{
	logger->info("Unsupported D3D method called: SetRenderTarget");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::GetRenderTarget(THIS_ d3d8::IDirect3DSurface8** ppRenderTarget)
{
	logger->info("Unsupported D3D method called: GetRenderTarget");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::GetDepthStencilSurface(THIS_ d3d8::IDirect3DSurface8** ppZStencilSurface)
{
	logger->info("Unsupported D3D method called: GetDepthStencilSurface");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::BeginScene(THIS)
{
	return handleD3dError("BeginScene", delegate->BeginScene());
}

HRESULT Direct3DDevice8Adapter::EndScene(THIS)
{
	return handleD3dError("EndScene", delegate->EndScene());
}

HRESULT Direct3DDevice8Adapter::Clear(THIS_ DWORD Count, CONST d3d8::D3DRECT* pRects, DWORD Flags, d3d8::D3DCOLOR Color, float Z, DWORD Stencil)
{
	return handleD3dError("Clear", delegate->Clear(Count, (const D3DRECT*)pRects, Flags, (D3DCOLOR)Color, Z, Stencil));
}

HRESULT Direct3DDevice8Adapter::SetTransform(THIS_ d3d8::D3DTRANSFORMSTATETYPE State, CONST d3d8::D3DMATRIX* pMatrix)
{
	return handleD3dError("SetTransform", delegate->SetTransform((D3DTRANSFORMSTATETYPE)State, (const D3DMATRIX*)pMatrix));
}

HRESULT Direct3DDevice8Adapter::GetTransform(THIS_ d3d8::D3DTRANSFORMSTATETYPE State, d3d8::D3DMATRIX* pMatrix)
{
	return handleD3dError("GetTransform", delegate->GetTransform((D3DTRANSFORMSTATETYPE)State, (D3DMATRIX*)pMatrix));
}

HRESULT Direct3DDevice8Adapter::MultiplyTransform(THIS_ d3d8::D3DTRANSFORMSTATETYPE State, CONST d3d8::D3DMATRIX* pMatrix)
{
	return handleD3dError("MultiplyTransform", delegate->MultiplyTransform((D3DTRANSFORMSTATETYPE)State, (D3DMATRIX*)pMatrix));
}

HRESULT Direct3DDevice8Adapter::SetViewport(THIS_ CONST d3d8::D3DVIEWPORT8* pViewport)
{
	logger->info("Unsupported D3D method called: SetViewport");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::GetViewport(THIS_ d3d8::D3DVIEWPORT8* pViewport)
{
	logger->info("Unsupported D3D method called: GetViewport");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::SetMaterial(THIS_ CONST d3d8::D3DMATERIAL8* pMaterial)
{
	// Structures seem identical
	return delegate->SetMaterial((const D3DMATERIAL9*)pMaterial);
}

HRESULT Direct3DDevice8Adapter::GetMaterial(THIS_ d3d8::D3DMATERIAL8* pMaterial)
{
	// Structures seem identical
	return delegate->GetMaterial((D3DMATERIAL9*)pMaterial);
}

HRESULT Direct3DDevice8Adapter::SetLight(THIS_ DWORD Index, CONST d3d8::D3DLIGHT8* Light)
{
	// Structures seem similar
	return delegate->SetLight(Index, (const D3DLIGHT9*)Light);
}

HRESULT Direct3DDevice8Adapter::GetLight(THIS_ DWORD Index, d3d8::D3DLIGHT8* Light)
{
	// Structures seem similar
	return delegate->GetLight(Index, (D3DLIGHT9*)Light);
}

HRESULT Direct3DDevice8Adapter::LightEnable(THIS_ DWORD Index, BOOL Enable)
{
	return delegate->LightEnable(Index, Enable);
}

HRESULT Direct3DDevice8Adapter::GetLightEnable(THIS_ DWORD Index, BOOL* pEnable)
{
	return delegate->GetLightEnable(Index, pEnable);
}

HRESULT Direct3DDevice8Adapter::SetClipPlane(THIS_ DWORD Index, CONST float* pPlane)
{
	logger->info("Unsupported D3D method called: SetClipPlane");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::GetClipPlane(THIS_ DWORD Index, float* pPlane)
{
	logger->info("Unsupported D3D method called: GetClipPlane");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::SetRenderState(THIS_ d3d8::D3DRENDERSTATETYPE State, DWORD Value)
{
	return delegate->SetRenderState(convert(State), Value);
}

HRESULT Direct3DDevice8Adapter::GetRenderState(THIS_ d3d8::D3DRENDERSTATETYPE State, DWORD* pValue)
{
	return delegate->GetRenderState(convert(State), pValue);
}

HRESULT Direct3DDevice8Adapter::BeginStateBlock(THIS)
{
	logger->info("Unsupported D3D method called: BeginStateBlock");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::EndStateBlock(THIS_ DWORD* pToken)
{
	logger->info("Unsupported D3D method called: EndStateBlock");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::ApplyStateBlock(THIS_ DWORD Token)
{
	logger->info("Unsupported D3D method called: ApplyStateBlock");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::CaptureStateBlock(THIS_ DWORD Token)
{
	logger->info("Unsupported D3D method called: CaptureStateBlock");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::DeleteStateBlock(THIS_ DWORD Token)
{
	logger->info("Unsupported D3D method called: DeleteStateBlock");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::CreateStateBlock(THIS_ d3d8::D3DSTATEBLOCKTYPE Type, DWORD* pToken)
{
	logger->info("Unsupported D3D method called: CreateStateBlock");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::SetClipStatus(THIS_ CONST d3d8::D3DCLIPSTATUS8* pClipStatus)
{
	logger->info("Unsupported D3D method called: SetClipStatus");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::GetClipStatus(THIS_ d3d8::D3DCLIPSTATUS8* pClipStatus)
{
	logger->info("Unsupported D3D method called: GetClipStatus");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::GetTexture(THIS_ DWORD Stage, d3d8::IDirect3DBaseTexture8** ppTexture)
{
	assert(Stage < 16);
	auto result = stages[Stage];
	if (result)
	{
		result->AddRef();
	}
	*ppTexture = result;
	return D3D_OK;
}

HRESULT Direct3DDevice8Adapter::SetTexture(THIS_ DWORD Stage, d3d8::IDirect3DBaseTexture8* pTexture)
{
	assert(Stage < 16);
	auto adapter = (Direct3DTexture8Adapter*)pTexture;
	stages[Stage] = adapter;
	HRESULT result;
	if (pTexture)
	{
		result = delegate->SetTexture(Stage, adapter->delegate);
	}
	else
	{
		result = delegate->SetTexture(Stage, 0);
	}
	return result;
}

HRESULT Direct3DDevice8Adapter::GetTextureStageState(THIS_ DWORD Stage, d3d8::D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue)
{
	// _D3DTEXTUREOP is identical
	// D3DTA_ is a subset in d3d8
	// Up until type 11 the type enums are identical
	if (Type <= D3DTSS_TEXCOORDINDEX
		|| Type == D3DTSS_TEXTURETRANSFORMFLAGS
		|| Type == D3DTSS_COLORARG0
		|| Type == D3DTSS_ALPHAARG0
		|| Type == D3DTSS_RESULTARG)
	{
		return delegate->GetTextureStageState(Stage, (D3DTEXTURESTAGESTATETYPE)Type, pValue);
	}

	// The rest apparently moved to sampler stage
	switch (Type)
	{
	case d3d8::D3DTSS_MINFILTER:
		return delegate->GetSamplerState(Stage, D3DSAMP_MINFILTER, pValue);
	case d3d8::D3DTSS_MAGFILTER:
		return delegate->GetSamplerState(Stage, D3DSAMP_MAGFILTER, pValue);
	case d3d8::D3DTSS_MIPFILTER:
		return delegate->GetSamplerState(Stage, D3DSAMP_MIPFILTER, pValue);
	case d3d8::D3DTSS_MIPMAPLODBIAS:
		return delegate->GetSamplerState(Stage, D3DSAMP_MIPMAPLODBIAS, pValue);
	case d3d8::D3DTSS_MAXMIPLEVEL:
		return delegate->GetSamplerState(Stage, D3DSAMP_MAXMIPLEVEL, pValue);
	case d3d8::D3DTSS_ADDRESSU:
		return delegate->GetSamplerState(Stage, D3DSAMP_ADDRESSU, pValue);
	case d3d8::D3DTSS_ADDRESSV:
		return delegate->GetSamplerState(Stage, D3DSAMP_ADDRESSV, pValue);
	}

	logger->info("Unknwon texture state type: {}", Type);
	abort();
}

HRESULT Direct3DDevice8Adapter::SetTextureStageState(THIS_ DWORD Stage, d3d8::D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	// _D3DTEXTUREOP is identical
	// D3DTA_ is a subset in d3d8
	// Up until type 11 the type enums are identical
	if (Type <= D3DTSS_TEXCOORDINDEX
		|| Type == D3DTSS_TEXTURETRANSFORMFLAGS
		|| Type == D3DTSS_COLORARG0
		|| Type == D3DTSS_ALPHAARG0
		|| Type == D3DTSS_RESULTARG)
	{
		return delegate->SetTextureStageState(Stage, (D3DTEXTURESTAGESTATETYPE)Type, Value);
	}

	// The rest apparently moved to sampler stage
	switch (Type)
	{
	case d3d8::D3DTSS_MINFILTER:
		return delegate->SetSamplerState(Stage, D3DSAMP_MINFILTER, Value);
	case d3d8::D3DTSS_MAGFILTER:
		return delegate->SetSamplerState(Stage, D3DSAMP_MAGFILTER, Value);
	case d3d8::D3DTSS_MIPFILTER:
		return delegate->SetSamplerState(Stage, D3DSAMP_MIPFILTER, Value);
	case d3d8::D3DTSS_MIPMAPLODBIAS:
		return delegate->SetSamplerState(Stage, D3DSAMP_MIPMAPLODBIAS, Value);
	case d3d8::D3DTSS_MAXMIPLEVEL:
		return delegate->SetSamplerState(Stage, D3DSAMP_MAXMIPLEVEL, Value);
	case d3d8::D3DTSS_ADDRESSU:
		return delegate->SetSamplerState(Stage, D3DSAMP_ADDRESSU, Value);
	case d3d8::D3DTSS_ADDRESSV:
		return delegate->SetSamplerState(Stage, D3DSAMP_ADDRESSV, Value);
	}

	logger->info("Unknwon texture state type: {}", Type);
	abort();
}

HRESULT Direct3DDevice8Adapter::ValidateDevice(THIS_ DWORD* pNumPasses)
{
	logger->info("Unsupported D3D method called: ValidateDevice");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::GetInfo(THIS_ DWORD DevInfoID, void* pDevInfoStruct, DWORD DevInfoStructSize)
{
	logger->info("Unsupported D3D method called: GetInfo");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::SetPaletteEntries(THIS_ UINT PaletteNumber, CONST PALETTEENTRY* pEntries)
{
	logger->info("Unsupported D3D method called: SetPaletteEntries");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::GetPaletteEntries(THIS_ UINT PaletteNumber, PALETTEENTRY* pEntries)
{
	logger->info("Unsupported D3D method called: GetPaletteEntries");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::SetCurrentTexturePalette(THIS_ UINT PaletteNumber)
{
	logger->info("Unsupported D3D method called: SetCurrentTexturePalette");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::GetCurrentTexturePalette(THIS_ UINT* PaletteNumber)
{
	logger->info("Unsupported D3D method called: GetCurrentTexturePalette");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::DrawPrimitive(THIS_ d3d8::D3DPRIMITIVETYPE type, UINT StartVertex, UINT PrimitiveCount)
{
	return handleD3dError("DrawPrimitive", delegate->DrawPrimitive((D3DPRIMITIVETYPE)type, StartVertex, PrimitiveCount));
}

HRESULT Direct3DDevice8Adapter::DrawIndexedPrimitive(THIS_ d3d8::D3DPRIMITIVETYPE type, UINT minIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	return handleD3dError("DrawIndexedPrimitive", delegate->DrawIndexedPrimitive((D3DPRIMITIVETYPE)type, 0, minIndex, NumVertices, startIndex, primCount));	
}

HRESULT Direct3DDevice8Adapter::DrawPrimitiveUP(THIS_ d3d8::D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	logger->info("Unsupported D3D method called: DrawPrimitiveUP");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::DrawIndexedPrimitiveUP(THIS_ d3d8::D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices, UINT PrimitiveCount, CONST void* pIndexData, d3d8::D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	logger->info("Unsupported D3D method called: DrawIndexedPrimitiveUP");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::ProcessVertices(THIS_ UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, d3d8::IDirect3DVertexBuffer8* pDestBuffer, DWORD Flags)
{
	logger->info("Unsupported D3D method called: ProcessVertices");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::CreateVertexShader(THIS_ CONST DWORD* pDeclaration, CONST DWORD* pFunction, DWORD* pHandle, DWORD Usage)
{
	logger->info("Unsupported D3D method called: CreateVertexShader");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::SetVertexShader(THIS_ DWORD Handle)
{
	return handleD3dError("SetVertexShader", delegate->SetFVF(Handle));
}

HRESULT Direct3DDevice8Adapter::GetVertexShader(THIS_ DWORD* pHandle)
{
	return delegate->GetFVF(pHandle);
}

HRESULT Direct3DDevice8Adapter::DeleteVertexShader(THIS_ DWORD Handle)
{
	logger->info("Unsupported D3D method called: DeleteVertexShader");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::SetVertexShaderConstant(THIS_ DWORD Register, CONST void* pConstantData, DWORD ConstantCount)
{
	logger->info("Unsupported D3D method called: SetVertexShaderConstant");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::GetVertexShaderConstant(THIS_ DWORD Register, void* pConstantData, DWORD ConstantCount)
{
	logger->info("Unsupported D3D method called: GetVertexShaderConstant");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::GetVertexShaderDeclaration(THIS_ DWORD Handle, void* pData, DWORD* pSizeOfData)
{
	logger->info("Unsupported D3D method called: GetVertexShaderDeclaration");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::GetVertexShaderFunction(THIS_ DWORD Handle, void* pData, DWORD* pSizeOfData)
{
	logger->info("Unsupported D3D method called: GetVertexShaderFunction");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::SetStreamSource(THIS_ UINT StreamNumber, d3d8::IDirect3DVertexBuffer8* pStreamData, UINT Stride)
{
	auto adapter = (Direct3DVertexBuffer8Adapter*)pStreamData;
	streamSources[StreamNumber] = adapter;
	if (adapter)
	{
		return handleD3dError("SetStreamSource", delegate->SetStreamSource(StreamNumber, adapter->delegate, 0, Stride));
	}
	else
	{
		return handleD3dError("SetStreamSource", delegate->SetStreamSource(StreamNumber, 0, 0, Stride));
	}
}

HRESULT Direct3DDevice8Adapter::GetStreamSource(THIS_ UINT StreamNumber, d3d8::IDirect3DVertexBuffer8** ppStreamData, UINT* pStride)
{
	IDirect3DVertexBuffer9* vbuf;
	UINT offset;
	HRESULT result = delegate->GetStreamSource(StreamNumber, &vbuf, &offset, pStride);
	handleD3dError("GetStreamSource", result);

	// TODO this is clumsy?
	// Check if our adapter is still valid
	auto ss = streamSources[StreamNumber];
	if (vbuf && ss && ss->delegate == vbuf)
	{
		*ppStreamData = ss;
	}
	else
	{
		*ppStreamData = 0;
	}

	return result;
}

HRESULT Direct3DDevice8Adapter::SetIndices(THIS_ d3d8::IDirect3DIndexBuffer8* pIndexData, UINT BaseVertexIndex)
{
	assert(BaseVertexIndex == 0);

	indices = (Direct3DIndexBuffer8Adapter*)pIndexData;

	if (indices)
	{
		return delegate->SetIndices(indices->delegate);
	}
	else
	{
		return delegate->SetIndices(NULL);
	}
}

HRESULT Direct3DDevice8Adapter::GetIndices(THIS_ d3d8::IDirect3DIndexBuffer8** ppIndexData, UINT* pBaseVertexIndex)
{
	pBaseVertexIndex = 0;
	IDirect3DIndexBuffer9* current;
	auto result = delegate->GetIndices(&current);
	handleD3dError("GetIndices", result);
	if (current && indices && indices->delegate == current)
	{
		*ppIndexData = indices;
	}
	else
	{
		*ppIndexData = 0;
	}
	return result;
}

HRESULT Direct3DDevice8Adapter::CreatePixelShader(THIS_ CONST DWORD* pFunction, DWORD* pHandle)
{
	logger->info("Unsupported D3D method called: CreatePixelShader");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::SetPixelShader(THIS_ DWORD Handle)
{
	logger->info("Unsupported D3D method called: SetPixelShader");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::GetPixelShader(THIS_ DWORD* pHandle)
{
	logger->info("Unsupported D3D method called: GetPixelShader");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::DeletePixelShader(THIS_ DWORD Handle)
{
	logger->info("Unsupported D3D method called: DeletePixelShader");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::SetPixelShaderConstant(THIS_ DWORD Register, CONST void* pConstantData, DWORD ConstantCount)
{
	logger->info("Unsupported D3D method called: SetPixelShaderConstant");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::GetPixelShaderConstant(THIS_ DWORD Register, void* pConstantData, DWORD ConstantCount)
{
	logger->info("Unsupported D3D method called: GetPixelShaderConstant");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::GetPixelShaderFunction(THIS_ DWORD Handle, void* pData, DWORD* pSizeOfData)
{
	logger->info("Unsupported D3D method called: GetPixelShaderFunction");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::DrawRectPatch(THIS_ UINT Handle, CONST float* pNumSegs, CONST d3d8::D3DRECTPATCH_INFO* pRectPatchInfo)
{
	logger->info("Unsupported D3D method called: DrawRectPatch");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::DrawTriPatch(THIS_ UINT Handle, CONST float* pNumSegs, CONST d3d8::D3DTRIPATCH_INFO* pTriPatchInfo)
{
	logger->info("Unsupported D3D method called: DrawTriPatch");
	abort();
	return EFAULT;
}

HRESULT Direct3DDevice8Adapter::DeletePatch(THIS_ UINT Handle)
{
	logger->info("Unsupported D3D method called: DeletePatch");
	abort();
	return EFAULT;
}

Direct3DDevice8Adapter::Direct3DDevice8Adapter() : delegate(0), indices(0)
{
	memset(stages, 0, sizeof(stages));
	memset(streamSources, 0, sizeof(streamSources));
}

Direct3DDevice8Adapter::~Direct3DDevice8Adapter()
{
}
