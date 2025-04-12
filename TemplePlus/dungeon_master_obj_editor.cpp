#include "stdafx.h"
#include <debugui.h>

#include <condition.h>
#include "gamesystems/objects/objsystem.h"
#include <critter.h>
#include "ai.h"
#include "gamesystems/d20/d20stats.h"
#include "python/python_integration_obj.h"
#include "python/python_header.h"

#include "config/config.h"
#include "dungeon_master.h"


static std::vector<std::string> classNames; // offset by 1 wrt the d20ClassSys.classEnums vector
static std::vector<std::string> skillNames;
static std::map<int, std::string> spellNames;
static std::map<int, std::string> featNames;
static std::map<Alignment, std::string> alignmentNames = {
	{ALIGNMENT_CHAOTIC_EVIL, "Chaotic Evil"} ,
	{ALIGNMENT_CHAOTIC_GOOD, "Chaotic Good"} ,
	{ALIGNMENT_CHAOTIC, "Chaotic Neutral"} ,

	{ALIGNMENT_LAWFUL_EVIL, "Lawful Evil"} ,
	{ALIGNMENT_LAWFUL_GOOD, "Lawful Good"} ,
	{ALIGNMENT_LAWFUL, "Lawful Neutral"} ,
	
	{ALIGNMENT_GOOD, "Neutral Good"} ,
	{ALIGNMENT_EVIL, "Neutral Evil"} ,

	{ALIGNMENT_NEUTRAL, "True Neutral"} ,
};
static std::vector<int> alignmentSelectionList = {
	ALIGNMENT_TRUE_NEUTRAL,
	ALIGNMENT_LAWFUL_NEUTRAL,
	ALIGNMENT_CHAOTIC_NEUTRAL,

	ALIGNMENT_NEUTRAL_GOOD,
	ALIGNMENT_LAWFUL_NEUTRAL,
	ALIGNMENT_CHAOTIC_GOOD,

	ALIGNMENT_NEUTRAL_EVIL,
	ALIGNMENT_LAWFUL_EVIL,
	ALIGNMENT_CHAOTIC_EVIL,
};
static std::vector<int> featSelectionList;
static std::vector<CondStruct*> condStructs;

DungeonMaster::ObjEditor critEditor;


void DungeonMaster::InitObjEditor()
{
	classNames.push_back("");
	for (auto it : d20ClassSys.classEnums) {
		classNames.push_back(d20Stats.GetStatName((Stat)it));
	}

	// Spells
	spellSys.DoForSpellEntries([](SpellEntry& spEntry) {
		auto spEnum = spEntry.spellEnum;
		if (spEnum < 3000) { // the range above 3000 is reserved for class pseudo spells
			spellNames[spEnum] = spellSys.GetSpellName(spEnum);
		}
		});

	// Skills
	skillSys.DoForAllSkills([](SkillEnum skill) {
		auto skillName = skillSys.GetSkillName(skill);
		skillNames.push_back(skillName);
		});


	// Feats
	feats.DoForAllFeats([](int featEnum) {
		auto feat = (feat_enums)featEnum;
		featNames[featEnum] = feats.GetFeatName(feat);
		if (feats.IsFeatMultiSelectMaster(feat)) {
			featSelectionList.push_back(featEnum);
			return;
		}
		if (feats.IsFeatPartOfMultiselect(feat))
			return;

		featSelectionList.push_back(featEnum);
		});
	std::sort(featSelectionList.begin(), featSelectionList.end(),
		[](int first, int second) {

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

	std::sort(condStructs.begin(), condStructs.end(), [](const CondStruct* a, const CondStruct* b)->bool {
		auto name1 = a->condName;
		auto name2 = b->condName;
		auto nameCmp = _strcmpi(name1, name2);
		return nameCmp < 0;
		});
}

// Object Editor
void DungeonMaster::RenderEditedObj() {
	ImGui::Begin("Edit Object");

	auto obj = objSystem->GetObject(mEditedObj);
	if (!obj)
		return;

	ImGui::GetIO().FontGlobalScale = config.dmGuiScale;

	ImGui::Text(fmt::format("Name: {} , Proto: {}", critEditor.name, objSystem->GetProtoId(mEditedObj)).c_str());

	ImGui::SameLine();
	if (ImGui::Button("Clone"))
		ActivateClone(mEditedObj);
	ImGui::SameLine();
	if (ImGui::Button("Move"))
		ActivateMove(mEditedObj);
	ImGui::Text(fmt::format("Location: {}", obj->GetLocationFull()).c_str());

	ImGui::Text(fmt::format("Factions: {}", critEditor.factions).c_str());

	// HP
	{
		auto curHp = objects.GetHPCur(mEditedObj);
		auto maxHp = objects.StatLevelGet(mEditedObj, Stat::stat_hp_max);
		ImGui::Text(fmt::format("HP: {}/{}", curHp, maxHp).c_str());
		ImGui::SameLine();

		auto hpDmg = obj->GetInt32(obj_f_hp_damage);
		auto baseHp = obj->GetInt32(obj_f_hp_pts);
		ImGui::PushItemWidth(85);
		if (ImGui::InputInt("Damage ", &hpDmg)) {
			if (hpDmg >= 0) {
				obj->SetInt32(obj_f_hp_damage, hpDmg);
			}
		}
		ImGui::PopItemWidth();

		ImGui::SameLine();
		if (ImGui::Button("Heal")) {
			obj->SetInt32(obj_f_hp_damage, 0);
		}

		ImGui::PushItemWidth(85);
		if (ImGui::InputInt("Base HP", &baseHp)) {
			if (baseHp >= 0) {
				obj->SetInt32(obj_f_hp_pts, baseHp);
			}
		}
		ImGui::PopItemWidth();
	}



	// Stats
	if (ImGui::TreeNodeEx("Stats", ImGuiTreeNodeFlags_CollapsingHeader)) {
		const char* statNames[] = { "STR", "DEX", "CON", "INT", "WIS", "CHA" };

		std::vector<int> statsChang;
		auto statIdx = 0;
		for (auto it : critEditor.stats) {
			statsChang.push_back(it);
			if (ImGui::InputInt(fmt::format("{}", statNames[statIdx]).c_str(), &statsChang[statIdx])) {
				critEditor.stats[statIdx] = statsChang[statIdx];
			}
			statIdx++;
		}
		ImGui::TreePop();
	}

	// Critter Attributes (Alignment...)
	if (ImGui::TreeNodeEx("Critter Attributes", ImGuiTreeNodeFlags_CollapsingHeader)) {
		auto curAlignment = (Alignment)obj->GetInt32(obj_f_critter_alignment);
		auto curAlignmentChoice = (Alignment)obj->GetInt32(obj_f_critter_alignment_choice);
		static char* alignChoiceNames[3] = { "Negative", "Positive", "(moot)"};
		ImGui::Text( fmt::format("Alignment: {}", alignmentNames[curAlignment] ).c_str());
		ImGui::Text( fmt::format("Alignment Choice: {}", alignChoiceNames[curAlignmentChoice]).c_str());

		static int alignmentIdx = 0;
		static int alignmentChoiceIdx = 0;
		static auto alignmentNameGetter = [](void* data, int idx, const char** outTxt)->bool {
			if ((uint32_t)idx >= alignmentSelectionList.size())
				return false;
			auto alignment = (Alignment) alignmentSelectionList[idx];
			*outTxt = alignmentNames[alignment].c_str();
			return true;
		};
		if (ImGui::Combo("Change Alignment", &alignmentIdx, alignmentNameGetter, nullptr, alignmentSelectionList.size(), alignmentSelectionList.size())) {
			if (alignmentIdx >= 0) {
				auto newAlignment = (Alignment)alignmentSelectionList[alignmentIdx];
				obj->SetInt32(obj_f_critter_alignment, newAlignment);
			}
		}
		
		if (ImGui::Combo("Change Alignment Choice", &alignmentChoiceIdx, (const char**)alignChoiceNames, 2, 2)) {
			if (alignmentChoiceIdx >= 0 && alignmentChoiceIdx < 2) {
				obj->SetInt32(obj_f_critter_alignment_choice, alignmentChoiceIdx);
			}
		}
		
		ImGui::TreePop();
	}
	

	// NPC props
	if (obj->IsNPC() && ImGui::TreeNodeEx("NPC properties", ImGuiTreeNodeFlags_CollapsingHeader)) {

		auto hitDiceNum = obj->GetInt32(obj_f_npc_hitdice_idx, 0);
		if (ImGui::InputInt("HD ", &hitDiceNum)) {
			if (hitDiceNum >= 0) {
				obj->SetInt32(obj_f_npc_hitdice_idx, 0, hitDiceNum);
			}
		}

		auto npAcBonus = obj->GetInt32(obj_f_npc_ac_bonus);
		if (ImGui::InputInt("AC Bonus ", &npAcBonus)) {
			if (npAcBonus >= 0) {
				obj->SetInt32(obj_f_npc_ac_bonus, 0, npAcBonus);
			}
		}

		ImGui::TreePop();
	}

	// Class Levels
	if (ImGui::TreeNodeEx("Class Levels", ImGuiTreeNodeFlags_CollapsingHeader)) {
		static int classCur = 0;
		static auto classNameGetter = [](void* data, int idx, const char** outTxt)->bool
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

	// Skills
	if (ImGui::TreeNodeEx("Skill Ranks", ImGuiTreeNodeFlags_CollapsingHeader)) {
		static int skillCur = 0;
		static auto skillNameGetter = [](void* data, int idx, const char** outTxt)->bool
		{
			if ((uint32_t)idx >= skillNames.size())
				return false;
			*outTxt = skillNames[idx].c_str();
			return true;
		};


		std::vector<float> skillRnkChang;
		auto skillIdx = 0;
		for (auto it : critEditor.skillRanks) {
			auto skill = it.first;
			if (!skillSys.IsEnabled(skill))
				continue;
			skillRnkChang.push_back(it.second / 2.0f);
			if (ImGui::InputFloat(skillSys.GetSkillName(skill), &skillRnkChang[skillIdx], 0.5f, 1.0f, 1)) {
				critEditor.skillRanks[skill] = round(2.0 * skillRnkChang[skillIdx]);
			}
			skillIdx++;
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Inventory", ImGuiTreeNodeFlags_CollapsingHeader)) {

		if(ImGui::Button("Prune invalids")) {
			obj->PruneNullInventoryItems();
		}

		auto invArr = obj->GetObjectIdArray(obj_f_critter_inventory_list_idx);
		auto invenCount = invArr.GetSize();
		
		auto invenCountField = obj->GetInt32(obj_f_critter_inventory_num);
		ImGui::Text(fmt::format("obj_f_critter_inventory_num: {}", invenCountField).c_str());
		for (auto i = 0u; i < invenCount; ++i) {
			auto itemHandle = obj->GetObjHndl(obj_f_critter_inventory_list_idx,i);
			auto item = objSystem->GetObject(itemHandle);
			
			if (item) {
				auto itemInvIdx = item->GetInt32(obj_f_item_inv_location);
				ImGui::Text(fmt::format("{} (inv idx = {}): {} {}", i, itemInvIdx, itemHandle, item->id.ToString()).c_str());
			}
			else {
				ImGui::Text(fmt::format("{} : {}", i, itemHandle).c_str());
			}
			
		}
		ImGui::TreePop();
	}

	// Feats
	if (ImGui::TreeNodeEx("Feats", ImGuiTreeNodeFlags_CollapsingHeader)) {

		static auto featNameGetter = [](void* data, int idx, const char** outTxt)->bool
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
				if (featCur < 0 || featCur >(int)featSelectionList.size())
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
					if (static_cast<size_t>(subFeatCur) < childFeats.size())
						subFeat = childFeats[subFeatCur];
				}
			}

			if (featEnum != FEAT_NONE) {

				if (!feats.IsFeatMultiSelectMaster(featEnum)) {
					if (ImGui::Button("Add")) {
						feats.FeatAdd(mEditedObj, featEnum, false);
						critEditor.feats.push_back(featEnum);
					}
				}
				else {

					auto subfeatNameGetter = [](void* data, int idx, const char** outTxt)->bool {
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
						feats.FeatAdd(mEditedObj, subFeat, false); // not checking prereqs because some of them may use the char editor interface which needs a handle
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

	// Spells Known & Memorized

	static auto spellNameGetter = [](void* data, int idx, const char** outTxt)->bool {
		if ((size_t)idx >= spellNames.size())
			return false;
		auto it = spellNames.begin();
		std::advance(it, idx);
		if (it == spellNames.end())
			return false;

		*outTxt = it->second.c_str();
		return true;
	};
	static auto spellClassNameGetter = [](void* data, int idx, const char** outTxt)->bool
	{
		if ((size_t)idx >= d20ClassSys.classEnumsWithSpellLists.size())
			return false;
		*outTxt = d20Stats.GetStatName(d20ClassSys.classEnumsWithSpellLists[idx]);
		return true;
	};

	if (ImGui::TreeNodeEx("Spells Known", ImGuiTreeNodeFlags_CollapsingHeader)) {


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
		for (auto &it : critEditor.spellsKnown) {
			if (spellNames.find(it.spellEnum) == spellNames.end())
				continue;
			if (ImGui::TreeNode(fmt::format("{} ({})", spellNames[it.spellEnum], it.spellEnum).c_str())) {
				if (spellSys.isDomainSpell(it.classCode))
					auto dummy = 1;
				else
					ImGui::Text(fmt::format("Spell Class: {}", d20Stats.GetStatName(spellSys.GetCastingClass(it.classCode))).c_str());

				ImGui::Text(fmt::format("Spell Level: {}", it.spellLevel).c_str());
				ImGui::TreePop();
			}
		}



		ImGui::TreePop();
	}

	// Spells Memorized
	if (ImGui::TreeNodeEx("Spells Memorized", ImGuiTreeNodeFlags_CollapsingHeader)) {

		if (ImGui::Button("Pending to Memorized")) {
			spellSys.SpellsPendingToMemorized(mEditedObj);
		}

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
					critEditor.spellsMemorized.push_back(SpellStoreData(spEnum, spLvl, spellSys.GetSpellClass(spellClassCur), 0, SpellStoreType::spellStoreKnown));
				}
			}


			ImGui::TreePop();
		}

		// Existing memorized spells
		for (auto &it : critEditor.spellsMemorized) {
			if (spellNames.find(it.spellEnum) == spellNames.end())
				continue;
			if (ImGui::TreeNode(fmt::format("{} ({})", spellNames[it.spellEnum], it.spellEnum).c_str())) {
				if (spellSys.isDomainSpell(it.classCode))
					auto dummy = 1;
				else
					ImGui::Text(fmt::format("Spell Class: {}", d20Stats.GetStatName(spellSys.GetCastingClass(it.classCode))).c_str());

				ImGui::Text(fmt::format("Spell Level: {}", it.spellLevel).c_str());
				ImGui::TreePop();
			}
		}



		ImGui::TreePop();
	}

	// Modifiers & Conditions
	if (ImGui::TreeNodeEx("Modifiers & Conditions", ImGuiTreeNodeFlags_CollapsingHeader)) {

		static auto condNameGetter = [](void* data, int idx, const char** outTxt)->bool {
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

		auto displayCondUi = [](objHndl handle, Dispatcher* dispatcher, obj_f fieldType) {
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

	if (ImGui::TreeNodeEx("Scripts", ImGuiTreeNodeFlags_CollapsingHeader)) {
		static auto scriptNameGetter = [](void* data, int idx, const char** outTxt)->bool
		{
			if (idx > SAN::san_unlock_attempt)
				return false;
			*outTxt = pythonObjIntegration.GetEventName((ObjScriptEvent)idx).c_str();
			return true;
		};
		std::vector<int> scriptsChang;
		ImGui::PushItemWidth(145);
		for (auto idx = 0; idx <= SAN::san_unlock_attempt; idx++) {
			scriptsChang.push_back(critEditor.scripts[idx]);
			if (ImGui::InputInt(pythonObjIntegration.GetEventName((ObjScriptEvent)idx).c_str(), &scriptsChang[idx])) {
				critEditor.scripts[idx] = scriptsChang[idx];
			}
		}
		ImGui::PopItemWidth();
		ImGui::TreePop();
	}

	// AI
	if (obj->IsNPC() || !objects.IsPlayerControlled(mEditedObj)) {
		auto curAiStartegyIdx = obj->GetInt32(obj_f_critter_strategy);
		auto aiStratGetter = [](void* data, int idx, const char** outTxt)->bool {
			AiStrategy* aiStrat = aiSys.GetAiStrategy((uint32_t)idx);
			if (!aiStrat)
				return false;
			*outTxt = aiStrat->name.c_str();
			return true;
		};

		if (ImGui::Combo("AI Type", &curAiStartegyIdx, aiStratGetter, nullptr, aiSys.aiStrategies.size(), 8)) {
			obj->SetInt32(obj_f_critter_strategy, curAiStartegyIdx);
		}
	}

	if (ImGui::Button("Apply")) {
		ApplyObjEdit(mEditedObj);
	}

	if (ImGui::IsWindowHovered()) {
		SetMoused(true);
	}
	
	ImGui::End();
}



void DungeonMaster::SetObjEditor(objHndl handle) {

	critEditor = ObjEditor();

	auto obj = objSystem->GetObject(handle);
	if (!obj || !objSystem->IsValidHandle(handle)) {
		mEditedObj = objHndl::null;
		return;
	}

	critEditor.name = description.getDisplayName(handle);
	auto facs = obj->GetInt32Array(obj_f_npc_faction);
	for (auto i = 0u; i < facs.GetSize(); i++) {
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
	for (auto i = 0; i <= SAN::san_unlock_attempt; i++) {
		auto& script = scriptsArray[i];
		critEditor.scripts.push_back(script.scriptId);
	}

	// Skill ranks
	auto skillRanks = obj->GetInt32Array(obj_f_critter_skill_idx);
	skillSys.DoForAllSkills([&](SkillEnum skill) {
		auto skillRnk = skillRanks[skill];
		critEditor.skillRanks[skill] = skillRnk;
		});

	// Feats
	static objHndl thisHandle;
	thisHandle = handle;
	feats.DoForAllFeats([](int featEnum) {
		if (feats.HasFeatCount(thisHandle, (feat_enums)featEnum)) {
			critEditor.feats.push_back((feat_enums)featEnum);
		}
		});

	// Spells
	auto spellsKnown = obj->GetSpellArray(obj_f_critter_spells_known_idx);
	for (auto i = 0u; i < spellsKnown.GetSize(); i++) {
		critEditor.spellsKnown.push_back(spellsKnown[i]);
	}
	auto spellsMemoed= obj->GetSpellArray(obj_f_critter_spells_memorized_idx);
	for (auto i = 0u; i < spellsMemoed.GetSize(); i++) {
		critEditor.spellsMemorized.push_back(spellsMemoed[i]);
	}

}

void DungeonMaster::ApplyObjEdit(objHndl handle) {
	auto obj = objSystem->GetObject(mEditedObj);
	if (!obj || !objSystem->IsValidHandle(mEditedObj))
		return;

	// Factions
	if (obj->IsNPC()) {
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

	// Skills
	{
		obj->ClearArray(obj_f_critter_skill_idx);
		for (auto it : critEditor.skillRanks) {
			auto skillValue = it.second;
			auto skill = it.first;
			obj->SetInt32(obj_f_critter_skill_idx, skill, it.second);
		}
	}


	// Scripts
	auto scriptIdx = 0u;
	for (auto it : critEditor.scripts) {
		ObjectScript script = obj->GetScript(obj_f_scripts_idx, scriptIdx);
		script.scriptId = it;
		obj->SetScript(obj_f_scripts_idx, scriptIdx++, script);
	}

	// Class Levels
	obj->ClearArray(obj_f_critter_level_idx);
	for (auto it : critEditor.classLevels) {
		for (auto i = 0; i < it.second; i++)
			obj->AppendInt32(obj_f_critter_level_idx, it.first); // todo: preserve order...
	}

	// Spells
	obj->ClearArray(obj_f_critter_spells_known_idx);
	auto spellIdx = 0;
	for (auto &it : critEditor.spellsKnown) {
		obj->SetSpell(obj_f_critter_spells_known_idx, spellIdx++, it);
	}
	{
		obj->ClearArray(obj_f_critter_spells_memorized_idx);
		auto spellIdx = 0;
		for (auto& it : critEditor.spellsMemorized) {
			obj->SetSpell(obj_f_critter_spells_memorized_idx, spellIdx++, it);
		}
	}
	


	d20StatusSys.D20StatusRefresh(handle);
}
