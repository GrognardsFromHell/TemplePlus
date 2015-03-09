
#include "stdafx.h"
#include "d3d8to9_surface.h"
#include "d3d8to9_convert.h"

Direct3DSurface8Adapter::Direct3DSurface8Adapter()
{
}

Direct3DSurface8Adapter::~Direct3DSurface8Adapter()
{
}

/*** IUnknown methods ***/
HRESULT Direct3DSurface8Adapter::QueryInterface(THIS_ REFIID riid, void** ppvObj)
{
	LOG(error) << "Called unsupported method: IDirect3DSurface8::QueryInterface";
	abort();
}

ULONG Direct3DSurface8Adapter::AddRef(THIS)
{
	return delegate->AddRef();
}

ULONG Direct3DSurface8Adapter::Release(THIS)
{
	// TODO: Free this once refcount reaches zero
	return delegate->Release();
}

/*** IDirect3DSurface8 methods ***/
HRESULT Direct3DSurface8Adapter::GetDevice(THIS_ d3d8::IDirect3DDevice8** ppDevice)
{
	LOG(error) << "Called unsupported method: IDirect3DSurface8::GetDevice";
	abort();
}

HRESULT Direct3DSurface8Adapter::SetPrivateData(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags)
{
	LOG(error) << "Called unsupported method: IDirect3DSurface8::SetPrivateData";
	abort();
}

HRESULT Direct3DSurface8Adapter::GetPrivateData(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData)
{
	LOG(error) << "Called unsupported method: IDirect3DSurface8::GetPrivateData";
	abort();
}

HRESULT Direct3DSurface8Adapter::FreePrivateData(THIS_ REFGUID refguid)
{
	LOG(error) << "Called unsupported method: IDirect3DSurface8::FreePrivateData";
	abort();
}

HRESULT Direct3DSurface8Adapter::GetContainer(THIS_ REFIID riid, void** ppContainer)
{
	LOG(error) << "Called unsupported method: IDirect3DSurface8::GetContainer";
	abort();
}

HRESULT Direct3DSurface8Adapter::GetDesc(THIS_ d3d8::D3DSURFACE_DESC* pDesc)
{
	D3DSURFACE_DESC desc;
	ZeroMemory(&desc, sizeof(D3DSURFACE_DESC));

	auto result = delegate->GetDesc(&desc);
	pDesc->Format = convert(desc.Format);
	pDesc->Type = (d3d8::D3DRESOURCETYPE) desc.Type;
	pDesc->Usage = desc.Usage;
	pDesc->Pool = (d3d8::D3DPOOL) desc.Pool;
	pDesc->MultiSampleType = (d3d8::D3DMULTISAMPLE_TYPE) desc.MultiSampleType;
	pDesc->Width = desc.Width;
	pDesc->Height = desc.Height;

	return handleD3dError("GetDesc", result);
}

HRESULT Direct3DSurface8Adapter::LockRect(THIS_ d3d8::D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
	return handleD3dError("LockRect", delegate->LockRect((D3DLOCKED_RECT*)pLockedRect, pRect, Flags));
}

HRESULT Direct3DSurface8Adapter::UnlockRect(THIS)
{
	return handleD3dError("UnlockRect", delegate->UnlockRect());
}
