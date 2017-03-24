
#include "stdafx.h"
#include "dungeon_master.h"
#include <debugui.h>
#include "gamesystems/objects/objsystem.h"
#include "ui/ui.h"
//#include "ui/ui_debug.h"
#include "ui/widgets/widgets.h"
#include "tig/tig_startup.h"
#include <graphics/shaperenderer2d.h>
#include <ui/ui_assets.h>
#include <tig/tig_texture.h>
#include <ui/ui_render.h>

DungeonMaster dmSys;

static bool isActive = true;

bool DungeonMaster::IsActive(){
	return isActive;
}

void DungeonMaster::Show(){
	isActive = true;
}

void DungeonMaster::Hide(){
	isActive = false;
}

void DungeonMaster::Toggle(){
	if (IsActive())
		Hide();
	else
		Show();
}



void DungeonMaster::Render(){

	if (!IsActive())
		return;

	auto rect = TigRect(0,0,96,96);
	ImGui::Begin("Dungeon Master", &isActive);
	auto wndPos = ImGui::GetWindowPos();
	auto wndWidth = ImGui::GetWindowWidth();
	rect.x = wndPos.x + wndWidth/2 - rect.width/2; rect.y = wndPos.y - rect.height;
	

	if (ImGui::CollapsingHeader("Monsters")) {
		for (auto it: monsters){
			if (it.first < 14020)
				ImGui::BulletText("%d | %s ", it.first, it.second.name.c_str());
			else
				break;
		}
	}

	ImGui::End();
	//if (!mIsInited) {
	//	//textureFuncs.RegisterTexture("art\\interface\\item_creation_ui\\ITEM_CREATION_WIDENED_0_0.tga", &mTexId);
	//	mIsInited = true;
	//}

	UiRenderer::DrawTexture(mTexId, rect);
	//UiRenderer::DrawTexture(590, rect);
}

void DungeonMaster::InitEntry(int protoNum){
	auto protHndl = objSystem->GetProtoHandle(protoNum);
	if (!protHndl)
		return;

	auto obj = objSystem->GetObject(protHndl);
	if (obj->IsNPC()){
		auto newRecord = DungeonMaster::Record();
		newRecord.protoId = protoNum;
		auto desc = description.getDisplayName(protHndl);
		if (desc)
			newRecord.name = desc;
		else
			newRecord.name = "Unknown";
		monsters[protoNum] = newRecord;
	}

	if (!mIsInited) {
		textureFuncs.RegisterTexture("art\\interface\\dungeon_master_ui\\DU.tga", &mTexId);
		mIsInited = true;
	}
}
