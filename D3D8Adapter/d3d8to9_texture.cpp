
#include "stdafx.h"
#include "d3d.h"
#include "d3d8to9_texture.h"

IDirect3DTexture9 *GetTextureDelegate(Direct3DTexture8Adapter *adapter)
{
	return adapter->delegate;
}

Direct3DTexture8Adapter *CreateTextureAdapter(IDirect3DTexture9 *delegate) {
	return new Direct3DTexture8Adapter(delegate);
}

void DeleteTextureAdapter(Direct3DTexture8Adapter *adapter) {
	if (adapter->delegate) {
		adapter->delegate->Release();
	}
	delete adapter;
}

void SetTextureDelegate(Direct3DTexture8Adapter *adapter, IDirect3DTexture9 *delegate) {
	// Free old delegate?
	adapter->delegate = delegate;
}

HRESULT Direct3DTexture8Adapter::QueryInterface(THIS_ REFIID /*riid*/, void** /*ppvObj*/)
{
	return E_NOINTERFACE;
}

ULONG Direct3DTexture8Adapter::AddRef(THIS)
{
	return delegate->AddRef();
}

ULONG Direct3DTexture8Adapter::Release(THIS)
{
	// TODO: Free once Release == 0
	return delegate->Release();
}

HRESULT Direct3DTexture8Adapter::GetDevice(THIS_ d3d8::IDirect3DDevice8** ppDevice)
{
	logger->error("Unsupported d3d function called: GetDevice");
	abort();
}

HRESULT Direct3DTexture8Adapter::SetPrivateData(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags)
{
	// TODO: Inspect Flags
	return delegate->SetPrivateData(refguid, pData, SizeOfData, Flags);
}

HRESULT Direct3DTexture8Adapter::GetPrivateData(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData)
{
	return delegate->GetPrivateData(refguid, pData, pSizeOfData);
}

HRESULT Direct3DTexture8Adapter::FreePrivateData(THIS_ REFGUID refguid)
{
	return delegate->FreePrivateData(refguid);
}

DWORD Direct3DTexture8Adapter::SetPriority(THIS_ DWORD PriorityNew)
{
	return delegate->SetPriority(PriorityNew);
}

DWORD Direct3DTexture8Adapter::GetPriority(THIS)
{
	return delegate->GetPriority();
}

void Direct3DTexture8Adapter::PreLoad(THIS)
{
	delegate->PreLoad();
}

d3d8::D3DRESOURCETYPE Direct3DTexture8Adapter::GetType(THIS)
{
	// enums are compatible
	return (d3d8::D3DRESOURCETYPE) delegate->GetType();
}

DWORD Direct3DTexture8Adapter::SetLOD(THIS_ DWORD LODNew)
{
	return delegate->SetLOD(LODNew);
}

DWORD Direct3DTexture8Adapter::GetLOD(THIS)
{
	return delegate->GetLOD();
}

DWORD Direct3DTexture8Adapter::GetLevelCount(THIS)
{
	return delegate->GetLevelCount();
}

HRESULT Direct3DTexture8Adapter::GetLevelDesc(THIS_ UINT Level, d3d8::D3DSURFACE_DESC* pDesc)
{
	logger->error("Unsupported method called: GetLevelDesc");
	abort();
	return EFAULT;
}

HRESULT Direct3DTexture8Adapter::GetSurfaceLevel(THIS_ UINT Level, d3d8::IDirect3DSurface8** ppSurfaceLevel)
{
	logger->error("Unsupported method called: GetSurfaceLevel");
	abort();
	return EFAULT;
}

HRESULT Direct3DTexture8Adapter::LockRect(THIS_ UINT Level, d3d8::D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
	return D3DLOG(delegate->LockRect(Level, (D3DLOCKED_RECT*)pLockedRect, pRect, Flags));	
}

HRESULT Direct3DTexture8Adapter::UnlockRect(THIS_ UINT Level)
{
	return D3DLOG(delegate->UnlockRect(Level));
}

HRESULT Direct3DTexture8Adapter::AddDirtyRect(THIS_ CONST RECT* pDirtyRect)
{
	logger->error("Unsupported method called: AddDirtyRect");
	abort();
	return EFAULT;
}

Direct3DTexture8Adapter::Direct3DTexture8Adapter() : delegate(0)
{
}

Direct3DTexture8Adapter::~Direct3DTexture8Adapter()
{
}
