
#include "stdafx.h"
#include "tig_mouse.h"
#include "tig_shader.h"
#include "tig_texture.h"
#include "graphics.h"

#include <vector>

MouseFuncs mouseFuncs;
GlobalStruct<TigMouseState, 0x10D25184> mouseState;

vector<int> stashedCursorShaderIds;

struct OriginalMouseFuncs : AddressTable {

	int(__cdecl *SetCursor)(int shaderId);
	void(__cdecl *ResetCursor)();

	OriginalMouseFuncs() {
		rebase(SetCursor, 0x101DDDD0);
		rebase(ResetCursor, 0x101DD780);
	}
} orgMouseFuncs;

static bool SetCursorFromShaderId(int shaderId) {
	TigShader shader;
	if (shaderFuncs.GetLoaded(shaderId, &shader)) {
		logger->error("Unable to get or load cursor shader {}", shaderId);
		return false;
	}

	int textureId;
	shader.GetTextureId(shader.data, &textureId);

	TigTextureRegistryEntry textureEntry;
	if (textureFuncs.GetLoaded(textureId, &textureEntry)) {
		logger->error("Unable to get mouse cursor texture {}", textureId);
		return false;
	}

	auto texture = textureEntry.buffer->d3dtexture->delegate;
	IDirect3DSurface9 *surface = nullptr;
	if (handleD3dError("GetSurfaceLevel", texture->GetSurfaceLevel(0, &surface)) != D3D_OK) {
		logger->error("Unable to get surface of cursor texture.");
		return false;
	}

	auto device = video->d3dDevice->delegate;
	if (handleD3dError("SetCursorProperties", device->SetCursorProperties(0, 0, surface)) != D3D_OK) {		
		logger->error("Unable to set cursor properties.");
	}
	surface->Release();
	return true;
}

void MouseFuncs::RefreshCursor() {
	if (!stashedCursorShaderIds.empty()) {
		SetCursorFromShaderId(stashedCursorShaderIds.back());
	}
}

/**
 * Sadly ToEE calls this not with a cursor ID but rather with a shader id. So what we do here is we extract the texture from the shader,
 * then the surface from the texture and use the D3D9 API to set the cursor.
 */
int __cdecl MouseFuncs::SetCursor(int shaderId) {
	if (!SetCursorFromShaderId(shaderId)) {
		return 10; // some non 0 number
	}

	stashedCursorShaderIds.push_back(shaderId);
	return 0;
}

void __cdecl MouseFuncs::ResetCursor() {
	// The back is the one on screen
	if (!stashedCursorShaderIds.empty()) {
		stashedCursorShaderIds.pop_back();
	}

	// The back is the one on screen
	if (!stashedCursorShaderIds.empty()) {
		auto shaderId = stashedCursorShaderIds.back();
		SetCursorFromShaderId(shaderId);
	}
}

void hook_mouse() {

	MH_CreateHook(orgMouseFuncs.SetCursor, MouseFuncs::SetCursor, reinterpret_cast<void**>(&orgMouseFuncs.SetCursor));
	MH_CreateHook(orgMouseFuncs.ResetCursor, MouseFuncs::ResetCursor, reinterpret_cast<void**>(&orgMouseFuncs.ResetCursor));

}
