#pragma once

#include "d3d8to9_private.h"

struct Direct3D8Adapter : public d3d8::IDirect3D8
{
	Direct3D8Adapter();
	virtual ~Direct3D8Adapter();

	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3D8 methods ***/
	STDMETHOD(RegisterSoftwareDevice)(THIS_ void* pInitializeFunction);
	STDMETHOD_(UINT, GetAdapterCount)(THIS);
	STDMETHOD(GetAdapterIdentifier)(THIS_ UINT Adapter, DWORD Flags, d3d8::D3DADAPTER_IDENTIFIER8* pIdentifier);
	STDMETHOD_(UINT, GetAdapterModeCount)(THIS_ UINT Adapter);
	STDMETHOD(EnumAdapterModes)(THIS_ UINT Adapter, UINT Mode, d3d8::D3DDISPLAYMODE* pMode);
	STDMETHOD(GetAdapterDisplayMode)(THIS_ UINT Adapter, d3d8::D3DDISPLAYMODE* pMode);
	STDMETHOD(CheckDeviceType)(THIS_ UINT Adapter, d3d8::D3DDEVTYPE CheckType, d3d8::D3DFORMAT DisplayFormat, d3d8::D3DFORMAT BackBufferFormat, BOOL Windowed);
	STDMETHOD(CheckDeviceFormat)(THIS_ UINT Adapter, d3d8::D3DDEVTYPE DeviceType, d3d8::D3DFORMAT AdapterFormat, DWORD Usage, d3d8::D3DRESOURCETYPE RType, d3d8::D3DFORMAT CheckFormat);
	STDMETHOD(CheckDeviceMultiSampleType)(THIS_ UINT Adapter, d3d8::D3DDEVTYPE DeviceType, d3d8::D3DFORMAT SurfaceFormat, BOOL Windowed, d3d8::D3DMULTISAMPLE_TYPE MultiSampleType);
	STDMETHOD(CheckDepthStencilMatch)(THIS_ UINT Adapter, d3d8::D3DDEVTYPE DeviceType, d3d8::D3DFORMAT AdapterFormat, d3d8::D3DFORMAT RenderTargetFormat, d3d8::D3DFORMAT DepthStencilFormat);
	STDMETHOD(GetDeviceCaps)(THIS_ UINT Adapter, d3d8::D3DDEVTYPE DeviceType, d3d8::D3DCAPS8* pCaps);
	STDMETHOD_(HMONITOR, GetAdapterMonitor)(THIS_ UINT Adapter);
	STDMETHOD(CreateDevice)(THIS_ UINT Adapter, d3d8::D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, d3d8::D3DPRESENT_PARAMETERS* pPresentationParameters, d3d8::IDirect3DDevice8** ppReturnedDeviceInterface);


	IDirect3D9Ex* delegate;
};
