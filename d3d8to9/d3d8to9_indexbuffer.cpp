
#include "d3d8to9_indexbuffer.h"

Direct3DIndexBuffer8Adapter::Direct3DIndexBuffer8Adapter()
{
}

Direct3DIndexBuffer8Adapter::~Direct3DIndexBuffer8Adapter()
{
}

/*** IUnknown methods ***/
HRESULT Direct3DIndexBuffer8Adapter::QueryInterface(THIS_ REFIID /*riid*/, void** /*ppvObj*/) {
	return E_NOINTERFACE;
}
ULONG Direct3DIndexBuffer8Adapter::AddRef(THIS) {
	return delegate->AddRef();
}
ULONG Direct3DIndexBuffer8Adapter::Release(THIS) {
	// TODO: Free once Release == 0
	return delegate->Release();
}

/*** IDirect3DResource8 methods ***/
HRESULT Direct3DIndexBuffer8Adapter::GetDevice(THIS_ d3d8::IDirect3DDevice8** ppDevice) {
	LOG(error) << "Unsupported d3d function called: GetDevice";
	abort();
}
HRESULT Direct3DIndexBuffer8Adapter::SetPrivateData(THIS_ REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags) {
	// TODO: Inspect Flags
	return delegate->SetPrivateData(refguid, pData, SizeOfData, Flags);
}
HRESULT Direct3DIndexBuffer8Adapter::GetPrivateData(THIS_ REFGUID refguid, void* pData, DWORD* pSizeOfData) {
	return delegate->GetPrivateData(refguid, pData, pSizeOfData);
}
HRESULT Direct3DIndexBuffer8Adapter::FreePrivateData(THIS_ REFGUID refguid) {
	return delegate->FreePrivateData(refguid);
}
DWORD Direct3DIndexBuffer8Adapter::SetPriority(THIS_ DWORD PriorityNew) {
	return delegate->SetPriority(PriorityNew);
}
DWORD Direct3DIndexBuffer8Adapter::GetPriority(THIS) {
	return delegate->GetPriority();
}
void Direct3DIndexBuffer8Adapter::PreLoad(THIS) {
	delegate->PreLoad();
}
d3d8::D3DRESOURCETYPE Direct3DIndexBuffer8Adapter::GetType(THIS) {
	// enums are compatible
	return (d3d8::D3DRESOURCETYPE) delegate->GetType();
}
HRESULT Direct3DIndexBuffer8Adapter::Lock(THIS_ UINT OffsetToLock, UINT SizeToLock, BYTE** ppbData, DWORD Flags) {
	// TODO: Inspect flags
	return delegate->Lock(OffsetToLock, SizeToLock, (void**)ppbData, Flags);
}
HRESULT Direct3DIndexBuffer8Adapter::Unlock(THIS) {
	return delegate->Unlock();
}
HRESULT Direct3DIndexBuffer8Adapter::GetDesc(THIS_ d3d8::D3DINDEXBUFFER_DESC *pDesc) {
	LOG(error) << "Unsupported d3d function called: GetDesc";
	abort();
}
