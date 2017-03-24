
#include "stdafx.h"
#include "dungeon_master.h"
#include <debugui.h>
#include "gamesystems/objects/objsystem.h"
#include <critter.h>
//#include "ui/ui.h"
//#include "ui/ui_debug.h"
//#include "ui/widgets/widgets.h"
//#include "tig/tig_startup.h"
//#include <graphics/shaperenderer2d.h>

DungeonMaster dmSys;

static bool isActive = true;
static bool isActionActive = false;

bool DungeonMaster::IsActive(){
	return isActive;
}

bool DungeonMaster::IsActionActive(){
	return isActionActive;
}



void DungeonMaster::Render(){

	
	
	ImGui::Begin("Dungeon Master", &isActive);


	if (ImGui::CollapsingHeader("Monsters")) {
		static bool bugbears = true;
		ImGui::Checkbox("Bugbears", &bugbears);
		

		for (auto it: monsters){

			if (FilterResult(it.second)){
				if (ImGui::Button(fmt::format("%d | %s ", it.first, it.second.name.c_str()).c_str())){
					mObjSpawnProto = it.first;
					isActionActive = true;
				}
			}

			
		}
	}

	if (ImGui::CollapsingHeader("Weapons")) {

		for (auto it : weapons) {
			ImGui::BulletText("%d | %s ", it.first, it.second.name.c_str());
		}
	}

	ImGui::End();

}

bool DungeonMaster::HandleMsg(TigMsg & msg){

	return false;
}

void DungeonMaster::InitEntry(int protoNum){
	auto protHndl = objSystem->GetProtoHandle(protoNum);
	if (!protHndl)
		return;

	auto obj = objSystem->GetObject(protHndl);
	if (obj->IsNPC()){

		auto race = obj->GetInt32(obj_f_critter_race);
		auto newRecord = DungeonMaster::Record();
		newRecord.protoId = protoNum;
		auto desc = description.getDisplayName(protHndl);
		if (desc)
			newRecord.name = desc;
		else
			newRecord.name = "Unknown";
		
		newRecord.monsterCat = critterSys.GetCategory(protHndl);

		if (newRecord.monsterCat == MonsterCategory::mc_type_humanoid){
			humanoids[protoNum] = newRecord;
		}
		else{
			monsters[protoNum] = newRecord;
		}
		
		// get factions
		std::vector<int> factions;
		for (int i = 0; i < 50; i++) {
			int fac = obj->GetInt32(obj_f_npc_faction, i);
			if (fac == 0) break;
			factions.push_back(fac);
		}
		


	}

	if (obj->type == obj_t_weapon){
		auto newRecord = DungeonMaster::Record();
		newRecord.protoId = protoNum;
		auto desc = description.getDisplayName(protHndl);
		if (desc)
			newRecord.name = desc;
		else
			newRecord.name = "Unknown";
		weapons[protoNum] = newRecord;
	}
}

bool DungeonMaster::FilterResult(Record & record){
	if (record.protoId < 14020)
		return true;

	return false;
}
