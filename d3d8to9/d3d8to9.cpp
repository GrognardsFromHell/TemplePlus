
#include "d3d8to9_private.h"

#include "d3d8to9_rootobj.h"

bool useD3dEx = true;

d3d8::IDirect3D8* WINAPI HookedDirect3DCreate8(UINT sdkVersion) {
	// Effectively we will store the actual pointer to the adaptee behind the adapter
	auto adapter = new Direct3D8Adapter;

	if (useD3dEx) {
		auto result = Direct3DCreate9Ex(D3D_SDK_VERSION, &adapter->delegate);
		handleError("CreateDirect3D8", result);
	} else {
		adapter->delegate = static_cast<IDirect3D9Ex*>(Direct3DCreate9(D3D_SDK_VERSION));
	}

	return reinterpret_cast<d3d8::IDirect3D8*>(adapter);
}

void hook_directx() {

	HMODULE d3d8Library = GetModuleHandleA("d3d8");
	LPVOID direct3dCreate = GetProcAddress(d3d8Library, "Direct3DCreate8");

	MH_CreateHook(direct3dCreate, HookedDirect3DCreate8, NULL);

}
