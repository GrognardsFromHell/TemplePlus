#include "stdafx.h"
#include "tig_mouse.h"
#include "tig_shader.h"
#include "tig_texture.h"
#include "ui/ui_render.h"
#include "tig/tig_msg.h"

#include "tig/tig_startup.h"

#include <graphics/device.h>

#include <vector>
#include <atlcomcli.h>
#include <util/fixes.h>
#include "messages/messagequeue.h"
#include <graphics/mdfmaterials.h>
#include <util/time.h>

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

void MouseFuncs::SetButtonState(MouseButton button, bool pressed)
{
	auto buttonIndex = (uint32_t)button;

	static auto &mouseState = temple::GetRef<TigMouseState>(0x10D25184);

	static const int buttonStatePressed1[3] = { 0x2, 0x10, 0x80 };
	static const int buttonStatePressed2[3] = { 0x4, 0x20, 0x100 };
	static const int buttonStateReleased[3] = { 0x8, 0x40, 0x200 };
	static const MouseStateFlags msgFlagButtonDown[3] = { MouseStateFlags::MSF_LMB_CLICK, MouseStateFlags::MSF_RMB_CLICK, MouseStateFlags::MSF_MMB_CLICK };
	static const MouseStateFlags msgFlagButtonDownRepeat[3] = { MouseStateFlags::MSF_LMB_DOWN, MouseStateFlags::MSF_RMB_DOWN, MouseStateFlags::MSF_MMB_DOWN };
	static const MouseStateFlags msgFlagButtonReleased[3] = { MouseStateFlags::MSF_LMB_RELEASED, MouseStateFlags::MSF_RMB_RELEASED, MouseStateFlags::MSF_MMB_RELEASED };

	auto currentFlags = mouseState.flags;
	mouseState.flags = 0;

	static std::array<uint32_t, 3> buttonEventTimes{ 0, 0, 0 };
	uint32_t now = GetSystemTime();

	bool wasPressed = (currentFlags & (buttonStatePressed1[buttonIndex] | buttonStatePressed2[buttonIndex])) != 0;

	TigMsgMouse msg;
	msg.type = TigMsgType::MOUSE;
	msg.createdMs = now;
	msg.x = mouseState.x;
	msg.y = mouseState.y;
	msg.arg3 = 0;
	msg.buttonStateFlags = 0;

	if (pressed)
	{
		// How is this even triggered... It's processing a state-change to "pressed" while it was already pressed
		if (wasPressed)
		{
			mouseState.flags = buttonStatePressed2[buttonIndex];
			// This might be mouse-button repeats (?) This would not trigger in windowed-mode however
			if (now - buttonEventTimes[buttonIndex] > 250) {
				mouseState.flags |= buttonStatePressed1[buttonIndex];
				buttonEventTimes[buttonIndex] = now;
				msg.buttonStateFlags = msgFlagButtonDownRepeat[buttonIndex];
			}
		}
		else {
			mouseState.flags = buttonStatePressed1[buttonIndex];
			msg.buttonStateFlags = msgFlagButtonDown[buttonIndex];
			buttonEventTimes[buttonIndex] = now;
		}
	}
	else
	{
		if (wasPressed) {
			mouseState.flags = buttonStateReleased[buttonIndex];
			buttonEventTimes[buttonIndex] = now;
			msg.buttonStateFlags = msgFlagButtonReleased[buttonIndex];
		}
	}

	// Restore the "mouse cursor hidden" flag
	if (currentFlags & TigMouseFlags::MF_HIDE_CURSOR) {
		mouseState.flags |= TigMouseFlags::MF_HIDE_CURSOR;
	}

	// Only send the msg if there's a state change
	if (msg.buttonStateFlags) {
		messageQueue->Enqueue(msg);
	}
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
