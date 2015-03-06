
#include "d3d8to9_rootobj.h"
#include "d3d8to9_device.h"
#include "d3d8to9_convert.h"

Direct3D8Adapter::Direct3D8Adapter()
{
}

Direct3D8Adapter::~Direct3D8Adapter()
{
}

HRESULT Direct3D8Adapter::QueryInterface(THIS_ REFIID riid, void** ppvObj) {
	return delegate->QueryInterface(riid, ppvObj);
}
ULONG Direct3D8Adapter::AddRef(THIS) {
	return delegate->AddRef();
}
ULONG Direct3D8Adapter::Release(THIS) {
	// TODO Clean the adapter up here as well if refcount reaches 0
	return delegate->Release();
}

/*** IDirect3D8 methods ***/
HRESULT Direct3D8Adapter::RegisterSoftwareDevice(THIS_ void* pInitializeFunction) {
	abort();
}
UINT Direct3D8Adapter::GetAdapterCount(THIS) {
	return delegate->GetAdapterCount();
}
HRESULT Direct3D8Adapter::GetAdapterIdentifier(THIS_ UINT Adapter, DWORD Flags, d3d8::D3DADAPTER_IDENTIFIER8* pIdentifier) {
	D3DADAPTER_IDENTIFIER9 id9;
	HRESULT result = delegate->GetAdapterIdentifier(Adapter, Flags, &id9);
	// TODO Copy over the stuff that matters
	ZeroMemory(pIdentifier, sizeof(d3d8::D3DADAPTER_IDENTIFIER8));
	memcpy(&pIdentifier->Description, id9.Description, MAX_DEVICE_IDENTIFIER_STRING);
	memcpy(&pIdentifier->Driver, id9.Driver, MAX_DEVICE_IDENTIFIER_STRING);
	pIdentifier->DriverVersion = id9.DriverVersion;

	pIdentifier->VendorId = id9.VendorId;
	pIdentifier->DeviceId = id9.DeviceId;
	pIdentifier->SubSysId = id9.SubSysId;
	pIdentifier->Revision = id9.Revision;
	pIdentifier->DeviceIdentifier = id9.DeviceIdentifier;
	pIdentifier->WHQLLevel = id9.WHQLLevel;

	return result;
}
UINT Direct3D8Adapter::GetAdapterModeCount(THIS_ UINT Adapter) {
	return delegate->GetAdapterModeCount(Adapter, D3DFMT_X8R8G8B8);
}
HRESULT Direct3D8Adapter::EnumAdapterModes(THIS_ UINT Adapter, UINT Mode, d3d8::D3DDISPLAYMODE* pMode) {
	// TODO d3d format is NOT equivalent, needs conversion
	// Although here we explicitly set a format
	return delegate->EnumAdapterModes(Adapter, D3DFMT_X8R8G8B8, Mode, (D3DDISPLAYMODE*)pMode);
}
HRESULT Direct3D8Adapter::GetAdapterDisplayMode(THIS_ UINT Adapter, d3d8::D3DDISPLAYMODE* pMode) {
	// TODO structs are NOT equivalent
	HRESULT result = delegate->GetAdapterDisplayMode(Adapter, (D3DDISPLAYMODE*)pMode);
	pMode->Format = convert((D3DFORMAT)pMode->Format);
	handleError("GetAdapterDisplayMode", result);
	return result;
}
HRESULT Direct3D8Adapter::CheckDeviceType(THIS_ UINT Adapter, d3d8::D3DDEVTYPE CheckType, d3d8::D3DFORMAT DisplayFormat, d3d8::D3DFORMAT BackBufferFormat, BOOL Windowed) {
	return -1;
}
HRESULT Direct3D8Adapter::CheckDeviceFormat(THIS_ UINT Adapter, d3d8::D3DDEVTYPE DeviceType, d3d8::D3DFORMAT AdapterFormat, DWORD Usage, d3d8::D3DRESOURCETYPE RType, d3d8::D3DFORMAT CheckFormat) {
	HRESULT result = delegate->CheckDeviceFormat(Adapter, (D3DDEVTYPE)DeviceType, convert(AdapterFormat), Usage, (D3DRESOURCETYPE)RType, convert(CheckFormat));
	handleError("CheckDeviceFormat", result);
	return result;
}
HRESULT Direct3D8Adapter::CheckDeviceMultiSampleType(THIS_ UINT Adapter, d3d8::D3DDEVTYPE DeviceType, d3d8::D3DFORMAT SurfaceFormat, BOOL Windowed, d3d8::D3DMULTISAMPLE_TYPE MultiSampleType) {
	DWORD qualityLevels;
	return delegate->CheckDeviceMultiSampleType(Adapter, (D3DDEVTYPE)DeviceType, convert(SurfaceFormat), Windowed, (D3DMULTISAMPLE_TYPE)MultiSampleType, &qualityLevels);
}
HRESULT Direct3D8Adapter::CheckDepthStencilMatch(THIS_ UINT Adapter, d3d8::D3DDEVTYPE DeviceType, d3d8::D3DFORMAT AdapterFormat, d3d8::D3DFORMAT RenderTargetFormat, d3d8::D3DFORMAT DepthStencilFormat) {
	return -1;
}
HRESULT Direct3D8Adapter::GetDeviceCaps(THIS_ UINT Adapter, d3d8::D3DDEVTYPE DeviceType, d3d8::D3DCAPS8* pCaps) {
	D3DCAPS9 caps;
	HRESULT result = delegate->GetDeviceCaps(Adapter, (D3DDEVTYPE)DeviceType, &caps);
	convert(caps, pCaps);
	return result;
}
HMONITOR Direct3D8Adapter::GetAdapterMonitor(THIS_ UINT Adapter) {
	return delegate->GetAdapterMonitor(Adapter);
}
HRESULT Direct3D8Adapter::CreateDevice(THIS_ UINT Adapter, d3d8::D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, d3d8::D3DPRESENT_PARAMETERS* pPresentationParameters, d3d8::IDirect3DDevice8** ppReturnedDeviceInterface) {

	// TODO For some reason, ToEE specifies flags=D3DCREATE_SOFTWARE_VERTEXPROCESSING here. Maybe we should change this to HARDARE
	BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;

	auto adapter = new Direct3DDevice8Adapter;

	D3DPRESENT_PARAMETERS presentParams;
	memset(&presentParams, 0, sizeof(presentParams));
	presentParams.BackBufferWidth = pPresentationParameters->BackBufferWidth;
	presentParams.BackBufferHeight = pPresentationParameters->BackBufferHeight;
	presentParams.BackBufferFormat = convert(pPresentationParameters->BackBufferFormat);
	presentParams.BackBufferCount = pPresentationParameters->BackBufferCount;

	presentParams.MultiSampleType = (D3DMULTISAMPLE_TYPE)pPresentationParameters->MultiSampleType;
	presentParams.MultiSampleQuality = 0;

	// We always use DISCARD
	presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParams.hDeviceWindow = pPresentationParameters->hDeviceWindow;
	presentParams.Windowed = pPresentationParameters->Windowed;
	presentParams.EnableAutoDepthStencil = pPresentationParameters->EnableAutoDepthStencil;
	presentParams.AutoDepthStencilFormat = convert(pPresentationParameters->AutoDepthStencilFormat);
	presentParams.Flags = 0; // Ignoring the flags that are passed in

	presentParams.FullScreen_RefreshRateInHz = pPresentationParameters->FullScreen_RefreshRateInHz;
	presentParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	// FullScreen_PresentationInterval is ignored

	D3DDISPLAYMODEEX fullScreenMode;
	D3DDISPLAYMODEEX* pFullScreenMode = NULL;
	if (!presentParams.Windowed) {
		fullScreenMode.Size = sizeof(D3DDISPLAYMODEEX);
		fullScreenMode.Width = pPresentationParameters->BackBufferWidth;
		fullScreenMode.Height = pPresentationParameters->BackBufferHeight;
		fullScreenMode.RefreshRate = pPresentationParameters->FullScreen_RefreshRateInHz;
		fullScreenMode.Format = convert(pPresentationParameters->BackBufferFormat);

		pFullScreenMode = &fullScreenMode;
	}

	HRESULT result;
	if (useD3dEx) {
		result = delegate->CreateDeviceEx(Adapter, D3DDEVTYPE_HAL, hFocusWindow, BehaviorFlags, &presentParams, pFullScreenMode, &adapter->delegate);
	}
	else {
		result = delegate->CreateDevice(Adapter, D3DDEVTYPE_HAL, hFocusWindow, BehaviorFlags, &presentParams, (IDirect3DDevice9**)&adapter->delegate);
	}

	handleError("CreateDevice", result);
	*ppReturnedDeviceInterface = adapter;

	return result;
}
