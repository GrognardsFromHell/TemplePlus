#include "d3d8to9_private.h"

struct Direct3DTexture8Adapter : public d3d8::IDirect3DTexture8
{
	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID /*riid*/, void** /*ppvObj*/);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DResource8 methods ***/
	STDMETHOD(GetDevice)(THIS_ d3d8::IDirect3DDevice8** ppDevice);
	STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
	STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData);
	STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid);
	STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew);
	STDMETHOD_(DWORD, GetPriority)(THIS);
	STDMETHOD_(void, PreLoad)(THIS);
	STDMETHOD_(d3d8::D3DRESOURCETYPE, GetType)(THIS);
	STDMETHOD_(DWORD, SetLOD)(THIS_ DWORD LODNew);
	STDMETHOD_(DWORD, GetLOD)(THIS);
	STDMETHOD_(DWORD, GetLevelCount)(THIS);

	STDMETHOD(GetLevelDesc)(THIS_ UINT Level, d3d8::D3DSURFACE_DESC* pDesc);

	STDMETHOD(GetSurfaceLevel)(THIS_ UINT Level, d3d8::IDirect3DSurface8** ppSurfaceLevel);

	STDMETHOD(LockRect)(THIS_ UINT Level, d3d8::D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags);

	STDMETHOD(UnlockRect)(THIS_ UINT Level);

	STDMETHOD(AddDirtyRect)(THIS_ CONST RECT* pDirtyRect);

	Direct3DTexture8Adapter();

	Direct3DTexture8Adapter(IDirect3DTexture9* _delegate) : delegate(_delegate) {}

	virtual ~Direct3DTexture8Adapter();

	IDirect3DTexture9* delegate;
};
