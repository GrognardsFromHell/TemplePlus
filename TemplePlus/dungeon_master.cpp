
#include "stdafx.h"
#include "dungeon_master.h"
#include <debugui.h>
#include "gamesystems/objects/objsystem.h"
//#include "ui/ui.h"
//#include "ui/ui_debug.h"
//#include "ui/widgets/widgets.h"
//#include "tig/tig_startup.h"
//#include <graphics/shaperenderer2d.h>

DungeonMaster dmSys;

static bool isActive = true;

bool DungeonMaster::IsActive(){
	return isActive;
}



void DungeonMaster::Render(){

	
	
	ImGui::Begin("Dungeon Master", &isActive);


	if (ImGui::CollapsingHeader("Monsters")) {
		for (auto it: monsters){
			if (it.first < 14020)
				ImGui::BulletText("%d | %s ", it.first, it.second.name.c_str());
			else
				break;
		}
	}

	ImGui::End();

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
}
