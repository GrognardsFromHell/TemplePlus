
#include "stdafx.h"
#include "dungeon_master.h"
#include <debugui.h>
#include "gamesystems/objects/objsystem.h"
#include "gamesystems/gamesystems.h"
#include <critter.h>
#include "ui/ui.h"
//#include "ui/ui_debug.h"
#include "ui/widgets/widgets.h"
#include "tig/tig_startup.h"
#include <graphics/shaperenderer2d.h>
#include <ui/ui_assets.h>
#include <tig/tig_texture.h>
#include <ui/ui_render.h>
#include <location.h>
#include <graphics/mdfmaterials.h>
#include <tig/tig_mouse.h>

DungeonMaster dmSys;

static bool isActive = true;
static bool isActionActive = false;

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



bool DungeonMaster::IsActionActive(){
	return isActionActive;
}



void DungeonMaster::Render(){

	if (!IsActive())
		return;

	auto rect = TigRect(0,0,96,96);
	ImGui::Begin("Dungeon Master", &isActive);
	auto wndPos = ImGui::GetWindowPos();
	auto wndWidth = ImGui::GetWindowWidth();
	rect.x = wndPos.x + wndWidth/2 - rect.width/2; rect.y = wndPos.y - rect.height;
	

	if (ImGui::CollapsingHeader("Spawn Monster")) {
		
		const char* listbox_items[] = { "Any" , "Aberration", "Animal", "Beast", "Construct", "Dragon", "Elemental", "Fey", "Giant", "Humanoid" 
			, "Magical Beast", "Monstrous Humanoid", "Ooze" , "Outsider" , "Plant" , "Shapechanger" , "Undead" , "Vermin"};
		const char* subcatItems[] = { "Any", "Air", "Aquatic", "Extraplanar", "Cold", "Chaotic", "Demon", "Devil", "Dwarf", "Earth", "Electricity", "Elf", "Evil", "Fire", "Formian"
			, "Gnoll" , "Gnome" , "Goblinoid" , "Good" , "Guardinal" , "Half Orc" , "Halfling" , "Human" , "Lawful" , "Incorporeal", "Orc", "Reptilian", "Slaadi", "Water" };
		ImGui::Combo("Category", &mCategoryFilter, listbox_items, 18, 8);
		ImGui::Combo("Subcategory", &mSubcategoryFilter, subcatItems, 29, 8);

		for (auto it: monsters){

			if (FilterResult(it.second)){
				
				if (ImGui::CollapsingHeader(fmt::format("{} | {}", it.first, it.second.name.c_str()).c_str())) {
					auto protHndl = objSystem->GetProtoHandle(it.first);
					if (ImGui::Button("Spawn")) {
						mObjSpawnProto = it.first;
						isActionActive = true;
						mActionTimeStamp = timeGetTime();
						mouseFuncs.SetCursorFromMaterial("art\\interface\\cursors\\DungeonMaster.mdf");
					}

					auto obj = objSystem->GetObject(protHndl);
					
					//auto hdNum = objects.GetHitDiceNum(protHndl);
					auto hdNum = objects.GetHitDiceNum(protHndl);
					auto lvls = objects.StatLevelGet(protHndl, stat_level);
					ImGui::Text(fmt::format("HD {}  Class Levels {}", hdNum, lvls ).c_str());
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

	UiRenderer::DrawTexture(mTexId, rect);
}

bool DungeonMaster::HandleMsg(const TigMsg & msg){

	if (msg.createdMs < mActionTimeStamp + 100) {
		return false;
	}

	if (msg.type == TigMsgType::MOUSE) {
		auto &mouseMsg = *(TigMsgMouse*)&msg;

		// RMB - release
		if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_RMB_RELEASED) {
			mObjSpawnProto = 0;
			isActionActive = false;
			return true;
		}


		if (!mObjSpawnProto)
			return false;

		objHndl protHndl = objSystem->GetProtoHandle(mObjSpawnProto);
		if (!protHndl)
			return false;

		LocAndOffsets mouseLoc;
		locSys.GetLocFromScreenLocPrecise(mouseMsg.x,mouseMsg.y, mouseLoc);

		auto newHndl = objSystem->CreateObject(protHndl, mouseLoc.location);
		
		mObjSpawnProto = 0;
		isActionActive = false;

		return true;

	}

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
		
		// get category
		newRecord.monsterCat = critterSys.GetCategory(protHndl);
		newRecord.monsterSubtypes = critterSys.GetSubcategoryFlags(protHndl);

		// get factions
		std::vector<int> factions;
		for (int i = 0; i < 50; i++) {
			int fac = obj->GetInt32(obj_f_npc_faction, i);
			if (fac == 0) break;
			factions.push_back(fac);
		}
		if (factions.size())
			newRecord.factions = factions;

		if (newRecord.monsterCat == MonsterCategory::mc_type_humanoid 
			&& (newRecord.monsterSubtypes & MonsterSubcategoryFlag::mc_subtype_human
				|| !newRecord.monsterSubtypes) ){
			humanoids[protoNum] = newRecord;
		}
		else{
			monsters[protoNum] = newRecord;
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

	if (!mIsInited) {
		textureFuncs.RegisterTexture("art\\interface\\dungeon_master_ui\\DU.tga", &mTexId);
		tig->GetMdfFactory().LoadMaterial("art\\interface\\cursors\\DungeonMaster.mdf");
		mIsInited = true;
	}
}

bool DungeonMaster::FilterResult(Record & record){

	if (mCategoryFilter > 0) {
		MonsterCategory asdf = (MonsterCategory)(mCategoryFilter - 1);
		if (record.monsterCat != asdf)
			return false;
	}
	
	if (mSubcategoryFilter > 0) {
		auto subcatFlag = (MonsterSubcategoryFlag)(1 << (mSubcategoryFilter-1) );
		if (!(record.monsterSubtypes & subcatFlag) ) {
			return false;
		}
	}

	

	if (record.protoId > 14999)
		return false;

	return true;
}


