
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
#include "ui/ui_intgame_select.h"
#include "ui/ui_intgame_turnbased.h"
#include "ui/ui_systems.h"
#include "ui/ui_ingame.h"
#include "raycast.h"

DungeonMaster dmSys;

static bool isActive = true;
static bool isActionActive = false;
static bool isMinimized = false;

bool DungeonMaster::IsActive(){
	return isActive;
}

bool DungeonMaster::IsMinimized(){
	return isMinimized;
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
		isMinimized = ImGui::GetWindowCollapsed();
		auto wndPos = ImGui::GetWindowPos();
		auto wndWidth = ImGui::GetWindowWidth();
		rect.x = wndPos.x + wndWidth/2 - rect.width/2; rect.y = wndPos.y - rect.height;
	

		if (ImGui::TreeNodeEx("Monsters", ImGuiTreeNodeFlags_CollapsingHeader)) {
		
			if (ImGui::CollapsingHeader("Filter")){
				const char* listbox_items[] = { "Any" , "Aberration", "Animal", "Beast", "Construct", "Dragon", "Elemental", "Fey", "Giant", "Humanoid"
					, "Magical Beast", "Monstrous Humanoid", "Ooze" , "Outsider" , "Plant" , "Shapechanger" , "Undead" , "Vermin" };
				const char* subcatItems[] = { "Any", "Air", "Aquatic", "Extraplanar", "Cold", "Chaotic", "Demon", "Devil", "Dwarf", "Earth", "Electricity", "Elf", "Evil", "Fire", "Formian"
					, "Gnoll" , "Gnome" , "Goblinoid" , "Good" , "Guardinal" , "Half Orc" , "Halfling" , "Human" , "Lawful" , "Incorporeal", "Orc", "Reptilian", "Slaadi", "Water" };
				ImGui::Combo("Category", &mCategoryFilter, listbox_items, 18, 8);
				ImGui::Combo("Subcategory", &mSubcategoryFilter, subcatItems, 29, 8);
				ImGui::InputInt("Faction", &mFactionFilter);
			}
			
			
			for (auto it: monsters){

				if (FilterResult(it.second)){
					RenderMonster(it.second);

				}

			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Weapons", ImGuiTreeNodeFlags_CollapsingHeader)) {

			for (auto it : weapons) {
				ImGui::BulletText("%d | %s ", it.first, it.second.name.c_str());
			}
			ImGui::TreePop();
		}

	ImGui::End();

	if (!isMinimized){
		UiRenderer::DrawTexture(mTexId, rect);


		if (mTgtObj && objSystem->IsValidHandle(mTgtObj)){
			UiRenderer::PushFont(PredefinedFont::PRIORY_12);
			tigFont.Draw(description.getDisplayName(mTgtObj), TigRect(rect.x+105, rect.y + 50,100,15), TigTextStyle::standardWhite);
			UiRenderer::PopFont();
			auto loc = objSystem->GetObject(mTgtObj)->GetLocationFull();
			uiIntgameTb.RenderPositioningBlueCircle(loc, mTgtObj);
		}
		else{
			mTgtObj = objHndl::null;
		}

		if (mEditedObj && objSystem->IsValidHandle(mEditedObj)){
			RenderEditedObj();
		} 
		else{
			mEditedObj = objHndl::null;
		}
	}
	


}

void DungeonMaster::RenderEditedObj(){
	ImGui::Begin("Edit Object");
	ImGui::Text(fmt::format("Name: {}", description.getDisplayName(mEditedObj)).c_str());
	ImGui::End();
}

bool DungeonMaster::HandleMsg(const TigMsg & msg){

	/*if (msg.createdMs < mActionTimeStamp + 100) {
		return false;
	}*/

	if (msg.type == TigMsgType::MOUSE) {
		auto &mouseMsg = *(TigMsgMouse*)&msg;

		// acquire target from cursor
		if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_POS_CHANGE){
			mTgtObj = uiSystems->GetInGame().PickObject(mouseMsg.x,mouseMsg.y,RaycastFlags::HasRadius);
		}

		if (HandleSpawning(msg))
			return true;

		if (HandleEditing(msg))
			return true;
	}

	return false;
}

bool DungeonMaster::HandleSpawning(const TigMsg& msg){
	
	if (!IsActionActive())
		return false;

	if (msg.type == TigMsgType::MOUSE) {
		auto &mouseMsg = *(TigMsgMouse*)&msg;

		// RMB - click (so it seizes the event and doesn't spawn a radial menu)
		if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_RMB_CLICK) {
			if (IsActionActive()) {
				mObjSpawnProto = 0;
				isActionActive = false;
				mouseFuncs.ResetCursor();
				return true;
			}
		}


		if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_LMB_CLICK) {
			if (!mObjSpawnProto)
				return false;

			objHndl protHndl = objSystem->GetProtoHandle(mObjSpawnProto);
			if (!protHndl)
				return false;

			LocAndOffsets mouseLoc;
			locSys.GetLocFromScreenLocPrecise(mouseMsg.x, mouseMsg.y, mouseLoc);

			auto newHndl = objSystem->CreateObject(protHndl, mouseLoc.location);

			mObjSpawnProto = 0;
			isActionActive = false;
			mouseFuncs.ResetCursor();

			return true;
		}
	}
	return false;
}

bool DungeonMaster::HandleEditing(const TigMsg & msg){

	if (msg.type == TigMsgType::MOUSE) {
		auto &mouseMsg = *(TigMsgMouse*)&msg;

		// RMB - click (so it seizes the event and doesn't spawn a radial menu)
		if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_RMB_CLICK) {
			mEditedObj = mTgtObj;
			return true;
		}
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


		// class levels
		auto levels = obj->GetInt32Array(obj_f_critter_level_idx);
		for (auto i = 0; i < levels.GetSize(); i++) {
			auto classLvl = (Stat)levels[i];
			newRecord.classLevels[classLvl]++;
		}

		auto hdNum = objects.GetHitDiceNum(protHndl);
		newRecord.hitDice = hdNum;

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

void DungeonMaster::RenderMonster(Record& record){
	
	if (ImGui::CollapsingHeader(fmt::format("{} | {} ({} HD)", record.protoId, record.name.c_str(), record.hitDice).c_str())) {
		auto protHndl = objSystem->GetProtoHandle(record.protoId);
		if (ImGui::Button("Spawn")) {
			mObjSpawnProto = record.protoId;
			isActionActive = true;
			mActionTimeStamp = timeGetTime();
			mouseFuncs.SetCursorFromMaterial("art\\interface\\cursors\\DungeonMaster.mdf");
		}

		auto obj = objSystem->GetObject(protHndl);

		
		auto lvls = objects.StatLevelGet(protHndl, stat_level);
		ImGui::Text(fmt::format("Class Levels {}", lvls).c_str());

		// Factions
		if (record.factions.size()){
			std::string factionText(fmt::format("Factions: "));
			for (auto it:record.factions){
				factionText.append(fmt::format("{} ", it));
			}
			ImGui::Text(factionText.c_str());
		}
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

	if (mFactionFilter){
		auto hasFaction = false;
		for (auto it: record.factions){
			if (it == mFactionFilter){
				hasFaction = true;
				break;
			}
				
		}
		if (!hasFaction)
			return false;
	}

	if (record.protoId > 14999)
		return false;

	return true;
}


