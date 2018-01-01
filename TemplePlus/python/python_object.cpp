#include "stdafx.h"
#include "python_object.h"
#include "python_support.h"
#include "../maps.h"
#include "../inventory.h"
#include "../gamesystems/timeevents.h"
#include "../gamesystems/gamesystems.h"
#include "../reputations.h"
#include "../critter.h"
#include "../anim.h"
#include "../ai.h"
#include "../combat.h"
#include "../dispatcher.h"
#include "../location.h"
#include "../party.h"
#include "../tig/tig_mes.h"
#include "../float_line.h"
#include "../condition.h"
#include "../ui/ui.h"
#include "../damage.h"
#include "../sound.h"
#include "../skill.h"
#include "../ui/ui_dialog.h"
#include "../dialog.h"
#include "../gamesystems/objects/objsystem.h"
#include "python_dice.h"
#include "python_objectscripts.h"
#include <objlist.h>
#include "python_integration_obj.h"
#include <action_sequence.h>
#include <ui/ui_picker.h>
#include <config/config.h>
#include <infrastructure/mesparser.h>
#include <infrastructure/elfhash.h>
#include "temple_functions.h"
#include <gamesystems/objects/objfields.h>
#include <d20_level.h>
#include <turn_based.h>
#include <gamesystems/objects/objevent.h>
#include "ui/ui_systems.h"
#include "ui/ui_legacysystems.h"
#include "pathfinding.h"

#include <pybind11/embed.h>
#include <pybind11/cast.h>
#include <pybind11/stl.h>
#include "python_spell.h"

namespace py = pybind11;
using namespace py::literals;

class objScriptsWrapper {
	// TODO
};

class objHndlWrapper {
public:

	objHndlWrapper(objHndl handle) : handle_(handle) {
		// Get GUID from handle
		if (handle) {
			id_ = objects.GetId(handle);

			// The obj handle is invalid
			if (!id_) {
				throw TempleException("The object handle {} is invalid.", handle.handle);
			}
		}
	}

	objHndlWrapper(uint64_t handle_value = 0u) : objHndlWrapper(objHndl{ handle_value }) {}

	operator objHndl() const {
		return resolve_handle();
	}

	std::string to_string() const {
		auto handle = resolve_handle();
		if (!handle_) {
			return "OBJ_HANDLE_NULL";
		}
		else {
			auto name = objects.GetDisplayName(handle, handle);
			return format("{}({})", name, handle);
		}
	}

	py::object get_char_classes() const {
		auto handle = resolve_handle();
		if (!handle) {
			return py::none();
		}

		auto toEEobj = gameSystems->GetObj().GetObject(handle);
		auto charClasses = toEEobj->GetInt32Array(obj_f_critter_level_idx);
		int count = charClasses.GetSize();

		auto tuple = py::tuple(count);
		for (int i = 0; i < count; i++) {
			tuple[i] = charClasses[i];
		}
		return tuple;
	}

	py::str get_description() const {
		return objects.description.getDisplayName(get_required_handle());
	}

	int get_name() const {
		return get_game_object().GetName();
	}


	int get_proto_id() const {
		return gameSystems->GetObj().GetProtoId(get_required_handle());
	}

	uint64_t get_location() const {
		return objects.GetLocation(get_required_handle());
	}

	ObjectType get_type() const {
		return get_game_object().GetType();
	}

	float get_radius() const {
		return objects.GetRadius(get_required_handle());
	}

	void set_radius(float radius) {
		objects.SetRadius(get_required_handle(), radius);
	}

	float get_render_height() const {
		return objects.GetRenderHeight(get_required_handle());
	}

	void set_render_height(float render_height) const {
		return objects.SetRenderHeight(get_required_handle(), render_height);
	}

	float get_rotation() const {
		return objects.GetRotation(get_required_handle());
	}

	void set_rotation(float rotation) {
		objects.SetRotation(get_required_handle(), rotation);
	}

	Dice get_hit_dice() const {
		return objects.GetHitDice(get_required_handle());
	}

	int get_hit_dice_num() const {
		return objects.GetHitDiceNum(get_required_handle());
	}

	int get_size() const {
		return objects.GetSize(get_required_handle());
	}

	float get_offset_x() const {
		return objects.GetOffsetX(get_required_handle());
	}

	float get_offset_y() const {
		return objects.GetOffsetY(get_required_handle());
	}

	std::unique_ptr<objScriptsWrapper> get_scripts() const {
		return std::make_unique<objScriptsWrapper>();
	}

	int get_origin_map_id() const {
		return objects.GetOriginMapId(get_required_handle());
	}

	void set_origin_map_id(int map_id) {
		if (!maps.IsValidMapId(map_id)) {
			throw TempleException("The map id {} is invalid.", map_id);
		}
		objects.SetOriginMapId(get_required_handle(), map_id);
	}

	objHndlWrapper get_substitute_inventory() const {
		return inventory.GetSubstituteInventory(get_required_handle());
	}

	void set_substitute_inventory(objHndlWrapper wrapper) {
		auto gameObj = get_game_object();
		gameObj.SetObjHndl(obj_f_npc_substitute_inventory, wrapper);
	}

	py::tuple get_feats() const {
		auto feats = objects.feats.GetFeats(get_required_handle());

		py::tuple result(feats.size());
		for (size_t i = 0; i < feats.size(); ++i) {
			result[i] = feats[i];
		}
		return result;
	}

	py::tuple get_factions() const {
		auto faction_array = get_game_object().GetInt32Array(obj_f_npc_faction);

		auto handle = get_required_handle();
		int num_factions = 0;

		for (size_t i = 0; i < faction_array.GetSize(); i++) {
			if (faction_array[i] == 0) {
				break;
			}
			num_factions++;
		}

		py::tuple result(num_factions);
		for (int i = 0; i < num_factions; i++) {
			result[i] = faction_array[i];
		}
		return result;
	}

	py::tuple get_class_levels() const {
		auto class_levels = get_game_object().GetInt32Array(obj_f_critter_level_idx);
		int count = class_levels.GetSize();

		py::tuple result(count);
		for (int i = 0; i < count; i++) {
			result[i] = class_levels[i];
		}
		return result;
	}

	int get_highest_arcane_class() const {
		auto handle = get_required_handle();
		auto highestClass = (Stat)0;
		auto highestLvl = 0;

		for (auto it : d20ClassSys.classEnumsWithSpellLists) {
			auto classEnum = (Stat)it;
			if (d20ClassSys.IsArcaneCastingClass(classEnum)) {
				auto lvlThis = objects.StatLevelGet(handle, classEnum);
				if (lvlThis > highestLvl) {
					highestLvl = lvlThis;
					highestClass = classEnum;
				}
			}
		}

		return highestClass;
	}

	int get_highest_divine_class() const {
		auto handle = get_required_handle();
		auto highestClass = (Stat)0;
		auto highestLvl = 0;

		for (auto it : d20ClassSys.classEnumsWithSpellLists) {
			auto classEnum = (Stat)it;
			if (d20ClassSys.IsDivineCastingClass(classEnum)) {
				auto lvlThis = objects.StatLevelGet(handle, classEnum);
				if (lvlThis > highestLvl) {
					highestLvl = lvlThis;
					highestClass = classEnum;
				}
			}
		}

		return highestClass;
	}

	py::tuple get_spells_known() const {
		return get_spell_list_as_tuple(obj_f_critter_spells_known_idx);
	}

	py::tuple get_spells_memorized() const {
		return get_spell_list_as_tuple(obj_f_critter_spells_memorized_idx);
	}

	int get_loots() const {
		return critterSys.GetLootBehaviour(get_required_handle());
	}

	void set_loots(int loots) {
		critterSys.SetLootBehaviour(get_required_handle(), loots);
	}

	int compare_to(const objHndlWrapper &other) {
		objHndl handleA = handle_;
		objHndl handleB = other.handle_;

		if (handleA.handle < handleB.handle) {
			return -1;
		} else if (handleA.handle > handleB.handle) {
			return 1;
		} else {
			return 0;
		}
	}

#pragma region Pickle Protocol 

	py::tuple reduce() {
		auto constructor = py::cast(*this).get_type();
		auto unpickleArgs = py::make_tuple();
		auto pickledValue = get_state(*this);

		return py::make_tuple(constructor, unpickleArgs, pickledValue);
	}

	// Only the object GUID is pickled
	// The format has to be compatible with old ToEE to be save compatible
	static py::tuple get_state(const objHndlWrapper &wrapper) {
		// Mobile GUID
		if (wrapper.id_.IsPermanent()) {
			const auto& guid = wrapper.id_.body.guid;
			return py::make_tuple(
				2,
				py::make_tuple(
					guid.Data1,
					guid.Data2,
					guid.Data3,
					py::make_tuple(
						guid.Data4[0],
						guid.Data4[1],
						guid.Data4[2],
						guid.Data4[3],
						guid.Data4[4],
						guid.Data4[5],
						guid.Data4[6],
						guid.Data4[7]
					)
				)
			);
		}
		return py::make_tuple((int)wrapper.id_.subtype);
	}

	void set_state(py::tuple &pickled_data) {

		if (pickled_data.size() == 1) {
			ObjectIdKind subtype = (ObjectIdKind)pickled_data[0].cast<int>();
			if (subtype != ObjectIdKind::Null) {
				throw TempleException("GUID type {} was specified, but actual GUID body is missing.", subtype);
			}
			id_.subtype = ObjectIdKind::Null;
			return;
		}
		
		if (pickled_data.size() != 2) {
			throw TempleException("Expected pickled object id to either have 1 or 2 elements. But got: {}", py::str(pickled_data));
		}

		ObjectId id;
		id.subtype = (ObjectIdKind)pickled_data[0].cast<int>();
		py::tuple guid_content = pickled_data[1].cast<py::tuple>();

		if (!id) {
			// This is the null obj handle
			id_.subtype = ObjectIdKind::Null;
			return;
		}

		// Try parsing the GUID tuple
		id.subtype = ObjectIdKind::Permanent;

		if (guid_content.size() != 4) {
			throw TempleException("Expected GUID body to have 4 elements. Got: {}", py::str(guid_content));
		}

		auto& guid = id.body.guid;
		guid.Data1 = guid_content[0].cast<unsigned long>();
		guid.Data2 = guid_content[1].cast<unsigned short>();
		guid.Data3 = guid_content[2].cast<unsigned short>();
		
		auto data4_tuple = guid_content[3].cast<py::tuple>();
		if (data4_tuple.size() != 8) {
			throw TempleException("Expected data4 part of GUID to have 8 elements, but got: {}", py::str(data4_tuple));
		}
		for (int i = 0; i < 8; i++) {
			guid.Data4[i] = data4_tuple[i].cast<unsigned char>();
		}

		// Finally look up the handle for the GUID
		id_ = id;
		handle_ = gameSystems->GetObj().GetHandleById(id);
	}
#pragma endregion

#pragma region Methods

	/*
	Schedules a Python dialog time event in 1ms. Probably so the rest of the calling
	script executes before the Python dialog UI is created.
	self: PC
	target: NPC
	line: The initial line in the NPCs dialog to show
	*/
	void BeginDialog(objHndlWrapper target, int line) {
		auto handle = get_required_handle();

		if (critterSys.IsPC(target))
		{
			TimeEvent evt;
			evt.system = TimeEventType::PythonDialog;
			evt.params[0].handle = handle;
			evt.params[1].handle = target;
			evt.params[2].int32 = line;
			gameSystems->GetTimeEvent().Schedule(evt, 1);
		}
		else
		{
			// TODO: Add a "Party Leader Override" option that attempts to initiate dialogue with the Party Leader if possible
		}

	}

	void Barter() {
		logger->info("There was never any .barter() method in ToEE, you've been copypasting nonsense all along. Trolololo!");
	}

#pragma region Python Obj Faction Manipulation
	int FactionHas(int objFaction) {
		return objects.factions.FactionHas(get_required_handle(), objFaction);
	}

	int FactionAdd(int nFac) {
		if (nFac == 0) {
			return 0;
		}

		auto handle = get_required_handle();
		if (!objects.factions.FactionHas(handle, nFac)) {
			objects.factions.FactionAdd(handle, nFac);
		}

		return 1;
	}
#pragma endregion

	void FallDown(int fallDownArg = 73)
	{
		if (fallDownArg > 75 || fallDownArg < 73)
			fallDownArg = 73;
		animationGoals.PushFallDown(get_required_handle(), fallDownArg);
	}

	int ReactionGet(objHndlWrapper towards) {
		return objects.GetReaction(get_required_handle(), towards);
	}

	void ReactionSet(objHndlWrapper towards, int desired_reaction) {
		auto handle = get_required_handle();
		auto currentReaction = objects.GetReaction(handle, towards);
		auto adjustment = desired_reaction - currentReaction;
		objects.AdjustReaction(handle, towards, adjustment);
	}

	void ReactionAdjust(objHndlWrapper towards, int adjustment) {
		objects.AdjustReaction(get_required_handle(), towards, adjustment);
	}

	void RefreshTurn() {
		auto curSeq = *actSeqSys.actSeqCur;
		curSeq->tbStatus.hourglassState = 4;
		curSeq->tbStatus.surplusMoveDistance = 0;
		curSeq->tbStatus.tbsFlags = 0;
	}

	objHndlWrapper ItemFind(int name_id) {
		return inventory.FindItemByName(get_required_handle(), name_id);
	}

	int ItemTransferTo(objHndlWrapper target, int name_id) {
		auto item = inventory.FindItemByName(get_required_handle(), name_id);
		auto result = 0;
		if (item) {
			result = inventory.SetItemParent(item, target, 0);
		}
		return result;
	}

	objHndlWrapper ItemFindByProto(int proto_id) {
		return inventory.FindItemByProtoId(get_required_handle(), proto_id);
	}

	int ItemTransferToByProto(objHndlWrapper target, int proto_id) {
		auto item = inventory.FindItemByProtoId(get_required_handle(), proto_id);
		auto result = 0;
		if (item) {
			result = inventory.SetItemParent(item, target, 0);
		}
		return result;
	}

	int MoneyGet() {
		return objects.StatLevelGet(get_required_handle(), stat_money);
	}

	void MoneyAdj(int copper_adj) {
		if (copper_adj <= 0) {
			critterSys.TakeMoney(get_required_handle(), 0, 0, 0, -copper_adj);
		}
		else {
			critterSys.GiveMoney(get_required_handle(), 0, 0, 0, copper_adj);
		}
	}

	int CanSneakAttack(objHndlWrapper target) {
		auto target_handle = static_cast<objHndl>(target);
		if (!target_handle) {
			return 0;
		}
		auto handle = get_required_handle();

		auto sneakAtkDice = d20Sys.D20QueryPython(handle, "Sneak Attack Dice");

		if (!sneakAtkDice)
			return 0;
		if (d20Sys.d20Query(target_handle, DK_QUE_Critter_Is_Immune_Critical_Hits))
			return 0;

		auto isFlanked = combatSys.IsFlankedBy(target_handle, handle);


		auto result = (isFlanked
			|| d20Sys.d20Query(target_handle, DK_QUE_SneakAttack)
			|| !critterSys.CanSense(target_handle, handle));

		return result;
	}


	void CastSpell(uint32_t spell_enum, objHndlWrapper target = objHndlWrapper()) {

		PickerArgs pickArgs;
		SpellPacketBody spellPktBody;
		SpellEntry spellEntry;
		objHndl targetObj = target;
		objHndl caster = get_required_handle();

		LocAndOffsets loc;
		D20SpellData d20SpellData;

		// get spell known data
		// I've set up a really large buffer just in case, 
		// because in theory a player might have permutations 
		// of spells due to metamagic, different casting classes etc.
		uint32_t classCodes[10000] = { 0, };
		uint32_t spellLevels[10000] = { 0, };
		uint32_t numSpells = 0;
		if (!spellSys.spellKnownQueryGetData(caster, spell_enum, classCodes, spellLevels, &numSpells)) {
			return;
		}
		if (numSpells <= 0) {
			return;
		}
		spellSys.spellPacketBodyReset(&spellPktBody);
		spellPktBody.spellEnum = spell_enum;
		spellPktBody.spellEnumOriginal = spell_enum;
		spellPktBody.caster = caster;

		/*if (*actSeqSys.actSeqPickerActive){
		auto dummy = 1;
		}*/

		for (uint32_t i = 0; i < numSpells; i++) {
			if (!spellSys.spellCanCast(caster, spell_enum, classCodes[i], spellLevels[i]))
				continue;
			spellPktBody.spellKnownSlotLevel = spellLevels[i];
			spellPktBody.spellClass = classCodes[i];
			spellSys.SpellPacketSetCasterLevel(&spellPktBody);
			if (!spellSys.spellRegistryCopy(spell_enum, &spellEntry))
				continue;
			if (!spellSys.pickerArgsFromSpellEntry(&spellEntry, &pickArgs, caster, spellPktBody.casterLevel))
				continue;
			pickArgs.result = { 0, };
			pickArgs.flagsTarget = (UiPickerFlagsTarget)(
				(uint64_t)pickArgs.flagsTarget | (uint64_t)pickArgs.flagsTarget & UiPickerFlagsTarget::LosNotRequired
				- (uint64_t)pickArgs.flagsTarget & UiPickerFlagsTarget::Range
				);

			if (static_cast<uint64_t>(pickArgs.modeTarget) & static_cast<uint64_t>(UiPickerType::Single)) {
				objects.loc->getLocAndOff(targetObj, &loc);
				uiPicker.SetSingleTarget(targetObj, &pickArgs);
			}
			else if (static_cast<uint64_t>(pickArgs.modeTarget) & static_cast<uint64_t>(UiPickerType::Multi)) {
				objects.loc->getLocAndOff(targetObj, &loc);
				uiPicker.SetSingleTarget(targetObj, &pickArgs);
			}
			else if (static_cast<uint64_t>(pickArgs.modeTarget) & static_cast<uint64_t>(UiPickerType::Cone)) {
				objects.loc->getLocAndOff(targetObj, &loc);
				uiPicker.SetConeTargets(&loc, &pickArgs);

			}
			else if (static_cast<uint64_t>(pickArgs.modeTarget) & static_cast<uint64_t>(UiPickerType::Area)) {
				if (spellEntry.spellRangeType == SRT_Personal)
					objects.loc->getLocAndOff(caster, &loc);
				else
					objects.loc->getLocAndOff(targetObj, &loc);
				uiPicker.GetListRange(&loc, &pickArgs);
			}
			else if (static_cast<uint64_t>(pickArgs.modeTarget) & static_cast<uint64_t>(UiPickerType::Personal)) {
				objects.loc->getLocAndOff(caster, &loc);
				uiPicker.SetSingleTarget(caster, &pickArgs);
			}

			spellSys.ConfigSpellTargetting(&pickArgs, &spellPktBody);
			if (spellPktBody.targetCount <= 0)
				continue;
			if (!actSeqSys.TurnBasedStatusInit(caster))
				continue;
			d20Sys.GlobD20ActnInit();
			d20Sys.GlobD20ActnSetTypeAndData1(D20A_CAST_SPELL, 0);
			actSeqSys.ActSeqCurSetSpellPacket(&spellPktBody, 1);
			d20Sys.D20ActnSetSpellData(&d20SpellData, spell_enum, classCodes[i], spellLevels[i], 0xFF, 0);
			d20Sys.GlobD20ActnSetSpellData(&d20SpellData);
			d20Sys.GlobD20ActnSetTarget(targetObj, &loc);
			actSeqSys.ActionAddToSeq();
			actSeqSys.sequencePerform();
		}
	}

	int CanCastSpell(const SpellStoreData &spell_store) {
		auto handle = get_required_handle();

		if (spellSys.spellCanCast(handle, spell_store.spellEnum, spell_store.classCode, spell_store.spellLevel) != TRUE) {
			return 0;
		}

		if (spell_store.classCode == spellSys.GetSpellClass(stat_level_paladin) && d20Sys.d20Query(handle, DK_QUE_IsFallenPaladin)) {
			return 0;
		}

		return 1;
	}

	int CanFindPathToObj(objHndlWrapper target, int flags) {
		auto handle = get_required_handle();
		objHndl tgtObj = target;

		PathQuery pathQ;
		PathQueryResult pqr;
		pathQ.critter = handle;
		pathQ.from = objSystem->GetObject(handle)->GetLocationFull();
		pathQ.targetObj = tgtObj;
		if (pathQ.critter == pathQ.targetObj)
			return 0;

		const float fourPointSevenPlusEight = 4.714045f + 8.0f;
		pathQ.flags = static_cast<PathQueryFlags>(PathQueryFlags::PQF_TO_EXACT | PathQueryFlags::PQF_HAS_CRITTER | PathQueryFlags::PQF_800
			| PathQueryFlags::PQF_TARGET_OBJ | PathQueryFlags::PQF_ADJUST_RADIUS | PathQueryFlags::PQF_ADJ_RADIUS_REQUIRE_LOS);
		*((int*)&pathQ.flags) |= flags;

		auto reach = critterSys.GetReach(handle, D20A_UNSPECIFIED_MOVE);
		if (reach < 0.1) { reach = 3.0; }
		pathQ.distanceToTargetMin = 0.0;
		pathQ.tolRadius = reach * 12.0f - fourPointSevenPlusEight;

		auto nodeCount = pathfindingSys.FindPath(&pathQ, &pqr);

		return (int)pathfindingSys.GetPathLength(&pqr);
	}

	int SkillLevelGet(objHndlWrapper target, SkillEnum skill_id) {
		return dispatch.dispatch1ESkillLevel(get_required_handle(), skill_id, nullptr, target, 1);
	}

	int SkillLevelGet(SkillEnum skill_id) {
		return dispatch.dispatch1ESkillLevel(get_required_handle(), skill_id, nullptr, objHndl::null, 1);
	}

	int SkillRanksGet(SkillEnum skill_id) {
		return critterSys.SkillBaseGet(get_required_handle(), skill_id);
	}

	int SkillRoll(SkillEnum skill_id, int dc, int flags = 0) {
		int deltaFromDc;
		return skillSys.SkillRoll(get_required_handle(), skill_id, dc, &deltaFromDc, flags);
	}

	int HasMet(objHndlWrapper critter) {
		return critterSys.HasMet(get_required_handle(), critter);
	}

	/*
	Checks if the player has a follower with a given name id.
	*/
	int HasFollower(int name_id) {
		ObjList followers;
		followers.ListFollowers(get_required_handle());

		for (int i = 0; i < followers.size(); ++i) {
			auto follower = followers[i];
			if (objects.GetNameId(follower) == name_id) {
				return 1;
			}
		}

		return 0;
	}

	py::tuple GroupList() {
		auto len = party.GroupListGetLen();
		py::tuple result(len);
		for (uint32_t i = 0; i < len; ++i) {
			result[i] = party.GroupListGetMemberN(i);
		}
		return result;
	}

	int GetBaseAttackBonus() {
		return critterSys.GetBaseAttackBonus(get_required_handle());
	}

	int StatLevelGet(Stat stat, int stat_arg) {
		return objects.StatLevelGet(get_required_handle(), stat, stat_arg); // WIP currently just handles stat_caster_level expansion
	}

	int StatLevelGet(Stat stat) {
		return objects.StatLevelGet(get_required_handle(), stat);
	}

	int StatLevelGetBase(Stat stat) {
		return objects.StatLevelGetBase(get_required_handle(), stat);
	}

	int StatLevelSetBase(Stat stat, int value) {
		return objects.StatLevelSetBase(get_required_handle(), stat, value);
	}

	int FollowerAdd(objHndlWrapper follower) {
		auto result = critterSys.AddFollower(follower, get_required_handle(), 1, false);
		uiSystems->GetParty().Update();
		return result;
	}

	int FollowerRemove(objHndlWrapper follower) {
		auto result = critterSys.RemoveFollower(follower, 1);
		uiSystems->GetParty().Update();
		return result;
	}

	int FollowerAtMax() {
		auto followers = party.GroupNPCFollowersLen();
		auto pcs = party.GroupPCsLen();
		if (config.maxPCsFlexible)
		{
			return (followers + pcs >= PARTY_SIZE_MAX) || followers >= 5;
		}
		return (followers >= PARTY_SIZE_MAX - (uint32_t)config.maxPCs) || followers >= 5;
	}

	int AiFollowerAdd(objHndlWrapper follower) {
		auto result = critterSys.AddFollower(follower, get_required_handle(), 1, true);
		uiSystems->GetParty().Update();
		return result;
	}

	int RemoveFromAllGroups(objHndlWrapper dude) {
		auto result = party.ObjRemoveFromAllGroupArrays(dude);
		uiSystems->GetParty().Update();
		return result;
	}

	int PCAdd(objHndlWrapper dude) {
		auto result = party.AddToPCGroup(get_required_handle());
		uiSystems->GetParty().Update();
		return result;
	}

	// Okay.. Apparently the AI follower methods should better not be called
	int ReturnZero() {
		return 0;
	}

	objHndlWrapper LeaderGet() {
		return critterSys.GetLeader(get_required_handle());
	}

	// CanSee and HasLos are the same
	int HasLos(objHndlWrapper target) {
		auto obstacles = critterSys.HasLineOfSight(get_required_handle(), target);
		return obstacles == 0;
	}

	/*
	Pretty stupid name. Checks if the critter currently wears an item with the
	given name id.
	*/
	int HasWielded(int name_id) {
		auto handle = get_required_handle();
		for (auto i = 0; i < (int)EquipSlot::Count; ++i) {
			auto item = critterSys.GetWornItem(handle, (EquipSlot)i);
			if (item && objects.GetNameId(item) == name_id) {
				return 1;
			}
		}
		return 0;
	}

	bool HasItem(int name_id) {
		return !!inventory.FindItemByName(get_required_handle(), name_id);
	}

	objHndlWrapper ItemWornAt(EquipSlot slot) {
		return critterSys.GetWornItem(get_required_handle(), slot);
	}

	objHndlWrapper InventoryItem(int n) {
		auto handle = get_required_handle();
		bool bRetunProtos = 0;
		int invFieldType = inventory.GetInventoryListField(handle);

		int nMax = CRITTER_MAX_ITEMS + 100; // PLACEHOLDER!
		if (invFieldType == obj_f_container_inventory_list_idx) {
			nMax = CONTAINER_MAX_ITEMS;
		}
		if (n < nMax) {
			return objects.inventory.GetItemAtInvIdx(handle, n);
		}

		return objHndl::null;
	}


	int ArcaneSpellLevelCanCast() {
		auto arcaneSpellLvlMax = 0;
		auto handle = get_required_handle();

		critterSys.GetSpellLvlCanCast(handle, SpellSourceType::Arcane, SpellReadyingType::Any);

		for (auto it : d20ClassSys.classEnums) {
			auto classEnum = (Stat)it;
			if (d20ClassSys.IsArcaneCastingClass(classEnum)) {
				arcaneSpellLvlMax = max(arcaneSpellLvlMax, spellSys.GetMaxSpellLevel(handle, classEnum, 0));
			}
		}

		return arcaneSpellLvlMax;
	}

	int DivineSpellLevelCanCast() {
		auto handle = get_required_handle();

		auto divineSpellLvlMax = 0;

		critterSys.GetSpellLvlCanCast(handle, SpellSourceType::Divine, SpellReadyingType::Any);

		for (auto it : d20ClassSys.classEnums) {
			auto classEnum = (Stat)it;
			if (d20ClassSys.IsDivineCastingClass(classEnum)) {
				divineSpellLvlMax = max(divineSpellLvlMax, spellSys.GetMaxSpellLevel(handle, classEnum, 0));
			}
		}


		return divineSpellLvlMax;
	}


	int SpontaneousSpellLevelCanCast() {
		auto handle = get_required_handle();
		auto spontCastLvl = 0;

		//critterSys.GetSpellLvlCanCast(self->handle, SpellSourceType::Arcane, SpellReadyingType::Any);

		for (auto it : d20ClassSys.classEnums) {
			auto classEnum = (Stat)it;
			if (d20ClassSys.IsNaturalCastingClass(classEnum)) {
				spontCastLvl = max(spontCastLvl, spellSys.GetMaxSpellLevel(handle, classEnum, 0));
			}
		}

		return spontCastLvl;
	}


	void Attack(objHndlWrapper target) {
		objHndl target_handle = target;
		if (!target_handle) {
			logger->warn("Python attack called with null target");
			return;
		}

		auto handle = get_required_handle();

		// This is pretty odd since it causes the opposite oO
		if (!party.IsInParty(target_handle)) {
			critterSys.Attack(handle, target_handle, 1, 2);
		}
		else {
			// This apparently causes the NPC to attack ALL of the party, not just the one PC
			for (uint32_t i = 0; i < party.GroupListGetLen(); ++i) {
				auto partyMember = party.GroupListGetMemberN(i);
				if (objects.IsPlayerControlled(partyMember)) {
					critterSys.Attack(partyMember, handle, 1, 2);
				}
			}
		}
	}

	void TurnTowards(objHndlWrapper target) {
		objHndl target_handle = target;
		if (!target_handle) {
			logger->warn("Python turn_towards called with OBJ_HANDLE_NULL target");
			return;
		}
		auto handle = get_required_handle();
		auto relativeAngle = objects.GetRotationTowards(handle, target_handle);
		if (!objSystem->GetObject(handle)->IsCritter()) {
			objects.SetRotation(handle, relativeAngle);
			return;
		}
		animationGoals.PushRotate(handle, relativeAngle);
	}

	int TripCheck(objHndlWrapper target) {
		auto handle = get_required_handle();
		objHndl target_handle = target;
		if (!target_handle)
			return 0;

		if (combatSys.TripCheck(handle, target_handle)) {
			return 1;
		}
		else {
			return 0;
		}
	}

	void FloatLine(int line_id, objHndlWrapper pc) {
		auto handle = get_required_handle();

		// Try to load the dialog file attached to this object.
		auto dlgScriptId = objects.GetScript(handle, (int)ObjScriptEvent::Dialog);
		auto dlgFile = dialogScripts.GetFilename(dlgScriptId);
		if (dlgFile.empty()) {
			throw TempleException("No dialog script attached to this object.");
		}
		DialogState dlgState;
		if (!dialogScripts.Load(dlgFile, dlgState.dialogHandle)) {
			throw TempleException("Unable to load dialog file {}", dlgFile);
		}

		// Fill in the Dialog State
		dlgState.pc = pc;
		dlgState.reqNpcLineId = line_id;

		/*
		Previously, the sid was used here, but this can actually screw up speech and play back the wrong samples,
		if a different script id is attached for san_dialog (which is used above) than for whatever san triggered
		this function.
		*/
		dlgState.dialogScriptId = dlgScriptId;
		dlgState.npc = handle;
		dialogScripts.LoadNpcLine(dlgState, true);

		uiDialog.ShowTextBubble(dlgState.npc, dlgState.pc, dlgState.npcLineText, dlgState.speechId);

		dialogScripts.Free(dlgState.dialogHandle);
	}

	void Damage(objHndlWrapper attacker, DamageType damage_type, Dice dice, int attack_power = 0, D20ActionType action_type = D20A_NONE) {
		damage.DealDamageFullUnk(get_required_handle(), attacker, dice, damage_type, attack_power, action_type);
	}

	void DamageWithReduction(objHndlWrapper attacker, DamageType damage_type, Dice dice, int attack_power, int reduction, D20ActionType action_type = D20A_NONE) {
		// Line 105: Saving Throw
		damage.DealDamage(get_required_handle(), attacker, dice, damage_type, attack_power, reduction, 105, action_type);
	}

	void DealAttackDamage(objHndlWrapper attacker, int d20data, D20CAF flags, D20ActionType action_type) {
		damage.DealAttackDamage(attacker, get_required_handle(), d20data, flags, action_type);
	}

	void Heal(objHndlWrapper healer, Dice dice, D20ActionType action_type = D20A_NONE, int dummy_spell_id = 0) {
		// dummy_spell_id is not really used, just makign the code tolerate 4 args
		damage.Heal(get_required_handle(), healer, dice, action_type);
	}

	void HealSubdual(objHndlWrapper healer, Dice dice, D20ActionType action_type = D20A_NONE, int spellId = 0) {
		auto amount = dice.Roll();
		damage.HealSubdual(get_required_handle(), amount);
	}

	void SpellDamage(objHndlWrapper attacker, DamageType type, Dice dice, int attack_power_type = 0, D20ActionType action_type = D20A_NONE, int spell_id = 0) {
		damage.DealSpellDamageFullUnk(get_required_handle(), attacker, dice, type, attack_power_type, action_type, spell_id, 0);
	}

	void SpellDamageWithReduction(objHndlWrapper attacker, DamageType type, Dice dice, int attack_power_type = 0, int reduction = 100, D20ActionType action_type = D20A_NONE, int spell_id = 0) {
		// Line 105: Saving Throw
		damage.DealSpellDamage(get_required_handle(), attacker, dice, type, attack_power_type, reduction, 105, action_type, spell_id, 0);
	}

	void SpellDamageWeaponlike(objHndlWrapper attacker, DamageType type, Dice dice, int attack_power_type = 0, int reduction = 100, D20ActionType action_type = D20A_NONE, int spell_id = 0, D20CAF flags = D20CAF_NONE, int projectile_idx = 0) {
		// Line 105: Saving Throw
		damage.DealWeaponlikeSpellDamage(get_required_handle(), attacker, dice, type, attack_power_type, reduction, 105, action_type, spell_id, flags, projectile_idx);
	}

	void StealFrom(objHndlWrapper target) {
		animationGoals.PushUseSkillOn(get_required_handle(), target, skill_pick_pocket);
	}

	int ReputationHas(int reputation_id) {
		return partyReputation.Has(reputation_id);
	}

	void ReputationAdd(int reputation_id) {
		partyReputation.Add(reputation_id);
	}

	void ReputationRemove(int reputation_id) {
		partyReputation.Remove(reputation_id);
	}

	static void ParseCondNameAndArgs(const char *condition_name, py::args &args, CondStruct*& condStructOut, vector<int>& argsOut) {
		auto cond = conds.GetByName(condition_name);
		if (!cond) {
			throw TempleException("Unknown condition name: {}", condition_name);
		}

		// Following arguments all have to be integers and gel with the condition argument count
		auto extra_args = args.size();
		vector<int> condArgs(cond->numArgs, 0);
		for (unsigned int i = 0; i < cond->numArgs; ++i) {
			if (extra_args > i) {
				try {
					condArgs[i] = args[i].cast<int>();
				}
				catch (py::cast_error &e) {
					throw TempleException("Condition argument {} couldn't be cast to an integer.", e.what());
				}
			}
		}

		condStructOut = cond;
		argsOut = condArgs;
	}

	int ItemConditionAdd(const char *condition_name, py::args &args) {
		CondStruct* cond;
		vector<int> condArgs;
		ParseCondNameAndArgs(condition_name, args, cond, condArgs);

		conds.AddToItem(get_required_handle(), cond, condArgs);
		return 1;
	}

	int ConditionAddWithArgs(const char *condition_name, py::args &args) {
		CondStruct* cond;
		vector<int> condArgs;
		ParseCondNameAndArgs(condition_name, args, cond, condArgs);

		return conds.AddTo(get_required_handle(), cond, condArgs);
	}


	int IsFlankedBy(objHndlWrapper critter) {
		objHndl critter_handle = critter;
		if (!critter_handle) {
			return 0;
		}
		return combatSys.IsFlankedBy(get_required_handle(), critter);
	}


	int IsFriendly(objHndlWrapper pc) {
		return critterSys.IsFriendly(pc, get_required_handle());
	}

	void FadeTo(int target_opacity, int transition_time_ms, int tick_quantum, int callback_mode = 0) {
		objects.FadeTo(get_required_handle(), target_opacity, transition_time_ms, tick_quantum, callback_mode);
	}

	void Move(uint64_t location, float off_x = 0, float off_y = 0) {
		LocAndOffsets newLoc;
		newLoc.location = locXY::fromField(location);
		newLoc.off_x = 0;
		newLoc.off_y = 0;
		objects.Move(get_required_handle(), newLoc);
	}

	void Move(int tile_x, int tile_y, float off_x = 0, float off_y = 0) {
		LocAndOffsets newLoc;
		newLoc.location.locx = tile_x;
		newLoc.location.locy = tile_y;
		newLoc.off_x = off_x;
		newLoc.off_y = off_y;
		objects.Move(get_required_handle(), newLoc);
	}

	void FloatMesFileLine(const char *mes_filename, int mes_line_key, FloatLineColor color_id = FloatLineColor::White) {
		MesFile::Content content = MesFile::ParseFile(mes_filename);

		auto it = content.find(mes_line_key);
		if (it == content.end()) {
			throw TempleException("Could not find line {} in mes file {}.", mes_line_key, mes_filename);
		}

		floatSys.floatMesLine(get_required_handle(), 1, color_id, it->second.c_str());
	}

	void FloatTextLine(const char *text_line, FloatLineColor color_id = FloatLineColor::White) {
		floatSys.floatMesLine(get_required_handle(), 1, color_id, text_line);
	}

	// Generic methods to get/set/clear flags
	template <obj_f flags_field>
	int GetFlags() {
		return get_game_object().GetInt32(flags_field);
	}

	template <obj_f flags_field>
	void SetFlag(uint32_t flag) {
		auto &game_object = get_game_object();
		auto current_flags = game_object.GetInt32(flags_field);
		current_flags |= flag;
		game_object.SetInt32(flags_field, current_flags);
	}

	template <obj_f flags_field>
	void ClearFlag(uint32_t flag) {
		auto &game_object = get_game_object();
		auto current_flags = game_object.GetInt32(flags_field);
		current_flags &= ~flag;
		game_object.SetInt32(flags_field, current_flags);
	}

	void ObjectFlagSet(ObjectFlag flag) {
		objects.SetFlag(get_required_handle(), flag);
	}

	void ObjectFlagUnset(ObjectFlag flag) {
		objects.ClearFlag(get_required_handle(), flag);
	}

	void PortalToggleOpen() {
		objects.PortalToggleOpen(get_required_handle());
	}

	void ContainerToggleOpen() {
		objects.ContainerToggleOpen(get_required_handle());
	}

	int SavingThrow(int dc, SavingThrowType type, D20SavingThrowFlag flags = (D20SavingThrowFlag)0, objHndlWrapper attacker = objHndlWrapper(), int d20a_type = 0) {
		return damage.SavingThrow(get_required_handle(), attacker, dc, type, flags);
	}

	int SavingThrowSpell(int dc, SavingThrowType type, D20SavingThrowFlag flags, objHndlWrapper attacker, int spell_id, int spell_id2 = 0) {
		// spellId2 seems like Python bug really, but I'd hate to break vanilla scripts
		return damage.SavingThrowSpell(get_required_handle(), attacker, dc, type, flags, spell_id);
	}

	int ReflexSaveAndDamage(objHndlWrapper attacker, int dc, int reduction, D20SavingThrowFlag flags, Dice damage_dice, DamageType damage_type, D20AttackPower attack_power, D20ActionType action_type = D20A_NONE, int spell_id = 0) {
		return damage.ReflexSaveAndDamage(get_required_handle(), attacker, dc, reduction, flags, damage_dice, damage_type, attack_power, action_type, spell_id);
	}

	int SoundmapCritter(int soundmap_id) {
		return critterSys.SoundmapCritter(get_required_handle(), soundmap_id);
	}

	int Footstep() {
		auto handle = get_required_handle();
		auto soundId = critterSys.SoundmapCritter(handle, 7);
		return sound.PlaySoundAtObj(soundId, handle);
	}

	int SecretdoorDetect(objHndlWrapper viewer) {
		objHndl viewer_handle = viewer;
		if (!viewer_handle) {
			throw TempleException("Called with an invalid viewer.");
		}
		return objects.SecretdoorDetect(get_required_handle(), viewer);
	}

	int HasSpellEffects() {
		return objects.HasSpellEffects(get_required_handle());
	}

	void Kill() {
		critterSys.Kill(get_required_handle(), objHndl::null);
	}

	void KillByEffect(objHndlWrapper killer = objHndlWrapper()) {
		critterSys.KillByEffect(get_required_handle(), killer);
	}

	void Destroy() {
		objects.Destroy(get_required_handle());
		handle_ = objHndl::null; // Clear the obj handle
	}

	int ItemGet(objHndlWrapper item, int flags = ItemInsertFlags::IIF_None) {
		auto handle = get_required_handle();
		if (inventory.IsVisibleInventoryFull(handle) && (flags & ItemInsertFlags::IIF_Use_Max_Idx_200)) {
			return inventory.SetItemParent(item, handle, ItemInsertFlags::IIF_Use_Max_Idx_200);
		}
		else {
			return inventory.SetItemParent(item, handle, ItemInsertFlags::IIF_None);
		}
	}

	int PerformTouchAttack(objHndlWrapper target, int isMelee = 0) {
		D20Actn action(D20A_TOUCH_ATTACK);
		action.d20ATarget = target;

		// Build a to-hit action, do hit processing and return the result
		action.d20APerformer = get_required_handle();
		if (!isMelee)
			action.d20Caf = D20CAF_TOUCH_ATTACK | D20CAF_RANGED;
		else
			action.d20Caf = D20CAF_TOUCH_ATTACK;
		action.data1 = 1;

		d20Sys.ToHitProc(&action);
		d20Sys.CreateRollHistory(action.rollHistId1);
		d20Sys.CreateRollHistory(action.rollHistId2);
		d20Sys.CreateRollHistory(action.rollHistId0);

		return action.d20Caf;
	}

	void AddToInitiative() {
		combatSys.AddToInitiative(get_required_handle());
	}

	void RemoveFromInitiative() {
		combatSys.RemoveFromInitiative(get_required_handle());
	}

	int GetInitiative() {
		return combatSys.GetInitiative(get_required_handle());
	}

	void SetInitiative(int initiative) {
		combatSys.SetInitiative(get_required_handle(), initiative);
	}

	int D20Query(const char *query_string) {
		return d20Sys.D20QueryPython(get_required_handle(), query_string);
	}

	int D20Query(int query_key) {
		auto dispatcherKey = (D20DispatcherKey)(DK_QUE_Helpless + query_key);
		return d20Sys.d20Query(get_required_handle(), dispatcherKey);
	}

	int D20QueryHasSpellCond(int spell_enum) {
		// Get the condition struct from the spell id
		auto cond = spellSys.GetCondFromSpellIdx(spell_enum);
		if (!cond) {
			return 0;
		}

		return d20Sys.d20QueryWithData(get_required_handle(), DK_QUE_Critter_Has_Condition, (uint32_t)cond, 0);
	}

	int D20QueryWithData(const char *query_name, int arg1 = 0, int arg2 = 0) {
		return d20Sys.D20QueryPython(get_required_handle(), query_name, arg1, arg2);
	}

	int D20QueryWithData(int query_key, int arg1, int arg2 = 0) {
		auto dispatcherKey = (D20DispatcherKey)(DK_QUE_Helpless + query_key);
		return d20Sys.d20QueryWithData(get_required_handle(), dispatcherKey, arg1, arg2);
	}

	bool D20QueryTestData(int query_key, int test_data) {
		auto dispatcherKey = (D20DispatcherKey)(DK_QUE_Helpless + query_key);
		return d20Sys.d20QueryReturnData(get_required_handle(), dispatcherKey) == test_data;
	}

	uint64_t D20QueryGetData(int query_key) {
		auto dispatcherKey = (D20DispatcherKey)(DK_QUE_Helpless + query_key);
		return d20Sys.d20QueryReturnData(get_required_handle(), dispatcherKey);
	}

	int CritterGetAlignment() {
		return get_game_object().GetInt32(obj_f_critter_alignment);
	}

	float DistanceTo(objHndlWrapper target) {
		return locSys.DistanceToObj(get_required_handle(), target); // Returns feet
	}

	float DistanceTo(uint64_t location, float off_x = 0, float off_y = 0) {
		LocAndOffsets targetLoc;
		targetLoc.location = locXY::fromField(location);
		targetLoc.off_x = off_x;
		targetLoc.off_y = off_y;
		auto dist = locSys.DistanceToLoc(get_required_handle(), targetLoc);
		if (dist < 0)
			dist = 0;
		return locSys.InchesToFeet(dist);
	}

	/*
	This seems to have been an earlier impl of the frog tongue, but apparently only the failed state
	is used now.
	*/
	void AnimCallback(int id) {
		auto handle = get_required_handle();

		int oldGrappleState;
		switch (id) {
		case 0: // ANIM_CALLBACK_FROG_FAILED_LATCH
			objects.setInt32(handle, obj_f_grapple_state, 1);
			break;
		case 1: // ANIM_CALLBACK_FROG_LATCH
			objects.setInt32(handle, obj_f_grapple_state, 3);
			break;
		case 2: // ANIM_CALLBACK_FROG_PULL
			oldGrappleState = objects.getInt32(handle, obj_f_grapple_state);
			objects.setInt32(handle, obj_f_grapple_state, oldGrappleState & 0xFFFF0005 | 5);
			break;
		case 3: // ANIM_CALLBACK_FROG_SWALLOW
			oldGrappleState = objects.getInt32(handle, obj_f_grapple_state);
			objects.setInt32(handle, obj_f_grapple_state, oldGrappleState & 0xFFFF0007 | 7);
			break;
		default:
			throw TempleException("Unknown animation id passed to anim_callback: {}", id);
		}
	}

	int ObjectScriptExecute(objHndlWrapper triggerer, ObjScriptEvent script_event) {
		return pythonObjIntegration.ExecuteObjectScript(triggerer, get_required_handle(), script_event);
	}

	void AnimGoalInterrupt() {
		animationGoals.Interrupt(get_required_handle(), AGP_HIGHEST, false);
	}

	int AnimGoalPushAttack(objHndlWrapper tgt, int anim_idx = 0, int is_crit = 0, int is_secondary = 0) {
		return animationGoals.PushAttackAnim(get_required_handle(), tgt, -1, anim_idx, is_crit, is_secondary);
	}

	int AnimGoalGetNewId() {
		return animationGoals.GetActionAnimId(get_required_handle());
	}

	int ApplyProjectileParticles(objHndlWrapper projectile, D20CAF flags = D20CAF_NONE) {
		return temple::GetRef<BOOL(__cdecl)(objHndl, objHndl, D20CAF)>(0x1004F330)(get_required_handle(), projectile, flags);
	}

	int ApplyProjectileHitParticles(objHndlWrapper projectile, D20CAF flags = D20CAF_NONE) {
		return temple::GetRef<BOOL(__cdecl)(objHndl, objHndl, D20CAF)>(0x1004F420)(get_required_handle(), projectile, flags);
	}

	void D20StatusInit() {
		d20Sys.d20Status->D20StatusInit(get_required_handle());
	}

	/*
	Sets one of the critters stand points to a jump point.
	*/
	void StandpointSet(StandPointType type, int jump_point_id) {
		JumpPoint jumpPoint;
		if (!maps.GetJumpPoint(jump_point_id, jumpPoint)) {
			throw TempleException("Unknown jump point id {} used to set a standpoint.", jump_point_id);
		}

		StandPoint standPoint;
		standPoint.location.location = jumpPoint.location;
		standPoint.location.off_x = 0;
		standPoint.location.off_y = 0;
		standPoint.mapId = jumpPoint.mapId;
		standPoint.jumpPointId = jump_point_id;
		critterSys.SetStandPoint(get_required_handle(), type, standPoint);
	}

	void RunOff(uint64_t location, float off_x = 0, float off_y = 0) {
		auto handle = get_required_handle();

		LocAndOffsets loc;
		loc.location = locXY::fromField(location);
		loc.off_x = off_x;
		loc.off_y = off_y;

		objects.SetFlag(handle, OF_CLICK_THROUGH);
		aiSys.SetAiFlag(handle, AiFlag::RunningOff);
		objects.FadeTo(handle, 0, 25, 5, 2);
		animationGoals.PushRunNearTile(handle, loc, 5);
	}

	void RunTo(uint64_t location, float off_x = 0, float off_y = 0) {
		LocAndOffsets loc;
		loc.location = locXY::fromField(location);
		loc.off_x = off_x;
		loc.off_y = off_y;

		animationGoals.PushRunNearTile(get_required_handle(), loc, 5);
	}

	int GetCategoryType() {
		return critterSys.GetCategory(get_required_handle());
	}

	int IsActiveCombatant() {
		if (combatSys.isCombatActive() && tbSys.IsInInitiativeList(get_required_handle())) {
			return 1;
		}

		return 0;
	}


	int IsCategoryType(MonsterCategory type) {
		return critterSys.IsCategoryType(get_required_handle(), type);
	}

	int IsCategorySubtype(MonsterSubcategoryFlag type) {
		return critterSys.IsCategorySubtype(get_required_handle(), type);
	}

	void RumorLogAdd(int rumor_id) {
		party.RumorLogAdd(get_required_handle(), rumor_id);
	}

	int ObjectEventAppend(ObjectListFilter filter, float radius_feet) {
		return objEvents.EventAppend(get_required_handle(), 0, 1, filter, radius_feet * (float)12.0, 0.0, XM_2PI);
	}

	int WallEventAppend(ObjectListFilter filter, float radius_feet, float wall_angle) {
		return objEvents.EventAppend(get_required_handle(), OBJ_EVENT_WALL_ENTERED_HANDLER_ID, OBJ_EVENT_WALL_EXITED_HANDLER_ID, filter, radius_feet * (float)12.0, wall_angle, XM_2PI);
	}

	void SetInt(obj_f field, int value) {
		auto handle = get_required_handle();

		if (field == obj_f_critter_subdual_damage) {
			critterSys.SetSubdualDamage(handle, value);
		}
		else {

			if (objectFields.GetType(field) == ObjectFieldType::Int32)
			{
				objects.setInt32(handle, field, value);
			}
			else if (objectFields.GetType(field) == ObjectFieldType::Float32)
			{
				objSystem->GetObject(handle)->SetFloat(field, (float)value);
			}
			else
			{
				logger->warn("Wrong field type for set_int, {}", (int)(field));
			}
		}
	}

	void SetFloat(obj_f field, float value) {
		get_game_object().SetFloat(field, value);
	}


	void SetInt64(obj_f field, int64_t value) {
		auto objBody = get_game_object();
		if (objectFields.GetType(field) == ObjectFieldType::Int64)
		{
			objBody.SetInt64(field, value);
		}
		else
		{
			logger->warn("Wrong field type for set_int, {}", (int)(field));
		}
	}


	void SetIdxInt(obj_f field, int idx, int value) {
		get_game_object().SetInt32(field, idx, value);
	}

	void SetIdxInt64(obj_f field, int idx, int64_t value) {
		get_game_object().SetInt64(field, idx, value);
	}

	void SetObj(obj_f field, objHndlWrapper value) {
		objects.SetFieldObjHnd(get_required_handle(), field, value);
	}

	int GetInt(obj_f field) {
		auto game_object = get_game_object();
		if (objectFields.GetType(field) == ObjectFieldType::Int32)
		{
			return game_object.GetInt32(field);
		}
		else if (objectFields.GetType(field) == ObjectFieldType::Float32)
		{
			return (int)game_object.GetFloat(field);
		}
		else
		{
			throw TempleException("Trying to read an int from field {}, which is not an integer field.", field);
		}
	}

	int GetIdxInt(obj_f field, int index = 0) {
		assert(index >= 0);
		return objects.getArrayFieldInt32(get_required_handle(), field, index);
	}

	int64_t GetInt64(obj_f field) {
		return objects.getInt64(get_required_handle(), field);
	}

	objHndlWrapper GetObj(obj_f field) {
		return objects.getObjHnd(get_required_handle(), field);
	}

	SpellStoreData GetSpell(obj_f field, int index = 0) {
		assert(index >= 0);
		return get_game_object().GetSpell(field, index);
	}

	int HasFeat(const char *feat_name) {
		return HasFeat((feat_enums)ElfHash::Hash(feat_name));
	}

	int HasFeat(feat_enums feat_id) {
		return _HasFeatCountByClass(get_required_handle(), feat_id, (Stat)0, 0);
	}

	void FeatAdd(const char *feat_name, int do_refresh = 0) {
		FeatAdd((feat_enums)ElfHash::Hash(feat_name), do_refresh);
	}

	void FeatAdd(feat_enums feat_id, int do_refresh = 0) {

		if (feat_id == 0) {
			return;
		}

		auto handle = get_required_handle();
		objects.feats.FeatAdd(handle, feat_id);
		if (do_refresh) {
			d20Sys.d20Status->D20StatusRefresh(handle);
		}
	}

	void SpellKnownAdd(int spell_idx, int spell_class_code, int slot_level) {
		spellSys.SpellKnownAdd(get_required_handle(), spell_idx, spell_class_code & 0x7F | 0x80, slot_level, 1, 0);
	}

	void SpellMemorizedAdd(int spell_idx, int spell_class_code, int slot_level) {
		spellSys.SpellKnownAdd(get_required_handle(), spell_idx, spell_class_code & 0x7F | 0x80, slot_level, 2, 0);
	}

	void SpellHeal(objHndlWrapper healer, Dice dice, D20ActionType action_type = D20A_NONE, int spell_id = 0) {
		damage.HealSpell(get_required_handle(), healer, dice, action_type, spell_id);
	}

	void IdentifyAll() {
		inventory.IdentifyAll(get_required_handle());
	}

	void AiFleeAdd(objHndlWrapper target) {
		aiSys.FleeAdd(get_required_handle(), target);
	}

	void AiShitlistAdd(objHndlWrapper target) {
		aiSys.ShitlistAdd(get_required_handle(), target);
	}

	void AiShitlistRemove(objHndlWrapper target) {
		aiSys.ShitlistRemove(get_required_handle(), target);
	}

	void AiStopAttacking() {
		aiSys.StopAttacking(get_required_handle());
	}

	int AllegianceShared(objHndlWrapper target) {
		return critterSys.NpcAllegianceShared(get_required_handle(), target);
	}

	int GetDeity() {
		return objects.GetDeity(get_required_handle());
	}

	int GetWieldType(objHndlWrapper weapon = objHndlWrapper(), bool regard_enlargement = false) {
		auto result = 0; // default - light weapon
		auto handle = get_required_handle();

		objHndl weapon_handle = weapon;

		if (weapon_handle == objHndl::null) {
			weapon_handle = inventory.ItemWornAt(handle, EquipSlot::WeaponPrimary);
			if (!weapon_handle)
				weapon_handle = inventory.ItemWornAt(handle, EquipSlot::WeaponSecondary);
			if (!weapon_handle) { // no weapon at all!
				return result;
			}
		}

		return inventory.GetWieldType(handle, weapon_handle, regard_enlargement);
	}

	int GetWeaponProjectileProto() {
		return temple::GetRef<int(__cdecl)(objHndl)>(0x10065760)(get_required_handle());
	}

	void Unwield(EquipSlot equip_slot, bool drop = false) {
		if (drop) {
			inventory.ItemDrop(get_required_handle(), equip_slot);
		}
		else {
			inventory.ItemUnwield(get_required_handle(), equip_slot);
		}
	}

	void Wield(objHndlWrapper item, EquipSlot equip_slot) {
		auto handle = get_required_handle();

		if (inventory.GetParent(item) != handle) {
			if (inventory.IsVisibleInventoryFull(handle)) {
				inventory.SetItemParent(item, handle, ItemInsertFlags::IIF_Use_Max_Idx_200);
				inventory.SetItemParent(item, handle, 8);
			}
			else {
				inventory.SetItemParent(item, handle, 0);
			}
		}

		inventory.Wield(handle, item, equip_slot);
	}

	void WieldBestAll(objHndlWrapper target) {
		inventory.WieldBestAll(get_required_handle(), target);
	}

	void AwardExperience(int xp_awarded) {
		critterSys.AwardXp(get_required_handle(), xp_awarded);
		uiSystems->GetParty().Update();
	}

	void HasAtoned() {
		d20Sys.d20SendSignal(get_required_handle(), DK_SIG_Atone_Fallen_Paladin, 0, 0);
	}


	void D20SendSignal(const char *signal_name, py::object arg = py::none()) {
		auto key = (D20DispatcherKey)ElfHash::Hash(signal_name);
		D20SendSignal(true, key, arg);
	}

	void D20SendSignal(int signal_id, py::object arg = py::none()) {
		auto key = (D20DispatcherKey)(DK_SIG_HP_Changed + signal_id);
		D20SendSignal(false, key, arg);
	}

	void D20SendSignal(bool python_sig, D20DispatcherKey dispatcher_key, py::object arg) {

		if (py::isinstance<objHndlWrapper>(arg)) {
			objHndl hndl = arg.cast<objHndlWrapper>();
			if (python_sig)
				logger->error("Unimplemented D20SignalPython with handle arg");
			else
				d20Sys.d20SendSignal(get_required_handle(), dispatcher_key, hndl);
		}

		else if (!arg.is_none())
		{
			auto val = arg.cast<int>();
			if (python_sig)
				d20Sys.D20SignalPython(get_required_handle(), dispatcher_key, val, 0);
			else
				d20Sys.d20SendSignal(get_required_handle(), dispatcher_key, val, 0);
		}
		else {
			if (python_sig)
				d20Sys.D20SignalPython(get_required_handle(), dispatcher_key);
			else
				d20Sys.d20SendSignal(get_required_handle(), dispatcher_key, 0, 0);
		}

	}

	void D20SendSignalEx(int signal_id, objHndlWrapper arg = objHndlWrapper()) {

		D20DispatcherKey dispKey = (D20DispatcherKey)(DK_SIG_HP_Changed + signal_id);
		D20Actn d20a(D20A_CAST_SPELL);
		d20a.d20APerformer = get_required_handle();
		d20a.d20ATarget = arg;

		d20a.d20Caf = D20CAF_HIT;
		if (dispKey == DK_SIG_TouchAttack) {
			d20a.d20Caf = PerformTouchAttack(d20a.d20ATarget);
		}

		d20Sys.d20SendSignal(get_required_handle(), dispKey, &d20a, 0);
	}

	void BalorDeath() {
		critterSys.BalorDeath(get_required_handle());
	}

	void ConcealedSet(int concealed) {
		critterSys.SetConcealed(get_required_handle(), concealed);
	}

	int Unconceal() {
		return animationGoals.PushUnconceal(get_required_handle());
	}

	void PendingToMemorized() {
		spellSys.spellsPendingToMemorized(get_required_handle());
	}

	void PendingToMemorized(Stat class_enum) {
		spellSys.SpellsPendingToMemorizedByClass(get_required_handle(), class_enum);
	}

	void SpellsCastReset(Stat class_enum = (Stat)-1) {
		spellSys.SpellsCastReset(get_required_handle(), class_enum);
	}

	void MemorizedForget() {
		spellSys.ForgetMemorized(get_required_handle());
	}

	int Resurrect(ResurrectType type, int unk = 0) {
		return critterSys.Resurrect(get_required_handle(), type, unk);
	}

	int Dominate(objHndlWrapper caster) {
		return critterSys.Dominate(get_required_handle(), caster);
	}

	int IsUnconscious() {
		return critterSys.IsDeadOrUnconscious(get_required_handle());
	}

	void MakeWizard(int level) {
		if (level <= 0 || (uint32_t)level > config.maxLevel) {
			throw TempleException("Level must be between 1 and {}", config.maxLevel);
		}

		auto gameObj = get_game_object();

		for (int i = 0; i < level; i++) {
			gameObj.SetInt32(obj_f_critter_level_idx, i, stat_level_wizard);
		}

		d20Sys.d20Status->D20StatusRefresh(get_required_handle());
	}

	void MakeClass(Stat stat_class, int level) {

		if (level <= 0 || (uint32_t)level > config.maxLevel) {
			throw TempleException("Level must be between 1 and {}", config.maxLevel);
		}

		auto gameObj = get_game_object();
		for (int i = 0; i < level; i++) {
			gameObj.SetInt32(obj_f_critter_level_idx, i, stat_class);
		}

		d20Sys.d20Status->D20StatusRefresh(get_required_handle());
	}

private:

	py::tuple get_spell_list_as_tuple(obj_f field) const {
		auto gameobj = get_game_object();
		auto spells_known = gameobj.GetSpellArray(field);

		auto spell_count = spells_known.GetSize();
		py::tuple result(spell_count);
		for (size_t i = 0; i < spell_count; ++i) {
			result[i] = spells_known[i];
		}
		return result;
	}

	/**
	* returns the object handle for the object represented by this object and automatically
	* tries to restore the handle from a persistent id in case this object was loaded from a
	* save game recently.
	*/
	objHndl resolve_handle() const {
		if (!handle_ && id_) {
			handle_ = gameSystems->GetObj().GetHandleById(id_);
		}

		if (!gameSystems->GetObj().IsValidHandle(handle_)) {
			handle_ = gameSystems->GetObj().GetHandleById(id_);
		}

		return handle_;
	}

	objHndl get_required_handle() const {
		auto handle = resolve_handle();
		if (!handle) {
			throw TempleException("Cannot access the state of a null object handle.");
		}
		return handle;
	}

	GameObjectBody *try_get_game_object() const {
		auto handle = resolve_handle();
		if (!handle) {
			return nullptr;
		}
		return gameSystems->GetObj().GetObject(handle);
	}
	GameObjectBody &get_game_object() const {
		auto handle = resolve_handle();
		if (!handle) {
			throw TempleException("Tried to resolve a null object handle.");
		}
		auto obj = gameSystems->GetObj().GetObject(handle);
		if (!obj) {
			throw TempleException("Could't obtain game object for object handle {}", handle.handle);
		}
		return *obj;
	}

	ObjectId id_;
	mutable objHndl handle_ = objHndl::null;
};

BOOL ConvertObjHndl(PyObject* obj, objHndl* pHandleOut) {
	if (obj == Py_None) {
		*pHandleOut = 0;
		return TRUE;
	}

	*pHandleOut = py::cast<objHndlWrapper>(py::reinterpret_borrow<py::object>(obj));
	return TRUE;
}

PyObject* PyObjHndl_Create(objHndl handle) {
	auto obj = py::cast(objHndlWrapper(handle));
	obj.inc_ref();
	return obj.ptr();
}

PyObject* PyObjHndl_CreateNull() {
	auto obj = py::cast(objHndlWrapper());
	obj.inc_ref();
	return obj.ptr();
}

objHndl PyObjHndl_AsObjHndl(PyObject* obj) {
	assert(PyObjHndl_Check(obj));
	return py::cast<objHndlWrapper>(py::reinterpret_borrow<py::object>(obj));
}

bool PyObjHndl_Check(PyObject* obj) {
	try {
		py::cast<objHndlWrapper>(py::reinterpret_borrow<py::object>(obj));
		return true;
	}
	catch (py::cast_error) {
		return false;
	}
}

void init_objhndl_class(pybind11::module &m)
{
	auto obj_class = py::class_<objHndlWrapper>(m, "PyObjHandle", R"(Represents a handle to any object in the game world (scenery, monsters, players, etc.).)")
		.def(py::init<uint64_t>(), R"(Creates an object handle object from a numeric handle value)", "handle_value"_a = 0)
		.def("__repr__", &objHndlWrapper::to_string)
		.def("__nonzero__", [](objHndlWrapper &wrapper) -> bool {
		return static_cast<bool>(static_cast<objHndl>(wrapper));
	})
		.def("__cmp__", &objHndlWrapper::compare_to)
		.def("__reduce__", &objHndlWrapper::reduce)
		.def("setstate", &objHndlWrapper::set_state) // Workaround

		.def("add_to_initiative", &objHndlWrapper::AddToInitiative)
		.def("ai_flee_add", &objHndlWrapper::AiFleeAdd)
		.def("ai_follower_add", &objHndlWrapper::AiFollowerAdd)
		.def("ai_follower_remove", &objHndlWrapper::ReturnZero)
		.def("ai_follower_atmax", &objHndlWrapper::ReturnZero)
		.def("ai_shitlist_add", &objHndlWrapper::AiShitlistAdd)
		.def("ai_shitlist_remove", &objHndlWrapper::AiShitlistRemove)
		.def("ai_stop_attacking", &objHndlWrapper::AiStopAttacking)
		.def("allegiance_shared", &objHndlWrapper::AllegianceShared)
		.def("anim_callback", &objHndlWrapper::AnimCallback)
		.def("anim_goal_interrupt", &objHndlWrapper::AnimGoalInterrupt)
		.def("anim_goal_push_attack", &objHndlWrapper::AnimGoalPushAttack)
		.def("anim_goal_get_new_id", &objHndlWrapper::AnimGoalGetNewId)
		.def("apply_projectile_particles", &objHndlWrapper::ApplyProjectileParticles)
		.def("apply_projectile_hit_particles", &objHndlWrapper::ApplyProjectileHitParticles)
		.def("arcane_spell_level_can_cast", &objHndlWrapper::ArcaneSpellLevelCanCast)
		.def("attack", &objHndlWrapper::Attack)
		.def("award_experience", &objHndlWrapper::AwardExperience)


		.def("balor_death", &objHndlWrapper::BalorDeath)
		.def("begin_dialog", &objHndlWrapper::BeginDialog)
		.def("begian_dialog", &objHndlWrapper::BeginDialog, "I make this typo so much that I want it supported :P")
		.def("barter", &objHndlWrapper::Barter)

		.def("cast_spell", &objHndlWrapper::CastSpell)
		.def("can_cast_spell", &objHndlWrapper::CanCastSpell)
		.def("can_find_path_to_obj", &objHndlWrapper::CanFindPathToObj)
		.def("can_see", &objHndlWrapper::HasLos)
		.def("can_sneak_attack", &objHndlWrapper::CanSneakAttack)
		.def("concealed_set", &objHndlWrapper::ConcealedSet)
		.def("condition_add_with_args", &objHndlWrapper::ConditionAddWithArgs)
		.def("condition_add", &objHndlWrapper::ConditionAddWithArgs)
		.def("container_flags_get", &objHndlWrapper::GetFlags<obj_f_container_flags>)
		.def("container_flag_set", &objHndlWrapper::SetFlag<obj_f_container_flags>)
		.def("container_flag_unset", &objHndlWrapper::ClearFlag<obj_f_container_flags>)
		.def("container_toggle_open", &objHndlWrapper::ContainerToggleOpen)
		.def("critter_flags_get", &objHndlWrapper::GetFlags<obj_f_critter_flags>)
		.def("critter_flag_set", &objHndlWrapper::SetFlag<obj_f_critter_flags>)
		.def("critter_flag_unset", &objHndlWrapper::ClearFlag<obj_f_critter_flags>)
		.def("critter_get_alignment", &objHndlWrapper::CritterGetAlignment)
		.def("critter_kill", &objHndlWrapper::Kill)
		.def("critter_kill_by_effect", &objHndlWrapper::KillByEffect)

		.def("d20_query", py::overload_cast<const char *>(&objHndlWrapper::D20Query))
		.def("d20_query", py::overload_cast<int>(&objHndlWrapper::D20Query))

		.def("d20_query_has_spell_condition", &objHndlWrapper::D20QueryHasSpellCond)
		.def("d20_query_with_data", py::overload_cast<const char *, int, int>(&objHndlWrapper::D20QueryWithData))
		.def("d20_query_with_data", py::overload_cast<int, int, int>(&objHndlWrapper::D20QueryWithData))
		.def("d20_query_test_data", &objHndlWrapper::D20QueryTestData)
		.def("d20_query_get_data", &objHndlWrapper::D20QueryGetData)
		.def("d20_send_signal", py::overload_cast<const char*, py::object>(&objHndlWrapper::D20SendSignal))
		.def("d20_send_signal", py::overload_cast<int, py::object>(&objHndlWrapper::D20SendSignal))
		.def("d20_send_signal_ex", &objHndlWrapper::D20SendSignalEx)
		.def("d20_status_init", &objHndlWrapper::D20StatusInit)
		.def("damage", &objHndlWrapper::Damage)
		.def("damage_with_reduction", &objHndlWrapper::DamageWithReduction)
		.def("deal_attack_damage", &objHndlWrapper::DealAttackDamage)
		.def("destroy", &objHndlWrapper::Destroy)
		.def("distance_to", py::overload_cast<objHndlWrapper>(&objHndlWrapper::DistanceTo))
		.def("distance_to", py::overload_cast<uint64_t, float, float>(&objHndlWrapper::DistanceTo))
		.def("divine_spell_level_can_cast", &objHndlWrapper::DivineSpellLevelCanCast)
		.def("dominate", &objHndlWrapper::Dominate)

		.def("faction_has", &objHndlWrapper::FactionHas, "Check if NPC has faction. Doesn't work on PCs!")
		.def("faction_add", &objHndlWrapper::FactionAdd, "Adds faction to NPC. Doesn't work on PCs!")
		.def("fade_to", &objHndlWrapper::FadeTo)
		.def("feat_add", py::overload_cast<const char *, int>(&objHndlWrapper::FeatAdd), "Gives you a feat by name")
		.def("feat_add", py::overload_cast<feat_enums, int>(&objHndlWrapper::FeatAdd), "Gives you a feat by using it's id")
		.def("fall_down", &objHndlWrapper::FallDown, "Makes a Critter fall down")
		.def("follower_add", &objHndlWrapper::FollowerAdd)
		.def("follower_remove", &objHndlWrapper::FollowerRemove)
		.def("follower_atmax", &objHndlWrapper::FollowerAtMax)
		.def("footstep", &objHndlWrapper::Footstep)
		.def("float_line", &objHndlWrapper::FloatLine)
		.def("float_mesfile_line", &objHndlWrapper::FloatMesFileLine)
		.def("float_text_line", &objHndlWrapper::FloatTextLine)

		.def("get_base_attack_bonus", &objHndlWrapper::GetBaseAttackBonus)
		.def("get_category_type", &objHndlWrapper::GetCategoryType)
		.def("get_initiative", &objHndlWrapper::GetInitiative)
		.def("get_deity", &objHndlWrapper::GetDeity)
		.def("get_wield_type", &objHndlWrapper::GetWieldType)
		.def("get_weapon_projectile_proto", &objHndlWrapper::GetWeaponProjectileProto)
		.def("group_list", &objHndlWrapper::GroupList)

		.def("has_atoned", &objHndlWrapper::HasAtoned)
		.def("has_feat", py::overload_cast<const char*>(&objHndlWrapper::HasFeat))
		.def("has_feat", py::overload_cast<feat_enums>(&objHndlWrapper::HasFeat))
		.def("has_follower", &objHndlWrapper::HasFollower)
		.def("has_item", &objHndlWrapper::HasItem)
		.def("has_los", &objHndlWrapper::HasLos)
		.def("has_met", &objHndlWrapper::HasMet)
		.def("has_spell_effects", &objHndlWrapper::HasSpellEffects)
		.def("has_wielded", &objHndlWrapper::HasWielded)
		.def("hasMet", &objHndlWrapper::HasMet)
		.def("heal", &objHndlWrapper::Heal)
		.def("healsubdual", &objHndlWrapper::HealSubdual)


		.def("identify_all", &objHndlWrapper::IdentifyAll)
		.def("inventory_item", &objHndlWrapper::InventoryItem)
		.def("is_active_combatant", &objHndlWrapper::IsActiveCombatant)
		.def("is_category_type", &objHndlWrapper::IsCategoryType)
		.def("is_category_subtype", &objHndlWrapper::IsCategorySubtype)
		.def("is_flanked_by", &objHndlWrapper::IsFlankedBy)
		.def("is_friendly", &objHndlWrapper::IsFriendly)
		.def("is_unconscious", &objHndlWrapper::IsUnconscious)
		.def("item_condition_add_with_args", &objHndlWrapper::ItemConditionAdd)
		.def("item_find", &objHndlWrapper::ItemFind)
		.def("item_get", &objHndlWrapper::ItemGet)
		.def("item_transfer_to", &objHndlWrapper::ItemTransferTo)
		.def("item_find_by_proto", &objHndlWrapper::ItemFindByProto)
		.def("item_transfer_to_by_proto", &objHndlWrapper::ItemTransferToByProto)
		.def("item_worn_at", &objHndlWrapper::ItemWornAt)
		.def("item_flags_get", &objHndlWrapper::GetFlags<obj_f_item_flags>)
		.def("item_flag_set", &objHndlWrapper::SetFlag<obj_f_item_flags>)
		.def("item_flag_unset", &objHndlWrapper::ClearFlag<obj_f_item_flags>)
		.def("item_worn_unwield", &objHndlWrapper::Unwield)
		.def("item_wield", &objHndlWrapper::Wield)
		.def("item_wield_best_all", &objHndlWrapper::WieldBestAll)


		.def("leader_get", &objHndlWrapper::LeaderGet)

		.def("make_wiz", &objHndlWrapper::MakeWizard, "Makes you a wizard of level N")
		.def("make_class", &objHndlWrapper::MakeClass, "Makes you a CLASS N of level M")
		.def("money_get", &objHndlWrapper::MoneyGet)
		.def("money_adj", &objHndlWrapper::MoneyAdj)
		.def("move", py::overload_cast<uint64_t, float, float>(&objHndlWrapper::Move))
		.def("move", py::overload_cast<int, int, float, float>(&objHndlWrapper::Move))

		.def("npc_flags_get", &objHndlWrapper::GetFlags<obj_f_npc_flags>)
		.def("npc_flag_set", &objHndlWrapper::SetFlag<obj_f_npc_flags>)
		.def("npc_flag_unset", &objHndlWrapper::ClearFlag<obj_f_npc_flags>)

		.def("object_event_append", &objHndlWrapper::ObjectEventAppend)
		.def("object_event_append_wall", &objHndlWrapper::WallEventAppend)
		.def("obj_get_int", &objHndlWrapper::GetInt)
		.def("obj_get_idx_int", &objHndlWrapper::GetIdxInt)
		.def("obj_get_int64", &objHndlWrapper::GetInt64, "Gets 64 bit field")
		.def("obj_get_obj", &objHndlWrapper::GetObj, "Gets Object field")
		.def("obj_get_spell", &objHndlWrapper::GetSpell)
		.def("obj_remove_from_all_groups", &objHndlWrapper::RemoveFromAllGroups, "Removes the object from all the groups (GroupList, PCs, NPCs, AI controlled followers, Currently Selected")
		.def("obj_set_int", &objHndlWrapper::SetInt)
		.def("obj_set_float", &objHndlWrapper::SetFloat)
		.def("obj_set_obj", &objHndlWrapper::SetObj)
		.def("obj_set_idx_int", &objHndlWrapper::SetIdxInt)
		.def("obj_set_int64", &objHndlWrapper::SetInt64)
		.def("obj_set_idx_int64", &objHndlWrapper::SetIdxInt64)
		.def("object_flags_get", &objHndlWrapper::GetFlags<obj_f_flags>)
		.def("object_flag_set", &objHndlWrapper::ObjectFlagSet)
		.def("object_flag_unset", &objHndlWrapper::ObjectFlagUnset)
		.def("object_script_execute", &objHndlWrapper::ObjectScriptExecute)

		.def("pc_add", &objHndlWrapper::PCAdd, "Adds object as a PC party member.")
		.def("perform_touch_attack", &objHndlWrapper::PerformTouchAttack)
		.def("portal_flags_get", &objHndlWrapper::GetFlags<obj_f_portal_flags>)
		.def("portal_flag_set", &objHndlWrapper::SetFlag<obj_f_portal_flags>)
		.def("portal_flag_unset", &objHndlWrapper::ClearFlag<obj_f_portal_flags>)
		.def("portal_toggle_open", &objHndlWrapper::PortalToggleOpen)


		.def("reaction_get", &objHndlWrapper::ReactionGet)
		.def("reaction_set", &objHndlWrapper::ReactionSet)
		.def("reaction_adj", &objHndlWrapper::ReactionAdjust)
		.def("reflex_save_and_damage", &objHndlWrapper::ReflexSaveAndDamage)
		.def("refresh_turn", &objHndlWrapper::RefreshTurn)
		.def("reputation_has", &objHndlWrapper::ReputationHas)
		.def("reputation_add", &objHndlWrapper::ReputationAdd)
		.def("remove_from_initiative", &objHndlWrapper::RemoveFromInitiative)
		.def("reputation_remove", &objHndlWrapper::ReputationRemove)
		.def("resurrect", &objHndlWrapper::Resurrect)
		.def("rumor_log_add", &objHndlWrapper::RumorLogAdd)
		.def("runoff", &objHndlWrapper::RunOff)
		.def("run_to", &objHndlWrapper::RunTo)

		.def("saving_throw", &objHndlWrapper::SavingThrow)
		.def("saving_throw_with_args", &objHndlWrapper::SavingThrow)
		.def("saving_throw_spell", &objHndlWrapper::SavingThrowSpell)
		.def("secretdoor_detect", &objHndlWrapper::SecretdoorDetect)
		.def("set_initiative", &objHndlWrapper::SetInitiative)
		.def("skill_level_get", py::overload_cast<objHndlWrapper, SkillEnum>(&objHndlWrapper::SkillLevelGet))
		.def("skill_level_get", py::overload_cast<SkillEnum>(&objHndlWrapper::SkillLevelGet))
		.def("skill_ranks_get", &objHndlWrapper::SkillRanksGet)
		.def("skill_roll", &objHndlWrapper::SkillRoll)
		.def("soundmap_critter", &objHndlWrapper::SoundmapCritter)
		.def("spell_known_add", &objHndlWrapper::SpellKnownAdd)
		.def("spell_memorized_add", &objHndlWrapper::SpellMemorizedAdd)
		.def("spell_damage", &objHndlWrapper::SpellDamage)
		.def("spell_damage_with_reduction", &objHndlWrapper::SpellDamageWithReduction)
		.def("spell_damage_weaponlike", &objHndlWrapper::SpellDamageWeaponlike)
		.def("spell_heal", &objHndlWrapper::SpellHeal)
		.def("spells_pending_to_memorized", py::overload_cast<>(&objHndlWrapper::PendingToMemorized))
		.def("spells_pending_to_memorized", py::overload_cast<Stat>(&objHndlWrapper::PendingToMemorized))
		.def("spells_cast_reset", &objHndlWrapper::SpellsCastReset)
		.def("spells_memorized_forget", &objHndlWrapper::MemorizedForget)
		.def("spontaneous_spell_level_can_cast", &objHndlWrapper::SpontaneousSpellLevelCanCast)
		.def("standpoint_set", &objHndlWrapper::StandpointSet)
		.def("stat_level_get", py::overload_cast<Stat>(&objHndlWrapper::StatLevelGet))
		.def("stat_level_get", py::overload_cast<Stat, int>(&objHndlWrapper::StatLevelGet))
		.def("stat_base_get", &objHndlWrapper::StatLevelGetBase)
		.def("stat_base_set", &objHndlWrapper::StatLevelSetBase)
		.def("steal_from", &objHndlWrapper::StealFrom)

		.def("turn_towards", &objHndlWrapper::TurnTowards)
		.def("trip_check", &objHndlWrapper::TripCheck)

		.def("unconceal", &objHndlWrapper::Unconceal)


		.def_property_readonly("area", [](objHndlWrapper&) {
		return maps.GetCurrentAreaId();
	}, R"(The current area id)")
		.def_property_readonly("map", [](objHndlWrapper&) {
		return maps.GetCurrentMapId();
	}, R"(The current map id)")
		.def_property_readonly("char_classes", &objHndlWrapper::get_char_classes, R"(a tuple containing the character classes array)")
		.def_property_readonly("description", &objHndlWrapper::get_description, R"(Returns the object name from description.mes. Safe to use on null handles.)")
		.def_property_readonly("name", &objHndlWrapper::get_name, R"(Returns the numeric name id for this object.)")
		.def_property_readonly("proto", &objHndlWrapper::get_proto_id, R"(Returns the prototype id for this object.)")
		.def_property_readonly("type", &objHndlWrapper::get_type, R"(Returns the type of this object.)")
		.def_property_readonly("highest_arcane_class", &objHndlWrapper::get_highest_arcane_class, R"(Highest Arcane spell casting class)")
		.def_property_readonly("highest_divine_class", &objHndlWrapper::get_highest_divine_class, R"(Highest Divine spell casting class)")
		.def_property_readonly("location", &objHndlWrapper::get_location, R"(The location of this object in the game world)")
		.def_property("radius", &objHndlWrapper::get_radius, &objHndlWrapper::set_radius, R"(The ingame radius of this object in inches. Usually it is set automatically from the 3D model.)")
		.def_property("height", &objHndlWrapper::get_render_height, &objHndlWrapper::set_render_height, R"(The height of the object in inches. Usually set automatically from the 3D model.)")
		.def_property("rotation", &objHndlWrapper::get_rotation, &objHndlWrapper::set_rotation, R"(The current rotation of this object expressed in radians.)")
		.def_property_readonly("hit_dice", &objHndlWrapper::get_hit_dice, R"(The full hit dice of this object)")
		.def_property_readonly("hit_dice_num", &objHndlWrapper::get_hit_dice_num, R"(The number of dice in the hit dice of this object.)")
		.def_property_readonly("get_size", &objHndlWrapper::get_size, R"(The size category f this object.)")
		.def_property_readonly("off_x", &objHndlWrapper::get_offset_x, R"(The X-axis offset of this object on the tile it's on. An offset of 0 is the center of the tile. The offset is expressed in inches.)")
		.def_property_readonly("off_y", &objHndlWrapper::get_offset_y, R"(The Y-axis offset of this object on the tile it's on. An offset of 0 is the center of the tile. The offset is expressed in inches.)")
		.def_property_readonly("scripts", &objHndlWrapper::get_scripts, R"(The scripts that are associated with this object.)")
		.def_property("origin", &objHndlWrapper::get_origin_map_id, &objHndlWrapper::set_origin_map_id, R"(The map id this object was originally on.)")
		.def_property("substitute_inventory", &objHndlWrapper::get_substitute_inventory, &objHndlWrapper::set_substitute_inventory, R"(The substite inventory for vendors.)")
		.def_property_readonly("factions", &objHndlWrapper::get_factions, R"(The factions that this object belongs to.)")
		.def_property_readonly("feats", &objHndlWrapper::get_feats, R"(The list of feats this object has access to.)")
		.def_property_readonly("spells_known", &objHndlWrapper::get_spells_known, R"(The spells known by this critter.)")
		.def_property_readonly("spells_memorized", &objHndlWrapper::get_spells_memorized, R"(The spells memorized by this critter.)")
		.def_property("loots", &objHndlWrapper::get_loots, &objHndlWrapper::set_loots, R"(The loot table id for this critter.)")		
		;

	// Workaround for pybind11 Python 2.7 issue
	obj_class.attr("__setstate__") = obj_class.attr("setstate");

	m.attr("OBJ_HANDLE_NULL") = objHndlWrapper(objHndl::null);

}

namespace pybind11 {
	namespace detail {
		template <> struct type_caster<objHndl> {
		public:
			PYBIND11_TYPE_CASTER(objHndl, _("handle"));

			/**
			* Conversion part 1 (Python->C++): convert a PyObject into a inty
			* instance or return false upon failure. The second argument
			* indicates whether implicit conversions should be applied.
			*/
			bool load(handle src, bool) {
				try {
					value = src.cast<objHndlWrapper>();
					return true;
				}
				catch (py::cast_error) {
					return false;
				}

			}

			static handle cast(objHndl src, return_value_policy /* policy */, handle /* parent */) {
				return py::cast<objHndlWrapper>(objHndlWrapper(src));
			}
		};

		template <> struct type_caster<Stat> {
		public:
			PYBIND11_TYPE_CASTER(Stat, _("Stat"));

			bool load(handle src, bool) {
				try {
					auto numeric_value = src.cast<int>();
					if (numeric_value < 0 || numeric_value >= _stat_count) {
						return false;
					}
					value = (Stat)numeric_value;
					return true;
				}
				catch (py::cast_error) {
					return false;
				}

			}

			static handle cast(Stat src, return_value_policy /* policy */, handle /* parent */) {
				return py::int_((int)src);
			}
		};
	}
} // namespace pybind11::detail

