
#pragma once

#include "d3d8to9_private.h"

struct Direct3DSurface8Adapter : public d3d8::IDirect3DSurface8 {

	Direct3DSurface8Adapter();

	virtual ~Direct3DSurface8Adapter();

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DSurface8 methods ***/
	STDMETHOD(GetDevice)(THIS_ d3d8::IDirect3DDevice8** ppDevice);
	STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
	STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData);
	STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid);

	STDMETHOD(GetContainer)(THIS_ REFIID riid, void** ppContainer);
	STDMETHOD(GetDesc)(THIS_ d3d8::D3DSURFACE_DESC *pDesc);
	STDMETHOD(LockRect)(THIS_ d3d8::D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags);

	STDMETHOD(UnlockRect)(THIS);

	IDirect3DSurface9 *delegate;
};
