
#include "stdafx.h"

#include <regex>

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
#include <condition.h>
#include <infrastructure\keyboard.h>
#include <infrastructure/vfs.h>


#include <ui\ui_systems.h>
#include <ui\ui_legacysystems.h>

#include "animgoals/anim.h"
#include "critter.h"
#include "party.h"
#include "fade.h"

#include "gamesystems/gamesystems.h"
#include "gamesystems/mapsystem.h"
#include "gamesystems/legacysystems.h"
#include "gamesystems/objects/objsystem.h"
#include <util\savegame.h>
#include "ai.h"
#include "mainwindow.h"
#include "gameview.h"
#include "ui/ui_mainmenu.h"
#include "config/config.h"
#include "ui/ui_dm.h"
#include "ui/ui_pc_creation.h"
#include "python/python_integration_obj.h"
#include "python/python_header.h"

#include <path_node.h>
#include <animgoals/animgoals_debugrenderer.h>

DungeonMaster dmSys;

static std::vector<VfsSearchResult> mFlist;

static bool isActive = false;
static bool isActionActive = false;
static bool isMinimized = false;
static bool isMoused = false;
static bool isHandlingMsg = false;
static bool showPathNodes;

// monster modify
DungeonMaster::CritterBooster critBoost;
DungeonMaster::ObjEditor critEditor;
static std::vector<std::string> classNames; // offset by 1 wrt the d20ClassSys.classEnums vector
static std::map<int, std::string> spellNames;
static std::map<int, std::string> featNames;
static std::vector<int> featSelectionList;
static std::map<int, std::string> mapNames;
static std::vector<CondStruct*> condStructs;
static int mMonModFactionNew;
static bool mMonModFactionIsOverride = false;

const char* itemTypes[] = { "All", "Weapon" , "Ammo", "Armor & Wearables", "Potion", "Scroll", "Wand", "Magical Item", "Other" };
enum class DmItemType: int {
Weapon= 0,
Ammo,
Armor,
Potion,
Scroll, 
Wand,
MagicalItem,
Other,
Count
};


void DungeonMaster::Render() {

	isMoused = false;

	if (IsUnavailable())
		return;
	
	RenderDmButton();
	
	if (!IsActive())
		return;

	

	auto dmPortraitRect = TigRect(0, 0, 96, 96);

	ImGui::Begin("Dungeon Master", &isActive);
	if (mJustOpened) {
		//ImGui::SetWindowCollapsed(true);
		mFlist.clear();
	}
		
	isMinimized = ImGui::GetWindowCollapsed();
	if (isMinimized && IsActionActive()) {
		DeactivateAction();
	}
	auto wndPos = ImGui::GetWindowPos();
	auto wndWidth = ImGui::GetWindowWidth();
	dmPortraitRect.x = (int)(wndPos.x + wndWidth / 2 - dmPortraitRect.width / 2); dmPortraitRect.y = (int)(wndPos.y - dmPortraitRect.height);

	auto blyat = gameView->MapToScene(dmPortraitRect.x, dmPortraitRect.y);//gameView->MapFromScene(config.renderWidth - dmPortraitRect.x     , config.renderHeight - -dmPortraitRect.y);
	auto cyka  = gameView->MapToScene(dmPortraitRect.x + 96, dmPortraitRect.y + 96);//gameView->MapFromScene(config.renderWidth - dmPortraitRect.x + 96, config.renderHeight - -dmPortraitRect.y +96);

	TigRect adjDmPortrait(blyat.x, blyat.y+1, cyka.x - blyat.x, cyka.y - blyat.y);
	uiDm.SetDmPortraitRect(adjDmPortrait);


	if (party.GetConsciousPartyLeader() && ImGui::TreeNodeEx("Maps", ImGuiTreeNodeFlags_CollapsingHeader)) {
		RenderMaps();

		ImGui::TreePop();
	}

	// Roll Fudger
	if (ImGui::TreeNodeEx("Fudge Rolls", ImGuiTreeNodeFlags_CollapsingHeader)) {

		RenderFudgeRolls();
		ImGui::TreePop();
	}

	// Monster Tree
	if (ImGui::TreeNodeEx("Monsters", ImGuiTreeNodeFlags_CollapsingHeader)) {

		if (ImGui::CollapsingHeader("Filter"))
			RenderMonsterFilter();
		if (ImGui::CollapsingHeader("Modify"))
			RenderMonsterModify();


		for (auto it : monsters) {
			if (FilterResult(it.second)) {
				RenderMonster(it.second);
			}
		}

		ImGui::TreePop();
	}

	// Items
	if (ImGui::TreeNodeEx("Items", ImGuiTreeNodeFlags_CollapsingHeader)) {

		//if (ImGui::CollapsingHeader("Filter"))
		RenderItemFilter();

		for (auto it : items) {
			if (FilterItemResult(it.second)) {
				RenderItem(it.second);
			}
		}

		ImGui::TreePop();
	}

	if (ImGui::CollapsingHeader("Pathfinding")) {
		RenderPathfinding();
	}


	// Spawn Party from Save
	if (ImGui::TreeNodeEx("Import Rival Party", ImGuiTreeNodeFlags_CollapsingHeader)){
		RenderVsParty();
		ImGui::TreePop();
	}

	
	isMoused |= ImGui::IsWindowHovered();
	ImGui::End();

	if (!isMinimized){
		// render the name of the hovered item
		if (mTgtObj && objSystem->IsValidHandle(mTgtObj)){
			UiRenderer::PushFont(PredefinedFont::PRIORY_12);
			tigFont.Draw(description.getDisplayName(mTgtObj), TigRect(dmPortraitRect.x+105, dmPortraitRect.y + 50,100,15), TigTextStyle::standardWhite);
			UiRenderer::PopFont();
			// auto loc = objSystem->GetObject(mTgtObj)->GetLocationFull();
			// uiIntgameTb.RenderPositioningBlueCircle(loc, mTgtObj);
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
	

	if (mJustOpened) {
		mJustOpened = false;
	}

}

void DungeonMaster::RenderDmButton(){

	//	auto w = cyka.x - blyat.x;
	//	auto h = cyka.y - blyat.y;
	//	ImVec2 wndPos(blyat.x-4, blyat.y-4);
	//	ImGui::SetWindowPos(wndPos);

	
	//	UiRenderer::DrawTexture(mIconTexId, TigRect(blyat.x, blyat.y, w, h));
	//}
}

void DungeonMaster::RenderMaps(){
	static std::map<int, std::string> mapNames;
	static std::map<int, std::vector<int>> mapsByArea;
	static int areaFilter = -1;

	// init
	if (!mapNames.size()) {
		for (auto i = 5001; i < gameSystems->GetMap().GetHighestMapId(); i++) {
			auto mapName = gameSystems->GetMap().GetMapDescription(i);
			if (mapName.size()) {
				mapNames[i] = mapName;
				auto mapArea = gameSystems->GetMap().GetArea(i);
				mapsByArea[mapArea].push_back(i);
			}
		}
	}

	// Area Filter

	static int curArea=0;
	static bool mapsOutdoorOnly = 0;
	if (ImGui::TreeNodeEx("Filter", ImGuiTreeNodeFlags_CollapsingHeader)) {
		ImGui::InputInt("Area", &curArea);

		ImGui::Checkbox("Outdoors Only", &mapsOutdoorOnly);

		ImGui::TreePop();
	}
	
	auto mapIdx = 0;
	for (auto it : mapNames) {

		auto mapArea = gameSystems->GetMap().GetArea(it.first);
		if (curArea && mapArea != curArea)
			continue;

		if (mapsOutdoorOnly && !gameSystems->GetMap().IsMapOutdoors(it.first))
			continue;

		if (ImGui::TreeNode(fmt::format("{} {}", it.first, it.second).c_str())) {
			ImGui::Text(fmt::format("Area: {}", mapArea).c_str());
			if (ImGui::Button("Teleport")) {
				TransitionToMap(it.first);
			}
			ImGui::TreePop();
		}
		mapIdx++;
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
			mTgtObj = uiSystems->GetInGame().PickObject(mouseMsg.x, mouseMsg.y, GRF_HITTEST_3D | GRF_ExcludePortals | GRF_ExcludeScenery | GRF_ExcludeItems);
		}

		
		if (HandleSpawning(msg))
			return true;

		if (HandleCloning(msg))
			return true;
		
		if (HandleMoving(msg))
			return true;

		if (HandleEditing(msg))
			return true;

		if (HandlePathnode(msg))
			return true;
	}

	return false;
}

bool DungeonMaster::HandleSpawning(const TigMsg& msg){
	
	if (!IsActionActive())
		return false;

	if (mActionType != DungeonMasterAction::Spawn)
		return false;

	if (msg.type == TigMsgType::MOUSE) {
		auto &mouseMsg = *(TigMsgMouse*)&msg;

		// RMB - click (so it seizes the event and doesn't spawn a radial menu)
		if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_RMB_CLICK) {
			if (IsActionActive()) {
				mObjSpawnProto = 0;
				DeactivateAction();
				return true;
			}
		}


		if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_LMB_RELEASED) {
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
			DeactivateAction();

			dmSys.SetIsHandlingMsg(true);

			return true;
		}
	}
	return false;
}

bool DungeonMaster::HandleCloning(const TigMsg & msg){
	if (!IsActionActive())
		return false;

	if (mActionType != DungeonMasterAction::Clone)
		return false;

	if (msg.type == TigMsgType::MOUSE) {
		auto &mouseMsg = *(TigMsgMouse*)&msg;

		// RMB - click (so it seizes the event and doesn't spawn a radial menu)
		if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_RMB_CLICK) {
			mCloningObj = objHndl::null;
			DeactivateAction();
			return true;
		}

		if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_LMB_RELEASED) {
			if (!mCloningObj || !objSystem->IsValidHandle(mCloningObj))
				return false;

			LocAndOffsets mouseLoc;
			locSys.GetLocFromScreenLocPrecise(mouseMsg.x, mouseMsg.y, mouseLoc);

			auto newHndl = objSystem->Clone(mCloningObj, mouseLoc.location);
			if (!newHndl)
				return false;

			// if Alt key is pressed, keep the action active
			if (!infrastructure::gKeyboard.IsKeyPressed(VK_LMENU) && !infrastructure::gKeyboard.IsKeyPressed(VK_RMENU)) {
				mCloningObj = objHndl::null;
				DeactivateAction();
			}
				
			dmSys.SetIsHandlingMsg(true);

			return true;
		}
	}
	return false;
}

bool DungeonMaster::HandleEditing(const TigMsg & msg){
	if (IsMinimized()) {
		return false;
	}

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

bool DungeonMaster::HandleMoving(const TigMsg & msg)
{
	if (!IsActionActive())
		return false;

	if (mActionType != DungeonMasterAction::Move)
		return false;

	if (msg.type == TigMsgType::MOUSE) {
		auto &mouseMsg = *(TigMsgMouse*)&msg;

		// RMB - click (so it seizes the event and doesn't spawn a radial menu)
		if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_RMB_CLICK) {
			mMovingObj = objHndl::null;
			DeactivateAction();
			return true;
		}

		if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_LMB_RELEASED) {
			if (!mMovingObj || !objSystem->IsValidHandle(mMovingObj))
				return false;

			LocAndOffsets mouseLoc;
			locSys.GetLocFromScreenLocPrecise(mouseMsg.x, mouseMsg.y, mouseLoc);

			objects.Move(mMovingObj, mouseLoc);
			
			// if Alt key is pressed, keep the action active
			if (!infrastructure::gKeyboard.IsKeyPressed(VK_LMENU) && !infrastructure::gKeyboard.IsKeyPressed(VK_RMENU)) {
				mCloningObj = objHndl::null;
				DeactivateAction();
			}

			dmSys.SetIsHandlingMsg(true);

			return true;
		}
	}
	return false;
}

bool DungeonMaster::HandlePathnode(const TigMsg& msg)
{
	
	if (msg.type == TigMsgType::MOUSE) {
		auto& mouseMsg = *(TigMsgMouse*)&msg;

		LocAndOffsets mouseLoc;
		locSys.GetLocFromScreenLocPrecise(mouseMsg.x, mouseMsg.y, mouseLoc);


		if (showPathNodes && (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_POS_CHANGE)){
			auto okToPickNew = true;
			if (mActionType == DungeonMasterAction::PathnodeMove) {
				okToPickNew = pathNodeSys.MoveActiveNode(&mouseLoc);
			}
			if (okToPickNew){
				pathNodeSys.SetActiveNodeFromLoc(mouseLoc);
			}
		}

		if (!IsActionActive())
			return false;

		if (mActionType == DungeonMasterAction::PathnodeCreate ) {
			// RMB - click (so it seizes the event and doesn't spawn a radial menu)
			if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_RMB_RELEASED) {
				DeactivateAction();
				return true;
			}

			if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_LMB_RELEASED) {
				pathNodeSys.AddPathNode(mouseLoc, true);

				//// if Alt key is pressed, keep the action active
				//if (!infrastructure::gKeyboard.IsKeyPressed(VK_LMENU) && !infrastructure::gKeyboard.IsKeyPressed(VK_RMENU)) {
				//	DeactivateAction();
				//}

				dmSys.SetIsHandlingMsg(true);
				return true;
			}
		}

		if (mActionType == DungeonMasterAction::PathnodeDelete) {
			// RMB - click (so it seizes the event and doesn't spawn a radial menu)
			if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_RMB_RELEASED) {
				DeactivateAction();
				return true;
			}

			if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_LMB_RELEASED) {
				pathNodeSys.DeleteActiveNode();

				//// if Alt key is pressed, keep the action active
				//if (!infrastructure::gKeyboard.IsKeyPressed(VK_LMENU) && !infrastructure::gKeyboard.IsKeyPressed(VK_RMENU)) {
				//	DeactivateAction();
				//}

				dmSys.SetIsHandlingMsg(true);
				return true;
			}
		}

		if (mActionType == DungeonMasterAction::PathnodeMove) {
			// RMB - click (so it seizes the event and doesn't spawn a radial menu)
			if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_RMB_RELEASED) {
				pathNodeSys.CancelMoveActiveNode();
				DeactivateAction();
				return true;
			}

			if (mouseMsg.buttonStateFlags & MouseStateFlags::MSF_LMB_RELEASED) {
				auto finishedMoving = pathNodeSys.MoveActiveNode();

				//// if Alt key is pressed, keep the action active
				//if (finishedMoving && !infrastructure::gKeyboard.IsKeyPressed(VK_LMENU) && !infrastructure::gKeyboard.IsKeyPressed(VK_RMENU)) {
				//	DeactivateAction();
				//}

				dmSys.SetIsHandlingMsg(true);
				return true;
			}
		}
			
		return false;

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
		auto newRecord = DungeonMaster::CritterRecord();
		newRecord.protoId = protoNum;
		auto desc = description.getDisplayName(protHndl);
		if (desc) {
			newRecord.name = desc;
			newRecord.lowerName = tolower(desc);
		}
		else {
			newRecord.name = "Unknown";
		}
			
		
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
		for (auto i = 0u; i < levels.GetSize(); i++) {
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

	if (obj->IsItem()){
		auto newr = DungeonMaster::ItemRecord();
		auto itemWearFlags = objects.GetItemWearFlags(protHndl);
		auto itemFlags = objects.GetItemFlags(protHndl);

		if (itemFlags & OIF_NO_DISPLAY) {
			return;
		}

		newr.protoId = protoNum;
		auto desc = description.getDisplayName(protHndl);
		if (desc) {
			newr.name = desc;
			newr.lowerName = tolower(desc);
		}
		else {
			newr.name = "Unknown";
			newr.lowerName = tolower("Unknown");
		}
		
		newr.itemType;
		if (obj->type == obj_t_weapon) {
			newr.itemType = (int)DmItemType::Weapon;
		}
		if (obj->type == obj_t_ammo) {
			newr.itemType = (int)DmItemType::Ammo;
		}
		else if (obj->type == obj_t_armor) {
			auto armorFlags = obj->GetInt32(obj_f_armor_flags);
			//auto armorType = inventory.GetArmorType(armorFlags);
			newr.itemType = (int)DmItemType::Armor;
		}
		else if (obj->type == obj_t_food) {
			newr.itemType = (int)DmItemType::Potion;
		}
		else if (obj->type == obj_t_scroll) {
			newr.itemType = (int)DmItemType::Scroll;
		}
		else if (obj->type == obj_t_generic) {
			if (itemFlags & OIF_USES_WAND_ANIM)
				newr.itemType = (int)DmItemType::Wand;
			else if (itemFlags & OIF_IS_MAGICAL) {
				newr.itemType = (int)DmItemType::MagicalItem;
			}
			else {
				newr.itemType = (int)DmItemType::Other;
			}
		}

		items[protoNum] = newr;
	}

}

void DungeonMaster::InitCaches(){
	classNames.push_back("");
	for (auto it : d20ClassSys.classEnums) {
		classNames.push_back(d20Stats.GetStatName((Stat)it));
	}

	spellSys.DoForSpellEntries([](SpellEntry &spEntry) {
		auto spEnum = spEntry.spellEnum;
		if (spEnum  < 3000) { // the range above 3000 is reserved for class pseudo spells
			spellNames[spEnum] = spellSys.GetSpellName(spEnum);
		}
	});

	// Feats
	feats.DoForAllFeats([](int featEnum) {
		auto feat = (feat_enums)featEnum;
		featNames[featEnum] = feats.GetFeatName(feat);
		if (feats.IsFeatMultiSelectMaster(feat)){
			featSelectionList.push_back(featEnum);
			return;
		}
		if (feats.IsFeatPartOfMultiselect(feat))
			return;

		featSelectionList.push_back(featEnum);
	});
	std::sort(featSelectionList.begin(), featSelectionList.end(), 
		[](int first, int second){
			
			auto firstFeat = (feat_enums)first;
			auto secFeat = (feat_enums)second;
			if (feats.IsFeatRacialOrClassAutomatic(firstFeat) && !feats.IsFeatRacialOrClassAutomatic(secFeat))
				return false;
			if (!feats.IsFeatRacialOrClassAutomatic(firstFeat) && feats.IsFeatRacialOrClassAutomatic(secFeat))
				return true;

			auto firstName = feats.GetFeatName((feat_enums)first);
			auto secondName = feats.GetFeatName((feat_enums)second);

			return _stricmp(firstName, secondName) < 0;
		});

	conds.DoForAllCondStruct([](CondStruct& condStruct) {
		condStructs.push_back(&condStruct);
	});

	std::sort(condStructs.begin(), condStructs.end(), [](const CondStruct * a, const CondStruct * b)->bool {
		auto name1 = a->condName;
		auto name2 = b->condName;
		auto nameCmp = _strcmpi(name1, name2);
		return nameCmp < 0;
	});

}

void DungeonMaster::RenderMonster(CritterRecord& record){
	
	if (ImGui::TreeNode(fmt::format("{} | {} ({} HD)", record.protoId, record.name.c_str(), record.hitDice).c_str())) {
		auto protHndl = objSystem->GetProtoHandle(record.protoId);
		if (ImGui::Button("Spawn")) {
			ActivateSpawn(record.protoId);
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
		ImGui::TreePop();
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
	if (!obj->IsCritter()) {
		return;
	}
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


bool DungeonMaster::FilterResult(CritterRecord & record){

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

void DungeonMaster::RenderItem(ItemRecord& record)
{

	if (ImGui::TreeNode(fmt::format("{} | {}", record.protoId, record.name.c_str()).c_str())) {
		auto protHndl = objSystem->GetProtoHandle(record.protoId);
		
		if (ImGui::Button("Give")) {
			auto leader = party.GetConsciousPartyLeader();
			auto loc = objects.GetLocation(leader);

			auto handleNew = gameSystems->GetObj().CreateObject(protHndl, loc);
			if (handleNew) {
				if (!inventory.SetItemParent(handleNew, leader, ItemInsertFlags::IIF_None)) {
					objects.Destroy(handleNew);
				}
				else {
					auto obj = gameSystems->GetObj().GetObject(handleNew);
					obj->SetItemFlag(OIF_IDENTIFIED, 1);
				}
			}
		}
		
		ImGui::SameLine();
		if (ImGui::Button("Spawn")) {
			ActivateSpawn(record.protoId);
		}

		// auto obj = objSystem->GetObject(protHndl);
		ImGui::TreePop();
	}

}
void DungeonMaster::RenderItemFilter()
{
	ImGui::Combo("Item type", &mItemProtoFilter, itemTypes, sizeof(itemTypes)/sizeof(itemTypes[0]), 8);
	if (ImGui::InputText("Name", &mItemNameFilterBuf[0], 256) ){
		
		std::string itemNameIn = mItemNameFilterBuf.data();
		mItemNameFilter.resize( itemNameIn.length()); // This discards trailing null-bytes
		mItemNameFilter = tolower(itemNameIn);
		trim(mItemNameFilter);
		
	};
}

bool DungeonMaster::FilterItemResult(ItemRecord& record)
{
	if (mItemProtoFilter > 0) {
		if (record.itemType != (mItemProtoFilter-1) )
			return false;
	}

	if (mItemNameFilter.size() > 0) {
		if (record.lowerName.find(mItemNameFilter) == std::string::npos ) {
			return false;
		}
	}

	return true;
}

bool DungeonMaster::PseudoLoad(std::string filename){

	if (!vfs->DirExists("Save\\ArenaTmp")) {
		vfs->MkDir("Save\\ArenaTmp");
	}

	if (!vfs->CleanDir("Save\\ArenaTmp")) {
		logger->error("Error clearing folder Save\\ArenaTmp");
		return false;
	}
	auto path = format("save\\{}", filename);
	try {
		SaveGameArchive::Unpack(path.c_str(), "Save\\ArenaTmp");
	}
	catch (const std::exception &e) {
		logger->error("Error restoring savegame archive {} to Save\\ArenaTmp: {}", path, e.what());
		return false;
	}

	auto file = tio_fopen("Save\\ArenaTmp\\data.sav", "rb");
	if (!file) {
		logger->error("Error reading data.sav\n");
		return false;
	}

	int saveVersion;
	if (tio_fread(&saveVersion, 4, 1, file) != 1) {
		logger->error("Error reading save version");
		tio_fclose(file);
		return false;
	}
	if (saveVersion != 0) {
		logger->error("Save game version mismatch error. Expected 0, read {}", saveVersion);
		tio_fclose(file);
		return false;
	}

	int ironmanFlag;
	if (tio_fread(&ironmanFlag, 4, 1, file) != 1) {
		logger->error("Unable to read ironman flag");
		tio_fclose(file);
		return false;
	}

	if (ironmanFlag) {
		int ironmanSaverNumber;
		if (tio_fread(&ironmanSaverNumber, 4, 1, file) != 1) {
			logger->error("Unable to read ironman save number.");
			tio_fclose(file);
			return false;
		}

		int savenameSize;
		if (tio_fread(&savenameSize, 4, 1, file) != 1) {
			logger->error("Unable to read ironman savename size");
			tio_fclose(file);
			return false;
		}

		std::string ironmanSaveName;
		ironmanSaveName.reserve(savenameSize + 1);

		if (tio_fread(&ironmanSaveName[0], 1, savenameSize, file) != savenameSize) {
			logger->error("Unable to read ironman savegame name.");
			tio_fclose(file);
			return false;
		}
		ironmanSaveName[savenameSize] = 0;
	}

	uint64_t startPos;
	tio_fgetpos(file, &startPos);

	GameSystemSaveFile saveFile;
	saveFile.file = file;
	saveFile.saveVersion = saveVersion;

	auto reportAfterLoad = [](TioFile *file, uint64_t & startPos) {
		// Report on the current file position
		uint64_t afterPos;
		tio_fgetpos(file, &afterPos);
		logger->debug(" read {} bytes up to {}", afterPos - startPos, afterPos);
		startPos = afterPos;
		// Read the sentinel value
		uint32_t sentinel;
		if (tio_fread(&sentinel, 4, 1, file) != 1) {
			logger->error("Unable to read the sentinel value.");
			tio_fclose(file);
			return false;
		}
		if (sentinel != 0xBEEFCAFE) {
			logger->error("Invalid sentinel value read: 0x{:x}", sentinel);
			tio_fclose(file);
			return false;
		}
		return true;
	};

	// Description
	std::vector<std::string> customNames;
	if (!gameSystems->GetDescription().ReadCustomNames(&saveFile, customNames))
		return false;
	if (!reportAfterLoad(file, startPos))
		return false;

	// Sector
	std::vector<SectorTime> sectorTimes;
	if (!gameSystems->GetSector().ReadSectorTimes(&saveFile, sectorTimes))
		return false;
	if (!reportAfterLoad(file, startPos))
		return false;

	// Skill
	int skillSysUnk;
	if (!gameSystems->GetSkill().ReadUnknown(&saveFile, skillSysUnk))
		return false;
	if (!reportAfterLoad(file, startPos))
		return false;

	// Script
	std::vector<int> globalVars;
	std::vector<int> globalFlagsData;
	int storyState;
	if (!gameSystems->GetScript().ReadGlobalVars(&saveFile, globalVars, globalFlagsData, storyState))
		return false;
	// Script subsystem: Game
	std::vector<int> encounterQueue;
	if (!gameSystems->GetScript().ReadEncounterQueue(&saveFile, encounterQueue))
		return false;
	if (!reportAfterLoad(file, startPos))
		return false;

	// Map
		// o_name
		// object_node
		// obj
		// proto
		// object
	if (!gameSystems->GetMap().PseudoLoad(&saveFile, "Save\\ArenaTmp", dynHandlesFromSave))
		return false;
	
	
	

	tio_fclose(file);
	return true;
}

void DungeonMaster::ActivateAction(DungeonMasterAction actionType){
	if (!isActionActive)
		mouseFuncs.SetCursorFromMaterial("art\\interface\\cursors\\DungeonMaster.mdf");
	isActionActive = true;
	mActionTimeStamp = timeGetTime();
	mActionType = actionType;
}

void DungeonMaster::DeactivateAction(){
	if (isActionActive) {
		mouseFuncs.ResetCursor();
	}
	isActionActive = false;
	mActionType = DungeonMasterAction::None;
}

void DungeonMaster::ActivateSpawn(int protoId){
	mObjSpawnProto = protoId;
	ActivateAction(DungeonMasterAction::Spawn);
}

void DungeonMaster::ActivateClone(objHndl handle){
	mCloningObj = handle;
	ActivateAction(DungeonMasterAction::Clone);
}

void DungeonMaster::ActivateMove(objHndl handle){
	mMovingObj = handle;
	ActivateAction(DungeonMasterAction::Move);
}

void DungeonMaster::ActivateRivalPc(objHndl handle){
	if (party.IsInParty(handle)) {
		return;
	}
	
	mMovingObj = handle;
	auto obj = objSystem->GetObject(handle);
	auto combatRole = aiSys.GetRole(handle);
	if (combatRole == AiCombatRole::caster){
		auto stratIdx = aiSys.GetStrategyIdx("Charmed PC Caster");
		obj->SetInt32(obj_f_critter_strategy, stratIdx);
	}
	
	if (!d20Sys.d20Query(handle, DK_QUE_Critter_Is_AIControlled))
		conds.AddTo(handle, "AI Controlled", {0,0,0,0});

	obj->SetInt32(obj_f_hp_damage, 0);

	ActivateAction(DungeonMasterAction::Move);
}


// Object Editor
void DungeonMaster::RenderEditedObj() {
	ImGui::Begin("Edit Object");

	auto obj = objSystem->GetObject(mEditedObj);
	if (!obj)
		return;

	ImGui::Text(fmt::format("Name: {} , Proto: {}",critEditor.name, objSystem->GetProtoId(mEditedObj) ).c_str());
	
	ImGui::SameLine();
	if (ImGui::Button("Clone"))
		ActivateClone(mEditedObj);
	ImGui::SameLine();
	if (ImGui::Button("Move"))
		ActivateMove(mEditedObj);
	ImGui::Text(fmt::format("Location: {}", obj->GetLocationFull()).c_str());

	ImGui::Text(fmt::format("Factions: {}", critEditor.factions).c_str());

	auto curHp = objects.GetHPCur(mEditedObj);
	auto maxHp = objects.StatLevelGet(mEditedObj, Stat::stat_hp_max);
	ImGui::Text(fmt::format("HP: {}/{}", curHp, maxHp).c_str());
	ImGui::SameLine();
	auto hpDmg = obj->GetInt32(obj_f_hp_damage);
	ImGui::PushItemWidth(85);
	if (ImGui::InputInt("Damage: ", &hpDmg)){
		if (hpDmg >=0 ){
			obj->SetInt32(obj_f_hp_damage, hpDmg);
		}
	}
	ImGui::PopItemWidth();
	ImGui::SameLine();
	if (ImGui::Button("Heal")){
		obj->SetInt32(obj_f_hp_damage, 0);
	}
	

	// Stats
	if (ImGui::TreeNodeEx("Stats", ImGuiTreeNodeFlags_CollapsingHeader)) {
		const char *statNames[] = { "STR", "DEX", "CON", "INT", "WIS", "CHA" };

		std::vector<int> statsChang;
		auto statIdx = 0;
		for (auto it : critEditor.stats) {
			statsChang.push_back(it);
			if (ImGui::InputInt(fmt::format("{}", statNames[statIdx]).c_str(), &statsChang[statIdx] )) {
				critEditor.stats[statIdx] = statsChang[statIdx];
			}
			statIdx++;
		}
		ImGui::TreePop();
	}

	// Class Levels
	if (ImGui::TreeNodeEx("Class Levels", ImGuiTreeNodeFlags_CollapsingHeader)){
		static int classCur = 0;
		static auto classNameGetter = [](void*data, int idx, const char** outTxt)->bool
		{
			if ((uint32_t)idx >= classNames.size())
				return false;
			*outTxt = classNames[idx].c_str();
			return true;
		};


		std::vector<int> classLvlChang;
		auto classIdx = 0;
		for (auto it : critEditor.classLevels) {
			auto classLvl = it.second;
			classLvlChang.push_back(classLvl);
			if (ImGui::InputInt(fmt::format("{}", d20Stats.GetStatName(it.first)).c_str(), &classLvlChang[classIdx])) {
				critEditor.classLevels[it.first] = classLvlChang[classIdx];
			};
			classIdx++;
		}

		if (ImGui::Combo("Add Class", &classCur, classNameGetter, nullptr, classNames.size(), 8)) {
			if (classCur > 0) {
				auto newClass = (Stat)d20ClassSys.classEnums[classCur - 1];
				if (critEditor.classLevels.find(newClass) == critEditor.classLevels.end()) {
					critEditor.classLevels[newClass] = 1;
				}
			}
		}
		ImGui::TreePop();
	}

	// Feats
	if (ImGui::TreeNodeEx("Feats", ImGuiTreeNodeFlags_CollapsingHeader)){

		static auto featNameGetter = [](void*data, int idx, const char** outTxt)->bool
		{
			if (idx >= (int)featSelectionList.size())
				return false;
			auto feat = static_cast<feat_enums>(featSelectionList[idx]);
			*outTxt = feats.GetFeatName(feat);
			return true;
		};


		// Add Feat
		if (ImGui::TreeNodeEx("Add Feat", ImGuiTreeNodeFlags_CollapsingHeader)) {
			static int featCur = 0;
			static int subFeatCur = 0;
			static feat_enums subFeat = FEAT_NONE;
			static std::vector<feat_enums> childFeats;

			auto getFeatEnum = []()->feat_enums {
				if (featCur < 0 || featCur > (int)featSelectionList.size())
					return FEAT_NONE;

				return static_cast<feat_enums>(featSelectionList[featCur]);
			};

			auto featEnum = getFeatEnum();

			if (ImGui::Combo("Feat", &featCur, featNameGetter, nullptr, featSelectionList.size(), 16)) {
				featEnum = getFeatEnum();
				subFeatCur = 0;
				if (feats.IsFeatMultiSelectMaster(featEnum)) {
					subFeat = FEAT_NONE;
					childFeats.clear();
					feats.MultiselectGetChildren(featEnum, childFeats);
					if ( static_cast<size_t>(subFeatCur) < childFeats.size())
						subFeat = childFeats[subFeatCur];
				}
			}

			if (featEnum != FEAT_NONE) {
				
				if (!feats.IsFeatMultiSelectMaster(featEnum)){
					if (ImGui::Button("Add")) {
						feats.FeatAdd(mEditedObj, featEnum);
						critEditor.feats.push_back(featEnum);
					}
				}
				else{
					
					auto subfeatNameGetter = [](void*data, int idx, const char** outTxt)->bool	{
						if (idx >= childFeats.size())
							return false;
						auto feat = static_cast<feat_enums>(childFeats[idx]);
						*outTxt = feats.GetFeatName(feat);
						return true;
					};

					if (ImGui::Combo("Sub feat", &subFeatCur, subfeatNameGetter, nullptr, childFeats.size(), 8)) {
						subFeat = childFeats[subFeatCur];
					}
					if (ImGui::Button("Add")) {
						feats.FeatAdd(mEditedObj, subFeat);
						critEditor.feats.push_back(subFeat);
					}
				}
			}


			ImGui::TreePop();
		}

		// Existing feats
		
		for (auto it : critEditor.feats) {
			if (featNames.find(it) == featNames.end())
				continue;
			ImGui::BulletText(fmt::format("{} ({})", featNames[it], it).c_str());
		}


		ImGui::TreePop();
	}

	// Spells Known
	if (ImGui::TreeNodeEx("Spells Known", ImGuiTreeNodeFlags_CollapsingHeader)) {

		static auto spellNameGetter = [](void *data, int idx, const char** outTxt)->bool {
			if ((size_t)idx >= spellNames.size())
				return false;
			auto it = spellNames.begin();
			std::advance(it, idx);
			if (it == spellNames.end())
				return false;

			*outTxt = it->second.c_str();
			return true;
		};
		static auto spellClassNameGetter = [](void*data, int idx, const char** outTxt)->bool
		{
			if ((size_t)idx >= d20ClassSys.classEnumsWithSpellLists.size())
				return false;
			*outTxt = d20Stats.GetStatName(d20ClassSys.classEnumsWithSpellLists[idx]);
			return true;
		};

		// Add Spell
		if (ImGui::TreeNodeEx("Add Spell", ImGuiTreeNodeFlags_CollapsingHeader)) {
			static int spellCur = 0;
			static int spLvl = 1;
			static int spellClassIdx = 0;
			static int spellClassCur = 0;
			

			auto getSpellEnum = []()->int {
				auto it = spellNames.begin();
				std::advance(it, spellCur);
				if (it != spellNames.end())
					return  it->first;
				return 0;
			};
			auto getSpellLevelForClass = [](int spellEnum)->int {
				SpellEntry spEntry(spellEnum);
				return spEntry.SpellLevelForSpellClass(spellSys.GetSpellClass(spellClassCur));
			};

			auto spEnum = getSpellEnum();

			if (ImGui::Combo("Spell", &spellCur, spellNameGetter, nullptr, spellNames.size(), 12)) {
				spEnum = getSpellEnum();
				auto spLvlSuggest = getSpellLevelForClass(spEnum);
				if (spLvlSuggest != -1)
					spLvl = spLvlSuggest;
			}

			
			if (spEnum) {
				// Spell Class
				spellClassCur = d20ClassSys.classEnumsWithSpellLists[spellClassIdx];
				if (ImGui::Combo("Class", &spellClassIdx, spellClassNameGetter, nullptr, d20ClassSys.classEnumsWithSpellLists.size(), 8)) {
					auto spLvlSuggest = getSpellLevelForClass(spEnum);
					if (spLvlSuggest != -1)
						spLvl = spLvlSuggest;
				}
				// Spell Level
				ImGui::InputInt("Level", &spLvl);

				if (ImGui::Button("Add")) {
					critEditor.spellsKnown.push_back(SpellStoreData(spEnum, spLvl, spellSys.GetSpellClass(spellClassCur), 0, SpellStoreType::spellStoreKnown));
				}
			}
			
			
			ImGui::TreePop();
		}
		
		// Existing known spells
		for (auto it : critEditor.spellsKnown) {
			if (spellNames.find(it.spellEnum) == spellNames.end())
				continue;
			if (ImGui::TreeNode( fmt::format("{} ({})", spellNames[it.spellEnum], it.spellEnum).c_str() ) ){
				if (spellSys.isDomainSpell(it.classCode))
					auto dummy = 1;
				else
					ImGui::Text(fmt::format("Spell Class: {}", d20Stats.GetStatName( spellSys.GetCastingClass( it.classCode))).c_str());

				ImGui::Text(fmt::format("Spell Level: {}", it.spellLevel).c_str());
				ImGui::TreePop();
			}
		}

		

		ImGui::TreePop();
	}

	// Modifiers & Conditions
	if (ImGui::TreeNodeEx("Modifiers & Conditions", ImGuiTreeNodeFlags_CollapsingHeader)) {
		
		static auto condNameGetter = [](void *data, int idx, const char** outTxt)->bool {
			if ((size_t)idx >= condStructs.size())
				return false;
			
			*outTxt = condStructs[idx]->condName;
			return true;
		};
		 
		if (ImGui::TreeNodeEx("Add New", ImGuiTreeNodeFlags_CollapsingHeader)) {

			static int condCur = 0;
			static int numArgs = 0;
			if (ImGui::Combo("Select", &condCur, condNameGetter, nullptr, condStructs.size(), 8)) {
				numArgs = condStructs[condCur]->numArgs;
			}
			ImGui::Text(fmt::format("Num args: {}", numArgs).c_str());

			if (ImGui::Button("Add")) {
				std::vector<int> condArgs;
				if (numArgs >= 0)
					condArgs.resize(numArgs);
				else
					condArgs.resize(0);
				conds.AddTo(mEditedObj, condStructs[condCur]->condName, condArgs);
			}

			ImGui::TreePop();
		}
		
		auto displayCondUi = [](objHndl handle, Dispatcher * dispatcher ,obj_f fieldType) {
			auto condTmp = dispatcher->conditions;
			if (fieldType == obj_f_permanent_mods)
				condTmp = dispatcher->permanentMods;
			else if (fieldType == obj_f_item_pad_wielder_condition_array)
				condTmp = dispatcher->itemConds;

			for (; condTmp; condTmp = condTmp->nextCondNode) {
				if (condTmp->IsExpired())
					continue;

				// Condition details
				if (ImGui::TreeNode(fmt::format("{}", condTmp->condStruct->condName).c_str())) {
					// args
					auto numArgs = condTmp->condStruct->numArgs;
					ImGui::Text(fmt::format("Num args: {}", numArgs).c_str());
					for (auto j = 0u; j < numArgs; j++) {
						ImGui::Text(fmt::format("Arg{}: {}", j, condTmp->args[j]).c_str());
					}
					// Remove
					if (ImGui::Button("Remove")) {
						conds.ConditionRemove(handle, condTmp);
					}
					ImGui::TreePop();
				}
			}
		};

		auto dispatcher = obj->GetDispatcher();
		displayCondUi(mEditedObj, dispatcher, obj_f_conditions);
		displayCondUi(mEditedObj, dispatcher, obj_f_permanent_mods);
		displayCondUi(mEditedObj, dispatcher, obj_f_item_pad_wielder_condition_array);
		
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Scripts", ImGuiTreeNodeFlags_CollapsingHeader)){
		static auto scriptNameGetter = [](void*data, int idx, const char** outTxt)->bool
		{
			if (idx > SAN::san_unlock_attempt)
				return false;
			*outTxt = pythonObjIntegration.GetEventName((ObjScriptEvent)idx).c_str();
			return true;
		};
		std::vector<int> scriptsChang;
		ImGui::PushItemWidth(145);
		for (auto idx = 0; idx <= SAN::san_unlock_attempt; idx++){
			scriptsChang.push_back(critEditor.scripts[idx]);
			if (ImGui::InputInt(pythonObjIntegration.GetEventName((ObjScriptEvent)idx).c_str(), &scriptsChang[idx])) {
				critEditor.scripts[idx] = scriptsChang[idx];
			}
		}
		ImGui::PopItemWidth();
		ImGui::TreePop();
	}

	// AI
	if (obj->IsNPC() || !objects.IsPlayerControlled(mEditedObj)){
		auto curAiStartegyIdx = obj->GetInt32(obj_f_critter_strategy);
		auto aiStratGetter = [](void*data, int idx, const char** outTxt)->bool{
			AiStrategy * aiStrat = aiSys.GetAiStrategy( (uint32_t)idx);
			if (!aiStrat)
				return false;
			*outTxt = aiStrat->name.c_str();
			return true;
		};

		if (ImGui::Combo("AI Type", &curAiStartegyIdx, aiStratGetter, nullptr, aiSys.aiStrategies.size(), 8)){
			obj->SetInt32(obj_f_critter_strategy, curAiStartegyIdx);
		}
	}

	if (ImGui::Button("Apply")) {
		ApplyObjEdit(mEditedObj);
	}

	isMoused |= ImGui::IsWindowHovered();
	ImGui::End();
}

void DungeonMaster::RenderVsParty(){

	if (dynHandlesFromSave.size()) {
		if (ImGui::TreeNodeEx("Rival PCs", ImGuiTreeNodeFlags_CollapsingHeader)) {
			for (auto it : dynHandlesFromSave) {
				auto dynObj = objSystem->GetObject(it);
				if (dynObj->IsPC() && objSystem->IsValidHandle(it)) {
					auto d = description.getDisplayName(it);
					if (ImGui::Button(fmt::format("{}", d ? d : "Unknown").c_str())) {
						SetObjEditor(it);
						ActivateRivalPc(it);
					};
				}
			}
			ImGui::TreePop();
		}
	}

	if (!mFlist.size())
		mFlist = vfs->Search("Save\\slot*.gsi");

	if (mFlist.size() && ImGui::TreeNodeEx("Save Games", ImGuiTreeNodeFlags_CollapsingHeader)) {
		for (int i = (int)mFlist.size()-1; i >= 0; i--) {
			auto &fileEntry = mFlist[i];
			regex saveFnameRegex("(slot\\d{4})(.*)\\.gsi", regex_constants::ECMAScript | regex_constants::icase);
			smatch saveFnameMatch;

			if (!regex_match(fileEntry.filename, saveFnameMatch, saveFnameRegex)) {
				continue;
			}

			// Load Mobiles from save
			std::string filename = fmt::format("{}", saveFnameMatch[1].str());
			if (ImGui::Button(fmt::format("Go {} {}", saveFnameMatch[1].str(), saveFnameMatch[2].str()).c_str())) {
				PseudoLoad(filename);
			}
		}
		ImGui::TreePop();
	}
	

	
}

void DungeonMaster::RenderFudgeRolls(){
	
	ImGui::Text("Force dice roll results:");
	ImGui::RadioButton("Normal", &mForceRollType, 0); ImGui::SameLine();
	ImGui::RadioButton("Min", &mForceRollType, 1); ImGui::SameLine();
	ImGui::RadioButton("Avg", &mForceRollType, 2); ImGui::SameLine();
	ImGui::RadioButton("Avg+2", &mForceRollType, 3); ImGui::SameLine();
	ImGui::RadioButton("Max", &mForceRollType, 4);


}

void DungeonMaster::RenderPathfinding()
{
	ImGui::Checkbox("Show Pathnodes", &showPathNodes);
	pathNodeSys.SetPathNodeVisibile(showPathNodes);
	AnimGoalsDebugRenderer::EnablePaths(showPathNodes);
	

	if (ImGui::Button("Create Node")) {
		showPathNodes = true;
		pathNodeSys.SetPathNodeVisibile(showPathNodes);
		ActivateAction(DungeonMasterAction::PathnodeCreate);
	}
	ImGui::SameLine();
	if (ImGui::Button("Move")) {
		showPathNodes = true;
		pathNodeSys.SetPathNodeVisibile(showPathNodes);
		ActivateAction(DungeonMasterAction::PathnodeMove);
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete")) {
		showPathNodes = true;
		pathNodeSys.SetPathNodeVisibile(showPathNodes);
		ActivateAction(DungeonMasterAction::PathnodeDelete);
	}
	

	if (ImGui::Button("Recalc All Neighbours")) {
		pathNodeSys.RecalculateAllNeighbours();
	}
	if (ImGui::IsItemHovered()) ImGui::SetTooltip("Recalculates all neighbours and traversal distances between them. This may take a while.");

	static std::string saveFolder = "";
	static bool persistNodes = false;
	if (ImGui::Button("Save Nodes")) {	
		auto mapId = gameSystems->GetMap().GetCurrentMapId();
		auto mapName = gameSystems->GetMap().GetMapName(mapId);
		if (persistNodes) {
			saveFolder = fmt::format("maps\\{}", mapName);
		}
		else {
			saveFolder = fmt::format("DM_PF\\{}", mapName);
		}
		tio_mkdir(saveFolder.c_str());
		pathNodeSys.FlushNodes(saveFolder.c_str());
	}
	if (ImGui::IsItemHovered())		ImGui::SetTooltip("Dumps path nodes to file.");
	ImGui::SameLine();
	ImGui::Checkbox("Persist", &persistNodes);
	if (ImGui::IsItemHovered())		ImGui::SetTooltip("If false, the changes will be temporary (reversed on reloading the map or save). If true, they'll replace the previous nodes completely.");

	if (saveFolder.size()) {
		ImGui::Text(fmt::format("Saved nodes to: modules\\{}\\{}",config.defaultModule,saveFolder).c_str() );
	}
	

	static std::string saveFolderClearance = "";
	static bool persistClearance = false;
	if (ImGui::Button("Generate Clearances")) {
		auto mapId = gameSystems->GetMap().GetCurrentMapId();
		auto mapName = gameSystems->GetMap().GetMapName(mapId);
		if (persistClearance) {
			saveFolderClearance = fmt::format("maps\\{}", mapName);
		}
		else {
			saveFolderClearance = fmt::format("DM_PF\\{}", mapName);
		}
		tio_mkdir(saveFolderClearance.c_str());
		pathNodeSys.GenerateClearanceFile(saveFolderClearance.c_str());
	}
	ImGui::SameLine();
	ImGui::Checkbox("Persist", &persistClearance);
	if (ImGui::IsItemHovered())		ImGui::SetTooltip("Generate tile clearance data. This speeds up pathfinding and allows the engine to do some more demanding PF queries.");
	if (saveFolderClearance.size()) {
		ImGui::Text(fmt::format("Saved clearances to: modules\\{}\\{}", config.defaultModule, saveFolderClearance).c_str());
	}

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
	for (auto i = 0u; i < facs.GetSize(); i++){
		critEditor.factions.push_back(facs[i]);
	}

	// Class Levels
	auto classLvls = obj->GetInt32Array(obj_f_critter_level_idx);
	for (auto i = 0u; i < classLvls.GetSize(); i++)
		critEditor.classLevels[(Stat)classLvls[i]]++;

	// Stats
	auto statScores = obj->GetInt32Array(obj_f_critter_abilities_idx);
	for (auto i = 0u; i < statScores.GetSize(); i++)
		critEditor.stats.push_back(statScores[i]);

	auto scriptsArray = obj->GetScriptArray(obj_f_scripts_idx);
	for (auto i=0; i <= SAN::san_unlock_attempt; i++){
		auto &script = scriptsArray[i];
		critEditor.scripts.push_back(script.scriptId);
	}

	// Feats
	static objHndl thisHandle;
	thisHandle = handle;
	feats.DoForAllFeats([](int featEnum){
		if (feats.HasFeatCount(thisHandle, (feat_enums)featEnum)){
			critEditor.feats.push_back((feat_enums)featEnum);
		}
	});

	// Spells
	auto spellsKnown = obj->GetSpellArray(obj_f_critter_spells_known_idx);
	for (auto i = 0u; i < spellsKnown.GetSize(); i++) {
		critEditor.spellsKnown.push_back(spellsKnown[i]);
	}

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
	
	// Stats
	obj->ClearArray(obj_f_critter_abilities_idx);
	auto statIdx = 0;
	for (auto it : critEditor.stats) {
		obj->SetInt32(obj_f_critter_abilities_idx, statIdx++, it);
	}

	// Scripts
	auto scriptIdx = 0u;
	for ( auto it : critEditor.scripts){
		ObjectScript script = obj->GetScript(obj_f_scripts_idx, scriptIdx);
		script.scriptId = it;
		obj->SetScript(obj_f_scripts_idx, scriptIdx++, script);
	}

	// Class Levels
	obj->ClearArray(obj_f_critter_level_idx);
	for (auto it : critEditor.classLevels) {
		for (auto i=0; i < it.second; i++)
			obj->AppendInt32(obj_f_critter_level_idx, it.first); // todo: preserve order...
	}

	// Spells
	obj->ClearArray(obj_f_critter_spells_known_idx);
	auto spellIdx = 0;
	for (auto it : critEditor.spellsKnown) {
		obj->SetSpell(obj_f_critter_spells_known_idx, spellIdx++, it);
	}


	d20StatusSys.D20StatusRefresh(handle);
}


DungeonMaster::DungeonMaster() : mItemNameFilterBuf({ '\0', })
{
}

bool DungeonMaster::IsActive() {
	return isActive;
}

bool DungeonMaster::IsMinimized() {
	return isMinimized;
}

bool DungeonMaster::IsUnavailable()
{
	return !gameView || !config.dungeonMaster || !uiSystems->GetUtilityBar().IsVisible(); // || !party.GetConsciousPartyLeader();
}

bool DungeonMaster::IsActionActive() {
	return isActionActive;
}

bool DungeonMaster::IsEditorActive(){
	return mEditedObj != objHndl::null && objSystem->IsValidHandle(mEditedObj);
}

objHndl DungeonMaster::GetHoveredCritter(){
	return (mTgtObj && objSystem->IsValidHandle(mTgtObj)) ? mTgtObj : objHndl::null;
}

bool DungeonMaster::IsHandlingMsg(){
	return isHandlingMsg;
}

void DungeonMaster::SetIsHandlingMsg(bool b){
	isHandlingMsg = b;
}

int DungeonMaster::GetDiceRollForcing(){
	if (IsUnavailable()) // so it doesn't affect PC creation stat rolls
		return 0;
	return mForceRollType;
}

bool DungeonMaster::IsMoused(){
	return isMoused;
}

void DungeonMaster::Show() {
	isActive = true;
	//isMinimized = true;
	mJustOpened = true;
}

void DungeonMaster::Hide() {
	isActive = false;
	isMoused = false;
	DeactivateAction();
	mEditedObj = objHndl::null;
}

void DungeonMaster::Toggle() {
	if (IsActive())
		Hide();
	else
		Show();
}



void DungeonMaster::GotoArena(){
	
	//velkorObj->SetInt32(obj_f_pc_voice_idx, 11);
	//critterSys.GenerateHp(velkor);
	//party.AddToPCGroup(velkor);

	//static auto spawn_velkor_equipment = temple::GetPointer<void(objHndl)>(0x1006d300);
	//spawn_velkor_equipment(velkor);

	//auto anim = objects.GetAnimHandle(velkor);
	//objects.UpdateRenderHeight(velkor, *anim);
	//objects.UpdateRadius(velkor, *anim);

	auto mapId = 5001;
	TransitionToMap(mapId);

	uiSystems->GetParty().UpdateAndShowMaybe();
	Hide();
	uiSystems->GetParty().Update();
}


void DungeonMaster::TransitionToMap(int mapId)
{
	FadeArgs fadeArgs;
	fadeArgs.flags = 0;
	fadeArgs.color = 0;
	fadeArgs.countSthgUsually48 = 1;
	fadeArgs.transitionTime = 0;
	fadeArgs.field10 = 0;
	fade.PerformFade(fadeArgs);
	gameSystems->GetAnim().StartFidgetTimer();

	FadeAndTeleportArgs fadeTp;
	fadeTp.destLoc = gameSystems->GetMap().GetStartPos(mapId);
	fadeTp.destMap = mapId;
	fadeTp.flags = 4;
	fadeTp.somehandle = party.GetLeader();

	auto enterMovie = gameSystems->GetMap().GetEnterMovie(mapId, true);
	if (enterMovie) {
		fadeTp.flags |= 1;
		fadeTp.field20 = 0;
		fadeTp.movieId = enterMovie;
	}
	fadeTp.field48 = 1;
	fadeTp.field4c = 0xFF000000;
	fadeTp.field50 = 64;
	fadeTp.somefloat2 = 3.0;
	fadeTp.field58 = 0;
	fade.FadeAndTeleport(fadeTp);

	gameSystems->GetSoundGame().StopAll(false);
	uiSystems->GetWMapRnd().StartRandomEncounterTimer();
	gameSystems->GetAnim().PopDisableFidget();
}
