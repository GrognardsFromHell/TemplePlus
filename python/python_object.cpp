#include "stdafx.h"
#include "python_object.h"
#include "python_support.h"
#include "../maps.h"
#include "../inventory.h"
#include "../timeevents.h"
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
#include "python_dice.h"
#include "python_objectscripts.h"
#include <objlist.h>
#include "python_integration_obj.h"
#include <action_sequence.h>
#include <ui/ui_picker.h>

struct PyObjHandle {
	PyObject_HEAD;
	ObjectId id;
	objHndl handle;
};

static PyObjHandle* GetSelf(PyObject* obj) {
	assert(PyObjHndl_Check(obj));

	auto self = (PyObjHandle*) obj;

	if (!self->handle && self->id.subtype) {
		self->handle = objects.GetHandle(self->id);
	}
	if (!objects.VerifyHandle(self->handle)) {
		self->handle = objects.GetHandle(self->id);
	}

	return self;
}

static PyObject* PyObjHandle_Repr(PyObject* obj) {
	auto self = (PyObjHandle*) obj;

	if (!self->handle) {
		return PyString_FromString("OBJ_HANDLE_NULL");
	} else {
		auto name = objects.GetDisplayName(self->handle, self->handle);
		auto displayName = format("{}({})", name, self->handle);

		return PyString_FromString(displayName.c_str());
	}
}

static int PyObjHandle_Cmp(PyObject* objA, PyObject* objB) {
	objHndl handleA = 0;
	objHndl handleB = 0;
	if (objA->ob_type == &PyObjHandleType) {
		handleA = ((PyObjHandle*)objA)->handle;
	}
	if (objB->ob_type == &PyObjHandleType) {
		handleB = ((PyObjHandle*)objB)->handle;
	}

	if (handleA < handleB) {
		return -1;
	} else if (handleA > handleB) {
		return 1;
	} else {
		return 0;
	}
}

#pragma region Pickle Protocol 

// Only the object GUID is pickled
// The format has to be compatible with old ToEE to be save compatible
static PyObject* PyObjHandle_getstate(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	// Mobile GUID
	if (self->id.subtype == 2) {
		const auto& guid = self->id.guid;
		return Py_BuildValue("i(iii(iiiiiiii))",
		                     2,
		                     guid.Data1,
		                     guid.Data2,
		                     guid.Data3,
		                     guid.Data4[0],
		                     guid.Data4[1],
		                     guid.Data4[2],
		                     guid.Data4[3],
		                     guid.Data4[4],
		                     guid.Data4[5],
		                     guid.Data4[6],
		                     guid.Data4[7]);
	}
	return Py_BuildValue("(i)", self->id.subtype);
}

static PyObject* PyObjHandle_reduce(PyObject* obj, PyObject* args) {
	auto constructor = (PyObject*) &PyObjHandleType;
	auto unpickleArgs = PyTuple_New(0);
	auto pickledValue = PyObjHandle_getstate(obj, args);

	auto result = Py_BuildValue("OOO", constructor, unpickleArgs, pickledValue);
	Py_DECREF(unpickleArgs);
	Py_DECREF(pickledValue);
	return result;
}

static PyObject* PyObjHandle_setstate(PyObject* obj, PyObject* args) {
	auto self = (PyObjHandle*) obj;
	PyObject* pickledData;

	// Expected is one argument, which is the tuple we returned from __getstate__
	if (!PyArg_ParseTuple(args, "O!:PyObjHandle.__setstate__", &PyTuple_Type, &pickledData)) {
		return 0;
	}

	PyObject* guidContent = 0;
	if (!PyArg_ParseTuple(pickledData, "i|O:PyObjHandle.__setstate__", &self->id.subtype, &guidContent)) {
		self->handle = 0;
		self->id.subtype = 0;
		return 0;
	}

	if (self->id.subtype == 0) {
		// This is the null obj handle
		self->handle = 0;
		Py_RETURN_NONE;
	}

	if (guidContent == 0) {
		PyErr_SetString(PyExc_ValueError, "GUID type other than 0 is given, but GUID is missing.");
		self->handle = 0;
		self->id.subtype = 0;
		return 0;
	}

	// Try parsing the GUID tuple
	auto& guid = self->id.guid;
	if (!PyArg_ParseTuple(guidContent, "iii(iiiiiiii)", &guid.Data1,
	                      &guid.Data2,
	                      &guid.Data3,
	                      &guid.Data4[0],
	                      &guid.Data4[1],
	                      &guid.Data4[2],
	                      &guid.Data4[3],
	                      &guid.Data4[4],
	                      &guid.Data4[5],
	                      &guid.Data4[6],
	                      &guid.Data4[7])) {
		self->handle = 0;
		self->id.subtype = 0;
		return 0;
	}

	// Finally look up the handle for the GUID
	self->handle = objects.GetHandle(self->id);

	Py_RETURN_NONE;
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
static PyObject* PyObjHandle_BeginDialog(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);

	objHndl target;
	int line;
	if (!PyArg_ParseTuple(args, "O&i:begin_dialog", &ConvertObjHndl, &target, &line)) {
		return 0;
	}

	TimeEvent evt;
	evt.system = TimeEventSystem::PythonDialog;
	evt.params[0].handle = self->handle;
	evt.params[1].handle = target;
	evt.params[2].int32 = line;
	timeEvents.Schedule(evt, 1);

	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_ReactionGet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);

	objHndl towards;
	if (!PyArg_ParseTuple(args, "O&:reaction_get", &ConvertObjHndl, &towards)) {
		return 0;
	}

	auto reaction = objects.GetReaction(self->handle, towards);
	return PyInt_FromLong(reaction);
}

static PyObject* PyObjHandle_ReactionSet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);

	objHndl towards;
	int desiredReaction;
	if (!PyArg_ParseTuple(args, "O&i:reaction_set", &ConvertObjHndl, &towards, &desiredReaction)) {
		return 0;
	}

	auto currentReaction = objects.GetReaction(self->handle, towards);
	auto adjustment = desiredReaction - currentReaction;
	objects.AdjustReaction(self->handle, towards, adjustment);

	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_ReactionAdjust(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);

	objHndl towards;
	int adjustment;
	if (!PyArg_ParseTuple(args, "O&i:reaction_adjust", &ConvertObjHndl, &towards, &adjustment)) {
		return 0;
	}

	objects.AdjustReaction(self->handle, towards, adjustment);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_ItemFind(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int nameId;
	if (!PyArg_ParseTuple(args, "i:objhndl.itemfind", &nameId)) {
		return 0;
	}

	return PyObjHndl_Create(inventory.FindItemByName(self->handle, nameId));
}

static PyObject* PyObjHandle_ItemTransferTo(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int nameId;
	objHndl target;
	if (!PyArg_ParseTuple(args, "O&i:objhndl.itemtransferto", &ConvertObjHndl, &target, &nameId)) {
		return 0;
	}

	auto item = inventory.FindItemByName(self->handle, nameId);
	auto result = 0;
	if (item) {
		result = inventory.SetItemParent(item, target, 0);
	}
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_ItemFindByProto(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int protoId;
	if (!PyArg_ParseTuple(args, "i:objhndl.itemfindbyproto", &protoId)) {
		return 0;
	}
	return PyObjHndl_Create(inventory.FindItemByProtoId(self->handle, protoId));
}

static PyObject* PyObjHandle_ItemTransferToByProto(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int protoId;
	objHndl target;
	if (!PyArg_ParseTuple(args, "O&i:objhndl.itemtransfertobyproto", &ConvertObjHndl, &target, &protoId)) {
		return 0;
	}

	auto item = inventory.FindItemByProtoId(self->handle, protoId);
	auto result = 0;
	if (item) {
		result = inventory.SetItemParent(item, target, 0);
	}
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_MoneyGet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	return PyInt_FromLong(objects.StatLevelGet(self->handle, stat_money));
}

static PyObject* PyObjHandle_MoneyAdj(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int copperAdj;
	if (!PyArg_ParseTuple(args, "i:objhndl.money_adj", &copperAdj)) {
		return 0;
	}

	if (copperAdj <= 0) {
		objects.TakeMoney(self->handle, 0, 0, 0, -copperAdj);
	} else {
		objects.GiveMoney(self->handle, 0, 0, 0, copperAdj);
	}

	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_CastSpell(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	uint32_t spellEnum;
	PickerArgs pickArgs;
	SpellPacketBody spellPktBody;
	SpellEntry spellEntry;
	objHndl targetObj = 0;
	if (!PyArg_ParseTuple(args, "i|O&:objhndl.cast_spell", &spellEnum, &ConvertObjHndl, &targetObj)) {
		return 0;
	}
	objHndl caster = self->handle;

	LocAndOffsets loc;
	D20SpellData d20SpellData;

	// get spell known data
	// I've set up a really large buffer just in case, 
	// because in theory a player might have permutations 
	// of spells due to metamagic, different casting classes etc.
	uint32_t classCodes[10000] = {0,};
	uint32_t spellLevels[10000] = {0,};
	uint32_t numSpells = 0;
	if (!spellSys.spellKnownQueryGetData(caster, spellEnum, classCodes, spellLevels, &numSpells)) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	if (numSpells <= 0) {
		Py_INCREF(Py_None);
		return Py_None;
	}
	spellSys.spellPacketBodyReset(&spellPktBody);
	spellPktBody.spellEnum = spellEnum;
	spellPktBody.spellEnumOriginal = spellEnum;
	spellPktBody.objHndCaster = caster;
	for (uint32_t i = 0; i < numSpells; i++) {
		if (!spellSys.spellCanCast(caster, spellEnum, classCodes[i], spellLevels[i]))
			continue;
		spellPktBody.spellKnownSlotLevel = spellLevels[i];
		spellPktBody.casterClassCode = classCodes[i];
		spellSys.spellPacketSetCasterLevel(&spellPktBody);
		if (!spellSys.spellRegistryCopy(spellEnum, &spellEntry))
			continue;
		if (!spellSys.pickerArgsFromSpellEntry(&spellEntry, &pickArgs, caster, spellPktBody.baseCasterLevel))
			continue;
		pickArgs.result = {0,};
		pickArgs.flagsTarget = (UiPickerFlagsTarget)(
			(uint64_t)pickArgs.flagsTarget | (uint64_t)pickArgs.flagsTarget & UiPickerFlagsTarget::Unknown100h
			- (uint64_t)pickArgs.flagsTarget & UiPickerFlagsTarget::Range
		);

		if (static_cast<uint64_t>(pickArgs.modeTarget) & static_cast<uint64_t>(UiPickerType::Single)) {
			objects.loc->getLocAndOff(targetObj, &loc);
			uiPicker.sub_100BA480(targetObj, &pickArgs);
		} else if (static_cast<uint64_t>(pickArgs.modeTarget) & static_cast<uint64_t>(UiPickerType::Multi)) {
			objects.loc->getLocAndOff(targetObj, &loc);
			uiPicker.sub_100BA480(targetObj, &pickArgs);
		} else if (static_cast<uint64_t>(pickArgs.modeTarget) & static_cast<uint64_t>(UiPickerType::Cone)) {
			objects.loc->getLocAndOff(targetObj, &loc);
			uiPicker.sub_100BA6A0(&loc, &pickArgs);

		} else if (static_cast<uint64_t>(pickArgs.modeTarget) & static_cast<uint64_t>(UiPickerType::Area)) {
			if (spellEntry.spellRangeType == SRT_Personal)
				objects.loc->getLocAndOff(caster, &loc);
			else
				objects.loc->getLocAndOff(targetObj, &loc);
			uiPicker.sub_100BA540(&loc, &pickArgs);
		} else if (static_cast<uint64_t>(pickArgs.modeTarget) & static_cast<uint64_t>(UiPickerType::Personal)) {
			objects.loc->getLocAndOff(caster, &loc);
			uiPicker.sub_100BA480(caster, &pickArgs);
		}

		spellSys.ConfigSpellTargetting(&pickArgs, &spellPktBody);
		if (spellPktBody.targetListNumItems <= 0)
			continue;
		if (!actSeqSys.TurnBasedStatusInit(caster))
			continue;
		d20Sys.GlobD20ActnInit();
		d20Sys.GlobD20ActnSetTypeAndData1(D20A_CAST_SPELL, 0);
		actSeqSys.ActSeqCurSetSpellPacket(&spellPktBody, 1);
		d20Sys.D20ActnSetSpellData(&d20SpellData, spellEnum, classCodes[i], spellLevels[i], 0xFF, 0);
		d20Sys.GlobD20ActnSetSpellData(&d20SpellData);
		d20Sys.GlobD20ActnSetTarget(targetObj, &loc);
		actSeqSys.ActionAddToSeq();
		actSeqSys.sequencePerform();

		Py_INCREF(Py_None);
		return Py_None;
	}
	Py_INCREF(Py_None);
	return Py_None;

	return 0;
}

static PyObject* PyObjHandle_SkillLevelGet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);

	objHndl handle;
	SkillEnum skillId;
	if (PyTuple_Size(args) == 1) {
		if (!PyArg_ParseTuple(args, "O&i:objhndl.skill_level_get", &ConvertObjHndl, &handle, &skillId)) {
			return 0;
		}
	} else {
		if (!PyArg_ParseTuple(args, "i:objhndl.skill_level_get", &skillId)) {
			return 0;
		}
	}

	auto skillLevel = dispatch.dispatch1ESkillLevel(self->handle, skillId, nullptr, handle, 1);

	return PyInt_FromLong(skillLevel);
}

static PyObject* PyObjHandle_HasMet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl critter;
	if (!PyArg_ParseTuple(args, "O&:objhndl.has_met", &ConvertObjHndl, &critter)) {
		return 0;
	}

	auto result = critterSys.HasMet(self->handle, critter);
	return PyInt_FromLong(result);
}

/*
	Checks if the player has a follower with a given name id.
*/
static PyObject* PyObjHandle_HasFollower(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int nameId;
	if (!PyArg_ParseTuple(args, "i:objhndl.has_follower", &nameId)) {
		return 0;
	}

	ObjList followers;
	followers.ListFollowers(self->handle);

	for (int i = 0; i < followers.size(); ++i) {
		auto follower = followers[i];
		if (objects.GetNameId(follower) == nameId) {
			return PyInt_FromLong(1);
		}
	}

	return PyInt_FromLong(0);
}

static PyObject* PyObjHandle_GroupList(PyObject* obj, PyObject* args) {
	auto len = party.GroupListGetLen();
	auto result = PyTuple_New(len);

	for (uint32_t i = 0; i < len; ++i) {
		auto hndl = PyObjHndl_Create(party.GroupListGetMemberN(i));
		PyTuple_SET_ITEM(result, i, hndl);
	}

	return result;
}

static PyObject* PyObjHandle_StatLevelGet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	Stat stat;
	if (!PyArg_ParseTuple(args, "i:objhndl:stat_level_get", &stat)) {
		return 0;
	}

	return PyInt_FromLong(objects.StatLevelGet(self->handle, stat));
}

static PyObject* PyObjHandle_StatLevelGetBase(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	Stat stat;
	if (!PyArg_ParseTuple(args, "i:objhndl:stat_level_get_base", &stat)) {
		return 0;
	}

	return PyInt_FromLong(objects.StatLevelGetBase(self->handle, stat));
}

static PyObject* PyObjHandle_StatLevelSetBase(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	Stat stat;
	int value;
	if (!PyArg_ParseTuple(args, "ii:objhndl:stat_level_set_base", &stat, &value)) {
		return 0;
	}

	auto result = objects.StatLevelSetBase(self->handle, stat, value);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_FollowerAdd(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl follower;
	if (!PyArg_ParseTuple(args, "O&:objhndl.follower_add", &ConvertObjHndl, &follower)) {
		return 0;
	}

	auto result = critterSys.AddFollower(follower, self->handle, 1, false);
	ui.UpdatePartyUi();
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_FollowerRemove(PyObject* obj, PyObject* args) {
	objHndl follower;
	if (!PyArg_ParseTuple(args, "O&:objhndl.follower_remove", &ConvertObjHndl, &follower)) {
		return 0;
	}

	auto result = critterSys.RemoveFollower(follower, 1);
	ui.UpdatePartyUi();
	return PyInt_FromLong(result);
}


static PyObject* PyObjHandle_FollowerAtMax(PyObject* obj, PyObject* args) {
	auto followers = party.GroupNPCFollowersLen();
	return PyInt_FromLong(followers >= 2);
}

static PyObject* PyObjHandle_AiFollowerAdd(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl follower;
	if (!PyArg_ParseTuple(args, "O&:objhndl.ai_follower_add", &ConvertObjHndl, &follower)) {
		return 0;
	}

	auto result = critterSys.AddFollower(follower, self->handle, 1, true);
	ui.UpdatePartyUi();
	return PyInt_FromLong(result);
}

// Okay.. Apparently the AI follower methods should better not be called
static PyObject* ReturnZero(PyObject* obj, PyObject* args) {
	return PyInt_FromLong(0);
}

static PyObject* PyObjHandle_LeaderGet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	auto leader = critterSys.GetLeader(self->handle);
	return PyObjHndl_Create(leader);
}

// CanSee and HasLos are the same
static PyObject* PyObjHandle_HasLos(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl target;
	if (!PyArg_ParseTuple(args, "O&:objHndl.has_los", &ConvertObjHndl, &target)) {
		return 0;
	}
	auto obstacles = critterSys.HasLineOfSight(self->handle, target);
	return PyInt_FromLong(obstacles == 0);
}

/*
	Pretty stupid name. Checks if the critter currently wears an item with the
	given name id.
*/
static PyObject* PyObjHandle_HasWielded(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int nameId;
	if (!PyArg_ParseTuple(args, "i:objhndl.has_wielded", &nameId)) {
		return 0;
	}

	for (auto i = 0; i < (int) EquipSlot::Count; ++i) {
		auto item = critterSys.GetWornItem(self->handle, (EquipSlot) i);
		if (objects.GetNameId(item) == nameId) {
			return PyInt_FromLong(1);
		}
	}

	return PyInt_FromLong(0);
}

static PyObject* PyObjHandle_HasItem(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int nameId;
	if (!PyArg_ParseTuple(args, "i:objhndl.has_item", &nameId)) {
		return 0;
	}

	auto hasItem = (inventory.FindItemByName(self->handle, nameId) != 0);
	return PyInt_FromLong(hasItem);
}

static PyObject* PyObjHandle_ItemWornAt(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	EquipSlot slot;
	if (!PyArg_ParseTuple(args, "i:objhndl.item_worn_at", &slot)) {
		return 0;
	}

	auto item = critterSys.GetWornItem(self->handle, slot);
	return PyObjHndl_Create(item);
}

static PyObject* PyObjHandle_Attack(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl target;
	if (!PyArg_ParseTuple(args, "O&:objhndl.attack", &ConvertObjHndl, &target)) {
		return 0;
	}

	// This is pretty odd since it causes the opposite oO
	if (!party.IsInParty(target)) {
		critterSys.Attack(self->handle, target, 1, 2);
	} else {
		// This apparently causes the NPC to attack ALL of the party, not just the one PC
		for (uint32_t i = 0; i < party.GroupListGetLen(); ++i) {
			auto partyMember = party.GroupListGetMemberN(i);
			if (objects.IsPlayerControlled(partyMember)) {
				critterSys.Attack(partyMember, self->handle, 1, 2);
			}
		}
	}

	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_TurnTowards(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl target;
	if (!PyArg_ParseTuple(args, "O&:objhndl.turn_towards", &ConvertObjHndl, &target)) {
		return 0;
	}

	auto targetRot = objects.GetRotationTowards(self->handle, target);
	animationGoals.PushRotate(self->handle, targetRot);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_FloatLine(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int lineId;
	objHndl pc;
	if (!PyArg_ParseTuple(args, "iO&:objhndl.float_line", &lineId, &ConvertObjHndl, &pc)) {
		return 0;
	}

	// Try to load the dialog file attached to this object.
	auto dlgScriptId = objects.GetScript(self->handle, (int)ObjScriptEvent::Dialog);
	auto dlgFile = dialogScripts.GetFilename(dlgScriptId);
	if (dlgFile.empty()) {
		PyErr_SetString(PyExc_AttributeError, "No dialog script attached to this object.");
		return 0;
	}
	DialogState dlgState;
	if (!dialogScripts.Load(dlgFile, dlgState.dialogHandle)) {
		logger->warn("Unable to load dialog file {}", dlgFile);
		PyErr_SetString(PyExc_RuntimeError, "Could not load Python dialog file.");
	}

	// Fill in the Dialog State
	dlgState.pc = pc;
	dlgState.reqNpcLineId = lineId;
	/*
		Previously, the sid was used here, but this can actually screw up speech and play back the wrong samples,
		if a different script id is attached for san_dialog (which is used above) than for whatever san triggered
		this function.
	*/
	dlgState.dialogScriptId = dlgScriptId;
	dlgState.npc = self->handle;
	dialogScripts.LoadNpcLine(dlgState, true);

	uiDialog.ShowTextBubble(dlgState.npc, dlgState.pc, dlgState.npcLineText, dlgState.speechId);

	dialogScripts.Free(dlgState.dialogHandle);

	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_Damage(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl attacker;
	DamageType damageType;
	Dice dice;
	int attackPower = 0;
	D20ActionType actionType = D20A_NONE;
	if (!PyArg_ParseTuple(args, "O&iO&|ii:objhndl.damage", &ConvertObjHndl, &attacker, &damageType, &ConvertDice, &dice, &attackPower, &actionType)) {
		return 0;
	}

	damage.DealDamageFullUnk(self->handle, attacker, dice, damageType, attackPower, actionType);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_DamageWithReduction(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl attacker;
	DamageType damageType;
	Dice dice;
	int reduction;
	int attackPower = 0;
	D20ActionType actionType = D20A_NONE;
	if (!PyArg_ParseTuple(args, "O&iO&ii|i:objhndl.damage", &ConvertObjHndl, &attacker, &damageType, &ConvertDice, &dice, &attackPower, &reduction, &actionType)) {
		return 0;
	}

	// Line 105: Saving Throw
	damage.DealDamage(self->handle, attacker, dice, damageType, attackPower, reduction, 105, actionType);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_Heal(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl healer;
	Dice dice;
	D20ActionType actionType = D20A_NONE;

	if (!PyArg_ParseTuple(args, "O&O&|i:objhndl.heal", &ConvertObjHndl, &healer, &ConvertDice, &dice, &actionType)) {
		return 0;
	}
	damage.Heal(self->handle, healer, dice, actionType);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_HealSubdual(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl healer;
	Dice dice;
	D20ActionType actionType = D20A_NONE;
	int spellId = 0; // TODO: check this default
	if (!PyArg_ParseTuple(args, "O&O&|ii:objhndl.heal_subdual", &ConvertObjHndl, &healer, &ConvertDice, &dice, &actionType, &spellId)) {
		return 0;
	}

	auto amount = dice.Roll();
	damage.HealSubdual(self->handle, amount);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_SpellDamage(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl attacker;
	Dice dice;
	DamageType type;
	int attackPowerType = 0;
	D20ActionType actionType = D20A_NONE;
	int spellId = 0;
	if (!PyArg_ParseTuple(args, "O&iO&|iii:objhndl.spell_damage", &ConvertObjHndl, &attacker, &type, &ConvertDice, &dice, &attackPowerType, &actionType, &spellId)) {
		return 0;
	}
	damage.DealSpellDamageFullUnk(self->handle, attacker, dice, type, attackPowerType, actionType, spellId, 0);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_SpellDamageWithReduction(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl attacker;
	Dice dice;
	DamageType type;
	int attackPowerType = 0;
	int reduction = 100;
	D20ActionType actionType = D20A_NONE;
	int spellId = 0;
	if (!PyArg_ParseTuple(args, "O&iO&|iiii:objhndl.spell_damage_with_reduction", &ConvertObjHndl, &attacker, &type, &ConvertDice, &dice, &attackPowerType, &reduction, &actionType, &spellId)) {
		return 0;
	}
	// Line 105: Saving Throw
	damage.DealSpellDamage(self->handle, attacker, dice, type, attackPowerType, reduction, 105, actionType, spellId, 0);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_StealFrom(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl target;
	if (!PyArg_ParseTuple(args, "O&:objhndl.steal_from", &ConvertObjHndl, &target)) {
		return 0;
	}

	animationGoals.PushUseSkillOn(self->handle, target, skill_pick_pocket);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_ReputationHas(PyObject* obj, PyObject* args) {
	int reputationId;
	if (!PyArg_ParseTuple(args, "i:objhndl.reputation_has", &reputationId)) {
		return 0;
	}
	return PyInt_FromLong(partyReputation.Has(reputationId));
}

static PyObject* PyObjHandle_ReputationAdd(PyObject* obj, PyObject* args) {
	int reputationId;
	if (!PyArg_ParseTuple(args, "i:objhndl.reputation_add", &reputationId)) {
		return 0;
	}
	partyReputation.Add(reputationId);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_ReputationRemove(PyObject* obj, PyObject* args) {
	int reputationId;
	if (!PyArg_ParseTuple(args, "i:objhndl.reputation_remove", &reputationId)) {
		return 0;
	}
	partyReputation.Remove(reputationId);
	Py_RETURN_NONE;
}

static bool ParseCondNameAndArgs(PyObject* args, CondStruct*& condStructOut, vector<int>& argsOut) {
	// First arg has to be the condition name
	if (PyTuple_GET_SIZE(args) < 1 || !PyString_Check(PyTuple_GET_ITEM(args, 0))) {
		PyErr_SetString(PyExc_RuntimeError, "item_condition_add_with_args has to be "
		                "called at least with the condition name.");
		return false;
	}

	auto condName = PyString_AsString(PyTuple_GET_ITEM(args, 0));
	auto cond = conds.GetByName(condName);
	if (!cond) {
		PyErr_Format(PyExc_ValueError, "Unknown condition name: %s", condName);
		return false;
	}

	// Following arguments all have to be integers and gel with the condition argument count
	vector<int> condArgs(cond->numArgs, 0);
	for (int i = 0; i < cond->numArgs; ++i) {
		if (PyTuple_GET_SIZE(args) > i + 1) {
			auto item = PyTuple_GET_ITEM(args, i + 1);
			if (!PyInt_Check(item)) {
				auto itemRepr = PyObject_Repr(item);
				PyErr_Format(PyExc_ValueError, "Argument %d for condition %s (requires %d args) is not of type int: %s",
				             i + 1, condName, cond->numArgs, PyString_AsString(itemRepr));
				Py_DECREF(itemRepr);
				return false;
			}
			condArgs[i] = PyInt_AsLong(item);
		}
	}

	condStructOut = cond;
	argsOut = condArgs;

	return true;
}

static PyObject* PyObjHandle_ItemConditionAdd(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);

	CondStruct* cond;
	vector<int> condArgs;
	if (!ParseCondNameAndArgs(args, cond, condArgs)) {
		return 0;
	}

	conds.AddToItem(self->handle, cond, condArgs);
	return PyInt_FromLong(1);
}

static PyObject* PyObjHandle_ConditionAddWithArgs(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);

	CondStruct* cond;
	vector<int> condArgs;
	if (!ParseCondNameAndArgs(args, cond, condArgs)) {
		return 0;
	}

	conds.AddTo(self->handle, cond, condArgs);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_IsFriendly(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl pc;
	if (!PyArg_ParseTuple(args, "O&:objhndl.is_friendly", &ConvertObjHndl, &pc)) {
		return 0;
	}

	auto result = critterSys.IsFriendly(pc, self->handle);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_FadeTo(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int targetOpacity, transitionTimeInMs, unk1, unk2 = 0;
	if (!PyArg_ParseTuple(args, "iii|i:objhndl.fade_to", &targetOpacity, &transitionTimeInMs, &unk1, &unk2)) {
		return 0;
	}

	objects.FadeTo(self->handle, targetOpacity, transitionTimeInMs, unk1, unk2);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_Move(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	LocAndOffsets newLoc;
	newLoc.off_x = 0;
	newLoc.off_y = 0;
	if (!PyArg_ParseTuple(args, "L|ff:objhndl.move", &newLoc.location, &newLoc.off_x, &newLoc.off_y)) {
		return 0;
	}
	objects.Move(self->handle, newLoc);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_FloatMesFileLine(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	char* mesFilename;
	int mesLineKey;
	auto colorId = FloatLineColor::White;
	if (!PyArg_ParseTuple(args, "si|i:", &mesFilename, &mesLineKey, &colorId)) {
		return 0;
	}
	MesFile mesFile(mesFilename);
	if (!mesFile.valid()) {
		PyErr_Format(PyExc_IOError, "Could not open mes file %s", mesFilename);
		return 0;
	}
	const char* mesLine;
	if (!mesFile.GetLine(mesLineKey, mesLine)) {
		PyErr_Format(PyExc_IOError, "Could not find line %d in mes file %s.", mesLineKey, mesFilename);
		return 0;
	}

	floatSys.floatMesLine(self->handle, 1, colorId, mesLine);
	Py_RETURN_NONE;
}

// Generic methods to get/set/clear flags
template <obj_f flagsField>
static PyObject* GetFlags(PyObject* obj, PyObject*) {
	auto self = GetSelf(obj);
	auto flags = objects.getInt32(self->handle, flagsField);
	return PyInt_FromLong(flags);
}

template <obj_f flagsField>
static PyObject* SetFlag(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	uint32_t flag;
	if (!PyArg_ParseTuple(args, "i:objhndl.generic_set_flag", &flag)) {
		return 0;
	}
	auto currentFlags = objects.getInt32(self->handle, flagsField);
	currentFlags |= flag;
	objects.setInt32(self->handle, flagsField, currentFlags);
	Py_RETURN_NONE;
}

template <obj_f flagsField>
static PyObject* ClearFlag(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	uint32_t flag;
	if (!PyArg_ParseTuple(args, "i:objhndl.generic_set_flag", &flag)) {
		return 0;
	}
	auto currentFlags = objects.getInt32(self->handle, flagsField);
	currentFlags &= ~ flag;
	objects.setInt32(self->handle, flagsField, currentFlags);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_ObjectFlagSet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	ObjectFlag flag;
	if (!PyArg_ParseTuple(args, "i:objhndl.object_flag_set", &flag)) {
		return 0;
	}
	objects.SetFlag(self->handle, flag);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_ObjectFlagUnset(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	ObjectFlag flag;
	if (!PyArg_ParseTuple(args, "i:objhndl.object_flag_unset", &flag)) {
		return 0;
	}
	objects.ClearFlag(self->handle, flag);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_PortalToggleOpen(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objects.PortalToggleOpen(self->handle);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_ContainerToggleOpen(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objects.ContainerToggleOpen(self->handle);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_SavingThrow(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int dc;
	SavingThrowType type;
	auto flags = (D20SavingThrowFlag) 0;
	objHndl attacker = 0;
	if (!PyArg_ParseTuple(args, "iii|O&:objhndl:saving_throw", &dc, &type, &flags, &ConvertObjHndl, &attacker)) {
		return 0;
	}

	auto result = damage.SavingThrow(self->handle, attacker, dc, type, flags);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_SavingThrowSpell(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int dc;
	SavingThrowType type;
	auto flags = (D20SavingThrowFlag)0;
	objHndl attacker;
	int spellId;
	if (!PyArg_ParseTuple(args, "iiiO&i:objhndl:saving_throw_spell", &dc, &type, &flags, &ConvertObjHndl, &attacker, &spellId)) {
		return 0;
	}

	auto result = damage.SavingThrowSpell(self->handle, attacker, dc, type, flags, spellId);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_ReflexSaveAndDamage(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int dc;
	auto flags = (D20SavingThrowFlag)0;
	objHndl attacker;
	int reduction;
	D20ActionType actionType;
	Dice damageDice;
	D20AttackPower attackPower;
	DamageType damageType;
	int spellId;
	if (!PyArg_ParseTuple(args, "O&iiiO&ii|ii:objhndl:reflex_save_and_damage", &ConvertObjHndl, &attacker, &dc, &reduction, &flags, &ConvertDice, &damageDice, &damageType, &attackPower, &actionType, &spellId)) {
		return 0;
	}

	auto result = damage.ReflexSaveAndDamage(self->handle, attacker, dc, reduction, flags, damageDice, damageType, attackPower, actionType, spellId);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_SoundmapCritter(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int soundmapId;
	if (!PyArg_ParseTuple(args, "i:objhndl.soundmap_critter", &soundmapId)) {
		return 0;
	}

	auto soundId = critterSys.SoundmapCritter(self->handle, soundmapId);
	return PyInt_FromLong(soundId);
}

static PyObject* PyObjHandle_Footstep(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	auto soundId = critterSys.SoundmapCritter(self->handle, 7);
	auto result = sound.PlaySoundAtObj(soundId, self->handle);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_SecretdoorDetect(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl viewer;
	if (!PyArg_ParseTuple(args, "O&:objhndl.secretdoor_detect", &ConvertObjHndl, &viewer)) {
		return 0;
	}
	if (!viewer) {
		PyErr_SetString(PyExc_ValueError, "Called with an invalid viewer.");
		return 0;
	}
	auto result = objects.SecretdoorDetect(self->handle, viewer);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_HasSpellEffects(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	return PyInt_FromLong(objects.HasSpellEffects(self->handle));
}

static PyObject* PyObjHandle_Kill(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	critterSys.Kill(self->handle, 0);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_KillByEffect(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	critterSys.KillByEffect(self->handle, 0);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_Destroy(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objects.Destroy(self->handle);
	self->handle = 0; // Clear the obj handle
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_ItemGet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl item;
	if (!PyArg_ParseTuple(args, "O&:objhndl.item_get", &ConvertObjHndl, &item)) {
		return 0;
	}
	auto result = inventory.SetItemParent(item, self->handle, 0);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_PerformTouchAttack(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	D20Actn action(D20A_TOUCH_ATTACK);
	if (!PyArg_ParseTuple(args, "O&:objhndl.perform_touch_attack", &ConvertObjHndl, &action.d20ATarget)) {
		return 0;
	}

	// Build a to-hit action, do hit processing and return the result
	action.d20APerformer = self->handle;
	action.d20Caf = D20CAF_TOUCH_ATTACK | D20CAF_RANGED;
	action.data1 = 1;

	d20Sys.ToHitProc(&action);
	d20Sys.CreateRollHistory(action.rollHist1);
	d20Sys.CreateRollHistory(action.rollHist2);
	d20Sys.CreateRollHistory(action.rollHist3);

	if (action.d20Caf & D20CAF_CRITICAL) {
		return PyInt_FromLong(D20CAF_CRITICAL);
	} else if (action.d20Caf & D20CAF_HIT) {
		return PyInt_FromLong(D20CAF_HIT);
	} else {
		return PyInt_FromLong(0);
	}
}

static PyObject* PyObjHandle_AddToInitiative(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	combatSys.AddToInitiative(self->handle);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_RemoveFromInitiative(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	combatSys.RemoveFromInitiative(self->handle);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_GetInitiative(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	return PyInt_FromLong(combatSys.GetInitiative(self->handle));
}

static PyObject* PyObjHandle_SetInitiative(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int initiative;
	if (!PyArg_ParseTuple(args, "i:objhndl.set_initiative", &initiative)) {
		return 0;
	}
	combatSys.SetInitiative(self->handle, initiative);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_D20Query(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int queryKey;
	if (!PyArg_ParseTuple(args, "i:objhndl.d20query", &queryKey)) {
		return 0;
	}
	auto dispatcherKey = (D20DispatcherKey)(DK_QUE_Helpless + queryKey);
	auto result = d20Sys.d20Query(self->handle, dispatcherKey);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_D20QueryHasSpellCond(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int spellId;
	if (!PyArg_ParseTuple(args, "i:objhndl.d20query_has_spell_cond", &spellId)) {
		return 0;
	}

	// Get the condition struct from the spell id
	auto cond = spellSys.GetCondFromSpellIdx(spellId);
	if (!cond) {
		return PyInt_FromLong(0);
	}

	auto result = d20Sys.d20QueryWithData(self->handle, DK_QUE_Critter_Has_Condition, (uint32_t) cond, 0);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_D20QueryWithData(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int queryKey;
	int data1, data2 = 0;
	if (!PyArg_ParseTuple(args, "ii|i:objhndl.d20query_with_data", &queryKey, &data1, &data2)) {
		return 0;
	}
	auto dispatcherKey = (D20DispatcherKey)(DK_QUE_Helpless + queryKey);
	auto result = d20Sys.d20QueryWithData(self->handle, dispatcherKey, data1, data2);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_D20QueryTestData(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int queryKey, testData;
	if (!PyArg_ParseTuple(args, "ii:objhndl.d20query_test_data", &queryKey, &testData)) {
		return 0;
	}
	auto dispatcherKey = (D20DispatcherKey)(DK_QUE_Helpless + queryKey);
	if (d20Sys.d20QueryReturnData(self->handle, dispatcherKey, 0, 0) == testData) {
		return PyInt_FromLong(1);
	} else {
		return PyInt_FromLong(1);
	}
}

static PyObject* PyObjHandle_D20QueryGetData(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int queryKey, testData;
	if (!PyArg_ParseTuple(args, "ii:objhndl.d20query_get_data", &queryKey, &testData)) {
		return 0;
	}
	auto dispatcherKey = (D20DispatcherKey)(DK_QUE_Helpless + queryKey);
	auto result = d20Sys.d20QueryReturnData(self->handle, dispatcherKey, 0, 0);
	return PyLong_FromLongLong(result);
}

static PyObject* PyObjHandle_CritterGetAlignment(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	auto alignment = objects.getInt32(self->handle, obj_f_critter_alignment);
	return PyInt_FromLong(alignment);
}

static PyObject* PyObjHandle_DistanceTo(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (PyTuple_GET_SIZE(args) != 1) {
		PyErr_SetString(PyExc_RuntimeError, "distance_to takes exactly one argument: objhndl or location");
		return 0;
	}
	PyObject* arg = PyTuple_GET_ITEM(args, 0);
	if (PyObjHndl_Check(arg)) {
		auto target = PyObjHndl_AsObjHndl(arg);
		auto dist = locSys.DistanceToObj(self->handle, target); // Returns feet
		return PyFloat_FromDouble(dist);
	} else if (PyLong_Check(arg)) {
		LocAndOffsets targetLoc;
		targetLoc.location = locXY::fromField(PyLong_AsUnsignedLongLong(arg));
		targetLoc.off_x = 0;
		targetLoc.off_y = 0;
		auto dist = locSys.DistanceToLoc(self->handle, targetLoc);
		dist = locSys.InchesToFeet(dist);
		return PyFloat_FromDouble(dist);
	}
	PyErr_SetString(PyExc_RuntimeError, "distance_to takes exactly one argument: objhndl or location");
	return 0;
}

/*
	This seems to have been an earlier impl of the frog tongue, but apparently only the failed state
	is used now.
*/
static PyObject* PyObjHandle_AnimCallback(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int id;
	if (!PyArg_ParseTuple(args, "i:obj.anim_callback", &id)) {
		return 0;
	}

	int oldGrappleState;
	switch (id) {
	case 0: // ANIM_CALLBACK_FROG_FAILED_LATCH
		objects.setInt32(self->handle, obj_f_grapple_state, 1);
		break;
	case 1: // ANIM_CALLBACK_FROG_LATCH
		objects.setInt32(self->handle, obj_f_grapple_state, 3);
		break;
	case 2: // ANIM_CALLBACK_FROG_PULL
		oldGrappleState = objects.getInt32(self->handle, obj_f_grapple_state);
		objects.setInt32(self->handle, obj_f_grapple_state, oldGrappleState & 0xFFFF0005 | 5);
		break;
	case 3: // ANIM_CALLBACK_FROG_SWALLOW
		oldGrappleState = objects.getInt32(self->handle, obj_f_grapple_state);
		objects.setInt32(self->handle, obj_f_grapple_state, oldGrappleState & 0xFFFF0007 | 7);
		break;
	default:
		PyErr_Format(PyExc_ValueError, "Unknown animation id passed to anim_callback: %d", id);
		return 0;
	}
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_ObjectScriptExecute(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl triggerer;
	ObjScriptEvent scriptEvent;
	if (!PyArg_ParseTuple(args, "O&i:objhndl.object_script_execute", &triggerer, &scriptEvent)) {
		return 0;
	}
	auto result = pythonObjIntegration.ExecuteObjectScript(triggerer, self->handle, scriptEvent);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_AnimGoalInterrupt(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	animationGoals.Interrupt(self->handle, AGP_HIGHEST, false);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_D20StatusInit(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	d20Sys.d20Status->D20StatusInit(self->handle);
	Py_RETURN_NONE;
}

/*
	Sets one of the critters stand points to a jump point.
*/
static PyObject* PyObjHandle_StandpointSet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	StandPointType type;
	int jumpPointId;
	if (!PyArg_ParseTuple(args, "ii:objhndl.standpoint_set", &type, &jumpPointId)) {
		return 0;
	}

	JumpPoint jumpPoint;
	if (!maps.GetJumpPoint(jumpPointId, jumpPoint)) {
		PyErr_Format(PyExc_ValueError, "Unknown jump point id %d used to set a standpoint.", jumpPointId);
		return 0;
	}

	StandPoint standPoint;
	standPoint.location.location = jumpPoint.location;
	standPoint.location.off_x = 0;
	standPoint.location.off_y = 0;
	standPoint.mapId = jumpPoint.mapId;
	standPoint.jumpPointId = jumpPointId;
	critterSys.SetStandPoint(self->handle, type, standPoint);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_RunOff(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	LocAndOffsets loc;
	loc.off_x = 0;
	loc.off_y = 0;
	if (!PyArg_ParseTuple(args, "L|ff:objhndl.runoff", &loc.location, &loc.off_x, &loc.off_y)) {
		return 0;
	}

	objects.SetFlag(self->handle, OF_CLICK_THROUGH);
	aiSys.SetAiFlag(self->handle, AiFlag::RunningOff);
	objects.FadeTo(self->handle, 0, 25, 5, 2);
	animationGoals.PushRunNearTile(self->handle, loc, 5);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_GetCategoryType(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	auto type = critterSys.GetCategory(self->handle);
	return PyInt_FromLong(type);
}

static PyObject* PyObjHandle_IsCategoryType(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int type;
	if (!PyArg_ParseTuple(args, "i:objhndl.is_category_type", &type)) {
		return 0;
	}
	auto result = critterSys.IsCategoryType(self->handle, (MonsterCategory) type);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_IsCategorySubtype(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int type;
	if (!PyArg_ParseTuple(args, "i:objhndl.is_category_subtype", &type)) {
		return 0;
	}
	auto result = critterSys.IsCategorySubtype(self->handle, (MonsterCategory)type);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_RumorLogAdd(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int rumorId;
	if (!PyArg_ParseTuple(args, "i:objhndl.rumor_log_add", &rumorId)) {
		return 0;
	}
	party.RumorLogAdd(self->handle, rumorId);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_SetInt(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	obj_f field;
	int value;
	if (!PyArg_ParseTuple(args, "ii:objhndl.obj_set_int", &field, &value)) {
		return 0;
	}
	if (field == obj_f_critter_subdual_damage) {
		critterSys.SetSubdualDamage(self->handle, value);
	} else {
		objects.setInt32(self->handle, field, value);
	}
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_GetInt(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	obj_f field;
	if (!PyArg_ParseTuple(args, "i:objhndl.obj_get_int", &field)) {
		return 0;
	}
	auto value = objects.getInt32(self->handle, field);
	return PyInt_FromLong(value);
}

static PyObject* PyObjHandle_HasFeat(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	feat_enums feat;
	if (!PyArg_ParseTuple(args, "i:objhndl.has_feat", &feat)) {
		return 0;
	}

	auto result = _HasFeatCountByClass(self->handle, feat, (Stat) 0, 0);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_SpellKnownAdd(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int spellIdx;
	int spellClassCode;
	int slotLevel;
	if (!PyArg_ParseTuple(args, "iii:objhndl.spell_known_add", &spellIdx, &spellClassCode, &slotLevel)) {
		return 0;
	}
	spellSys.SpellKnownAdd(self->handle, spellIdx, spellClassCode & 0x7F | 0x80, slotLevel, 1, 0);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_SpellMemorizedAdd(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int spellIdx;
	int spellClassCode;
	int slotLevel;
	if (!PyArg_ParseTuple(args, "iii:objhndl.spell_memorized_add", &spellIdx, &spellClassCode, &slotLevel)) {
		return 0;
	}
	spellSys.SpellKnownAdd(self->handle, spellIdx, spellClassCode & 0x7F | 0x80, slotLevel, 2, 0);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_SpellHeal(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl healer;
	Dice dice;
	D20ActionType actionType = D20A_NONE;
	int spellId = 0;
	if (!PyArg_ParseTuple(args, "O&O&|ii:objhndl.spell_heal", &ConvertObjHndl, &healer, &ConvertDice, &dice, &actionType, &spellId)) {
		return 0;
	}
	damage.HealSpell(self->handle, healer, dice, actionType, spellId);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_IdentifyAll(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	inventory.IdentifyAll(self->handle);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_AiFleeAdd(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl target;
	if (!PyArg_ParseTuple(args, "O&:objhndl.ai_flee_add", &ConvertObjHndl, &target)) {
		return 0;
	}
	aiSys.FleeAdd(self->handle, target);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_AiShitlistAdd(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl target;
	if (!PyArg_ParseTuple(args, "O&:objhndl.ai_shitlist_add", &ConvertObjHndl, &target)) {
		return 0;
	}
	aiSys.ShitlistAdd(self->handle, target);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_AiShitlistRemove(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl target;
	if (!PyArg_ParseTuple(args, "O&:objhndl.ai_shitlist_remove", &ConvertObjHndl, &target)) {
		return 0;
	}
	aiSys.ShitlistRemove(self->handle, target);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_AiStopAttacking(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	aiSys.StopAttacking(self->handle);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_GetDeity(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	return PyInt_FromLong(objects.GetDeity(self->handle));
}

static PyObject* PyObjHandle_WieldBestAll(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl targetHndl = 0;
	if (!PyArg_ParseTuple(args, "|O&:objhndl.wield_best_all", &ConvertObjHndl, &targetHndl)) {
		return 0;
	}
	inventory.WieldBestAll(self->handle, targetHndl);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_AwardExperience(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int xpAwarded;
	if (!PyArg_ParseTuple(args, "i:objhndl.award_experience", &xpAwarded)) {
		return 0;
	}
	critterSys.AwardXp(self->handle, xpAwarded);
	ui.UpdatePartyUi();
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_HasAtoned(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	d20Sys.d20SendSignal(self->handle, DK_SIG_Atone_Fallen_Paladin, 0, 0);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_D20SendSignal(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int signalId;
	PyObject* arg = 0;
	if (!PyArg_ParseTuple(args, "i|O:objhndl.d20_send_signal", &signalId, &arg)) {
		return 0;
	}
	D20DispatcherKey dispKey = (D20DispatcherKey)(DK_SIG_HP_Changed + signalId);

	if (arg && PyObjHndl_Check(arg)) {
		objHndl hndl = PyObjHndl_AsObjHndl(arg);
		d20Sys.d20SendSignal(self->handle, dispKey, hndl);
	} else if (arg && PyInt_Check(arg)) {
		auto val = PyInt_AsLong(arg);
		d20Sys.d20SendSignal(self->handle, dispKey, val, 0);
	} else {
		d20Sys.d20SendSignal(self->handle, dispKey, 0, 0);
	}

	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_D20SendSignalEx(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int signalId;
	objHndl arg = 0;
	if (!PyArg_ParseTuple(args, "i|O&:objhndl.d20_send_signal_ex", &signalId, &ConvertObjHndl, &arg)) {
		return 0;
	}
	D20DispatcherKey dispKey = (D20DispatcherKey)(DK_SIG_HP_Changed + signalId);

	d20Sys.d20SendSignal(self->handle, dispKey, arg);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_BalorDeath(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	critterSys.BalorDeath(self->handle);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_ConcealedSet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	bool concealed;
	if (!PyArg_ParseTuple(args, "i:objhndl.concealed_set", &concealed)) {
		return 0;
	}
	critterSys.SetConcealed(self->handle, concealed);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_Unconceal(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	auto result = animationGoals.PushUnconceal(self->handle);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_PendingToMemorized(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	spellSys.spellsPendingToMemorized(self->handle);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_MemorizedForget(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	spellSys.ForgetMemorized(self->handle);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_Resurrect(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	ResurrectType type;
	int unk = 0;
	if (!PyArg_ParseTuple(args, "i|i:objhndl.resurrect", &type, &unk)) {
		return 0;
	}
	auto result = critterSys.Resurrect(self->handle, type, unk);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_Dominate(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl caster;
	if (!PyArg_ParseTuple(args, "O&:objhndl.dominate", &ConvertObjHndl, &caster)) {
		return 0;
	}
	auto result = critterSys.Dominate(self->handle, caster);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_IsUnconscious(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	auto result = critterSys.IsDeadOrUnconscious(self->handle);
	return PyInt_FromLong(result);
}

static PyMethodDef PyObjHandleMethods[] = {
	{"__getstate__", PyObjHandle_getstate, METH_VARARGS, NULL},
	{"__reduce__", PyObjHandle_reduce, METH_VARARGS, NULL},
	{"__setstate__", PyObjHandle_setstate, METH_VARARGS, NULL},
	{"begin_dialog", PyObjHandle_BeginDialog, METH_VARARGS, NULL},
	{"reaction_get", PyObjHandle_ReactionGet, METH_VARARGS, NULL},
	{"reaction_set", PyObjHandle_ReactionSet, METH_VARARGS, NULL},
	{"reaction_adj", PyObjHandle_ReactionAdjust, METH_VARARGS, NULL},
	{"item_find", PyObjHandle_ItemFind, METH_VARARGS, NULL},
	{"item_transfer_to", PyObjHandle_ItemTransferTo, METH_VARARGS, NULL},
	{"item_find_by_proto", PyObjHandle_ItemFindByProto, METH_VARARGS, NULL},
	{"item_transfer_to_by_proto", PyObjHandle_ItemTransferToByProto, METH_VARARGS, NULL},
	{"money_get", PyObjHandle_MoneyGet, METH_VARARGS, NULL},
	{"money_adj", PyObjHandle_MoneyAdj, METH_VARARGS, NULL},
	{"cast_spell", PyObjHandle_CastSpell, METH_VARARGS, NULL},
	{"skill_level_get", PyObjHandle_SkillLevelGet, METH_VARARGS, NULL},
	{"has_met", PyObjHandle_HasMet, METH_VARARGS, NULL},
	{"hasMet", PyObjHandle_HasMet, METH_VARARGS, NULL},
	{"has_follower", PyObjHandle_HasFollower, METH_VARARGS, NULL},
	{"group_list", PyObjHandle_GroupList, METH_VARARGS, NULL},
	{"stat_level_get", PyObjHandle_StatLevelGet, METH_VARARGS, NULL},
	{"stat_base_get", PyObjHandle_StatLevelGetBase, METH_VARARGS, NULL},
	{"stat_base_set", PyObjHandle_StatLevelSetBase, METH_VARARGS, NULL},
	{"follower_add", PyObjHandle_FollowerAdd, METH_VARARGS, NULL},
	{"follower_remove", PyObjHandle_FollowerRemove, METH_VARARGS, NULL},
	{"follower_atmax", PyObjHandle_FollowerAtMax, METH_VARARGS, NULL},
	{"ai_follower_add", PyObjHandle_AiFollowerAdd, METH_VARARGS, NULL},
	{"ai_follower_remove", ReturnZero, METH_VARARGS, NULL},
	{"ai_follower_atmax", ReturnZero, METH_VARARGS, NULL},
	{"leader_get", PyObjHandle_LeaderGet, METH_VARARGS, NULL},
	{"can_see", PyObjHandle_HasLos, METH_VARARGS, NULL},
	{"has_wielded", PyObjHandle_HasWielded, METH_VARARGS, NULL},
	{"has_item", PyObjHandle_HasItem, METH_VARARGS, NULL},
	{"item_worn_at", PyObjHandle_ItemWornAt, METH_VARARGS, NULL},
	{"attack", PyObjHandle_Attack, METH_VARARGS, NULL},
	{"turn_towards", PyObjHandle_TurnTowards, METH_VARARGS, NULL},
	{"float_line", PyObjHandle_FloatLine, METH_VARARGS, NULL},
	{"damage", PyObjHandle_Damage, METH_VARARGS, NULL},
	{"damage_with_reduction", PyObjHandle_DamageWithReduction, METH_VARARGS, NULL},
	{"heal", PyObjHandle_Heal, METH_VARARGS, NULL},
	{"healsubdual", PyObjHandle_HealSubdual, METH_VARARGS, NULL},
	{"steal_from", PyObjHandle_StealFrom, METH_VARARGS, NULL},
	{"reputation_has", PyObjHandle_ReputationHas, METH_VARARGS, NULL},
	{"reputation_add", PyObjHandle_ReputationAdd, METH_VARARGS, NULL},
	{"reputation_remove", PyObjHandle_ReputationRemove, METH_VARARGS, NULL},
	{"item_condition_add_with_args", PyObjHandle_ItemConditionAdd, METH_VARARGS, NULL},
	{"condition_add_with_args", PyObjHandle_ConditionAddWithArgs, METH_VARARGS, NULL},
	{"condition_add", PyObjHandle_ConditionAddWithArgs, METH_VARARGS, NULL},
	{"is_friendly", PyObjHandle_IsFriendly, METH_VARARGS, NULL},
	{"fade_to", PyObjHandle_FadeTo, METH_VARARGS, NULL},
	{"move", PyObjHandle_Move, METH_VARARGS, NULL},
	{"float_mesfile_line", PyObjHandle_FloatMesFileLine, METH_VARARGS, NULL},
	{"object_flags_get", GetFlags<obj_f_flags>, METH_VARARGS, NULL},
	{"object_flag_set", PyObjHandle_ObjectFlagSet, METH_VARARGS, NULL},
	{"object_flag_unset", PyObjHandle_ObjectFlagUnset, METH_VARARGS, NULL},
	{"portal_flags_get", GetFlags<obj_f_portal_flags>, METH_VARARGS, NULL},
	{"portal_flag_set", SetFlag<obj_f_portal_flags>, METH_VARARGS, NULL},
	{"portal_flag_unset", ClearFlag<obj_f_portal_flags>, METH_VARARGS, NULL},
	{"container_flags_get", GetFlags<obj_f_container_flags>, METH_VARARGS, NULL},
	{"container_flag_set", SetFlag<obj_f_container_flags>, METH_VARARGS, NULL},
	{"container_flag_unset", ClearFlag<obj_f_container_flags>, METH_VARARGS, NULL},
	{"portal_toggle_open", PyObjHandle_PortalToggleOpen, METH_VARARGS, NULL},
	{"container_toggle_open", PyObjHandle_ContainerToggleOpen, METH_VARARGS, NULL},
	{"item_flags_get", GetFlags<obj_f_item_flags>, METH_VARARGS, NULL},
	{"item_flag_set", SetFlag<obj_f_item_flags>, METH_VARARGS, NULL},
	{"item_flag_unset", ClearFlag<obj_f_item_flags>, METH_VARARGS, NULL},
	{"critter_flags_get", GetFlags<obj_f_critter_flags>, METH_VARARGS, NULL},
	{"critter_flag_set", SetFlag<obj_f_critter_flags>, METH_VARARGS, NULL},
	{"critter_flag_unset", ClearFlag<obj_f_critter_flags>, METH_VARARGS, NULL},
	{"npc_flags_get", GetFlags<obj_f_npc_flags>, METH_VARARGS, NULL},
	{"npc_flag_set", SetFlag<obj_f_npc_flags>, METH_VARARGS, NULL},
	{"npc_flag_unset", ClearFlag<obj_f_npc_flags>, METH_VARARGS, NULL},
	{"saving_throw", PyObjHandle_SavingThrow, METH_VARARGS, NULL},
	{"saving_throw_with_args", PyObjHandle_SavingThrow, METH_VARARGS, NULL},
	{"saving_throw_spell", PyObjHandle_SavingThrowSpell, METH_VARARGS, NULL},
	{"reflex_save_and_damage", PyObjHandle_ReflexSaveAndDamage, METH_VARARGS, NULL},
	{"soundmap_critter", PyObjHandle_SoundmapCritter, METH_VARARGS, NULL},
	{"footstep", PyObjHandle_Footstep, METH_VARARGS, NULL},
	{"secretdoor_detect", PyObjHandle_SecretdoorDetect, METH_VARARGS, NULL},
	{"has_spell_effects", PyObjHandle_HasSpellEffects, METH_VARARGS, NULL},
	{"critter_kill", PyObjHandle_Kill, METH_VARARGS, NULL},
	{"critter_kill_by_effect", PyObjHandle_KillByEffect, METH_VARARGS, NULL},
	{"destroy", PyObjHandle_Destroy, METH_VARARGS, NULL},
	{"item_get", PyObjHandle_ItemGet, METH_VARARGS, NULL},
	{"perform_touch_attack", PyObjHandle_PerformTouchAttack, METH_VARARGS, NULL},
	{"add_to_initiative", PyObjHandle_AddToInitiative, METH_VARARGS, NULL},
	{"remove_from_initiative", PyObjHandle_RemoveFromInitiative, METH_VARARGS, NULL},
	{"get_initiative", PyObjHandle_GetInitiative, METH_VARARGS, NULL},
	{"set_initiative", PyObjHandle_SetInitiative, METH_VARARGS, NULL},
	{"d20_query", PyObjHandle_D20Query, METH_VARARGS, NULL},
	{"d20_query_has_spell_condition", PyObjHandle_D20QueryHasSpellCond, METH_VARARGS, NULL},
	{"d20_query_with_data", PyObjHandle_D20QueryWithData, METH_VARARGS, NULL},
	{"d20_query_test_data", PyObjHandle_D20QueryTestData, METH_VARARGS, NULL},
	{"d20_query_get_data", PyObjHandle_D20QueryGetData, METH_VARARGS, NULL},
	{"critter_get_alignment", PyObjHandle_CritterGetAlignment, METH_VARARGS, NULL},
	{"distance_to", PyObjHandle_DistanceTo, METH_VARARGS, NULL},
	{"anim_callback", PyObjHandle_AnimCallback, METH_VARARGS, NULL},
	{"anim_goal_interrupt", PyObjHandle_AnimGoalInterrupt, METH_VARARGS, NULL},
	{"d20_status_init", PyObjHandle_D20StatusInit, METH_VARARGS, NULL},
	{"object_script_execute", PyObjHandle_ObjectScriptExecute, METH_VARARGS, NULL},
	{"standpoint_set", PyObjHandle_StandpointSet, METH_VARARGS, NULL},
	{"runoff", PyObjHandle_RunOff, METH_VARARGS, NULL},
	{"get_category_type", PyObjHandle_GetCategoryType, METH_VARARGS, NULL},
	{"is_category_type", PyObjHandle_IsCategoryType, METH_VARARGS, NULL},
	{"is_category_subtype", PyObjHandle_IsCategorySubtype, METH_VARARGS, NULL},
	{"rumor_log_add", PyObjHandle_RumorLogAdd, METH_VARARGS, NULL},
	{"obj_set_int", PyObjHandle_SetInt, METH_VARARGS, NULL},
	{"obj_get_int", PyObjHandle_GetInt, METH_VARARGS, NULL},
	{"has_feat", PyObjHandle_HasFeat, METH_VARARGS, NULL},
	{"spell_known_add", PyObjHandle_SpellKnownAdd, METH_VARARGS, NULL},
	{"spell_memorized_add", PyObjHandle_SpellMemorizedAdd, METH_VARARGS, NULL},
	{"spell_damage", PyObjHandle_SpellDamage, METH_VARARGS, NULL},
	{"spell_damage_with_reduction", PyObjHandle_SpellDamageWithReduction, METH_VARARGS, NULL},
	{"spell_heal", PyObjHandle_SpellHeal, METH_VARARGS, NULL},
	{"identify_all", PyObjHandle_IdentifyAll, METH_VARARGS, NULL},
	{"ai_flee_add", PyObjHandle_AiFleeAdd, METH_VARARGS, NULL},
	{"get_deity", PyObjHandle_GetDeity, METH_VARARGS, NULL},
	{"item_wield_best_all", PyObjHandle_WieldBestAll, METH_VARARGS, NULL},
	{"award_experience", PyObjHandle_AwardExperience, METH_VARARGS, NULL},
	{"has_los", PyObjHandle_HasLos, METH_VARARGS, NULL},
	{"has_atoned", PyObjHandle_HasAtoned, METH_VARARGS, NULL},
	{"d20_send_signal", PyObjHandle_D20SendSignal, METH_VARARGS, NULL},
	{"d20_send_signal_ex", PyObjHandle_D20SendSignalEx, METH_VARARGS, NULL},
	{"balor_death", PyObjHandle_BalorDeath, METH_VARARGS, NULL},
	{"concealed_set", PyObjHandle_ConcealedSet, METH_VARARGS, NULL},
	{"ai_shitlist_add", PyObjHandle_AiShitlistAdd, METH_VARARGS, NULL},
	{"ai_shitlist_remove", PyObjHandle_AiShitlistRemove, METH_VARARGS, NULL},
	{"unconceal", PyObjHandle_Unconceal, METH_VARARGS, NULL},
	{"spells_pending_to_memorized", PyObjHandle_PendingToMemorized, METH_VARARGS, NULL},
	{"spells_memorized_forget", PyObjHandle_MemorizedForget, METH_VARARGS, NULL},
	{"ai_stop_attacking", PyObjHandle_AiStopAttacking, METH_VARARGS, NULL},
	{"resurrect", PyObjHandle_Resurrect, METH_VARARGS, NULL},
	{"dominate", PyObjHandle_Dominate, METH_VARARGS, NULL},
	{"is_unconscious", PyObjHandle_IsUnconscious, METH_VARARGS, NULL},
	{NULL, NULL, NULL, NULL}
};

#pragma endregion 

#pragma region Getters and Setters

static PyObject* PyObjHandle_GetNameId(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyInt_FromLong(objects.GetNameId(self->handle));
}

static PyObject* PyObjHandle_GetLocation(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyLong_FromLongLong(objects.GetLocation(self->handle));
}

static PyObject* PyObjHandle_GetType(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyInt_FromLong(objects.GetType(self->handle));
}

static PyObject* PyObjHandle_GetRadius(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyFloat_FromDouble(objects.GetRadius(self->handle));
}

static int PyObjHandle_SetRadius(PyObject* obj, PyObject* value, void*) {
	auto self = GetSelf(obj);
	float radius;
	if (!GetFloatLenient(value, radius)) {
		return -1;
	}

	// I think without setting the OBJ_F radius_set this pretty much does nothing
	objects.SetRadius(self->handle, radius);
	return 0;
}

static PyObject* PyObjHandle_GetRenderHeight(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyFloat_FromDouble(objects.GetRenderHeight(self->handle));
}

static int PyObjHandle_SetRenderHeight(PyObject* obj, PyObject* value, void*) {
	auto self = GetSelf(obj);
	float renderHeight;
	if (!GetFloatLenient(value, renderHeight)) {
		return -1;
	}

	objects.SetRenderHeight(self->handle, renderHeight);
	return 0;
}

static PyObject* PyObjHandle_GetRotation(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyFloat_FromDouble(objects.GetRotation(self->handle));
}

static int PyObjHandle_SetRotation(PyObject* obj, PyObject* value, void*) {
	auto self = GetSelf(obj);
	float rotation;
	if (!GetFloatLenient(value, rotation)) {
		return -1;
	}
	objects.SetRotation(self->handle, rotation);
	return 0;
}

static PyObject* PyObjHandle_GetHitDice(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyDice_FromDice(objects.GetHitDice(self->handle));
}

static PyObject* PyObjHandle_GetHitDiceNum(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyInt_FromLong(objects.GetHitDiceNum(self->handle));
}

static PyObject* PyObjHandle_GetSize(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyInt_FromLong(objects.GetSize(self->handle));
}

static PyObject* PyObjHandle_GetOffsetX(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyFloat_FromDouble(objects.GetOffsetX(self->handle));
}

static PyObject* PyObjHandle_GetOffsetY(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyFloat_FromDouble(objects.GetOffsetY(self->handle));
}

static PyObject* PyObjHandle_GetScripts(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyObjScripts_Create(self->handle);
}

static PyObject* PyObjHandle_GetOriginMapId(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyInt_FromLong(objects.GetOriginMapId(self->handle));
}

static int PyObjHandle_SetOriginMapId(PyObject* obj, PyObject* value, void*) {
	auto self = GetSelf(obj);
	int mapId;
	if (!GetInt(value, mapId)) {
		return -1;
	}
	if (!maps.IsValidMapId(mapId)) {
		auto msg = format("Map id {} is invalid.", mapId);
		PyErr_SetString(PyExc_ValueError, msg.c_str());
		return -1;
	}
	objects.SetOriginMapId(self->handle, mapId);
	return 0;
}

static PyObject* PyObjHandle_GetSubstituteInventory(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	auto handle = inventory.GetSubstituteInventory(self->handle);
	return PyObjHndl_Create(handle);
}

static PyObject* PyObjHandle_GetFeats(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	auto feats = objects.feats.GetFeats(self->handle);
	auto result = PyTuple_New(feats.size());
	for (size_t i = 0; i < feats.size(); ++i) {
		PyTuple_SET_ITEM(result, i, PyInt_FromLong(feats[i]));
	}
	return result;
}

// This is the NPC looting behaviour
static PyObject* PyObjHandle_GetLoots(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyInt_FromLong(critterSys.GetLootBehaviour(self->handle));
}

static int PyObjHandle_SetLoots(PyObject* obj, PyObject* value, void*) {
	auto self = GetSelf(obj);
	int loots;
	if (!GetInt(value, loots)) {
		return -1;
	}
	critterSys.SetLootBehaviour(self->handle, loots);
	return 0;
}

static PyObject* PyObjHandle_SafeForUnpickling(PyObject*, void*) {
	Py_RETURN_TRUE;
}

static PyGetSetDef PyObjHandleGetSets[] = {
	PY_INT_PROP_RO("area", maps.GetCurrentMapId, NULL),
	{"name", PyObjHandle_GetNameId, NULL, NULL},
	{"location", PyObjHandle_GetLocation, NULL, NULL},
	{"type", PyObjHandle_GetType, NULL, NULL},
	{"radius", PyObjHandle_GetRadius, PyObjHandle_SetRadius, NULL},
	{"height", PyObjHandle_GetRenderHeight, PyObjHandle_SetRenderHeight, NULL},
	{"rotation", PyObjHandle_GetRotation, PyObjHandle_SetRotation, NULL},
	PY_INT_PROP_RO("map", maps.GetCurrentMapId, NULL),
	{"hit_dice", PyObjHandle_GetHitDice, NULL, NULL},
	{"hit_dice_num", PyObjHandle_GetHitDiceNum, NULL, NULL},
	{"get_size", PyObjHandle_GetSize, NULL, NULL},
	{"off_x", PyObjHandle_GetOffsetX, NULL, NULL},
	{"off_y", PyObjHandle_GetOffsetY, NULL, NULL},
	{"scripts", PyObjHandle_GetScripts, NULL, NULL},
	{"origin", PyObjHandle_GetOriginMapId, PyObjHandle_SetOriginMapId, NULL},
	{"substitute_inventory", PyObjHandle_GetSubstituteInventory, NULL, NULL},
	{"feats", PyObjHandle_GetFeats, NULL, NULL},
	{"loots", PyObjHandle_GetLoots, PyObjHandle_SetLoots, NULL},
	{"__safe_for_unpickling__", PyObjHandle_SafeForUnpickling, NULL, NULL},
};

#pragma endregion

#pragma region Number Methods

static int PyObjHandle_NonZero(PyObject* obj) {
	auto self = GetSelf(obj);
	return self->handle != 0;
}

static PyNumberMethods PyObjHandleNumberMethods = {
	0, // nb_add
	0, // nb_subtract
	0, // nb_multiply
	0, // nb_divide
	0, // nb_remainder
	0, // nb_divmod
	0, // nb_power
	0, // nb_negative
	0, // nb_positive
	0, // nb_absolute
	PyObjHandle_NonZero, // nb_nonzero
	0,
};

#pragma endregion

#pragma region Initialization and New Methods
static int PyObjHandle_Init(PyObject* obj, PyObject* args, PyObject* kwargs) {
	auto self = (PyObjHandle*)obj;

	if (!PyArg_ParseTuple(args, "|L:PyObjHandle", &self->handle)) {
		return -1;
	}

	if (!self->handle) {
		self->id.subtype = 0;
	} else {
		// Get GUID from handle
		self->id = objects.GetId(self->handle);

		// The obj handle is invalid
		if (!self->id.subtype) {
			auto msg = format("The object handle {} is invalid.", self->handle);
			PyErr_SetString(PyExc_ValueError, msg.c_str());
			// Reset the handle to the null handle
			self->handle = 0;
			self->id.subtype = 0;
			return -1;
		}
	}

	return 0;
}

static PyObject* PyObjHandle_New(PyTypeObject*, PyObject*, PyObject*) {
	auto self = PyObject_New(PyObjHandle, &PyObjHandleType);
	self->handle = 0;
	self->id.subtype = 0;
	return (PyObject*)self;
}
#pragma endregion

PyTypeObject PyObjHandleType = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"toee.PyObjHandle", /*tp_name*/
	sizeof(PyObjHandle), /*tp_basicsize*/
	0, /*tp_itemsize*/
	(destructor) PyObject_Del, /*tp_dealloc*/
	0, /*tp_print*/
	0, /*tp_getattr*/
	0, /*tp_setattr*/
	PyObjHandle_Cmp, /*tp_compare*/
	PyObjHandle_Repr, /*tp_repr*/
	&PyObjHandleNumberMethods, /*tp_as_number*/
	0, /*tp_as_sequence*/
	0, /*tp_as_mapping*/
	0, /*tp_hash */
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro*/
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT, /*tp_flags*/
	0, /* tp_doc */
	0, /* tp_traverse */
	0, /* tp_clear */
	0, /* tp_richcompare */
	0, /* tp_weaklistoffset */
	0, /* tp_iter */
	0, /* tp_iternext */
	PyObjHandleMethods, /* tp_methods */
	0, /* tp_members */
	PyObjHandleGetSets, /* tp_getset */
	0, /* tp_base */
	0, /* tp_dict */
	0, /* tp_descr_get */
	0, /* tp_descr_set */
	0, /* tp_dictoffset */
	PyObjHandle_Init, /* tp_init */
	0, /* tp_alloc */
	PyObjHandle_New, /* tp_new */
};

bool ConvertObjHndl(PyObject* obj, objHndl* pHandleOut) {
	if (obj == Py_None) {
		*pHandleOut = 0;
		return true;
	}

	if (obj->ob_type != &PyObjHandleType) {
		PyErr_SetString(PyExc_TypeError, "Expected object handle.");
		return false;
	}

	*pHandleOut = GetSelf(obj)->handle;
	return true;
}

PyObject* PyObjHndl_Create(objHndl handle) {

	auto result = PyObject_New(PyObjHandle, &PyObjHandleType);
	result->handle = handle;
	if (handle) {
		result->id = objects.GetId(handle);
	} else {
		memset(&result->id, 0, sizeof(result->id));
	}

	return (PyObject*) result;

}

PyObject* PyObjHndl_CreateNull() {
	return PyObjHndl_Create(0);
}

objHndl PyObjHndl_AsObjHndl(PyObject* obj) {
	assert(PyObjHndl_Check(obj));
	return GetSelf(obj)->handle;
}

bool PyObjHndl_Check(PyObject* obj) {
	return obj->ob_type == &PyObjHandleType;
}
