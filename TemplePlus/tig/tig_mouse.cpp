#include "stdafx.h"
#include "tig_mouse.h"
#include "tig_shader.h"
#include "tig_texture.h"
#include "ui/ui_render.h"

#include "tig/tig_startup.h"

#include <graphics/device.h>

#include <vector>
#include <atlcomcli.h>
#include <util/fixes.h>
#include <graphics/mdfmaterials.h>

MouseFuncs mouseFuncs;
temple::GlobalStruct<TigMouseState, 0x10D25184> mouseState;

vector<int> stashedCursorShaderIds;

struct OriginalMouseFuncs : temple::AddressTable {

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
		rebase(draggedTexId, 0x10D2558C);
		rebase(draggedTexRect, 0x10D255B0);
		rebase(draggedCenterX, 0x10D255C0);
		rebase(draggedCenterY, 0x10D255C4);
	}

} orgMouseFuncs;

static bool SetCursorFromShaderId(int shaderId) {

	auto material = tig->GetMdfFactory().GetById(shaderId);

	if (!material) {
		logger->error("Unable to get or load cursor shader {}", shaderId);
		return false;
	}

	auto primaryTexture = material->GetPrimaryTexture();

	if (!primaryTexture) {
		logger->error("Material {} has no texture and cannot be used as a cursor", material->GetName());
		return false;
	}

	int hotspotX = 0;
	int hotspotY = 0;

	// Special handling for cursors that don't have their hotspot on 0,0
	if (strstr(primaryTexture->GetName().c_str(), "Map_GrabHand_Closed.tga")
		|| strstr(primaryTexture->GetName().c_str(), "Map_GrabHand_Open.tga")
		|| strstr(primaryTexture->GetName().c_str(), "SlidePortraits.tga")) {
		hotspotX = primaryTexture->GetContentRect().width / 2;
		hotspotY = primaryTexture->GetContentRect().height / 2;
	}

	tig->GetRenderingDevice().SetCursor(hotspotX, hotspotY, primaryTexture);
	return true;
}

void MouseFuncs::RefreshCursor() {
	if (!stashedCursorShaderIds.empty()) {
		SetCursorFromShaderId(stashedCursorShaderIds.back());
	}
}

void MouseFuncs::DrawItemUnderCursor() const {

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

bool MouseFuncs::SetDraggedIcon(uint32_t textureId, int centerX, int centerY)
{
	static auto tig_mouse_set_dragged_icon = temple::GetPointer<signed int(int textureId, int draggedCenterX, int draggedCenterY)>(0x101dd500);
	return tig_mouse_set_dragged_icon(textureId, centerX, centerY) == 0;
}

void MouseFuncs::SetCursorDrawCallback(CursorDrawCallback callback, uint32_t id)
{
	mCursorDrawCallback = callback;
	mCursorDrawCallbackId = id;
}

void MouseFuncs::InvokeCursorDrawCallback() const
{
	if (mCursorDrawCallback) {
		mCursorDrawCallback(mouseState->x, mouseState->y);
	}
}

/**
 * Sadly ToEE calls this not with a cursor ID but rather with a shader id. So what we do here is we extract the texture from the material.
 */
int MouseFuncs::SetCursor(int shaderId) {
	if (!SetCursorFromShaderId(shaderId)) {
		return 10; // some non 0 number
	}

	stashedCursorShaderIds.push_back(shaderId);
	return 0;
}

void MouseFuncs::ResetCursor() {
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

static class MouseFixes : TempleFix {
public:
	void apply() override {
		replaceFunction(0x101DDDD0, MouseFuncs::SetCursor);
		replaceFunction(0x101DD780, MouseFuncs::ResetCursor);

		using CursorDrawCallback = void(*)(int, int, void*);

		// set_mouse_capture_callback
		replaceFunction<int(CursorDrawCallback, void*)>(0x101dd5c0, [](CursorDrawCallback callback, void* arg) {
			if (!callback) {
				mouseFuncs.SetCursorDrawCallback(nullptr, 0);
			} else {
				// Bind the callback arg directly into the callback as a function object
				auto wrappedCallback = [=](int x, int y) { return callback(x, y, arg); };
				auto callbackId = (uint32_t)callback;
				mouseFuncs.SetCursorDrawCallback(wrappedCallback, callbackId);
			}
			return 0;
		});

		// get_mouse_capture_callback
		replaceFunction<CursorDrawCallback()>(0x101dd5e0, []() {
			return (CursorDrawCallback)mouseFuncs.GetCursorDrawCallbackId();
		});

	}
} fix;
