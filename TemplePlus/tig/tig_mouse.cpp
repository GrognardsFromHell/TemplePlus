#include "stdafx.h"
#include "tig_mouse.h"
#include "tig_shader.h"
#include "tig_texture.h"
#include "graphics/graphics.h"
#include "ui/ui_render.h"

#include <vector>
#include <atlcomcli.h>

MouseFuncs mouseFuncs;
temple::GlobalStruct<TigMouseState, 0x10D25184> mouseState;

vector<int> stashedCursorShaderIds;

struct OriginalMouseFuncs : temple::AddressTable {

	int (__cdecl *SetCursor)(int shaderId);
	void (__cdecl *ResetCursor)();

	/*
		If something is to be dragged around, this contains the texture id of the dragged item/object.
		0 if nothing is to be drawn under the cursor.
	*/
	int* draggedTexId;

	// Specifies the texture rectangle of the texture being dragged around (see draggedTexId).
	RECT* draggedTexRect;

	// The X coordinate within the texture where the user clicked to initiate the drag.
	int* draggedCenterX;

	// The Y coordinate within the texture where the user clicked to initiate the drag.
	int* draggedCenterY;

	OriginalMouseFuncs() {
		rebase(SetCursor, 0x101DDDD0);
		rebase(ResetCursor, 0x101DD780);

		rebase(draggedTexId, 0x10D2558C);
		rebase(draggedTexRect, 0x10D255B0);
		rebase(draggedCenterX, 0x10D255C0);
		rebase(draggedCenterY, 0x10D255C4);
	}

} orgMouseFuncs;

static bool SetCursorFromShaderId(int shaderId) {
	TigShader shader;
	if (shaderFuncs.GetLoaded(shaderId, &shader)) {
		logger->error("Unable to get or load cursor shader {}", shaderId);
		return false;
	}

	int textureId;
	if (shader.GetTextureId(shader.data, &textureId)) {
		logger->info("Cannot set mouse cursor to shader {}, beacuse it has no texture.", shaderId);
		return false;
	}

	TigTextureRegistryEntry textureEntry;
	if (textureFuncs.LoadTexture(textureId, &textureEntry)) {
		logger->error("Unable to get mouse cursor texture {}", textureId);
		return false;
	}

	auto texture = GetTextureDelegate(textureEntry.buffer->d3dtexture);
	CComPtr<IDirect3DSurface9> surface;
	if (D3DLOG(texture->GetSurfaceLevel(0, &surface)) != D3D_OK) {
		logger->error("Unable to get surface of cursor texture.");
		return false;
	}

	int hotspotX = 0;
	int hotspotY = 0;

	// Special handling for cursors that don't have their hotspot on 0,0
	if (strstr(textureEntry.name, "Map_GrabHand_Closed.tga")
		|| strstr(textureEntry.name, "Map_GrabHand_Open.tga")
		|| strstr(textureEntry.name, "SlidePortraits.tga")) {
		hotspotX = textureEntry.rect.width / 2;
		hotspotY = textureEntry.rect.height / 2;
	}

	auto device = graphics->device();
	if (D3DLOG(device->SetCursorProperties(hotspotX, hotspotY, surface)) != D3D_OK) {
		logger->error("Unable to set cursor properties.");
	}
	return true;
}

void MouseFuncs::RefreshCursor() {
	if (!stashedCursorShaderIds.empty()) {
		SetCursorFromShaderId(stashedCursorShaderIds.back());
	}
}

void MouseFuncs::DrawCursor() {

	auto texId = *orgMouseFuncs.draggedTexId;

	if (texId) {
		TigRect rect;
		rect.x = mouseState->x + *orgMouseFuncs.draggedCenterX;
		rect.y = mouseState->y + *orgMouseFuncs.draggedCenterY;
		rect.width = orgMouseFuncs.draggedTexRect->right;
		rect.height = orgMouseFuncs.draggedTexRect->bottom;

		UiRenderer::DrawTexture(texId, rect);
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
