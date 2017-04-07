
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
#include "gamesystems/d20/d20stats.h"

DungeonMaster dmSys;

static bool isActive = true;
static bool isActionActive = false;
static bool isMinimized = false;

// monster modify
DungeonMaster::CritterBooster critBoost;
DungeonMaster::ObjEditor critEditor;
static std::vector<std::string> classNames; // offset by 1 wrt the d20ClassSys.classEnums vector
static int mMonModFactionNew;
static bool mMonModFactionIsOverride = false;


void DungeonMaster::Render(){

	if (!IsActive())
		return;

	auto rect = TigRect(0,0,96,96);
	ImGui::Begin("Dungeon Master", &isActive);
		isMinimized = ImGui::GetWindowCollapsed();
		auto wndPos = ImGui::GetWindowPos();
		auto wndWidth = ImGui::GetWindowWidth();
		rect.x = wndPos.x + wndWidth/2 - rect.width/2; rect.y = wndPos.y - rect.height;
	
		// Monster Tree
		if (ImGui::TreeNodeEx("Monsters", ImGuiTreeNodeFlags_CollapsingHeader)) {
		
			if (ImGui::CollapsingHeader("Filter"))
				RenderMonsterFilter();
			if (ImGui::CollapsingHeader("Modify"))
				RenderMonsterModify();
			
			
			for (auto it: monsters){
				if (FilterResult(it.second)){
					RenderMonster(it.second);
				}
			}

			ImGui::TreePop();
		}

		// Weapons Tree
		if (ImGui::TreeNodeEx("Weapons", ImGuiTreeNodeFlags_CollapsingHeader)) {

			for (auto it : weapons) {
				ImGui::BulletText("%d | %s ", it.first, it.second.name.c_str());
			}
			ImGui::TreePop();
		}

	ImGui::End();

	if (!isMinimized){
		// render the dungeon master figurehead
		UiRenderer::DrawTexture(mTexId, rect);

		// render the name of the hovered item
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

		// render the object editing window
		if (mEditedObj && objSystem->IsValidHandle(mEditedObj)){
			RenderEditedObj();
		} 
		else{
			mEditedObj = objHndl::null;
		}
	}
	
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

			ApplyMonsterModify(newHndl);

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
			SetObjEditor(mEditedObj);
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

		classNames.push_back("");
		for (auto it: d20ClassSys.classEnums){
			classNames.push_back(d20Stats.GetStatName((Stat)it));
		}

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

void DungeonMaster::RenderMonsterFilter(){
	
	const char* listbox_items[] = { "Any" , "Aberration", "Animal", "Beast", "Construct", "Dragon", "Elemental", "Fey", "Giant", "Humanoid"
		, "Magical Beast", "Monstrous Humanoid", "Ooze" , "Outsider" , "Plant" , "Shapechanger" , "Undead" , "Vermin" };
	const char* subcatItems[] = { "Any", "Air", "Aquatic", "Extraplanar", "Cold", "Chaotic", "Demon", "Devil", "Dwarf", "Earth", "Electricity", "Elf", "Evil", "Fire", "Formian"
		, "Gnoll" , "Gnome" , "Goblinoid" , "Good" , "Guardinal" , "Half Orc" , "Halfling" , "Human" , "Lawful" , "Incorporeal", "Orc", "Reptilian", "Slaadi", "Water" };
	ImGui::Combo("Category", &mCategoryFilter, listbox_items, 18, 8);
	ImGui::Combo("Subcategory", &mSubcategoryFilter, subcatItems, 29, 8);
	ImGui::InputInt("Faction", &mFactionFilter);
	
}

void DungeonMaster::RenderMonsterModify(){
	

	// Faction
	ImGui::PushItemWidth(79);
	ImGui::InputInt("Faction", &mMonModFactionNew); 
	ImGui::PopItemWidth();
	ImGui::SameLine();  ImGui::Checkbox("Override", &mMonModFactionIsOverride);
	ImGui::SameLine();
	std::string factionBtnText = fmt::format("Add");
	if (mMonModFactionIsOverride){
		factionBtnText = fmt::format("Set");
	}

	if (ImGui::Button(factionBtnText.c_str())){
		if (mMonModFactionIsOverride){
			critBoost.factions.clear();
			if (mMonModFactionNew >= 0)
				critBoost.factions.push_back(mMonModFactionNew); // otherwise, it'll just have the null faction
		}
		else{
			auto isFound = false;
			for (auto it : critBoost.factions){
				if (it == mMonModFactionNew)
					isFound = true;
			}
			if (!isFound && mMonModFactionNew > 0)
				critBoost.factions.push_back(mMonModFactionNew);
		}
	}

	if (ImGui::Button("Clear")){
		critBoost = DungeonMaster::CritterBooster();
	}


	if (critBoost.factions.size())
		ImGui::Text(fmt::format("Added Factions: {}", critBoost.factions).c_str());

}

void DungeonMaster::ApplyMonsterModify(objHndl handle){
	auto obj = objSystem->GetObject(handle);

	// Factions
	if (mMonModFactionIsOverride){
		obj->ClearArray(obj_f_npc_faction);
		obj->SetInt32(obj_f_npc_faction, 0, 0);
		auto i = 0;
		for (auto it: critBoost.factions){
			obj->SetInt32(obj_f_npc_faction, i++, it);
		}
	} 
	else if (critBoost.factions.size()){
		auto i = obj->GetInt32Array(obj_f_npc_faction).GetSize();
		for (auto it : critBoost.factions) {
			obj->SetInt32(obj_f_npc_faction, i++, it);
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


// Object Editor
void DungeonMaster::RenderEditedObj() {
	ImGui::Begin("Edit Object");
	ImGui::Text(fmt::format("Name: {}", critEditor.name).c_str());
	ImGui::Text(fmt::format("Factions: {}", critEditor.factions).c_str());
	if (ImGui::TreeNodeEx("Class Levels", ImGuiTreeNodeFlags_CollapsingHeader)){
	//if (ImGui::CollapsingHeader("Class Levels")){
		static int classCur = 0;
		static auto classNameGetter = [](void*data, int idx, const char** outTxt)->bool
		{
			if (idx >= classNames.size())
				return false;
			*outTxt = classNames[idx].c_str();
			return true;
		};


		std::vector<int> classLvlChang;
		auto classIdx = 0;
		for (auto it : critEditor.classLevels) {
			classLvlChang.push_back(it.second);
			if (ImGui::InputInt(fmt::format("{}", d20Stats.GetStatName(it.first)).c_str(), &classLvlChang[classIdx])) {
				critEditor.classLevels[it.first] = classLvlChang[classIdx];
			};
			classIdx++;
		}

		ImGui::Combo("Add Class", &classCur, classNameGetter, nullptr, classNames.size(), 8);
		ImGui::TreePop();
	}

	if (ImGui::Button("Apply")) {
		ApplyObjEdit(mEditedObj);
	}
	ImGui::End();
}

void DungeonMaster::SetObjEditor(objHndl handle){
	
	critEditor = ObjEditor();

	auto obj = objSystem->GetObject(handle);
	if (!obj || !objSystem->IsValidHandle(handle)){
		mEditedObj = objHndl::null;
		return;
	}
		
	critEditor.name = description.getDisplayName(handle);
	auto facs = obj->GetInt32Array(obj_f_npc_faction);
	for (auto i = 0; i < facs.GetSize(); i++){
		critEditor.factions.push_back(facs[i]);
	}

	auto classLvls = obj->GetInt32Array(obj_f_critter_level_idx);
	for (auto i = 0; i < classLvls.GetSize(); i++)
		critEditor.classLevels[(Stat)classLvls[i]]++;
}

void DungeonMaster::ApplyObjEdit(objHndl handle){
	auto obj = objSystem->GetObject(mEditedObj);
	if (!obj  || !objSystem->IsValidHandle(mEditedObj))
		return;

	// Factions
	if (obj->IsNPC()){
		obj->ClearArray(obj_f_npc_faction);
		obj->SetInt32(obj_f_npc_faction, 0, 0);
		auto i = 0;
		for (auto it : critEditor.factions) {
			obj->SetInt32(obj_f_npc_faction, i++, it);
		}
	}
	

	// Class Levels
	obj->ClearArray(obj_f_critter_level_idx);
	for (auto it : critEditor.classLevels) {
		for (auto i=0; i < it.second; i++)
			obj->AppendInt32(obj_f_critter_level_idx, it.first); // todo: preserve order...
	}


	d20StatusSys.D20StatusRefresh(handle);
}


bool DungeonMaster::IsActive() {
	return isActive;
}

bool DungeonMaster::IsMinimized() {
	return isMinimized;
}

bool DungeonMaster::IsActionActive() {
	return isActionActive;
}

void DungeonMaster::Show() {
	isActive = true;
}

void DungeonMaster::Hide() {
	isActive = false;
}

void DungeonMaster::Toggle() {
	if (IsActive())
		Hide();
	else
		Show();
}

