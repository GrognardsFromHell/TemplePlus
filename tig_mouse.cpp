
#include "stdafx.h"
#include "tig_mouse.h"
#include "tig_shader.h"
#include "tig_texture.h"
#include "graphics.h"

#include <vector>

MouseFuncs mouseFuncs;

vector<int> stashedCursorShaderIds;

static bool SetCursorFromShaderId(int shaderId) {
	TigShader shader;
	if (shaderFuncs.GetLoaded(shaderId, &shader)) {
		LOG(error) << "Unable to get or load cursor shader " << shaderId;
		return false;
	}

	int textureId;
	shader.GetTextureId(shader.data, &textureId);

	TigTextureRegistryEntry textureEntry;
	if (textureFuncs.GetLoaded(textureId, &textureEntry)) {
		LOG(error) << "Unable to get mouse cursor texture " << textureId;
		return false;
	}

	auto texture = textureEntry.buffer->d3dtexture->delegate;
	IDirect3DSurface9 *surface = nullptr;
	if (handleD3dError("GetSurfaceLevel", texture->GetSurfaceLevel(0, &surface)) != D3D_OK) {
		LOG(error) << "Unable to get surface of cursor texture.";
		return false;
	}

	auto device = video->d3dDevice->delegate;
	if (handleD3dError("SetCursorProperties", device->SetCursorProperties(0, 0, surface)) != D3D_OK) {		
		LOG(error) << "Unable to set cursor properties.";
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
int __cdecl HookedSetCursor(int shaderId) {
	if (!SetCursorFromShaderId(shaderId)) {
		return 10; // some non 0 number
	}

	stashedCursorShaderIds.push_back(shaderId);
	return 0;
}

void __cdecl HookedResetCursor() {
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

	MH_CreateHook(mouseFuncs.SetCursor, HookedSetCursor, reinterpret_cast<void**>(&mouseFuncs.SetCursor));
	MH_CreateHook(mouseFuncs.ResetCursor, HookedResetCursor, reinterpret_cast<void**>(&mouseFuncs.ResetCursor));

}
