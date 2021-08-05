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
#include <tig\tig_msg.h>
#include <messages\messagequeue.h>

MouseFuncs mouseFuncs;
temple::GlobalStruct<TigMouseState, 0x10D25184> mouseState;

static std::vector<int> stashedCursorShaderIds;

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

/* 0x101DD070 */
void MouseFuncs::SetPos(int x, int y, int wheelDelta)
{

	//return temple::GetRef<void(__cdecl)(int, int, int)>(0x101DD070)(x, y, wheelDelta);

	TigMsgMouse msg;
	msg.createdMs = timeGetTime();
	msg.type = TigMsgType::MOUSE;
	msg.buttonStateFlags = 0;
	 

	auto& mouseIsStatic = temple::GetRef<int>(0x10D25588);
	auto& mouseLastChange = temple::GetRef<int>(0x10D251C4);
	auto systemTime = temple::GetRef<int>(0x11E74578);

	

	static int slowMoveRefX = mouseState->x, slowMoveRefY = mouseState->y;
	
	auto slowMoveDelta = abs(x - slowMoveRefX) + abs(y - slowMoveRefY);
	// systemTime += 2793682137;
	auto timeDeltaMs = abs(systemTime - mouseLastChange); // abs delta in case the time stamp is above the integer limit
	if (x != mouseState->x || y != mouseState->y ) {
		mouseIsStatic = 0;
		//if (slowMoveDelta  > 0) { // this was the effective vanilla condition
		// There was an issue where POS_CHANGE_SLOW wasn't fired, which caused issues with
		// hovering items in the inventory UI.
		// Not sure why, but in debug mode I see that these mouse messages would be farther apart,
		// i.e. with bigger time and x,y deltas. The only difference is slower framerate, so maybe
		// the cause of this bug is the increased frame rate - the smaller deltas between messages
		// means you'd never cross the 35ms threshold (below).
		if (slowMoveDelta > 5*timeDeltaMs || timeDeltaMs > 250) {

			/*TigCharMsg chrMsg;
			chrMsg.type = TigMsgType::CHAR;
			chrMsg.charCode = 'Y';
			chrMsg.createdMs = msg.createdMs;
			chrMsg._unused1 = timeDeltaMs;
			chrMsg._unused2 = slowMoveDelta;
			chrMsg._unused3 = slowX + slowY;
			messageQueue->Enqueue(chrMsg);*/

			slowMoveRefX = x; slowMoveRefY = y;
			mouseLastChange = systemTime;
		}
		msg.buttonStateFlags = MouseStateFlags::MSF_POS_CHANGE;

	}
	else if (!mouseIsStatic &&	( systemTime > (mouseLastChange + 35) ) ) {
		mouseIsStatic = 1;
		msg.buttonStateFlags |= MouseStateFlags::MSF_POS_CHANGE_SLOW;
	}
	//if (msg.buttonStateFlags == 0) { // DEBUG - none of the above:
	//	TigCharMsg chrMsg;
	//	chrMsg.type = TigMsgType::CHAR;
	//	chrMsg.charCode = 'Z';
	//	chrMsg.createdMs = msg.createdMs;
	//	chrMsg._unused1 = systemTime;
	//	chrMsg._unused2 = mouseLastChange;
	//	chrMsg._unused3 = x+y;
	//	messageQueue->Enqueue(chrMsg);
	//}
	if (wheelDelta) {
		msg.buttonStateFlags |= MouseStateFlags::MSF_SCROLLWHEEL_CHANGE;
	}

	mouseState->x = msg.x = x;
	mouseState->y = msg.y = y;
	msg.arg3 = wheelDelta;

	auto& downFlags = temple::GetRef<int[3]>(0x10300944);
	auto& downFlags2 = temple::GetRef<int[3]>(0x10300938);
	auto& mouseSettings = temple::GetRef<int[3]>(0x1030095C);

	for (int i = 0; i < 3; ++i) {
		if ((downFlags[i] | downFlags2[i]) & mouseState->flags) {
			msg.buttonStateFlags |= 2 * mouseSettings[i];
		}
	}

	if (msg.buttonStateFlags) {
		messageQueue->Enqueue(msg);
	}
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

int(MouseFuncs::SetCursorFromMaterial)(std::string matName){
	auto material = tig->GetMdfFactory().GetByName(matName);

	if (!material) {
		logger->error("Unable to get or load cursor material {}", matName);
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
	auto shaderId = material->GetId();
	stashedCursorShaderIds.push_back(shaderId);
	return true;
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

		//Mouse SetPos
		replaceFunction<void(__cdecl)(int, int, int)>(0x101DD070, [](int x, int y, int wheelDelta) {
			return mouseFuncs.SetPos(x, y, wheelDelta);
			});
			

	}
} mouseFixes;
