
#include "d3d8to9_private.h"

struct Direct3DIndexBuffer8Adapter : public d3d8::IDirect3DIndexBuffer8 {

	Direct3DIndexBuffer8Adapter();
	virtual ~Direct3DIndexBuffer8Adapter();

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
	STDMETHOD(Lock)(THIS_ UINT OffsetToLock, UINT SizeToLock, BYTE** ppbData, DWORD Flags);
	STDMETHOD(Unlock)(THIS);
	STDMETHOD(GetDesc)(THIS_ d3d8::D3DINDEXBUFFER_DESC *pDesc);

	IDirect3DIndexBuffer9 *delegate;
};
