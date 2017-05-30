
#include "stdafx.h"
#include "ui.h"
#include "ui_dm.h"
#include "ui/widgets/widgets.h"
#include <graphics/shaperenderer2d.h>
#include "ui_render.h"
#include "gamesystems/gamesystems.h"
#include "dungeon_master.h"
#include "ui_legacysystems.h"
#include "tig/tig_loadingscreen.h"
#include "tig/tig_texture.h"
#include "graphics/mdfmaterials.h"
#include "gamesystems/objects/objsystem.h"
#include "ui_intgame_turnbased.h"

UiDmWidgets uiDm;

bool UiDmWidgets::DmWidgetsInit(const UiSystemConf & conf)
{
	static LgcyWindow dmBtnWnd(conf.width-88, conf.height-102, 24, 24);
	dmBtnWnd.flags = 1;
	dmBtnWndId = uiManager->AddWindow(dmBtnWnd);

	LgcyButton dmBtn("DM btn", dmBtnWndId, 0, 0, 24, 24);

	dmBtn.x += dmBtnWnd.x; dmBtn.y += dmBtnWnd.y;
	dmBtn.render = [](int id) {uiDm.DmBtnRender(id); };
	dmBtn.handleMessage = [](int id, TigMsg* msg){
		if (!config.dungeonMaster)
			return FALSE;
		if (msg->type == TigMsgType::WIDGET){
			auto msgWid = (TigMsgWidget*)msg;
			if (msgWid->widgetEventType == TigMsgWidgetEvent::MouseReleased)
				dmSys.Toggle();
		}

		return TRUE;
	};
	dmBtn.SetDefaultSounds();
	dmBtnId = uiManager->AddButton(dmBtn, dmBtnWndId);

	
	// Load art
	textureFuncs.RegisterTexture("art\\interface\\dungeon_master_ui\\DU.tga", &mTexId);
	textureFuncs.RegisterTexture("art\\interface\\dungeon_master_ui\\Toolbar_Icon.tga", &mIconTexId);
	tig->GetMdfFactory().LoadMaterial("art\\interface\\cursors\\DungeonMaster.mdf");

	// Init caches
	dmSys.InitCaches();

	

	return true;
}

void UiDmWidgets::DmBtnRender(int widId){

	if (!config.dungeonMaster)
		return;

	auto btn = uiManager->GetButton(widId);
	if (!btn)
		return;
	UiRenderer::DrawTexture(mIconTexId, TigRect(btn->x, btn->y, 24,24 ));

	if (dmSys.IsActive() && !dmSys.IsMinimized()){
		UiRenderer::DrawTexture(mTexId, mRaptorRect, TigRect(0,0,96,96));

		auto tgtObj = dmSys.GetTgtObj();
		if (tgtObj && objSystem->IsValidHandle(tgtObj)){
			auto loc = objSystem->GetObject(tgtObj)->GetLocationFull();
			uiIntgameTb.RenderPositioningBlueCircle(loc, tgtObj);
		}

	}
}

void UiDmWidgets::Activate(){
	uiManager->SetHidden(dmBtnWndId, false);
}

void UiDmWidgets::SetButtonVis(bool newState){
	uiManager->SetHidden(dmBtnWndId, !newState);
}

void UiDmWidgets::SetDmPortraitRect(TigRect& rect){
	
	mRaptorRect = rect;
	if (mRaptorRect.width > mRaptorRect.height)
		mRaptorRect.height = mRaptorRect.width;

}
