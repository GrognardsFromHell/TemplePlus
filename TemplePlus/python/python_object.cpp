#include "stdafx.h"
#include "python_object.h"
#include "python_support.h"
#include "../maps.h"
#include "../inventory.h"
#include "../gamesystems/timeevents.h"
#include "../gamesystems/gamesystems.h"
#include "../reputations.h"
#include "../critter.h"
#include "../animgoals/anim.h"
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
#include "ui/ui_char_editor.h"
#include "pathfinding.h"
#include "secret_door.h"
#include "weapon.h"

#include <pybind11/embed.h>
#include <pybind11/cast.h>
#include <pybind11/stl.h>
#include "python_spell.h"

namespace py = pybind11;

struct PyObjHandle {
	PyObject_HEAD;
	ObjectId id;
	objHndl handle;
};

static PyObjHandle* GetSelf(PyObject* obj) {
	assert(PyObjHndl_Check(obj));

	auto self = (PyObjHandle*) obj;

	if (!self->handle && self->id) {
		self->handle = gameSystems->GetObj().GetHandleById(self->id);
	}

	if (!gameSystems->GetObj().IsValidHandle(self->handle)) {
		self->handle = gameSystems->GetObj().GetHandleById(self->id);
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
	objHndl handleA = objHndl::null;
	objHndl handleB = objHndl::null;
	if (objA->ob_type == &PyObjHandleType) {
		handleA = ((PyObjHandle*)objA)->handle;
	}
	if (objB->ob_type == &PyObjHandleType) {
		handleB = ((PyObjHandle*)objB)->handle;
	}

	if (handleA.handle < handleB.handle) {
		return -1;
	} else if (handleA.handle > handleB.handle) {
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
	if (self->id.IsPermanent()) {
		const auto& guid = self->id.body.guid;
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
		self->id.subtype = ObjectIdKind::Null;
		return 0;
	}

	if (!self->id) {
		// This is the null obj handle
		self->handle = 0;
		Py_RETURN_NONE;
	}

	if (guidContent == 0) {
		PyErr_SetString(PyExc_ValueError, "GUID type other than 0 is given, but GUID is missing.");
		self->handle = 0;
		self->id.subtype = ObjectIdKind::Null;
		return 0;
	}

	// Try parsing the GUID tuple
	self->id.subtype = ObjectIdKind::Permanent;
	auto& guid = self->id.body.guid;
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
		self->id.subtype = ObjectIdKind::Null;
		return 0;
	}

	// Finally look up the handle for the GUID
	self->handle = gameSystems->GetObj().GetHandleById(self->id);

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
	if ( critterSys.IsPC(self->handle))
	{	
		TimeEvent evt;
		evt.system = TimeEventType::PythonDialog;
		evt.params[0].handle = self->handle;
		evt.params[1].handle = target;
		evt.params[2].int32 = line;
		gameSystems->GetTimeEvent().Schedule(evt, 1);
		uiPicker.CancelPicker();
		
		// Temple+: added this, otherwise the combat continues briefly and goes on to the next combatant who can start an action
		// the dialog event callback (0x1014CED0) does this anyway, but 1ms later
		auto leader = party.GetConsciousPartyLeader();
		if (leader) {
			combatSys.forceEndedCombatNow = true;
			combatSys.CritterExitCombatMode(leader);
		}
		

	} else
	{
		// TODO: Add a "Party Leader Override" option that attempts to initiate dialogue with the Party Leader if possible
	}
	
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_Barter(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	logger->info("There was never any .barter() method in ToEE, you've been copypasting nonsense all along. Trolololo!");
	Py_RETURN_NONE;
}

#pragma region Python Obj Faction Manipulation
static PyObject * PyObjHandle_FactionHas(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int objFaction;
	if (!PyArg_ParseTuple(args, "i:faction_has", &objFaction)) {
		return nullptr;
	};
	return PyInt_FromLong(objects.factions.FactionHas(self->handle, objFaction));
};

static PyObject * PyObjHandle_FactionAdd(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int nFac;
	if (!PyArg_ParseTuple(args, "i:faction_add", &nFac)) {
		return nullptr;
	};

	if (nFac == 0){
		return PyInt_FromLong(0);
	}
	if (!objects.factions.FactionHas(self->handle, nFac)) {
		objects.factions.FactionAdd(self->handle, nFac);
	}

	return PyInt_FromLong(1);
};

#pragma endregion


static PyObject * PyObjHandle_FallDown(PyObject* obj, PyObject* args)
{
	auto self = GetSelf(obj);
	int fallDownArg = 73;
	if (!PyArg_ParseTuple(args, "|i:fall_down",  &fallDownArg)) {
		return 0;
	}
	if (fallDownArg > 75 || fallDownArg < 73) fallDownArg = 73;
	gameSystems->GetAnim().PushFallDown(self->handle, fallDownArg);
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

static PyObject* PyObjHandle_RefreshTurn(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);

	auto curSeq = *actSeqSys.actSeqCur;
	curSeq->tbStatus.hourglassState = 4;
	curSeq->tbStatus.surplusMoveDistance = 0;
	curSeq->tbStatus.tbsFlags = 0;

	Py_RETURN_NONE;
}


static PyObject* PyObjHandle_ItemFind(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle){
		return PyObjHndl_CreateNull();
	}
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
	if (!PyArg_ParseTuple(args, "O&i:objhndl.item_transfer_to_by_proto", &ConvertObjHndl, &target, &protoId)) {
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
		critterSys.TakeMoney(self->handle, 0, 0, 0, -copperAdj);
	} else {
		critterSys.GiveMoney(self->handle, 0, 0, 0, copperAdj);
	}

	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_CanSense(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	objHndl targetObj;
	if (!PyArg_ParseTuple(args, "O&:objhndl.can_sense", &ConvertObjHndl, &targetObj)) {
		return 0;
	}
	if (!targetObj)
		return PyInt_FromLong(0);

	return PyInt_FromLong(critterSys.CanSense(self->handle, targetObj));
}


static PyObject* PyObjHandle_CanSneakAttack(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle){
		return PyInt_FromLong(0);
	}

	objHndl targetObj;
	if (!PyArg_ParseTuple(args, "O&:objhndl.can_sneak_attack", &ConvertObjHndl, &targetObj)) {
		return 0;
	}
	if (!targetObj)
		return PyInt_FromLong(0);

	auto sneakAtkDice = d20Sys.D20QueryPython(self->handle, "Sneak Attack Dice");

	if (!sneakAtkDice)
		return PyInt_FromLong(0);
	if (d20Sys.d20Query(targetObj, DK_QUE_Critter_Is_Immune_Critical_Hits))
		return PyInt_FromLong(0);

	// Can't sneak attack if the target has concealment
	if (!critterSys.CanSense(self->handle, targetObj)) {
		return PyInt_FromLong(0);
	}
	
	auto isFlanked = combatSys.IsFlankedBy(targetObj, self->handle);

	auto result = (isFlanked 
		|| d20Sys.d20Query(targetObj, DK_QUE_SneakAttack)
		|| !critterSys.CanSense(targetObj, self->handle));

	return PyInt_FromLong(result);
}


static PyObject* PyObjHandle_CastSpell(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle){
		Py_RETURN_NONE;
	}

	uint32_t spellEnum;
	PickerArgs pickArgs;
	SpellPacketBody spellPktBody;
	SpellEntry spellEntry;
	objHndl targetObj = objHndl::null;
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
	spellPktBody.caster = caster;

	/*if (*actSeqSys.actSeqPickerActive){
		auto dummy = 1;
	}*/

	for (uint32_t i = 0; i < numSpells; i++) {
		if (!spellSys.spellCanCast(caster, spellEnum, classCodes[i], spellLevels[i]))
			continue;
		spellPktBody.spellKnownSlotLevel = spellLevels[i];
		spellPktBody.spellClass = classCodes[i];
		spellSys.SpellPacketSetCasterLevel(&spellPktBody);
		if (!spellSys.spellRegistryCopy(spellEnum, &spellEntry))
			continue;
		if (!spellSys.pickerArgsFromSpellEntry(&spellEntry, &pickArgs, caster, spellPktBody.casterLevel))
			continue;
		pickArgs.result = {0,};
		pickArgs.flagsTarget = (UiPickerFlagsTarget)(
			(uint64_t)pickArgs.flagsTarget | (uint64_t)pickArgs.flagsTarget & UiPickerFlagsTarget::LosNotRequired
			- (uint64_t)pickArgs.flagsTarget & UiPickerFlagsTarget::Range
		);

		// vanilla: was buggy, had & instead of this
		if ( pickArgs.IsBaseModeTarget(UiPickerType::Single)){ // static_cast<uint64_t>(pickArgs.modeTarget) & static_cast<uint64_t>(UiPickerType::Single)) {
			objects.loc->getLocAndOff(targetObj, &loc);
			uiPicker.SetSingleTarget(targetObj, &pickArgs);
		} else if (pickArgs.IsBaseModeTarget(UiPickerType::Multi)) {// static_cast<uint64_t>(pickArgs.modeTarget) & static_cast<uint64_t>(UiPickerType::Multi)) {
			objects.loc->getLocAndOff(targetObj, &loc);
			uiPicker.SetSingleTarget(targetObj, &pickArgs);
		} else if (pickArgs.IsBaseModeTarget(UiPickerType::Cone)) {//static_cast<uint64_t>(pickArgs.modeTarget) & static_cast<uint64_t>(UiPickerType::Cone)) {
			loc = objects.GetLocationFull(targetObj);
			uiPicker.SetConeTargets(&loc, &pickArgs);

		} else if (pickArgs.IsBaseModeTarget(UiPickerType::Area)) {//static_cast<uint64_t>(pickArgs.modeTarget) & static_cast<uint64_t>(UiPickerType::Area)) {
			if (spellEntry.spellRangeType == SRT_Personal)
				objects.loc->getLocAndOff(caster, &loc);
			else
				objects.loc->getLocAndOff(targetObj, &loc);
			uiPicker.GetListRange(&loc, &pickArgs);
		} else if (pickArgs.IsBaseModeTarget(UiPickerType::Personal)) {//static_cast<uint64_t>(pickArgs.modeTarget) & static_cast<uint64_t>(UiPickerType::Personal)) {
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

static PyObject* PyObjHandle_CanCastSpell(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	auto handle = self->handle;

	SpellStoreData spData;
	if (!PyArg_ParseTuple(args, "O&:objhndl.can_cast_spell", &ConvertSpellStore, &spData)) {
		return 0;
	}

	if (spellSys.spellCanCast(handle, spData.spellEnum, spData.classCode, spData.spellLevel) != TRUE) {
		return PyInt_FromLong(0);
	}

	if (spData.classCode == spellSys.GetSpellClass(stat_level_paladin) && d20Sys.d20Query(handle, DK_QUE_IsFallenPaladin)) {
		return PyInt_FromLong(0);
	}

	return PyInt_FromLong(1);
}

static PyObject* PyObjHandle_CanFindPathToObj(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	auto handle = self->handle;

	objHndl tgtObj;
	int flags;

	PathQuery pathQ;
	PathQueryResult pqr;

	// get target
	if (!PyArg_ParseTuple(args, "O&|i:objhndl.can_find_path_to_obj", &ConvertObjHndl, &tgtObj, &flags)) {
		return 0;
	}

	
	pathQ.critter = self->handle;
	pathQ.from = objSystem->GetObject(self->handle)->GetLocationFull();
	pathQ.targetObj = tgtObj;
	if (pathQ.critter == pathQ.targetObj)
		return PyInt_FromLong(0);

	const float fourPointSevenPlusEight = 4.714045f + 8.0f;
	pathQ.flags = static_cast<PathQueryFlags>(PathQueryFlags::PQF_TO_EXACT | PathQueryFlags::PQF_HAS_CRITTER | PathQueryFlags::PQF_800
		| PathQueryFlags::PQF_TARGET_OBJ | PathQueryFlags::PQF_ADJUST_RADIUS | PathQueryFlags::PQF_ADJ_RADIUS_REQUIRE_LOS);
	*((int*)&pathQ.flags) |= flags;

	auto reach = critterSys.GetReach(self->handle, D20A_UNSPECIFIED_MOVE);
	if (reach < 0.1) { reach = 3.0; }
	pathQ.distanceToTargetMin = 0.0;
	pathQ.tolRadius = reach * 12.0f - fourPointSevenPlusEight;

	auto nodeCount = pathfindingSys.FindPath(&pathQ, &pqr);
	
	auto pathLen = pathfindingSys.GetPathLength(&pqr);

	return PyInt_FromLong(static_cast<long>(pathLen));
}

static PyObject* PyObjHandle_FindPathToObj(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	auto handle = self->handle;

	objHndl tgtObj;
	int flags;

	PathQuery pathQ;
	PathQueryResult pqr;

	// get target
	if (!PyArg_ParseTuple(args, "O&|i:objhndl.find_path_to_obj", &ConvertObjHndl, &tgtObj, &flags)) {
		return 0;
	}


	pathQ.critter = self->handle;
	pathQ.from = objSystem->GetObject(self->handle)->GetLocationFull();
	pathQ.targetObj = tgtObj;
	if (pathQ.critter == pathQ.targetObj)
		return PyInt_FromLong(0);

	const float fourPointSevenPlusEight = 4.714045f + 8.0f;
	pathQ.flags = static_cast<PathQueryFlags>(PathQueryFlags::PQF_TO_EXACT | PathQueryFlags::PQF_HAS_CRITTER | PathQueryFlags::PQF_800
		| PathQueryFlags::PQF_TARGET_OBJ | PathQueryFlags::PQF_ADJUST_RADIUS);
	*((int*)&pathQ.flags) |= flags;

	auto reach = critterSys.GetReach(self->handle, D20A_UNSPECIFIED_MOVE);
	if (reach < 0.1) { reach = 3.0; }
	pathQ.distanceToTargetMin = 0.0;
	pathQ.tolRadius = reach * 12.0f - fourPointSevenPlusEight;

	auto nodeCount = pathfindingSys.FindPath(&pathQ, &pqr);
	if (!pqr.IsComplete())
		Py_RETURN_NONE;
	
	py::object pyLoc = py::cast(pqr.to);
	pyLoc.inc_ref();
	return pyLoc.ptr();
}

static PyObject* PyObjHandle_CanMelee(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	objHndl targetObj;
	if (!PyArg_ParseTuple(args, "O&:objhndl.can_melee", &ConvertObjHndl, &targetObj)) {
		return 0;
	}
	if (!targetObj)
		return PyInt_FromLong(0);

	auto result = combatSys.CanMeleeTarget(self->handle, targetObj);
	
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_SkillLevelGet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);

	if (!self->handle)
		return 0;

	objHndl handle = objHndl::null;
	SkillEnum skillId;
	if (PyTuple_Size(args) == 2) {
		if (!PyArg_ParseTuple(args, "O&i:objhndl.skill_level_get", &ConvertObjHndl, &handle, &skillId)) {
			return 0;
		}
	} else {
		if (!PyArg_ParseTuple(args, "i:objhndl.skill_level_get", &skillId)) {
			return 0;
		}
	}

	if (config.dialogueUseBestSkillLevel && pythonObjIntegration.IsInDialogGuard()) {
		auto maxSkillLevel = -1000;
		for (uint32_t i = 0; i < party.GroupPCsLen(); ++i) {
			auto pc = party.GroupPCsGetMemberN(i);
			if (critterSys.IsDeadNullDestroyed(pc) || critterSys.IsDeadOrUnconscious(pc) || !objSystem->IsValidHandle(pc))
				continue;
			auto skillLevel = dispatch.dispatch1ESkillLevel(pc, skillId, nullptr, handle, 1);
			if (skillLevel > maxSkillLevel)
				maxSkillLevel = skillLevel;
		}
		return PyInt_FromLong(maxSkillLevel);
	}
	
	auto skillLevel = dispatch.dispatch1ESkillLevel(self->handle, skillId, nullptr, handle, 1);
	return PyInt_FromLong(skillLevel);
}

static PyObject* PyObjHandle_SkillRanksGet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);

	if (!self->handle)
		return 0;

	objHndl handle = objHndl::null;
	SkillEnum skillId;
	
	if (!PyArg_ParseTuple(args, "i:objhndl.skill_ranks_get", &skillId)) {
		return 0;
	}
	
	auto skillRanks = critterSys.SkillBaseGet(self->handle, skillId);

	return PyInt_FromLong(skillRanks);
}

static PyObject* PyObjHandle_SkillRanksSet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);

	if (!self->handle)
		return 0;
	int skillRanks;
	SkillEnum skillId;

	if (!PyArg_ParseTuple(args, "ii:objhndl.skill_ranks_set", &skillId, &skillRanks)) {
		return 0;
	}
	gameSystems->GetObj().GetObject(self->handle)->SetInt32(obj_f_critter_skill_idx, skillId, skillRanks*2);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_SkillRoll(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);

	if (!self->handle)
		return 0;

	SkillEnum skillId;
	int dc, flags =0;
	if (!PyArg_ParseTuple(args, "ii|i:objhndl.skill_roll", &skillId, &dc, &flags)) {
		return 0;
	}

	int deltaFromDc;
	auto result = skillSys.SkillRoll(self->handle, skillId, dc, &deltaFromDc, flags);

	return PyInt_FromLong(result);
}
static PyObject* PyObjHandle_SkillRollDelta(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);

	if (!self->handle)
		return 0;

	SkillEnum skillId;
	int dc, flags = 0;
	if (!PyArg_ParseTuple(args, "ii|i:objhndl.skill_roll_delta", &skillId, &dc, &flags)) {
		return 0;
	}

	int deltaFromDc;
	auto result = skillSys.SkillRoll(self->handle, skillId, dc, &deltaFromDc, flags);

	return PyInt_FromLong(deltaFromDc);
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

static PyObject*PyObjHandle_GetItemWearFlags(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	auto res = objects.GetItemWearFlags(self->handle);

	return PyInt_FromLong(res);
}

static PyObject* PyObjHandle_GetLevelForSpellSelection(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	Stat classCode;
	if (!PyArg_ParseTuple(args, "i:objhndl:get_level_for_spell_selection", &classCode)) {
		return 0;
	}

	//This is the level for selecting spells on levelup.  It does not include practiced spell caster.
	auto res = dispatch.DispatchGetBaseCasterLevel(self->handle, classCode);

	return PyInt_FromLong(res);
}

static PyObject* PyObjHandle_GetMaxDexBonus(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	auto res = GetMaxDexBonus(self->handle);

	return PyInt_FromLong(res);
}

static PyObject* PyObjHandle_GetNumSpellsPerDay(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	Stat spellClass;
	int spLvl;
	if (!PyArg_ParseTuple(args, "ii:objhndl:get_num_spells_per_day", &spellClass, &spLvl)) {
		return 0;
	}

	auto res = spellSys.GetNumSpellsPerDay(self->handle, spellSys.GetCastingClass(spellClass), spLvl);

	return PyInt_FromLong(res);
}

static PyObject* PyObjHandle_GetNumSpellsUsed(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	Stat spellClass;
	int spLvl;
	if (!PyArg_ParseTuple(args, "ii:objhndl:get_num_spells_used", &spellClass, &spLvl)) {
		return 0;
	}

	auto res = spellSys.NumSpellsInLevel(self->handle, obj_f_critter_spells_cast_idx, spellClass, spLvl);

	return PyInt_FromLong(res);
}

static PyObject* PyObjHandle_SpontaneousSpellsRemaining(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	Stat spellClass;
	int spLvl;
	if (!PyArg_ParseTuple(args, "ii:objhndl:spontaneous_spells_remaining", &spellClass, &spLvl)) {
		return 0;
	}

	const auto spellsPerDay = spellSys.GetNumSpellsPerDay(self->handle, spellSys.GetCastingClass(spellClass), spLvl);
	const auto spellUsed = spellSys.NumSpellsInLevel(self->handle, obj_f_critter_spells_cast_idx, spellClass, spLvl);
	const bool res = spellsPerDay > spellUsed;
	return PyInt_FromLong(res?1:0);
}

static PyObject* PyObjectHandle_SpontaneousSpellsRemove(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	int percent = 100;
	if (!PyArg_ParseTuple(args, "|i:objhndl.spontaneous_spells_remove", &percent)) {
		Py_RETURN_NONE;
	}
	spellSys.DeductSpontaneous(self->handle, static_cast<Stat>(-1), percent);
	Py_RETURN_NONE;
}

// turns out you could already get this via .stat_base_get(stat_attack_bonus). Leaving it for backward compatibility...
static PyObject* PyObjHandle_GetBaseAttackBonus(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	auto bab = critterSys.GetBaseAttackBonus(self->handle);

	return PyInt_FromLong(bab);
}

static PyObject* PyObjHandle_StatLevelGet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	Stat stat;
	int statArg = 0;
	if (!PyArg_ParseTuple(args, "i|i:objhndl:stat_level_get", &stat, &statArg)) {
		return 0;
	}
	
	if (PyTuple_Size(args) >=2)
		return PyInt_FromLong(objects.StatLevelGet(self->handle, stat, statArg)); // WIP currently just handles stat_caster_level expansion

	auto statLevel = objects.StatLevelGet(self->handle, stat);

	return PyInt_FromLong(statLevel);
}

static PyObject* PyObjHandle_StatLevelGetBase(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	Stat stat;
	int statArg;
	if (!PyArg_ParseTuple(args, "i|i:objhndl:stat_level_get_base", &stat, &statArg)) {
		return 0;
	}

	return PyInt_FromLong(objects.StatLevelGetBase(self->handle, stat));
}

static PyObject* PyObjHandle_StatLevelSetBase(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
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
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	
	if (critterSys.FollowerAtMax()) {
		return PyInt_FromLong(false);
	}

	objHndl follower;
	if (!PyArg_ParseTuple(args, "O&:objhndl.follower_add", &ConvertObjHndl, &follower)) {
		return 0;
	}

	auto result = critterSys.AddFollower(follower, self->handle, 1, false);
	uiSystems->GetParty().Update();
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_FollowerRemove(PyObject* obj, PyObject* args) {
	objHndl follower;
	if (!PyArg_ParseTuple(args, "O&:objhndl.follower_remove", &ConvertObjHndl, &follower)) {
		return 0;
	}

	auto result = critterSys.RemoveFollower(follower, 1);
	uiSystems->GetParty().Update();
	return PyInt_FromLong(result);
}


static PyObject* PyObjHandle_FollowerAtMax(PyObject* obj, PyObject* args) {
	auto res = critterSys.FollowerAtMax();
	return PyInt_FromLong(res);
}
 
static PyObject* PyObjHandle_AiFollowerAdd(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	objHndl follower;
	if (!PyArg_ParseTuple(args, "O&:objhndl.ai_follower_add", &ConvertObjHndl, &follower)) {
		return 0;
	}

	auto result = critterSys.AddFollower(follower, self->handle, 1, true);
	uiSystems->GetParty().Update();
	return PyInt_FromLong(result);
}


static PyObject * PyObjHandle_RemoveFromAllGroups(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl dude;
	if (!PyArg_ParseTuple(args, "O&:objhndl.obj_remove_from_all_groups", &ConvertObjHndl, &dude)) {
		return 0;
	}
	auto result = party.ObjRemoveFromAllGroupArrays(dude);
	uiSystems->GetParty().Update();
	return PyInt_FromLong(result);
};

static PyObject * PyObjHandle_PCAdd(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	objHndl dude;
	if (!PyArg_ParseTuple(args, "O&:objhndl.obj_remove_from_all_groups", &ConvertObjHndl, &dude)) {
		return 0;
	}
	auto result = party.AddToPCGroup(self->handle);
	uiSystems->GetParty().Update();
	return PyInt_FromLong(result);
};



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
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	objHndl target;
	if (!PyArg_ParseTuple(args, "O&:objHndl.has_los", &ConvertObjHndl, &target)) {
		return 0;
	}
	auto obstacles = critterSys.HasLineOfSight(self->handle, target);
	return PyInt_FromLong(obstacles == 0);
}



// CanSee and HasLos are the same
static PyObject* PyObjHandle_CanHear(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	objHndl target;
	int tileRangeIdx = -1;
	if (!PyArg_ParseTuple(args, "O&|i:objHndl.can_hear", &ConvertObjHndl, &target, &tileRangeIdx)) {
		return 0;
	}
	if (!target) {
		return PyInt_FromLong(0);
	}

	if (tileRangeIdx < 0 || tileRangeIdx > 3) {
		
		tileRangeIdx = 
			(!critterSys.IsMovingSilently(target)
			&& !critterSys.IsConcealed(target)) ? 1 : 0;
	}

	auto obstacles = aiSys.CannotHear(self->handle, target, tileRangeIdx);
	return PyInt_FromLong(obstacles == 0);
}

static PyObject* PyObjHandle_CanBlindsee(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	objHndl target;
	if (!PyArg_ParseTuple(args, "O&:objHndl.can_blindsee", &ConvertObjHndl, &target)) {
		return 0;
	}
	if (!target) {
		return PyInt_FromLong(0);
	}
	auto result = critterSys.CanSeeWithBlindsight(self->handle, target) ? 1 : 0;
	return PyInt_FromLong(result);
}


/*
	Pretty stupid name. Checks if the critter currently wears an item with the
	given name id.
*/
static PyObject* PyObjHandle_HasWielded(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	int nameId;
	if (!PyArg_ParseTuple(args, "i:objhndl.has_wielded", &nameId)) {
		return 0;
	}

	for (auto i = 0; i < (int) EquipSlot::Count; ++i) {
		auto item = critterSys.GetWornItem(self->handle, (EquipSlot) i);
		if (item && objects.GetNameId(item) == nameId) {
			return PyInt_FromLong(1);
		}
	}

	return PyInt_FromLong(0);
}

static PyObject* PyObjHandle_HasItem(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	int nameId;
	if (!PyArg_ParseTuple(args, "i:objhndl.has_item", &nameId)) {
		return 0;
	}

	bool hasItem = !!inventory.FindItemByName(self->handle, nameId);
	return PyInt_FromLong(hasItem);
}

static PyObject* PyObjHandle_ItemWornAt(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	EquipSlot slot;
	if (!PyArg_ParseTuple(args, "i:objhndl.item_worn_at", &slot)) {
		return 0;
	}

	auto item = critterSys.GetWornItem(self->handle, slot);
	return PyObjHndl_Create(item);
}




static PyObject * PyObjHandle_InventoryItem(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyObjHndl_CreateNull();
	}

	objHndl objHnd = self->handle;
	bool bRetunProtos = 0;
	int invFieldType = inventory.GetInventoryListField(self->handle);
	int n = 0;
	if (PyArg_ParseTuple(args, "i", &n)){
		int nMax = 199; // PLACEHOLDER!
		if (invFieldType == obj_f_container_inventory_list_idx){
			nMax = CONTAINER_MAX_ITEMS;
		}
		if (n < nMax){
			return PyObjHndl_Create(objects.inventory.GetItemAtInvIdx(objHnd, n));
		}

	}
	return PyObjHndl_Create(objHndl::null);

};


static PyObject* PyObjHandle_ArcaneSpellLevelCanCast(PyObject* obj, PyObject* args){
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	auto arcaneSpellLvlMax = 0;

	critterSys.GetSpellLvlCanCast(self->handle, SpellSourceType::Arcane, SpellReadyingType::Any);

	for (auto it: d20ClassSys.classEnums){
		auto classEnum = (Stat)it;
		if (d20ClassSys.IsArcaneCastingClass(classEnum)){
			arcaneSpellLvlMax = max(arcaneSpellLvlMax , spellSys.GetMaxSpellLevel(self->handle, classEnum, 0));
		}
	}
	


	return PyInt_FromLong(arcaneSpellLvlMax);
}

static PyObject* PyObjHandle_ArcaneSpontaniousSpellLevelCanCast(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	auto arcaneSpontaniousSpellLvlMax = 0;

	critterSys.GetSpellLvlCanCast(self->handle, SpellSourceType::Arcane, SpellReadyingType::Any);

	for (auto it : d20ClassSys.classEnums) {
		auto classEnum = (Stat)it;
		if (d20ClassSys.IsArcaneCastingClass(classEnum) && d20ClassSys.IsNaturalCastingClass(classEnum)) {
			arcaneSpontaniousSpellLvlMax = max(arcaneSpontaniousSpellLvlMax, spellSys.GetMaxSpellLevel(self->handle, classEnum, 0));
		}
	}

	return PyInt_FromLong(arcaneSpontaniousSpellLvlMax);
}

static PyObject* PyObjHandle_ArcaneVancianSpellLevelCanCast(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	auto arcaneVancianSpellLvlMax = 0;

	critterSys.GetSpellLvlCanCast(self->handle, SpellSourceType::Arcane, SpellReadyingType::Any);

	for (auto it : d20ClassSys.classEnums) {
		auto classEnum = (Stat)it;
		if (d20ClassSys.IsArcaneCastingClass(classEnum) && d20ClassSys.IsVancianCastingClass(classEnum)) {
			arcaneVancianSpellLvlMax = max(arcaneVancianSpellLvlMax, spellSys.GetMaxSpellLevel(self->handle, classEnum, 0));
		}
	}

	return PyInt_FromLong(arcaneVancianSpellLvlMax);
}


static PyObject* PyObjHandle_DivineSpellLevelCanCast(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	auto divineSpellLvlMax = 0;

	critterSys.GetSpellLvlCanCast(self->handle, SpellSourceType::Divine, SpellReadyingType::Any);
	
	for (auto it : d20ClassSys.classEnums) {
		auto classEnum = (Stat)it;
		if (d20ClassSys.IsDivineCastingClass(classEnum)) {
			divineSpellLvlMax = max(divineSpellLvlMax, spellSys.GetMaxSpellLevel(self->handle, classEnum, 0));
		}
	}


	return PyInt_FromLong(divineSpellLvlMax);
}


static PyObject* PyObjHandle_SpontaneousSpellLevelCanCast(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	auto spontCastLvl = 0;

	//critterSys.GetSpellLvlCanCast(self->handle, SpellSourceType::Arcane, SpellReadyingType::Any);

	for (auto it : d20ClassSys.classEnums) {
		auto classEnum = (Stat)it;
		if (d20ClassSys.IsNaturalCastingClass(classEnum)) {
			spontCastLvl = max(spontCastLvl, spellSys.GetMaxSpellLevel(self->handle, classEnum, 0));
		}
	}



	return PyInt_FromLong(spontCastLvl);
}


static PyObject* PyObjHandle_Attack(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		logger->warn("Python attack called with null object");
		Py_RETURN_NONE;
	}

	objHndl target;
	int flags = 2;
	int rangeType = 1;
	if (!PyArg_ParseTuple(args, "O&|ii:objhndl.attack", &ConvertObjHndl, &target, &rangeType, &flags)) {
		return 0;
	}
	if (rangeType < 0)
		rangeType = 0;
	else if (rangeType > 3)
		rangeType = 3;

	if (!target){
		logger->warn("Python attack called with null target");
		Py_RETURN_NONE;
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
	if (!self->handle) {
		logger->warn("Python turn_towards called with OBJ_HANDLE_NULL object");
		Py_RETURN_NONE;
	}
	
	objHndl target;
	if (!PyArg_ParseTuple(args, "O&:objhndl.turn_towards", &ConvertObjHndl, &target)) {
		return 0;
	}
	if (!target){
		logger->warn("Python turn_towards called with OBJ_HANDLE_NULL target");
		Py_RETURN_NONE;
	}
	auto relativeAngle = objects.GetRotationTowards(self->handle, target);
	if (!objSystem->GetObject(self->handle)->IsCritter()){
		objects.SetRotation(self->handle, relativeAngle);
		Py_RETURN_NONE;
	}
	gameSystems->GetAnim().PushRotate(self->handle, relativeAngle);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_TurnTowardsLoc(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		logger->warn("Python turn_towards_loc called with OBJ_HANDLE_NULL object");
		Py_RETURN_NONE;
	}

	LocAndOffsets tLoc;
	if (!PyArg_ParseTuple(args, "L|ff:objhndl.turn_towards_loc", &tLoc.location, &tLoc.off_x, &tLoc.off_y)) {
		return 0;
	}

	auto relativeAngle = objects.GetRotationTowardsLoc(self->handle, tLoc);
	if (!objSystem->GetObject(self->handle)->IsCritter()){
		objects.SetRotation(self->handle, relativeAngle);
		Py_RETURN_NONE;
	}
	gameSystems->GetAnim().PushRotate(self->handle, relativeAngle);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_TripCheck(PyObject* obj, PyObject* args){
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	objHndl target = objHndl::null;
	if (!PyArg_ParseTuple(args, "O&:objhndl.trip_check", &ConvertObjHndl, &target)) {
		return 0;
	}

	if (!target)
		return PyInt_FromLong(0);

	if (combatSys.TripCheck(self->handle, target)) {
		return PyInt_FromLong(1);
	}
	else {
		return PyInt_FromLong(0);
	}
}

static PyObject* PyObjHandle_FloatLine(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle)
		Py_RETURN_NONE;
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

	uiDialog->ShowTextBubble(dlgState.npc, dlgState.pc, dlgState.npcLineText, dlgState.speechId);

	dialogScripts.Free(dlgState.dialogHandle);

	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_Damage(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle)
		Py_RETURN_NONE;
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
	if (!self->handle)
		Py_RETURN_NONE;
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

static PyObject* PyObjHandle_DealAttackDamage(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle)
		Py_RETURN_NONE;
	objHndl attacker;
	int d20data = 0;
	D20ActionType actionType = D20A_NONE;
	D20CAF flags;
	if (!PyArg_ParseTuple(args, "O&iii:objhndl.deal_attack_damage", &ConvertObjHndl, &attacker, &d20data,&flags, &actionType)) {
		return 0;
	}
	damage.DealAttackDamage(attacker, self->handle, d20data, flags, actionType);
	Py_RETURN_NONE;
}


static PyObject* PyObjHandle_Heal(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle)
		Py_RETURN_NONE;

	objHndl healer;
	Dice dice;
	D20ActionType actionType = D20A_NONE;
	int dummySpellId = 0; // not really used, just makign the code tolerate 4 args

	if (!PyArg_ParseTuple(args, "O&O&|ii:objhndl.heal", &ConvertObjHndl, &healer, &ConvertDice, &dice, &actionType, &dummySpellId)) {
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

static PyObject* PyObjHandle_SpellDamageWeaponlike(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl attacker;
	Dice dice;
	DamageType type;
	int attackPowerType = 0;
	int reduction = 100;
	D20ActionType actionType = D20A_NONE;
	int spellId = 0, projectileIdx = 0;
	D20CAF flags;
	if (!PyArg_ParseTuple(args, "O&iO&|iiiiii:objhndl.spell_damage_weaponlike", &ConvertObjHndl, &attacker, &type, &ConvertDice, &dice, &attackPowerType, &reduction, &actionType, &spellId, &flags, &projectileIdx)) {
		return 0;
	}
	// Line 105: Saving Throw
	damage.DealWeaponlikeSpellDamage(self->handle, attacker, dice, type, attackPowerType, reduction, 105, actionType, spellId, flags, projectileIdx);
	Py_RETURN_NONE;
}


static PyObject* PyObjHandle_StealFrom(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	objHndl target;
	if (!PyArg_ParseTuple(args, "O&:objhndl.steal_from", &ConvertObjHndl, &target)) {
		return 0;
	}

	gameSystems->GetAnim().PushUseSkillOn(self->handle, target, skill_pick_pocket);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_ReputationHas(PyObject* obj, PyObject* args) {
	int reputationId;
	if (!PyArg_ParseTuple(args, "i:objhndl.reputation_has", &reputationId)) {
		return 0;
	}
	auto has = partyReputation.Has(reputationId);
	auto result = PyInt_FromLong(has);
	return result;
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
	size_t j = 0;
	for (unsigned int i = 0; j < cond->numArgs; ++i) {
		// no more arguments
		if (static_cast<uint32_t>(PyTuple_GET_SIZE(args)) <= i+1)
			break;

		auto item = PyTuple_GET_ITEM(args, i + 1);
		if (PyLong_Check(item)){
			condArgs[j++] = PyLong_AsLong(item);
			continue;
		}

		if (PyInt_Check(item)) {
			condArgs[j++] = PyInt_AsLong(item);
			continue;
		}

		objHndl obj;
		if (ConvertObjHndl(item, &obj)) {
			if (j+1 < cond->numArgs) {
				condArgs[j++] = obj.GetHandleUpper();
				condArgs[j++] = obj.GetHandleLower();
				continue;
			} else {
				PyErr_Format(PyExc_ValueError, "Object argument %d for condition %s cannot fit into %d arguments. Objects take two arguments.", i + 1, condName, cond->numArgs);
				return false;
			}
		}

		auto itemRepr = PyObject_Repr(item);
		PyErr_Format(PyExc_ValueError, "Argument %d for condition %s (requires %d args) is not of type int, long or object: %s",
								 i + 1, condName, cond->numArgs, PyString_AsString(itemRepr));
		Py_DECREF(itemRepr);
		return false;
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
	
	auto parent = inventory.GetParent(self->handle);
	if (parent && objSystem->IsValidHandle(parent)) {
		d20StatusSys.initItemConditions(parent);
	}
	
	return PyInt_FromLong(1);
}

static PyObject* PyObjHandle_ItemConditionRemove(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle || !objSystem->IsValidHandle(self->handle)) {
		return PyInt_FromLong(0);
	}

	CondStruct* cond;
	auto spellId = -1;
	
	{
		char* condName = nullptr;
		if (!PyArg_ParseTuple(args, "s|i:objhndl.item_condition_remove", &condName, &spellId)) {
			return PyInt_FromLong(0);
		}
		cond = conds.GetByName(condName);
		if (!cond) {
			PyErr_Format(PyExc_ValueError, "Unknown condition name: %s", condName);
			return PyInt_FromLong(0);
		}
	}
	

	auto condId = conds.hashmethods.GetCondStructHashkey(cond);
	inventory.RemoveWielderCond(self->handle, condId, spellId);

	auto parent = inventory.GetParent(self->handle);
	if (parent && objSystem->IsValidHandle(parent)) {
		d20StatusSys.initItemConditions(parent);
	}

	return PyInt_FromLong(1);
}




static PyObject* PyObjHandle_ItemConditionHas(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	char* condName = nullptr;
	if (!PyArg_ParseTuple(args, "s:objhndl.item_has_condition", &condName)) {
		return 0;
	}
	if (!condName) {
		return 0;
	}

	auto condId = conds.hashmethods.StringHash(condName);
	auto cond = conds.GetById(condId);
	if (!cond) {
		logger->warn("item_has_condition: Nonexistant condition {}", condName);
		return PyInt_FromLong(0);
	}

	auto result = inventory.ItemHasCondition(self->handle, condId);
	
	return PyInt_FromLong(result ? TRUE: FALSE);
}


static PyObject* PyObjHandle_ItemD20Query(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	if (PyTuple_GET_SIZE(args) < 1) {
		PyErr_SetString(PyExc_RuntimeError, "item_d20_query called with no arguments!");
		return PyInt_FromLong(0);
	}

	PyObject* arg = PyTuple_GET_ITEM(args, 0);
	/*if (PyString_Check(arg)) {
		auto argString = fmt::format("{}", PyString_AsString(arg));
		return PyInt_FromLong(d20Sys.D20QueryPython(self->handle, argString));
	}*/

	int queryKey;
	if (!PyArg_ParseTuple(args, "i:objhndl.item_d20_query", &queryKey)) {
		return 0;
	}
	auto dispatcherKey = (D20DispatcherKey)(DK_QUE_Helpless + queryKey);
	auto result = dispatch.DispatchItemQuery(self->handle, dispatcherKey);
	return PyInt_FromLong(result);
}


static PyObject* PyObjHandle_ConditionAddWithArgs(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);

	CondStruct* cond;
	vector<int> condArgs;
	if (!ParseCondNameAndArgs(args, cond, condArgs)) {
		return 0;
	}

	auto result = conds.AddTo(self->handle, cond, condArgs);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_ConditionsGet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);

	auto dispatcher = objects.GetDispatcher(self->handle);
	if (!dispatcher)
		Py_RETURN_NONE;

	long kind = 0;
	if (PyTuple_GET_SIZE(args) > 0) {
		PyObject* arg = PyTuple_GET_ITEM(args, 0);
		if (PyInt_Check(arg) || PyLong_Check(arg)) {
			kind = PyInt_AsLong(arg);
		}
	}

	// default to all conditions by being distinct from 0 and 1
	uint32_t active = 0xff;
	if (PyTuple_GET_SIZE(args) > 1) {
		PyObject* arg = PyTuple_GET_ITEM(args, 1);
		if (PyInt_Check(arg) || PyLong_Check(arg)) {
			active = PyInt_AsLong(arg);
		}
	}

	CondNode* node = dispatcher->conditions;
	if (kind == 1) {
		node = dispatcher->permanentMods;
	} else
	if (kind == 2) {
		node = dispatcher->itemConds;
	}
	if (!node)
		Py_RETURN_NONE;

	auto list = PyList_New(0);
	while (node) {
		// if active == 1 and inactive flag is set, skip
		// if active == 0 and inactive flag isn't set, skip
		// if active is anything else, don't skip
		if ((node->flags & 1) == active) {
			node = node->nextCondNode;
			continue;
		}

		auto cname = PyString_FromString(node->condStruct->condName);
		auto tuple = PyTuple_New(2);
		PyTuple_SET_ITEM(tuple, 0, cname);
		if (node->condStruct->numArgs) {
			auto tupleArgs = PyTuple_New(node->condStruct->numArgs);
			for (unsigned int i = 0; i < node->condStruct->numArgs; i++)
			{
				PyTuple_SET_ITEM(tupleArgs, i, PyInt_FromSize_t(node->args[i]));
			}
			PyTuple_SET_ITEM(tuple, 1, tupleArgs);
		}
		
		PyList_Append(list, tuple);
		Py_DecRef(tuple);
		
		node = node->nextCondNode;
	}
	return list;
}


static PyObject* PyObjHandle_FavoredEnemy(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle)
		return PyInt_FromLong(0);

	objHndl critter;
	if (!PyArg_ParseTuple(args, "O&:objhndl.is_favored_enemy", &ConvertObjHndl, &critter)) {
		return 0;
	}

	if (!critter)
		return PyInt_FromLong(0);

	auto category = critterSys.GetCategory(critter);
	int result = 0;

	switch (category) {
		case mc_type_aberration:
			result = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_ABERRATION);
			break;
		case mc_type_animal:
			result = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_ANIMAL);
			break;
		case mc_type_beast:
			result = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_BEAST);
			break;
		case mc_type_construct:
			result = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_CONSTRUCT);
			break;
		case mc_type_elemental:
			result = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_ELEMENTAL);
			break;
		case mc_type_giant:
			result = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_GIANT);
			break;
		case mc_type_humanoid:
			{
				//Check the subtype (be careful since it could fall into more than one category, if so return the highest feat count)
				int flags = critterSys.GetSubcategoryFlags(critter);
				if (flags & mc_subtype_goblinoid) {
					int count = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_HUMANOID_GOBLINOID);
					if (count > result) result = count;
				}
				if (flags & mc_subtype_reptilian) {
					int count = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_HUMANOID_REPTILIAN);
					if (count > result) result = count;
				}
				if (flags & mc_subtype_dwarf) {
					int count = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_HUMANOID_DWARF);
					if (count > result) result = count;
				}
				if (flags & mc_subtype_elf) {
					int count = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_HUMANOID_ELF);
					if (count > result) result = count;
				}
				if (flags & mc_subtype_gnoll) {
					int count = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_HUMANOID_GNOLL);
					if (count > result) result = count;
				}
				if (flags & mc_subtype_gnome) {
					int count = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_HUMANOID_GNOME);
					if (count > result) result = count;
				}
				if (flags & mc_subtype_halfling) {
					int count = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_HUMANOID_HALFLING);
					if (count > result) result = count;
				}
				if (flags & mc_subtype_orc) {
					int count = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_HUMANOID_ORC);
					if (count > result) result = count;
				}
				if (flags & mc_subtype_human) {
					int count = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_HUMANOID_HUMAN);
					if (count > result) result = count;
				}
			}
			break;
		case mc_type_ooze:
			result = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_OOZE);
			break;
		case mc_type_plant:
			result = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_PLANT);
			break;
		case mc_type_shapechanger:
			result = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_SHAPECHANGER);
			break;
		case mc_type_vermin:
			result = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_VERMIN);
			break;
		case mc_type_dragon:
			result = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_DRAGON);
			break;
		case mc_type_magical_beast:
			result = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_BEAST);
			break;
		case mc_type_monstrous_humanoid:
			result = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_MONSTROUS_HUMANOID);
			break;
		case mc_type_outsider:
			{
				//Check the subtype (since it could fall into more than one category be careful to return the highest count)
				int flags = critterSys.GetSubcategoryFlags(critter);
				if (flags & mc_subtype_evil) {
					int count = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_OUTSIDER_EVIL);
					if (count > result) result = count;
				}
				if (flags & mc_subtype_good) {
					int count = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_OUTSIDER_GOOD);
					if (count > result) result = count;
				}
				if (flags & mc_subtype_lawful) {
					int count = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_OUTSIDER_LAWFUL);
					if (count > result) result = count;
				}
				if (flags & mc_subtype_chaotic) {
					int count = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_OUTSIDER_CHAOTIC);
					if (count > result) result = count;
				}
			}
			break;
		case mc_type_fey:
			result = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_FEY);
			break;
		case mc_type_undead:
			result = feats.HasFeatCountByClass(self->handle, FEAT_FAVORED_ENEMY_UNDEAD);
			break;
	}
	
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_IsFlankedBy(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle)
		return PyInt_FromLong(0);

	objHndl critter;
	if (!PyArg_ParseTuple(args, "O&:objhndl.is_flanked_by", &ConvertObjHndl, &critter)) {
		return 0;
	}

	if (!critter)
		return PyInt_FromLong(0);

	auto result = combatSys.IsFlankedBy(self->handle, critter);
	return PyInt_FromLong(result);
}


static PyObject* PyObjHandle_IsFriendly(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle)
		return PyInt_FromLong(0);

	objHndl pc;
	if (!PyArg_ParseTuple(args, "O&:objhndl.is_friendly", &ConvertObjHndl, &pc)) {
		return 0;
	}

	auto result = critterSys.IsFriendly(pc, self->handle);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_FadeTo(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle)
		Py_RETURN_NONE;

	int targetOpacity, transitionTimeInMs, unk1, unk2 = 0;
	if (!PyArg_ParseTuple(args, "iii|i:objhndl.fade_to", &targetOpacity, &transitionTimeInMs, &unk1, &unk2)) {
		return 0;
	}

	objects.FadeTo(self->handle, targetOpacity, transitionTimeInMs, unk1, unk2);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_Move(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle)
		Py_RETURN_NONE;

	LocAndOffsets newLoc;
	newLoc.off_x = 0;
	newLoc.off_y = 0;
	if (!PyArg_ParseTuple(args, "L|ff:objhndl.move", &newLoc.location, &newLoc.off_x, &newLoc.off_y)) {
		if (!PyArg_ParseTuple(args, "ii|ff:objhndl.move", &newLoc.location.locx, &newLoc.location.locy, &newLoc.off_x, &newLoc.off_y)) 
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


	MesFile::Content content;
	try {
		content = MesFile::ParseFile(mesFilename);
	} catch (TempleException&) {
		PyErr_Format(PyExc_IOError, "Could not open mes file %s", mesFilename);
		return 0;
	}

	auto it = content.find(mesLineKey);
	if (it == content.end()) {
		PyErr_Format(PyExc_IOError, "Could not find line %d in mes file %s.", mesLineKey, mesFilename);
		return 0;
	}
	if (!self->handle) {
		PyErr_Format(PyExc_IOError, "PyObjHandle_FloatMesFileLine: called with null handle.");
		return 0;
	}
	if (!objSystem->GetObject(self->handle)) {
		PyErr_Format(PyExc_IOError, "PyObjHandle_FloatMesFileLine: Called with invalid Object.");
		return 0;
	}
	floatSys.floatMesLine(self->handle, 1, colorId, it->second.c_str());
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_FloatTextLine(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	PyObject* line;
	auto colorId = FloatLineColor::White;
	if (!PyArg_ParseTuple(args, "O|i:objhndl.float_text_line", &line, &colorId)) {
		return 0;
	}
	if (!self->handle) {
		PyErr_Format(PyExc_IOError, "PyObjHandle_FloatMesFileLine: called with null handle.");
		return 0;
	}
	if (!objSystem->GetObject(self->handle)) {
		PyErr_Format(PyExc_IOError, "PyObjHandle_FloatMesFileLine: Called with invalid Object.");
		return 0;
	}
	if (line && PyString_Check(line)) {
		floatSys.floatMesLine(self->handle, 1, colorId, fmt::format("{}", PyString_AsString(line)).c_str());
	}

	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_GetAmmoUsed(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyObjHndl_CreateNull();
	}

	auto result = combatSys.CheckRangedWeaponAmmo(self->handle);	
	return PyObjHndl_Create(result);
}


// Generic methods to get/set/clear flags
template <obj_f flagsField>
static PyObject* GetFlags(PyObject* obj, PyObject*) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	auto flags = objects.getInt32(self->handle, flagsField);
	return PyInt_FromLong(flags);
}

template <obj_f flagsField>
static PyObject* SetFlag(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		logger->warn("Python SetFlag called with null handle.");
		return PyInt_FromLong(0);
	}
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
	if (!self->handle){
		logger->warn("Python ClearFlag called on null obj handle. Field was {}", (int)flagsField);
		return 0;
	}
		
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
	if (!self->handle){
		return 0;
	}
	ObjectFlag flag;
	if (!PyArg_ParseTuple(args, "i:objhndl.object_flag_set", &flag)) {
		return 0;
	}
	objects.SetFlag(self->handle, flag);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_ObjectFlagUnset(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return 0;
	}
	ObjectFlag flag;
	if (!PyArg_ParseTuple(args, "i:objhndl.object_flag_unset", &flag)) {
		return 0;
	}
	objects.ClearFlag(self->handle, flag);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_PortalToggleOpen(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return 0;
	}
	objects.PortalToggleOpen(self->handle);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_ContainerToggleOpen(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return 0;
	}
	objects.ContainerToggleOpen(self->handle);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_SavingThrow(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return 0;
	}
	int dc, d20aType;
	SavingThrowType type;
	auto flags = (D20SavingThrowFlag) 0;
	objHndl attacker = objHndl::null;
	if (!PyArg_ParseTuple(args, "iii|O&i:objhndl:saving_throw", &dc, &type, &flags, &ConvertObjHndl, &attacker, &d20aType)) {
		return 0;
	}

	auto result = damage.SavingThrow(self->handle, attacker, dc, type, flags);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_SavingThrowSpell(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return 0;
	}
	int dc;
	SavingThrowType type;
	auto flags = (D20SavingThrowFlag)0;
	objHndl attacker;
	int spellId;
	int spellId2; // seems like Python bug really, but I'd hate to break vanilla scripts
	if (!PyArg_ParseTuple(args, "iiiO&i|i:objhndl:saving_throw_spell", &dc, &type, &flags, &ConvertObjHndl, &attacker, &spellId, &spellId2)) {
		return 0;
	}

	auto result = damage.SavingThrowSpell(self->handle, attacker, dc, type, flags, spellId);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_ReflexSaveAndDamage(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return 0;
	}
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
	if (!self->handle) {
		return 0;
	}
	int soundmapId;
	if (!PyArg_ParseTuple(args, "i:objhndl.soundmap_critter", &soundmapId)) {
		return 0;
	}

	auto soundId = critterSys.SoundmapCritter(self->handle, soundmapId);
	return PyInt_FromLong(soundId);
}

static PyObject* PyObjHandle_SoundmapItem(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(-1);
	}
	int soundmapId;
	objHndl item = objHndl::null, tgt = objHndl::null;
	if (!PyArg_ParseTuple(args, "O&O&i:objhndl.soundmap_item", &ConvertObjHndl, &item, & ConvertObjHndl, &tgt , &soundmapId)) {
		return PyInt_FromLong(-1);
	}

	auto soundId = inventory.GetSoundIdForItemEvent(item, self->handle, tgt, soundmapId);
	return PyInt_FromLong(soundId);
}


static PyObject* PyObjHandle_SoundPlayFriendlyFire(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return 0;
	}

	objHndl ffer;
	if (!PyArg_ParseTuple(args, "O&:objhndl.sound_play_friendly_fire", &ConvertObjHndl, &ffer)) {
		return 0;
	}

	auto dlgGetFriendlyFireVoiceLine = temple::GetRef<void(__cdecl)(objHndl, objHndl, char*, int*)>(0x10037450);

	char ffText[1000]; int soundId;
	dlgGetFriendlyFireVoiceLine(self->handle, ffer, ffText, &soundId);

	critterSys.PlayCritterVoiceLine(self->handle, ffer, ffText, soundId);
	
	Py_RETURN_NONE;
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
	auto result = secretdoorSys.SecretDoorDetect(self->handle, viewer);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_HasSpellEffects(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	return PyInt_FromLong(objects.HasSpellEffects(self->handle));
}

static PyObject* PyObjHandle_Kill(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return 0;
	}
	critterSys.Kill(self->handle, objHndl::null);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_KillByEffect(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return 0;
	}

	// Temple+: added optional arg to set the killer (so XP is awarded)
	auto killer = objHndl::null;
	if (!PyArg_ParseTuple(args, "|O&:objhndl.critter_kill_by_effect", &ConvertObjHndl, &killer)) {
		return 0;
	}

	critterSys.KillByEffect(self->handle, killer);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_Banish(PyObject *obj, PyObject *args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return 0;
	}

	auto killer = objHndl::null;
	bool awardXp = true;
	if (!PyArg_ParseTuple(args, "|O&i:objhndl.critter_banish", &ConvertObjHndl, &killer, awardXp)) {
		return 0;
	}

	critterSys.Banish(self->handle, killer, awardXp);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_Destroy(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle)
	{
		logger->warn("PyObjHandle_Destroy: Attempted to destroy null object!");
		Py_RETURN_NONE;
	}
	objects.Destroy(self->handle);
	self->handle = 0; // Clear the obj handle
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_ItemGet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return 0;
	}
	objHndl item;
	int flags = ItemInsertFlags::IIF_None;
	if (!PyArg_ParseTuple(args, "O&|i:objhndl.item_get", &ConvertObjHndl, &item, &flags)) {
		return 0;
	}

	auto result = 0;
	if (inventory.IsVisibleInventoryFull(self->handle) && (flags & ItemInsertFlags::IIF_Use_Max_Idx_200 )) {
		result = inventory.SetItemParent(item, self->handle, ItemInsertFlags::IIF_Use_Max_Idx_200);
	}
	else {
		result = inventory.SetItemParent(item, self->handle, ItemInsertFlags::IIF_None);
	}
	
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_PerformTouchAttack(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return 0;
	}
	D20Actn action(D20A_TOUCH_ATTACK);
	int isMelee = 0;
	if (!PyArg_ParseTuple(args, "O&|i:objhndl.perform_touch_attack", &ConvertObjHndl, &action.d20ATarget, &isMelee)) {
		return 0;
	}

	// Build a to-hit action, do hit processing and return the result
	action.d20APerformer = self->handle;
	if (!isMelee)
		action.d20Caf = D20CAF_TOUCH_ATTACK | D20CAF_RANGED;
	else
		action.d20Caf = D20CAF_TOUCH_ATTACK ;
	action.data1 = 1;

	combatSys.ToHitProcessing(action);
	d20Sys.CreateRollHistory(action.rollHistId1);
	d20Sys.CreateRollHistory(action.rollHistId2);
	d20Sys.CreateRollHistory(action.rollHistId0);

	/*if (action.d20Caf & D20CAF_CRITICAL) {
		return PyInt_FromLong(D20CAF_CRITICAL);
	} else if (action.d20Caf & D20CAF_HIT) {
		return PyInt_FromLong(D20CAF_HIT);
	} else {
		return PyInt_FromLong(0);
	}*/
	return PyInt_FromLong(action.d20Caf);
}


static PyObject* PyObjHandle_ActionPerform(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle || !objSystem->IsValidHandle(self->handle)) {
		return nullptr;
	}
	auto performer = self->handle;
	D20Actn action;
	if (!PyArg_ParseTuple(args, "iO&|L:objhndl.action_perform", &action.d20ActType, &ConvertObjHndl, &action.d20ATarget, &action.destLoc.location)) {
		return 0;
	}
	if (action.d20ActType == D20A_NONE) {
		return nullptr;
	}
	if (!actSeqSys.TurnBasedStatusInit(performer)) {
		PyErr_SetString(PyExc_RuntimeError, "action_perform: couldn't init turn based status");
		return PyInt_FromLong(0);
	}

	auto hasLocation = action.destLoc.location.locx != 0 && action.destLoc.location.locy != 0;

	d20Sys.GlobD20ActnInit();
	d20Sys.GlobD20ActnSetTypeAndData1(action.d20ActType, 0);
	d20Sys.GlobD20ActnSetTarget(action.d20ATarget, hasLocation ? &action.destLoc : nullptr);
	auto result = actSeqSys.ActionAddToSeq();
	if (result != AEC_OK) {
		PyErr_SetString(PyExc_RuntimeError, "action_perform: ActionAddToSeq error");
		return PyInt_FromLong(0);
	}
	actSeqSys.sequencePerform();
	return PyInt_FromLong(1);
}


static PyObject* PyObjHandle_AddToInitiative(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return 0;
	}
	combatSys.AddToInitiative(self->handle);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_RemoveFromInitiative(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return 0;
	}
	combatSys.RemoveFromInitiative(self->handle);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_GetInitiative(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		logger->warn("PyObjHandle_GetInitiative called with null handle.");
		return PyInt_FromLong(0);
	}
	return PyInt_FromLong(combatSys.GetInitiative(self->handle));
}

static PyObject* PyObjHandle_SetHpDamage(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return 0;
	}
	int hpDam;
	if (!PyArg_ParseTuple(args, "i:objhndl.set_hp_damage", &hpDam)) {
		return 0;
	}
	critterSys.SetHpDamage(self->handle, hpDam);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_SetSubdualDamage(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return 0;
	}
	int dam;
	if (!PyArg_ParseTuple(args, "i:objhndl.set_subdual_damage", &dam)) {
		return 0;
	}
	critterSys.SetSubdualDamage(self->handle, dam);
	Py_RETURN_NONE;
}


static PyObject* PyObjHandle_SetInitiative(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return 0;
	}
	int initiative;
	if (!PyArg_ParseTuple(args, "i:objhndl.set_initiative", &initiative)) {
		return 0;
	}
	combatSys.SetInitiative(self->handle, initiative);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_D20Query(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	if (PyTuple_GET_SIZE(args) < 1) {
		PyErr_SetString(PyExc_RuntimeError, "d20_query called with no arguments!");
		return PyInt_FromLong(0);
	}

	PyObject* arg = PyTuple_GET_ITEM(args, 0);
	if (PyString_Check(arg)) {
		auto argString = fmt::format("{}",PyString_AsString(arg));
		return PyInt_FromLong( d20Sys.D20QueryPython(self->handle, argString) );
	}

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
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	int spellId;
	if (!PyArg_ParseTuple(args, "i:objhndl.d20query_has_spell_cond", &spellId)) {
		return 0;
	}

	// Get the condition struct from the spell id
	auto cond = spellSys.GetCondFromSpellCondId(spellId);
	if (!cond) {
		return PyInt_FromLong(0);
	}

	auto result = d20Sys.d20QueryWithData(self->handle, DK_QUE_Critter_Has_Condition, (uint32_t) cond, 0);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_D20QueryHasCond(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	char* name;
	if (!PyArg_ParseTuple(args, "s:objhndl.d20_query_has_condition", &name)) {
		return 0;
	}

	auto cond = conds.GetByName(format("{}", name));
	if (!cond) {
		return PyInt_FromLong(0);
	}

	auto result = d20Sys.d20QueryWithData(self->handle, DK_QUE_Critter_Has_Condition, (uint32_t)cond, 0);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_D20QueryWithData(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	if (PyTuple_GET_SIZE(args) < 1) {
		PyErr_SetString(PyExc_RuntimeError, "d20query_with_data called with no arguments!");
		return PyInt_FromLong(0);
	}

	PyObject* arg = PyTuple_GET_ITEM(args, 0);
	if (PyString_Check(arg)) {
		auto argString = fmt::format("{}", PyString_AsString(arg));
		auto arg1 = 0, arg2 = 0;
		if (PyTuple_GET_SIZE(args) >=2 ){
			PyObject* argTmp = PyTuple_GET_ITEM(args, 1);
			arg1 = PyLong_AsLong(argTmp);
		}
		if (PyTuple_GET_SIZE(args) >= 3) {
			PyObject* argTmp = PyTuple_GET_ITEM(args, 2);
			arg2 = PyLong_AsLong(argTmp);
		}
		return PyInt_FromLong(d20Sys.D20QueryPython(self->handle, argString, arg1, arg2));
	}

	int queryKey;
	int data1, data2 = 0;
	if (!PyArg_ParseTuple(args, "ii|i:objhndl.d20_query_with_data", &queryKey, &data1, &data2)) {
		return 0;
	}
	auto dispatcherKey = (D20DispatcherKey)(DK_QUE_Helpless + queryKey);
	auto result = d20Sys.d20QueryWithData(self->handle, dispatcherKey, data1, data2);
	return PyInt_FromLong(result);
}


static PyObject* PyObjHandle_D20QueryWithObject(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}

	int queryKey;
	objHndl queryObject;
	if (!PyArg_ParseTuple(args, "iO&:objhndl.d20_query_with_object", &queryKey, &ConvertObjHndl, &queryObject)) {
		return 0;
	}
	auto dispatcherKey = (D20DispatcherKey)(DK_QUE_Helpless + queryKey);
	auto result = d20Sys.d20QueryWithData(self->handle, dispatcherKey, queryObject);
	return PyInt_FromLong(result);
}


static PyObject* PyObjHandle_D20QueryTestData(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	int queryKey, testData;
	if (!PyArg_ParseTuple(args, "ii:objhndl.d20_query_test_data", &queryKey, &testData)) {
		return 0;
	}
	auto dispatcherKey = (D20DispatcherKey)(DK_QUE_Helpless + queryKey);
	if (d20Sys.d20QueryReturnData(self->handle, dispatcherKey) == testData) {
		return PyInt_FromLong(1);
	} else {
		return PyInt_FromLong(1);
	}
}

static PyObject* PyObjHandle_D20QueryGetData(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	int queryKey, testData = 0;
	if (!PyArg_ParseTuple(args, "i|i:objhndl.d20_query_get_data", &queryKey, &testData)) {
		return 0;
	}
	auto dispatcherKey = (D20DispatcherKey)(DK_QUE_Helpless + queryKey);
	auto result = d20Sys.d20QueryReturnData(self->handle, dispatcherKey);
	return PyLong_FromLongLong(result);
}

static PyObject* PyObjHandle_D20QueryGetObj(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	int queryKey, testData = 0;
	if (!PyArg_ParseTuple(args, "i|i:objhndl.d20_query_get_obj", &queryKey, &testData)) {
		return 0;
	}
	auto dispatcherKey = (D20DispatcherKey)(DK_QUE_Helpless + queryKey);
	objHndl handle;
	handle = d20Sys.d20QueryReturnData(self->handle, dispatcherKey, testData);
	if (!handle) {
		return PyObjHndl_CreateNull();
	}
	return PyObjHndl_Create(handle);
}

static PyObject* PyObjHandle_CritterGetAlignment(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	auto alignment = objects.getInt32(self->handle, obj_f_critter_alignment);
	return PyInt_FromLong(alignment);
}

static PyObject* PyObjHandle_DistanceTo(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	if (PyTuple_GET_SIZE(args) < 1) {
		PyErr_SetString(PyExc_RuntimeError, "distance_to takes at least one argument: objhndl or location");
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

		float off_x = 0, off_y = 0;
		if (PyTuple_GET_SIZE(args) >= 3) {
			PyObject* offxArg = PyTuple_GET_ITEM(args, 1);
			PyObject* offyArg = PyTuple_GET_ITEM(args, 2);
			if (PyFloat_Check(offxArg))
				off_x = static_cast<float>(PyFloat_AsDouble(offxArg));
			if (PyFloat_Check(offyArg))
				off_y = static_cast<float>(PyFloat_AsDouble(offyArg));
		}
		targetLoc.off_x = off_x;
		targetLoc.off_y = off_y;
		auto dist = locSys.DistanceToLoc(self->handle, targetLoc);
		if (dist < 0)
			dist = 0;
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
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
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
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	objHndl triggerer;
	ObjScriptEvent scriptEvent;
	if (!PyArg_ParseTuple(args, "O&i:objhndl.object_script_execute", &ConvertObjHndl, &triggerer, &scriptEvent)) {
		return 0;
	}
	auto result = pythonObjIntegration.ExecuteObjectScript(triggerer, self->handle, scriptEvent);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_AnimGoalInterrupt(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	AnimGoalPriority priority = AGP_HIGHEST;
	if (!PyArg_ParseTuple(args, "|i:objhndl.anim_goal_interrupt", &priority)) {
		return 0;
	}
	gameSystems->GetAnim().Interrupt(self->handle, priority, false);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_AnimGoalPush(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	int animId;
	if (!PyArg_ParseTuple(args, "i:objhndl.anim_goal_push", &animId)) {
		return 0;
	}

	// Guard against bad animIds, since they crash the game pretty hard
	if (animId <= 0) return PyInt_FromLong(-1);
	if (46 <= animId && animId <= 63) return PyInt_FromLong(-1);
	if (99 <= animId && animId <= 102) return PyInt_FromLong(-1);
	if (120 <= animId && animId <= 123) return PyInt_FromLong(-1);
	if (142 <= animId) return PyInt_FromLong(-1);

	return PyInt_FromLong(gameSystems->GetAnim().PushAnimate(self->handle, animId));
}

static PyObject* PyObjHandle_AnimGoalPushAttack(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	objHndl tgt = objHndl::null;
	int animIdx = 0, isCrit = 0, isSecondary = 0;
	if (!PyArg_ParseTuple(args, "O&|iii:objhndl.anim_goal_push_attack", &ConvertObjHndl, &tgt, &animIdx, &isCrit, &isSecondary)) {
		return 0;
	}
	return PyInt_FromLong(gameSystems->GetAnim().PushAttackAnim(self->handle, tgt, -1, animIdx, isCrit, isSecondary));
}

static PyObject* PyObjHandle_AnimGoalPushDodge(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	objHndl attacker = objHndl::null;
	if (!PyArg_ParseTuple(args, "O&|iii:objhndl.anim_goal_push_dodge", &ConvertObjHndl, &attacker)) {
		return 0;
	}
	return PyInt_FromLong(gameSystems->GetAnim().PushDodge(attacker, self->handle));
}


static PyObject* PyObjHandle_AnimGoalPushHitByWeapon(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	objHndl attacker = objHndl::null;
	if (!PyArg_ParseTuple(args, "O&|iii:objhndl.anim_goal_push_hit_by_weapon", &ConvertObjHndl, &attacker)) {
		return 0;
	}
	return PyInt_FromLong(gameSystems->GetAnim().PushGoalHitByWeapon( attacker, self->handle));
}

static PyObject* PyObjHandle_AnimGoalPushUseObject(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	objHndl tgt = objHndl::null;
	AnimGoalType goalType = ag_use_object;
	locXY tgtLoc = LocAndOffsets::null.location;
	int someFlag = 1;
	if (!PyArg_ParseTuple(args, "O&|iLi:objhndl.anim_goal_use_object", &ConvertObjHndl, &tgt, &goalType, &tgtLoc, &someFlag)) {
		return 0;
	}
	gameSystems->GetAnim().PushForMouseTarget(self->handle, goalType, tgt, tgtLoc, objHndl::null, someFlag);
	return PyInt_FromLong(1);
}

static PyObject* PyObjHandle_ContainerOpenUI(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	objHndl tgt = objHndl::null;
	if (!PyArg_ParseTuple(args, "O&:objhndl.container_open_ui", &ConvertObjHndl, &tgt)) {
		return 0;
	}
	inventory.UiOpenContainer(self->handle, tgt);
	return PyInt_FromLong(1);
}

static PyObject* PyObjHandle_AnimGoalGetNewId(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	return PyInt_FromLong(gameSystems->GetAnim().GetActionAnimId(self->handle));
}


static PyObject* PyObjHandle_ApplyProjectileParticles(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	D20CAF flags;
	objHndl projectile;
	if (!PyArg_ParseTuple(args, "O&|i:objhndl.apply_projectile_particles", &ConvertObjHndl, &projectile, &flags)) {
		return 0;
	}

	auto result = temple::GetRef<BOOL(__cdecl)(objHndl, objHndl, D20CAF)>(0x1004F330)(self->handle, projectile, flags);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_ApplyProjectileHitParticles(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	D20CAF flags;
	objHndl projectile;
	if (!PyArg_ParseTuple(args, "O&|i:objhndl.apply_projectile_hit_particles", &ConvertObjHndl, &projectile, &flags)) {
		return 0;
	}

	auto result = temple::GetRef<BOOL(__cdecl)(objHndl, objHndl, D20CAF)>(0x1004F420)(self->handle, projectile, flags);
	return PyInt_FromLong(result);
}


static PyObject* PyObjHandle_D20StatusInit(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	d20Sys.d20Status->D20StatusInit(self->handle);
	Py_RETURN_NONE;
}

/*
	Sets one of the critters stand points to a jump point.
*/
static PyObject* PyObjHandle_StandpointSet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	StandPointType type;
	StandPoint standPoint;
	standPoint.location = LocAndOffsets::null;
	standPoint.location.off_x = 0;
	standPoint.location.off_y = 0;
	standPoint.mapId = 0;
	standPoint.jumpPointId = -1;
	if (!PyArg_ParseTuple(args, "ii|Liff:objhndl.standpoint_set", &type, &standPoint.jumpPointId, &standPoint.location.location, &standPoint.mapId, &standPoint.location.off_x, &standPoint.location.off_y)) {
		return 0;
	}

	if (standPoint.jumpPointId >= 0) {
		JumpPoint jumpPoint;
		if (!maps.GetJumpPoint(standPoint.jumpPointId, jumpPoint)) {
			PyErr_Format(PyExc_ValueError, "Unknown jump point id %d used to set a standpoint.", standPoint.jumpPointId);
			return 0;
		}

		standPoint.location.location = jumpPoint.location;
		standPoint.location.off_x = 0;
		standPoint.location.off_y = 0;
		standPoint.mapId = jumpPoint.mapId;
	}
	else {
		if (!standPoint.mapId) {
			standPoint.mapId = maps.GetCurrentMapId();
		}
	}
	critterSys.SetStandPoint(self->handle, type, standPoint);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_StandpointGet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	StandPointType type = StandPointType::Day;
	if (!PyArg_ParseTuple(args, "|i:objhndl.standpoint_get", &type)) {
		return 0;
	}

	StandPoint standPoint = critterSys.GetStandPoint(self->handle, type);
	auto dict = PyDict_New();
	{
		PyDict_SetItemString(dict, "mapId", PyInt_FromLong(standPoint.mapId));
		PyDict_SetItemString(dict, "location", PyLong_FromLongLong(standPoint.location.location));
		PyDict_SetItemString(dict, "loc_x", PyInt_FromLong(standPoint.location.location.locx));
		PyDict_SetItemString(dict, "loc_y", PyInt_FromLong(standPoint.location.location.locy));
		PyDict_SetItemString(dict, "off_x", PyFloat_FromDouble(standPoint.location.off_x));
		PyDict_SetItemString(dict, "off_y", PyFloat_FromDouble(standPoint.location.off_y));
		PyDict_SetItemString(dict, "jumpPointId", PyInt_FromLong(standPoint.jumpPointId));
	}
	return dict;
}

static PyObject* PyObjHandle_RunOff(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	LocAndOffsets loc;
	loc.off_x = 0;
	loc.off_y = 0;
	if (!PyArg_ParseTuple(args, "L|ff:objhndl.runoff", &loc.location, &loc.off_x, &loc.off_y)) {
		return 0;
	}

	objects.SetFlag(self->handle, OF_CLICK_THROUGH);
	aiSys.SetAiFlag(self->handle, AiFlag::RunningOff);
	objects.FadeTo(self->handle, 0, 25, 5, 2);
	gameSystems->GetAnim().PushRunNearTile(self->handle, loc, 5);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_RunTo(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	LocAndOffsets loc;
	loc.off_x = 0;
	loc.off_y = 0;
	if (!PyArg_ParseTuple(args, "L|ff:objhndl.runoff", &loc.location, &loc.off_x, &loc.off_y)) {
		return 0;
	}
	gameSystems->GetAnim().PushRunNearTile(self->handle, loc, 5);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_GetCategoryType(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	auto type = critterSys.GetCategory(self->handle);
	return PyInt_FromLong(type);
}

template<typename Filter>
static PyObject* GetCharacterClassesSet(Filter filter, PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyTuple_New(0);
	}

	std::vector<int> classes;
	for (auto it : d20ClassSys.classEnums) {
		auto classEnum = (Stat)it;
		if (filter(classEnum)) {
			auto classLvl = objects.StatLevelGet(self->handle, classEnum);
			if (classLvl > 0) {
				classes.push_back(it);
			}
		}
	}

	auto result = PyTuple_New(classes.size());
	for (size_t i = 0; i < classes.size(); i++) {
		PyTuple_SET_ITEM(result, i, PyInt_FromLong(classes[i]));
	}

	return result;
}

static PyObject* PyObjHandle_GetCharacterAllClassesSet(PyObject* obj, PyObject* args) {
	return GetCharacterClassesSet([](Stat) { return true; }, obj, args);
}

static PyObject* PyObjHandle_GetCharacterBaseClassesSet(PyObject* obj, PyObject* args) {
	return GetCharacterClassesSet([](Stat classEnum) { return d20ClassSys.IsBaseClass(classEnum); }, obj, args);
}

static PyObject* PyObjHandle_GetHandleLower(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	return PyInt_FromLong(self->handle.GetHandleLower());
}

static PyObject* PyObjHandle_GetHandleUpper(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	return PyInt_FromLong(self->handle.GetHandleUpper());
}

static PyObject* PyObjHandle_IsActiveCombatant(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	
	if (combatSys.isCombatActive() && tbSys.IsInInitiativeList(self->handle)){
		return PyInt_FromLong(1);
	}

	return PyInt_FromLong(0);
}


static PyObject* PyObjHandle_IsArcaneSpellClass(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	int stat;
	if (!PyArg_ParseTuple(args, "i:objhndl.is_arcane_spell_class", &stat)) {
		return 0;
	}

	auto result = d20ClassSys.IsArcaneCastingClass(static_cast<Stat>(stat));
	return PyInt_FromLong(result?1:0);
}

static PyObject* PyObjHandle_IsCategoryType(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	int type;
	if (!PyArg_ParseTuple(args, "i:objhndl.is_category_type", &type)) {
		return 0;
	}
	auto result = critterSys.IsCategoryType(self->handle, (MonsterCategory) type);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_IsCategorySubtype(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	int type;
	if (!PyArg_ParseTuple(args, "i:objhndl.is_category_subtype", &type)) {
		return 0;
	}
	auto result = critterSys.IsCategorySubtype(self->handle, (MonsterSubcategoryFlag)type);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_IsCritter(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	
	auto result = (int) objects.IsCritter(self->handle);
	return PyInt_FromLong(result);
}


static PyObject* PyObjHandle_RumorLogAdd(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	int rumorId;
	if (!PyArg_ParseTuple(args, "i:objhndl.rumor_log_add", &rumorId)) {
		return 0;
	}
	party.RumorLogAdd(self->handle, rumorId);
	Py_RETURN_NONE;
}



static PyObject* PyObjHandle_ObjectEventAppend(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(-1);
	}

	float radiusFeet;
	ObjectListFilter filter;
	if (!PyArg_ParseTuple(args, "if:objhndl.object_event_append", &filter, &radiusFeet)) {
		return 0;
	}

	auto result = objEvents.EventAppend(self->handle, 0, 1, filter, radiusFeet * (float)12.0, 0.0, XM_2PI);

	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_WallEventAppend(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(-1);
	}

	float radiusFeet, wallAngle;
	ObjectListFilter filter;
	if (!PyArg_ParseTuple(args, "iff:objhndl.object_event_append_wall", &filter, &radiusFeet, &wallAngle)) {
		return 0;
	}

	auto result = objEvents.EventAppend(self->handle, OBJ_EVENT_WALL_ENTERED_HANDLER_ID, OBJ_EVENT_WALL_EXITED_HANDLER_ID, filter, radiusFeet * (float)12.0, wallAngle, XM_2PI);

	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_SetInt(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	obj_f field;
	int value;
	if (!PyArg_ParseTuple(args, "ii:objhndl.obj_set_int", &field, &value)) {
		return 0;
	}
	if (field == obj_f_critter_subdual_damage) {
		critterSys.SetSubdualDamage(self->handle, value);
	} else {

		if (objectFields.GetType(field) == ObjectFieldType::Int32)
		{
			objects.setInt32(self->handle, field, value);
		}
		else if (objectFields.GetType(field) == ObjectFieldType::Float32)
		{
			objSystem->GetObject(self->handle)->SetFloat(field, (float) value);
		} 
		else
		{
			logger->warn("Wrong field type for set_int, {}", (int)(field));
		}
	}
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_SetFloat(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	obj_f field;
	float value;
	if (!PyArg_ParseTuple(args, "if:objhndl.obj_set_float", &field, &value)) {
		return 0;
	}

	objSystem->GetObject(self->handle)->SetFloat(field, value);
	Py_RETURN_NONE;
}


static PyObject* PyObjHandle_SetInt64(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	obj_f field;
	int64_t value;
	if (!PyArg_ParseTuple(args, "iL:objhndl.obj_set_int64", &field, &value)) {
		return 0;
	}
	
	auto objBody = gameSystems->GetObj().GetObject(self->handle);
	if (objectFields.GetType(field) == ObjectFieldType::Int64)
	{
		objBody->SetInt64(field, value);
	}
	else
	{
		logger->warn("Wrong field type for set_int, {}", (int)(field));
	}
	
	Py_RETURN_NONE;
}


static PyObject* PyObjHandle_SetIdxInt(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	obj_f field;

	int value, idx = 0;
	if (!PyArg_ParseTuple(args, "iii:objhndl.obj_set_idx_int", &field, &idx, &value)) {
		return 0;
	}
	objSystem->GetObject(self->handle)->SetInt32(field, idx, value);
	
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_DelIdxInt(PyObject* pyobj, PyObject* args) {
	auto self = GetSelf(pyobj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	auto obj = objSystem->GetObject(self->handle);
	if (!obj) Py_RETURN_NONE;

	obj_f field;

	int value, idx = 0;
	if (!PyArg_ParseTuple(args, "iii:objhndl.obj_del_idx_int", &field, &idx, &value)) {
		return 0;
	}
	if (idx < static_cast<int>(obj->GetInt32Array(field).GetSize())) {
		obj->RemoveInt32(field, idx);
	}
	
	Py_RETURN_NONE;
}


static PyObject* PyObjHandle_SetIdxInt64(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	obj_f field;

	int64_t value =0;
	int idx = 0;
	if (!PyArg_ParseTuple(args, "iiL:objhndl.obj_set_idx_int64", &field, &idx, &value)) {
		return 0;
	}
	objSystem->GetObject(self->handle)->SetInt64(field, idx, value);

	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_SetObj(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	obj_f field;
	objHndl value;
	if (!PyArg_ParseTuple(args, "iO&:objhndl.obj_set_obj", &field, &ConvertObjHndl,&value)) {
		return PyInt_FromLong(0);
	}
	objects.SetFieldObjHnd(self->handle, field, value);
	return PyInt_FromLong(1);
}

static PyObject* PyObjHandle_SetString(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	obj_f field;
	char* value = nullptr;
	if (!PyArg_ParseTuple(args, "is:objhndl.obj_set_string", &field, &value)) {
		return PyInt_FromLong(0);
	}
	objects.SetFieldString(self->handle, field, value);
	return PyInt_FromLong(1);
}

static PyObject* PyObjHandle_GetString(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	obj_f field;
	if (!PyArg_ParseTuple(args, "i:objhndl.obj_get_string", &field)) {
		return PyInt_FromLong(0);
	}
	auto result = objects.getString(self->handle, field);
	return PyString_FromString(result);
}

static PyObject* PyObjHandle_GetInt(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	obj_f field;
	if (!PyArg_ParseTuple(args, "i:objhndl.obj_get_int", &field)) {
		return 0;
	}
	int value = 0;
	if (!self->handle) // python users aren't always so careful :P
	{
		logger->warn("Warning: Python GetInt called with null object handle! Field was {}, returning 0.", objectFields.GetFieldName(field));
		return PyInt_FromLong(value);
	}
	if (objectFields.GetType(field) == ObjectFieldType::Int32)
	{
		value = objects.getInt32(self->handle, field);

	} else if (objectFields.GetType(field) == ObjectFieldType::Float32)
	{
		value = (int) objSystem->GetObject(self->handle)->GetFloat(field);
	}
	
	return PyInt_FromLong(value);
}

static PyObject* PyObjHandle_GetIdxInt(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	obj_f field;
	int subIdx = 0;
	if (!PyArg_ParseTuple(args, "i|i:objhndl.obj_get_idx_int", &field, &subIdx)) {
		return 0;
	}
	assert(subIdx >= 0);
	int32_t value = objects.getArrayFieldInt32(self->handle, field, subIdx);
	return PyInt_FromLong(value);
}

static PyObject* PyObjHandle_GetIdxIntSize(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	obj_f field;
	if (!PyArg_ParseTuple(args, "i:objhndl.obj_get_idx_int_size", &field)) {
		return 0;
	}
	int32_t value = objects.getArrayFieldInt32Size(self->handle, field);
	return PyInt_FromLong(value);
}

static PyObject* PyObjHandle_GetIdxInt64(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	obj_f field;
	int subIdx = 0;
	if (!PyArg_ParseTuple(args, "i|i:objhndl.obj_get_idx_int64", &field, &subIdx)) {
		return 0;
	}
	assert(subIdx >= 0);
	int64_t value = objects.getArrayFieldInt64(self->handle, field, subIdx);
	return PyLong_FromLong((long)value);
}

static PyObject* PyObjHandle_GetIdxInt64Size(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	obj_f field;
	if (!PyArg_ParseTuple(args, "i:objhndl.obj_get_idx_int64_size", &field)) {
		return 0;
	}
	int64_t value = objects.getArrayFieldInt64Size(self->handle, field);
	return PyLong_FromLong((long)value);
}

static PyObject* PyObjHandle_GetIdxObj(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	obj_f field;
	int subIdx = 0;
	if (!PyArg_ParseTuple(args, "i|i:objhndl.obj_get_idx_obj", &field, &subIdx)) {
		return 0;
	}
	assert(subIdx >= 0);
	auto result = objSystem->GetObject(self->handle)->GetObjHndl(field, subIdx);
	
	return PyObjHndl_Create(result);
}

static PyObject* PyObjHandle_GetIdxObjSize(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	obj_f field;
	if (!PyArg_ParseTuple(args, "i:objhndl.obj_get_idx_obj_size", &field)) {
		return 0;
	}
	auto result = objSystem->GetObject(self->handle)->GetObjectIdArray(field).GetSize();
	return PyLong_FromLong((long)result);
}



static PyObject* PyObjHandle_GetInt64(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	obj_f field;
	if (!PyArg_ParseTuple(args, "i:objhndl.obj_get_int64", &field)) {
		return 0;
	}
	auto value = objects.getInt64(self->handle, field);
	return PyLong_FromLongLong(value);
}

static PyObject* PyObjHandle_GetObj(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	obj_f field;
	if (!PyArg_ParseTuple(args, "i:objhndl.obj_get_obj", &field)) {
		return 0;
	}
	auto value = objects.getObjHnd(self->handle, field);
	return PyObjHndl_Create(value);
}

static PyObject* PyObjHandle_GetWaypoints(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	int count = critterSys.GetWaypointsCount(self->handle);
	if (!count)
		Py_RETURN_NONE;
	auto list = PyList_New(0);
	Waypoint wp;
	for (int idx = 0; idx < count; idx++)
	{
		critterSys.GetWaypoint(self->handle, idx, wp);
		auto dict = PyDict_New();
		PyList_Append(list, dict);
		{
			PyDict_SetItemString(dict, "flags", PyLong_FromLong((long)wp.flags));
			PyDict_SetItemString(dict, "x", PyInt_FromLong(wp.location.location.locx));
			PyDict_SetItemString(dict, "y", PyInt_FromLong(wp.location.location.locy));
			PyDict_SetItemString(dict, "off_x", PyFloat_FromDouble(wp.location.off_x));
			PyDict_SetItemString(dict, "off_y", PyFloat_FromDouble(wp.location.off_y));
			PyDict_SetItemString(dict, "rotation", PyFloat_FromDouble(wp.rotation));
			PyDict_SetItemString(dict, "delay", PyInt_FromLong(wp.delay));
			PyDict_SetItemString(dict, "anim1", PyInt_FromSize_t(wp.anims[0]));
			PyDict_SetItemString(dict, "anim2", PyInt_FromSize_t(wp.anims[1]));
			PyDict_SetItemString(dict, "anim3", PyInt_FromSize_t(wp.anims[2]));
			PyDict_SetItemString(dict, "anim4", PyInt_FromSize_t(wp.anims[3]));
			PyDict_SetItemString(dict, "anim5", PyInt_FromSize_t(wp.anims[4]));
			PyDict_SetItemString(dict, "anim6", PyInt_FromSize_t(wp.anims[5]));
			PyDict_SetItemString(dict, "anim7", PyInt_FromSize_t(wp.anims[6]));
			PyDict_SetItemString(dict, "anim8", PyInt_FromSize_t(wp.anims[7]));
		}
		Py_DecRef(dict);
	}
	return list;
};

static PyObject* PyObjHandle_SetWaypoints(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	if (PyTuple_GET_SIZE(args) < 1 || PyTuple_GET_ITEM(args, 0) == Py_None)
	{
		critterSys.SetWaypointsCount(self->handle, 0);
		Py_RETURN_NONE;
	}

	if (!PyList_Check(PyTuple_GET_ITEM(args, 0))) {
		PyErr_SetString(PyExc_RuntimeError, "npc_waypoints_set args should have list of dicts as first argument!");
		return false;
	}

	auto list = PyTuple_GET_ITEM(args, 0);
	int count = PyList_GET_SIZE(list);

	critterSys.SetWaypointsCount(self->handle, count);
	Waypoint wp;
	for (int idx = 0; idx < count; idx++)
	{
		auto dict = PyList_GetItem(list, idx);
		if (!dict) {
			PyErr_SetString(PyExc_RuntimeError, "npc_waypoints_set list contains None at ?");
			critterSys.SetWaypointsCount(self->handle, 0);
			return false;
		}
		if (!PyDict_Check(dict)) {
			dict = PyObject_GetAttr(dict, PyString_FromString("__dict__"));
			if (!dict || !PyDict_Check(dict)) {
				PyErr_SetString(PyExc_RuntimeError, "npc_waypoints_set list item should be either dict or object!");
				critterSys.SetWaypointsCount(self->handle, 0);
				return false;
			}
		}
		auto flags = PyDict_GetItem(dict, PyString_FromString("flags"));
		if (flags) wp.flags = _PyInt_AsInt(flags);

		auto x = PyDict_GetItem(dict, PyString_FromString("x"));
		if (x) wp.location.location.locx = _PyInt_AsInt(x);

		auto y = PyDict_GetItem(dict, PyString_FromString("y"));
		if (y) wp.location.location.locy = _PyInt_AsInt(y);

		auto off_x = PyDict_GetItem(dict, PyString_FromString("off_x"));
		if (off_x) wp.location.off_x = (float)PyFloat_AsDouble(off_x);

		auto off_y = PyDict_GetItem(dict, PyString_FromString("off_y"));
		if (off_y) wp.location.off_y = (float)PyFloat_AsDouble(off_y);

		auto rotation = PyDict_GetItem(dict, PyString_FromString("rotation"));
		if (rotation) wp.rotation = (float)PyFloat_AsDouble(rotation);

		auto delay = PyDict_GetItem(dict, PyString_FromString("delay"));
		if (delay) wp.delay = _PyInt_AsInt(delay);

		auto anim1 = PyDict_GetItem(dict, PyString_FromString("anim1"));
		if (anim1) wp.anims[0] = _PyInt_AsInt(anim1);

		auto anim2 = PyDict_GetItem(dict, PyString_FromString("anim2"));
		if (anim2) wp.anims[1] = _PyInt_AsInt(anim2);

		auto anim3 = PyDict_GetItem(dict, PyString_FromString("anim3"));
		if (anim3) wp.anims[2] = _PyInt_AsInt(anim3);

		auto anim4 = PyDict_GetItem(dict, PyString_FromString("anim4"));
		if (anim4) wp.anims[3] = _PyInt_AsInt(anim4);

		auto anim5 = PyDict_GetItem(dict, PyString_FromString("anim5"));
		if (anim5) wp.anims[4] = _PyInt_AsInt(anim5);

		auto anim6 = PyDict_GetItem(dict, PyString_FromString("anim6"));
		if (anim6) wp.anims[5] = _PyInt_AsInt(anim6);

		auto anim7 = PyDict_GetItem(dict, PyString_FromString("anim7"));
		if (anim7) wp.anims[6] = _PyInt_AsInt(anim7);

		auto anim8 = PyDict_GetItem(dict, PyString_FromString("anim8"));
		if (anim8) wp.anims[7] = _PyInt_AsInt(anim8);

		critterSys.SetWaypoint(self->handle, idx, wp);
	}

	// loop reset
	{
		int loop_reset = 0;
		if (PyTuple_GET_SIZE(args) > 1 && PyInt_Check(PyTuple_GET_ITEM(args, 1))) {
			loop_reset = _PyInt_AsInt(PyTuple_GET_ITEM(args, 1));
		}

		if (loop_reset) {
			auto gameObj = objSystem->GetObject(self->handle);
			gameObj->SetInt32(obj_f::obj_f_npc_waypoint_current, 0);
			gameObj->SetInt64(obj_f::obj_f_npc_ai_flags64, gameObj->GetInt64(obj_f::obj_f_npc_ai_flags64) & ~(AiFlag::WaypointDelay | AiFlag::WaypointDelayed));
			gameObj->SetInt32(obj_f::obj_f_npc_waypoint_anim, 0);
			gameSystems->GetAnim().Interrupt(self->handle, AnimGoalPriority::AGP_4, false);
			if (count > 0 && critterSys.GetWaypoint(self->handle, 0, wp)) {
				gameSystems->GetAnim().PushWalkToTile(self->handle, wp.location);
			}
		}
	}

	Py_RETURN_NONE;
};

static PyObject* PyObjHandle_GetSpell(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	obj_f field;
	int subIdx = 0;
	if (!PyArg_ParseTuple(args, "i|i:objhndl.obj_get_spell", &field, &subIdx)) {
		return 0;
	}
	assert(subIdx >= 0);
	auto value = objSystem->GetObject(self->handle)->GetSpell(field, subIdx);
	return PySpellStore_Create(value);
}


static PyObject* PyObjHandle_HasFeat(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	feat_enums feat;

	if (PyTuple_GET_SIZE(args) < 1) {
		PyErr_SetString(PyExc_RuntimeError, "has_feat called with no arguments!");
		return PyInt_FromLong(0);
	}

	PyObject* arg = PyTuple_GET_ITEM(args, 0);
	if (PyString_Check(arg)) {
		auto argString = fmt::format("{}", PyString_AsString(arg));
		feat = (feat_enums)ElfHash::Hash(argString);
	}

	else if (!PyArg_ParseTuple(args, "i:objhndl.has_feat", &feat)) {
		return 0;
	}

	if (!objects.IsCritter(self->handle)) {
		logger->warn("Python has_feat ({}) called with non critter object: {}", feats.GetFeatName(feat), self->handle);
		return PyInt_FromLong(0);
	}

	Stat levelRaised = (Stat)0;
	uint32_t domain1 = 0;
	uint32_t domain2 = 0;
	uint32_t alignmentChoice = 0;

	auto result = feats.HasFeatCountByClass(self->handle, feat, levelRaised, 0, domain1, domain2, alignmentChoice);

	return PyInt_FromLong(result);
}

static PyObject * PyObjHandle_FeatAdd(PyObject* obj, PyObject * args){
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	if (PyTuple_GET_SIZE(args) < 1) {
		PyErr_SetString(PyExc_RuntimeError, "feat_add called with no arguments!");
		return PyInt_FromLong(0);
	}

	// get the feat enum
	feat_enums featCode;
	int doRefresh = 0;

	PyObject* arg = PyTuple_GET_ITEM(args, 0);
	if (PyString_Check(arg)) {
		auto argString = fmt::format("{}", PyString_AsString(arg));
		featCode = (feat_enums)ElfHash::Hash(argString);
	}
	else if (!PyArg_ParseTuple(args, "i|i:objhndl.feat_add", &featCode, &doRefresh)) {
		return nullptr;
	};


	if (featCode == 0){
		return PyInt_FromLong(0);
	}

	objects.feats.FeatAdd(self->handle, featCode);
	if (doRefresh)
		d20Sys.d20Status->D20StatusRefresh(self->handle);

	return PyInt_FromLong(1);
};

static PyObject* PyObjHandle_SpellKnownAddToCharClass(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	int spellIdx;
	int charClass;
	int slotLevel;
	int isDomain = 0;
	if (!PyArg_ParseTuple(args, "iii|i:objhndl.spell_known_add_to_char_class", &spellIdx, &charClass, &slotLevel, &isDomain)) {
		return 0;
	}

	auto spellClass = spellSys.GetSpellClass(charClass);
	spellSys.SpellKnownAdd(self->handle, spellIdx, spellClass, slotLevel, 1, 0);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_SpellKnownAdd(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	int spellIdx;
	int spellClassCode;
	int slotLevel;
	int isDomain = 0;
	if (!PyArg_ParseTuple(args, "iii|i:objhndl.spell_known_add", &spellIdx, &spellClassCode, &slotLevel, &isDomain)) {
		return 0;
	}
	spellClassCode = spellClassCode & 0x7F;
	if (!isDomain)
		spellClassCode = spellClassCode | 0x80;
	spellSys.SpellKnownAdd(self->handle, spellIdx, spellClassCode, slotLevel, 1, 0);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_SpellMemorizedAdd(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	int spellIdx;
	int spellClassCode;
	int slotLevel;
	int isDomain = 0;
	if (!PyArg_ParseTuple(args, "iii|i:objhndl.spell_memorized_add", &spellIdx, &spellClassCode, &slotLevel, &isDomain)) {
		return 0;
	}
	spellClassCode = spellClassCode & 0x7F;
	if (!isDomain)
		spellClassCode = spellClassCode | 0x80;
	spellSys.SpellMemorizedAdd(self->handle, spellIdx, spellClassCode, slotLevel, 2, 0);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_SpellHeal(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
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
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	inventory.IdentifyAll(self->handle);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_AiFleeAdd(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	objHndl target;
	if (!PyArg_ParseTuple(args, "O&:objhndl.ai_flee_add", &ConvertObjHndl, &target)) {
		return 0;
	}
	aiSys.FleeAdd(self->handle, target);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_AiShitlistAdd(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	objHndl target;
	if (!PyArg_ParseTuple(args, "O&:objhndl.ai_shitlist_add", &ConvertObjHndl, &target)) {
		return 0;
	}
	aiSys.ShitlistAdd(self->handle, target);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_AiShitlistRemove(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	objHndl target;
	if (!PyArg_ParseTuple(args, "O&:objhndl.ai_shitlist_remove", &ConvertObjHndl, &target)) {
		return 0;
	}
	aiSys.ShitlistRemove(self->handle, target);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_AiStopAttacking(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	aiSys.StopAttacking(self->handle);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_AiStrategySetCustom(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}

	auto strings = PyTuple_GetItem(args, 0);
	if (!PyList_Check(strings)){
		logger->error("ai_strategy_set_custom: expected list of strings as input");
		Py_RETURN_NONE;
	}

	auto count = PyList_Size(strings);
	if ( (count % 3) != 1){
		logger->error("ai_strategy_set_custom: expected list of string triplets as input");
		Py_RETURN_NONE;
	}

	std::vector<string> stringVector;
	for (auto i=0; i < count; i++){
		const char* s = PyString_AsString( PyList_GET_ITEM(strings, i));
		stringVector.push_back(s);
	}

	// optional "save" flag
	int save = 1;
	if (PyTuple_Size(args) >= 2) {
		auto psave = PyTuple_GetItem(args, 1);
		if (PyLong_Check(psave) || PyInt_Check(psave)) {
			save = PyLong_AsLong(psave);
		}
	}
	

	aiSys.SetCustomStrategy( self->handle, stringVector, save);
	Py_RETURN_NONE;
}


static PyObject* PyObjHandle_AllegianceShared(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	objHndl target;
	if (!PyArg_ParseTuple(args, "O&:objhndl.allegiance_shared", &ConvertObjHndl, &target)) {
		return 0;
	}

	return PyInt_FromLong(critterSys.NpcAllegianceShared(self->handle, target));
}

static PyObject* PyObjHandle_AllegianceStrength(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	objHndl target;
	if (!PyArg_ParseTuple(args, "O&:objhndl.allegiance_strength", &ConvertObjHndl, &target)) {
		return 0;
	}

	return PyInt_FromLong(aiSys.GetAllegianceStrength(self->handle, target));
}



static PyObject* PyObjHandle_GetDeity(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	return PyInt_FromLong(objects.GetDeity(self->handle));
}


static PyObject* PyObjHandle_GetWeaponType(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	auto result = objects.GetWeaponType(self->handle);

	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_IsThrownOnlyWeapon(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	auto weaponType = objects.GetWeaponType(self->handle);
	auto answer = weapons.IsThrownOnlyWeapon(weaponType);

	return PyInt_FromLong(answer?1:0);
}

static PyObject* PyObjHandle_GetWieldType(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	objHndl weapon = objHndl::null;
	int regardEnlargement = false;
	if (!PyArg_ParseTuple(args, "|O&i:objhndl.get_wield_type", &ConvertObjHndl, &weapon, &regardEnlargement))
		return 0;

	auto result = 0; // default - light weapon

	if (weapon == objHndl::null) {
		weapon = inventory.ItemWornAt(self->handle, EquipSlot::WeaponPrimary);
		if (!weapon)
			weapon = inventory.ItemWornAt(self->handle, EquipSlot::WeaponSecondary);
		if (!weapon){ // no weapon at all!
			return PyInt_FromLong(result);
		}
	}

	result = inventory.GetWieldType(self->handle, weapon, regardEnlargement != 0);

	return PyInt_FromLong(result); 
}

static PyObject* PyObjHandle_GetWeaponProjectileProto(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	auto result = temple::GetRef<int(__cdecl)(objHndl)>(0x10065760)(self->handle);

	return PyInt_FromLong(result);
}



static PyObject* PyObjHandle_Unwield(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}

	int es;
	int flag = 0;
	if (!PyArg_ParseTuple(args, "i|i:objhndl.item_worn_unwield",  &es, &flag)) {
		return 0;
	}


	if (es >= EquipSlot::Count || es < 0)
		es = EquipSlot::Invalid;
	auto equipSlot = (EquipSlot)es;

	if ( flag == 1 ) {
		inventory.ItemDrop(self->handle, equipSlot);
		Py_RETURN_NONE;
	}

	inventory.ItemUnwield(self->handle,  equipSlot);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_Wield(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	auto result = 0;
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	objHndl item = objHndl::null;
	int equipSlot;
	if (!PyArg_ParseTuple(args, "O&i:objhndl.item_wield", &ConvertObjHndl, &item, &equipSlot)) {
		return 0;
	}
	
	if ( inventory.GetParent(item) != self->handle ){
		if (inventory.IsVisibleInventoryFull(self->handle)) {
			auto result = inventory.SetItemParent(item, self->handle, ItemInsertFlags::IIF_Use_Max_Idx_200);
			result = inventory.SetItemParent(item, self->handle, 8);
		}
		else {
			result = inventory.SetItemParent(item, self->handle, 0);
		}
	}

	if (equipSlot >= EquipSlot::Count || equipSlot < 0)
		equipSlot = EquipSlot::Invalid;

	inventory.Wield(self->handle, item, (EquipSlot)equipSlot);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_WieldBestAll(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	objHndl targetHndl = objHndl::null;
	if (!PyArg_ParseTuple(args, "|O&:objhndl.item_wield_best_all", &ConvertObjHndl, &targetHndl)) {
		return 0;
	}
	inventory.WieldBestAll(self->handle, targetHndl);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_AwardExperience(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	int xpAwarded;
	if (!PyArg_ParseTuple(args, "i:objhndl.award_experience", &xpAwarded)) {
		return 0;
	}
	critterSys.AwardXp(self->handle, xpAwarded);
	uiSystems->GetParty().Update();
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_HasAtoned(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	d20Sys.d20SendSignal(self->handle, DK_SIG_Atone_Fallen_Paladin, 0, 0);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_D20SendSignal(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}

	
	PyObject* signalId = 0; //int signalId;
	PyObject* arg = 0;
	PyObject* arg2 = 0;
	if (!PyArg_ParseTuple(args, "O|OO:objhndl.d20_send_signal", &signalId, &arg, &arg2)) {
		return 0;
	}


	bool isPythonSig = false;
	D20DispatcherKey dispKey;

	if (PyInt_Check(signalId))
		dispKey = (D20DispatcherKey)(DK_SIG_HP_Changed + PyInt_AsLong(signalId));
	else if (PyString_Check(signalId))
	{
		dispKey = (D20DispatcherKey)ElfHash::Hash( PyString_AsString(signalId) );
		isPythonSig = true;
	}

	if (arg && PyObjHndl_Check(arg)) {
		objHndl hndl = PyObjHndl_AsObjHndl(arg);

		if (arg2) {
			logger->error("Error signal with handle and second argument");
			return 0;
		}

		if (isPythonSig)
			logger->error("Unimplemented D20SignalPython with handle arg");
		else
			d20Sys.d20SendSignal(self->handle, dispKey, hndl);
	}
	
	else if (arg && PyLong_Check(arg)) {
		long val2 = 0;
		auto val = PyLong_AsLong(arg);

		if (arg2) {
			val2 = PyLong_AsLong(arg2);
		}

		if (isPythonSig)
			d20Sys.D20SignalPython(self->handle, dispKey, val, val2);
		else
			d20Sys.d20SendSignal(self->handle, dispKey, val, val2);
	}
	
	else if (arg && PyInt_Check(arg))
	{
		int val2 = 0;
		auto val = PyInt_AsLong(arg);

		if (arg2) {
			val2 = PyInt_AsLong(arg2);
		}

		if (isPythonSig)
			d20Sys.D20SignalPython(self->handle, dispKey, val, val2);
		else
			d20Sys.d20SendSignal(self->handle, dispKey, val, val2);
	} 
	
	else {
		if (isPythonSig)
			d20Sys.D20SignalPython(self->handle, dispKey );
		else
			d20Sys.d20SendSignal(self->handle, dispKey, 0, 0);
	}

	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_D20SendSignalEx(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	int signalId;
	objHndl arg = objHndl::null;
	if (!PyArg_ParseTuple(args, "i|O&:objhndl.d20_send_signal_ex", &signalId, &ConvertObjHndl, &arg)) {
		return 0;
	}
	D20DispatcherKey dispKey = (D20DispatcherKey)(DK_SIG_HP_Changed + signalId);
	D20Actn d20a(D20A_CAST_SPELL);
	d20a.d20APerformer = self->handle;
	d20a.d20ATarget = arg;
	
	d20a.d20Caf = D20CAF_HIT;
	if (dispKey == DK_SIG_TouchAttack){
		auto pytarget = PyObjHndl_Create(d20a.d20ATarget);
		auto args2 = Py_BuildValue("(O)", pytarget);
		Py_DECREF(pytarget);
		auto pytaResult = PyObjHandle_PerformTouchAttack(obj, args2);
		d20a.d20Caf = PyInt_AsLong(pytaResult);
	}

	d20Sys.d20SendSignal(self->handle, dispKey, &d20a, 0);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_BalorDeath(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	critterSys.BalorDeath(self->handle);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_ConcealedSet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	int concealed;
	if (!PyArg_ParseTuple(args, "i:objhndl.concealed_set", &concealed)) {
		return 0;
	}
	critterSys.SetConcealed(self->handle, concealed);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_Unconceal(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	auto result = gameSystems->GetAnim().PushUnconceal(self->handle);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_UseItem(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	objHndl item = objHndl::null;
	objHndl target = objHndl::null;
	if (!PyArg_ParseTuple(args, "O&|O&:objhndl.use_item", &ConvertObjHndl, &item, &ConvertObjHndl, &target)) {
		return 0;
	}
	if (!target) target = self->handle;
	uint32_t result = combatSys.UseItem(self->handle, item, target);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_PendingToMemorized(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	Stat classEnum = (Stat)-1;
	if (!PyArg_ParseTuple(args, "|i:objhndl.spells_pending_to_memorized", &classEnum)) {
		return 0;
	}

	if (classEnum == (Stat)-1)
		spellSys.SpellsPendingToMemorized(self->handle);
	else
		spellSys.SpellsPendingToMemorizedByClass(self->handle, classEnum);
	
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_SpellsCastReset(PyObject* obj, PyObject* args){
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	Stat classEnum = (Stat)-1;
	if (!PyArg_ParseTuple(args, "|i:objhndl.spells_cast_reset", &classEnum)) {
		return 0;
	}

	spellSys.SpellsCastReset(self->handle, classEnum);

	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_MemorizedForget(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		Py_RETURN_NONE;
	}
	bool pending = false;
	int percent = 100;
	if (!PyArg_ParseTuple(args, "|ii:objhndl.spells_memorized_forget", &pending, &percent)) {
		Py_RETURN_NONE;
	}
	spellSys.ForgetMemorized(self->handle, pending, percent);
	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_Resurrect(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		PyInt_FromLong(0);
	}
	ResurrectType type;
	int caster_level = 0;
	if (!PyArg_ParseTuple(args, "i|i:objhndl.resurrect", &type, &caster_level)) {
		return 0;
	}
	auto result = critterSys.Resurrect(self->handle, type, caster_level);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_Dominate(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		PyInt_FromLong(0);
	}
	objHndl caster;
	if (!PyArg_ParseTuple(args, "O&:objhndl.dominate", &ConvertObjHndl, &caster)) {
		return 0;
	}
	auto result = critterSys.Dominate(self->handle, caster);
	return PyInt_FromLong(result);
}



static PyObject* PyObjHandle_IsDeadNullDestroyed(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(1);
	}
	auto result = critterSys.IsDeadNullDestroyed(self->handle);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_IsUnconscious(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(1);
	}
	auto result = critterSys.IsDeadOrUnconscious(self->handle);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_IsSpellKnown(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(1);
	}

	int spellClass = -1;
	int spellEnum;

	if (!PyArg_ParseTuple(args, "i|i", &spellEnum, &spellClass)) {
		return PyInt_FromLong(false);
	}

	auto result = spellSys.IsSpellKnown(self->handle, spellEnum, spellClass);
	return PyInt_FromLong(result);
}


static PyObject* PyObjHandle_IsBuckler(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(1);
	}
	auto result = inventory.IsBuckler(self->handle);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_IsDoubleWeapon(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	auto result = inventory.IsDoubleWeapon(self->handle);
	return PyInt_FromLong(result);
}

static PyObject* PyObjHandle_IsThrowingWeapon(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}
	if (objects.GetType(self->handle) != obj_t_weapon)
	{
		logger->warn("Python is_throwing_weapon called with non weapon object: {}", self->handle);
		return PyInt_FromLong(0);
	}

	auto result = inventory.IsThrowingWeapon(self->handle);
	return PyInt_FromLong(result);
}


static PyObject * PyObjHandle_MakeWizard(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	uint32_t level;
	if (!PyArg_ParseTuple(args, "i", &level)) {
		return nullptr;
	}

	if (level <= 0 || level > 20){
		return PyInt_FromLong(0);
	}

	auto gameObj = objSystem->GetObject(self->handle);

	for (uint32_t i = 0; i < level; i++) {
		gameObj->SetInt32(obj_f_critter_level_idx, i, stat_level_wizard);
	}

	d20Sys.d20Status->D20StatusRefresh(self->handle);


	return PyInt_FromLong(1);
};

static PyObject* PyObjHandle_MakeAOO(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		return PyInt_FromLong(0);
	}

	objHndl targetObj;
	if (!PyArg_ParseTuple(args, "O&:objhndl.make_aoo_if_possible", &ConvertObjHndl, &targetObj)) {
		return 0;
	}

	if (!targetObj) {
		return PyInt_FromLong(0);
	}

	//Check if we are too close to stop the process early when too close
	auto polearmDonutReach = config.disableReachWeaponDonut ? false : true;
	float minReach = 0.0f;
	(void)critterSys.GetReach(self->handle, D20A_UNSPECIFIED_ATTACK, &minReach);
	auto distToTgt = max(0.0f, locSys.DistanceToObj(self->handle, targetObj));
	auto tooClose = polearmDonutReach && (minReach > 0.0f) && distToTgt < minReach;
	if (tooClose) {
		return PyInt_FromLong(0);
	}

	if (!combatSys.CanMeleeTarget(self->handle, targetObj))
		return PyInt_FromLong(0);
	if (self->handle == targetObj)
		return PyInt_FromLong(0);
	if (critterSys.IsFriendly(self->handle, targetObj))
		return PyInt_FromLong(0);
	if (!d20Sys.d20QueryWithData(self->handle, DK_QUE_AOOPossible, targetObj))
		return PyInt_FromLong(0);
	if (!d20Sys.d20QueryWithData(self->handle, DK_QUE_AOOWillTake, targetObj))
		return PyInt_FromLong(0);

	actSeqSys.DoAoo(self->handle, targetObj);
	actSeqSys.sequencePerform();
	
	return PyInt_FromLong(1);
}

static PyObject * PyObjHandle_MakeClass(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	int level;
	Stat statClass = stat_level_barbarian;
	if (!PyArg_ParseTuple(args, "ii", &statClass, &level)) {
		return nullptr;
	}

	if (level <= 0 || (uint32_t) level > config.maxLevel){
		return PyInt_FromLong(0);
	}

	auto gameObj = objSystem->GetObject(self->handle);
	for (int i = 0; i < level; i++) {
		gameObj->SetInt32(obj_f_critter_level_idx, i, statClass);
	}

	d20Sys.d20Status->D20StatusRefresh(self->handle);


	return PyInt_FromLong(1);
};

static PyMethodDef PyObjHandleMethods[] = {
	{"__getstate__", PyObjHandle_getstate, METH_VARARGS, NULL},
	{"__reduce__", PyObjHandle_reduce, METH_VARARGS, NULL},
	{"__setstate__", PyObjHandle_setstate, METH_VARARGS, NULL},

	{ "action_perform", PyObjHandle_ActionPerform, METH_VARARGS, NULL},

	{ "add_to_initiative", PyObjHandle_AddToInitiative, METH_VARARGS, NULL },
	{ "ai_flee_add", PyObjHandle_AiFleeAdd, METH_VARARGS, NULL },
	{ "ai_follower_add", PyObjHandle_AiFollowerAdd, METH_VARARGS, NULL },
	{ "ai_follower_remove", ReturnZero, METH_VARARGS, NULL },
	{ "ai_follower_atmax", ReturnZero, METH_VARARGS, NULL },
	{ "ai_shitlist_add", PyObjHandle_AiShitlistAdd, METH_VARARGS, NULL },
	{ "ai_shitlist_remove", PyObjHandle_AiShitlistRemove, METH_VARARGS, NULL },
	{ "ai_stop_attacking", PyObjHandle_AiStopAttacking, METH_VARARGS, NULL },

	{ "ai_strategy_set_custom", PyObjHandle_AiStrategySetCustom, METH_VARARGS, NULL },

	{ "allegiance_shared", PyObjHandle_AllegianceShared, METH_VARARGS, NULL },
	{ "allegiance_strength", PyObjHandle_AllegianceStrength, METH_VARARGS, NULL },
	{ "anim_callback", PyObjHandle_AnimCallback, METH_VARARGS, NULL },
	{ "anim_goal_interrupt", PyObjHandle_AnimGoalInterrupt, METH_VARARGS, NULL },
	{ "anim_goal_push", PyObjHandle_AnimGoalPush, METH_VARARGS, NULL },
	{ "anim_goal_push_attack", PyObjHandle_AnimGoalPushAttack, METH_VARARGS, NULL },
	{ "anim_goal_push_dodge", PyObjHandle_AnimGoalPushDodge, METH_VARARGS, NULL },
	{ "anim_goal_push_hit_by_weapon", PyObjHandle_AnimGoalPushHitByWeapon, METH_VARARGS, NULL },
	{ "anim_goal_use_object", PyObjHandle_AnimGoalPushUseObject, METH_VARARGS, NULL },
	{ "anim_goal_get_new_id", PyObjHandle_AnimGoalGetNewId, METH_VARARGS, NULL },
	{ "apply_projectile_particles", PyObjHandle_ApplyProjectileParticles, METH_VARARGS, NULL },
	{ "apply_projectile_hit_particles", PyObjHandle_ApplyProjectileHitParticles, METH_VARARGS, NULL },
	{ "arcane_spell_level_can_cast", PyObjHandle_ArcaneSpellLevelCanCast, METH_VARARGS, NULL },
	{ "arcane_spontaneous_spell_level_can_cast", PyObjHandle_ArcaneSpontaniousSpellLevelCanCast, METH_VARARGS, NULL },
	{ "arcane_vancian_spell_level_can_cast", PyObjHandle_ArcaneVancianSpellLevelCanCast, METH_VARARGS, NULL },
	{ "attack", PyObjHandle_Attack, METH_VARARGS, NULL },
	{ "award_experience", PyObjHandle_AwardExperience, METH_VARARGS, NULL },


	{"balor_death", PyObjHandle_BalorDeath, METH_VARARGS, NULL },
	{"begin_dialog", PyObjHandle_BeginDialog, METH_VARARGS, NULL},
	{"begian_dialog", PyObjHandle_BeginDialog, METH_VARARGS, "I make this typo so much that I want it supported :P" },
	{"barter", PyObjHandle_Barter, METH_VARARGS, NULL },

	{"cast_spell", PyObjHandle_CastSpell, METH_VARARGS, NULL },
	{"can_cast_spell", PyObjHandle_CanCastSpell, METH_VARARGS, NULL },
	{"can_find_path_to_obj", PyObjHandle_CanFindPathToObj, METH_VARARGS, NULL },
	{"find_path_to_obj", PyObjHandle_FindPathToObj, METH_VARARGS, NULL },
	{ "can_hear", PyObjHandle_CanHear, METH_VARARGS, NULL},
	{"can_melee", PyObjHandle_CanMelee, METH_VARARGS, NULL },
	{"can_see", PyObjHandle_HasLos, METH_VARARGS, NULL },
	{"can_blindsee", PyObjHandle_CanBlindsee, METH_VARARGS, "obj has blind sight, and target is within observation range of blindsight ability"},
	{"can_sense", PyObjHandle_CanSense, METH_VARARGS, NULL },
	{ "can_sneak_attack", PyObjHandle_CanSneakAttack, METH_VARARGS, NULL },
	{ "concealed_set", PyObjHandle_ConcealedSet, METH_VARARGS, NULL },
	{ "condition_add_with_args", PyObjHandle_ConditionAddWithArgs, METH_VARARGS, NULL },
	{ "condition_add", PyObjHandle_ConditionAddWithArgs, METH_VARARGS, NULL },
	{ "conditions_get", PyObjHandle_ConditionsGet, METH_VARARGS, NULL },
	{ "container_flags_get", GetFlags<obj_f_container_flags>, METH_VARARGS, NULL },
	{ "container_flag_set", SetFlag<obj_f_container_flags>, METH_VARARGS, NULL },
	{ "container_flag_unset", ClearFlag<obj_f_container_flags>, METH_VARARGS, NULL },
	{ "container_toggle_open", PyObjHandle_ContainerToggleOpen, METH_VARARGS, NULL },
	{ "container_open_ui", PyObjHandle_ContainerOpenUI, METH_VARARGS, NULL },
	
	{ "critter_flags_get", GetFlags<obj_f_critter_flags>, METH_VARARGS, NULL },
	{ "critter_flag_set", SetFlag<obj_f_critter_flags>, METH_VARARGS, NULL },
	{ "critter_flag_unset", ClearFlag<obj_f_critter_flags>, METH_VARARGS, NULL },
	{ "critter_get_alignment", PyObjHandle_CritterGetAlignment, METH_VARARGS, NULL },
	{ "critter_kill", PyObjHandle_Kill, METH_VARARGS, NULL },
	{ "critter_kill_by_effect", PyObjHandle_KillByEffect, METH_VARARGS, NULL },
	{ "critter_banish", PyObjHandle_Banish, METH_VARARGS, NULL },

	{ "d20_query", PyObjHandle_D20Query, METH_VARARGS, NULL },
	{ "d20_query_has_spell_condition", PyObjHandle_D20QueryHasSpellCond, METH_VARARGS, NULL },
	{ "d20_query_has_condition", PyObjHandle_D20QueryHasCond, METH_VARARGS, NULL },
	{ "d20_query_with_data", PyObjHandle_D20QueryWithData, METH_VARARGS, NULL },
    { "d20_query_with_object", PyObjHandle_D20QueryWithObject, METH_VARARGS, NULL },
	{ "d20_query_test_data", PyObjHandle_D20QueryTestData, METH_VARARGS, NULL },
	{ "d20_query_get_data", PyObjHandle_D20QueryGetData, METH_VARARGS, NULL },
	{ "d20_query_get_obj", PyObjHandle_D20QueryGetObj, METH_VARARGS, NULL },
	{ "d20_send_signal", PyObjHandle_D20SendSignal, METH_VARARGS, NULL },
	{ "d20_send_signal_ex", PyObjHandle_D20SendSignalEx, METH_VARARGS, NULL },
	{ "d20_status_init", PyObjHandle_D20StatusInit, METH_VARARGS, NULL },
	{ "damage", PyObjHandle_Damage, METH_VARARGS, NULL },
	{ "damage_with_reduction", PyObjHandle_DamageWithReduction, METH_VARARGS, NULL },
	{ "deal_attack_damage", PyObjHandle_DealAttackDamage, METH_VARARGS, NULL },
	{ "destroy", PyObjHandle_Destroy, METH_VARARGS, NULL },
	{ "distance_to", PyObjHandle_DistanceTo, METH_VARARGS, NULL },
	{ "divine_spell_level_can_cast", PyObjHandle_DivineSpellLevelCanCast, METH_VARARGS, NULL },
	{ "dominate", PyObjHandle_Dominate, METH_VARARGS, NULL },

	{ "faction_has", PyObjHandle_FactionHas, METH_VARARGS, "Check if NPC has faction. Doesn't work on PCs!" },
	{ "faction_add", PyObjHandle_FactionAdd, METH_VARARGS, "Adds faction to NPC. Doesn't work on PCs!" },
	{ "fade_to", PyObjHandle_FadeTo, METH_VARARGS, NULL },
	{ "feat_add", PyObjHandle_FeatAdd, METH_VARARGS, "Gives you a feat" },
	{ "fall_down", PyObjHandle_FallDown, METH_VARARGS, "Makes a Critter fall down" },
	{ "follower_add", PyObjHandle_FollowerAdd, METH_VARARGS, NULL },
	{ "follower_remove", PyObjHandle_FollowerRemove, METH_VARARGS, NULL },
	{ "follower_atmax", PyObjHandle_FollowerAtMax, METH_VARARGS, NULL },
	{ "footstep", PyObjHandle_Footstep, METH_VARARGS, NULL },
	{ "float_line", PyObjHandle_FloatLine, METH_VARARGS, NULL },
	{ "float_mesfile_line", PyObjHandle_FloatMesFileLine, METH_VARARGS, NULL },
	{ "float_text_line", PyObjHandle_FloatTextLine, METH_VARARGS, NULL },

	{ "get_ammo_used", PyObjHandle_GetAmmoUsed, METH_VARARGS, "gets the handle of the ammo object. Checks validity - does it match the weapon? Otherwise returns null"},
	{ "get_base_attack_bonus", PyObjHandle_GetBaseAttackBonus, METH_VARARGS, NULL },
	{ "get_category_type", PyObjHandle_GetCategoryType, METH_VARARGS, NULL },
	{ "get_character_classes", PyObjHandle_GetCharacterAllClassesSet, METH_VARARGS, "Get tuple with classes enums" },
	{ "get_character_base_classes", PyObjHandle_GetCharacterBaseClassesSet, METH_VARARGS, "Get tuple with base classes enums" },
	{ "get_initiative", PyObjHandle_GetInitiative, METH_VARARGS, NULL },
	{ "get_item_wear_flags", PyObjHandle_GetItemWearFlags, METH_VARARGS, NULL },
	{ "get_level_for_spell_selection", PyObjHandle_GetLevelForSpellSelection, METH_VARARGS, NULL },
	{ "get_max_dex_bonus", PyObjHandle_GetMaxDexBonus, METH_VARARGS, NULL },
	{ "get_num_spells_per_day", PyObjHandle_GetNumSpellsPerDay, METH_VARARGS, NULL },
	{ "get_num_spells_used", PyObjHandle_GetNumSpellsUsed, METH_VARARGS, NULL },
    { "get_deity", PyObjHandle_GetDeity, METH_VARARGS, NULL },
	{ "get_weapon_type", PyObjHandle_GetWeaponType, METH_VARARGS, NULL },
	{ "get_wield_type", PyObjHandle_GetWieldType, METH_VARARGS, NULL },
	{ "get_weapon_projectile_proto", PyObjHandle_GetWeaponProjectileProto, METH_VARARGS, NULL },
	{ "group_list", PyObjHandle_GroupList, METH_VARARGS, NULL },
	
	
	{ "has_atoned", PyObjHandle_HasAtoned, METH_VARARGS, NULL },
	{ "has_feat", PyObjHandle_HasFeat, METH_VARARGS, NULL },
	{ "has_follower", PyObjHandle_HasFollower, METH_VARARGS, NULL },
	{ "has_item", PyObjHandle_HasItem, METH_VARARGS, NULL },
	{ "has_los", PyObjHandle_HasLos, METH_VARARGS, NULL },
	{ "has_met", PyObjHandle_HasMet, METH_VARARGS, NULL },
	{ "has_spell_effects", PyObjHandle_HasSpellEffects, METH_VARARGS, NULL },
	{ "has_wielded", PyObjHandle_HasWielded, METH_VARARGS, NULL },
	{ "hasMet", PyObjHandle_HasMet, METH_VARARGS, NULL },
	{ "heal", PyObjHandle_Heal, METH_VARARGS, NULL },
	{ "healsubdual", PyObjHandle_HealSubdual, METH_VARARGS, NULL },
	

	{ "identify_all", PyObjHandle_IdentifyAll, METH_VARARGS, NULL },
	{ "inventory_item", PyObjHandle_InventoryItem, METH_VARARGS, NULL },
	{ "is_active_combatant", PyObjHandle_IsActiveCombatant, METH_VARARGS, NULL },
	{ "is_arcane_spell_class", PyObjHandle_IsArcaneSpellClass, METH_VARARGS, NULL },
	{ "is_buckler", PyObjHandle_IsBuckler, METH_VARARGS, NULL },
	{ "is_double_weapon", PyObjHandle_IsDoubleWeapon, METH_VARARGS, NULL },
	{ "is_category_type", PyObjHandle_IsCategoryType, METH_VARARGS, NULL },
	{ "is_category_subtype", PyObjHandle_IsCategorySubtype, METH_VARARGS, NULL },
	{ "is_critter", PyObjHandle_IsCritter, METH_VARARGS, NULL},
	{ "is_dead_or_destroyed", PyObjHandle_IsDeadNullDestroyed, METH_VARARGS, NULL},
	{ "is_favored_enemy", PyObjHandle_FavoredEnemy, METH_VARARGS, NULL },
	{ "is_flanked_by", PyObjHandle_IsFlankedBy, METH_VARARGS, NULL },
	{ "is_friendly", PyObjHandle_IsFriendly, METH_VARARGS, NULL },
	{ "is_spell_known", PyObjHandle_IsSpellKnown, METH_VARARGS, NULL },
	{ "is_unconscious", PyObjHandle_IsUnconscious, METH_VARARGS, NULL },
	{ "is_throwing_weapon", PyObjHandle_IsThrowingWeapon, METH_VARARGS, NULL },
	{ "is_thrown_only_weapon", PyObjHandle_IsThrownOnlyWeapon, METH_VARARGS, NULL },
	{ "item_condition_add_with_args", PyObjHandle_ItemConditionAdd, METH_VARARGS, NULL },
	{ "item_condition_has", PyObjHandle_ItemConditionHas, METH_VARARGS, NULL },
	{ "item_condition_remove", PyObjHandle_ItemConditionRemove, METH_VARARGS, NULL },
	{ "item_has_condition", PyObjHandle_ItemConditionHas, METH_VARARGS, NULL },
	{ "item_d20_query", PyObjHandle_ItemD20Query, METH_VARARGS, NULL },
	{ "item_find", PyObjHandle_ItemFind, METH_VARARGS, NULL },
	{ "item_get", PyObjHandle_ItemGet, METH_VARARGS, NULL },
	{ "item_transfer_to", PyObjHandle_ItemTransferTo, METH_VARARGS, NULL },
	{ "item_find_by_proto", PyObjHandle_ItemFindByProto, METH_VARARGS, NULL },
	{ "item_transfer_to_by_proto", PyObjHandle_ItemTransferToByProto, METH_VARARGS, NULL },
	{ "item_worn_at", PyObjHandle_ItemWornAt, METH_VARARGS, NULL },
	{ "item_flags_get", GetFlags<obj_f_item_flags>, METH_VARARGS, NULL },
	{ "item_flag_set", SetFlag<obj_f_item_flags>, METH_VARARGS, NULL },
	{ "item_flag_unset", ClearFlag<obj_f_item_flags>, METH_VARARGS, NULL },
	{ "item_worn_unwield", PyObjHandle_Unwield, METH_VARARGS, NULL },
	{ "item_wield", PyObjHandle_Wield, METH_VARARGS, NULL },
	{ "item_wield_best_all", PyObjHandle_WieldBestAll, METH_VARARGS, NULL },


	{ "leader_get", PyObjHandle_LeaderGet, METH_VARARGS, NULL },

	{ "make_wiz", PyObjHandle_MakeWizard, METH_VARARGS, "Makes you a wizard of level N" },
	{ "make_aoo_if_possible", PyObjHandle_MakeAOO, METH_VARARGS, "Perform an AOO against opponent" },
	{ "make_class", PyObjHandle_MakeClass, METH_VARARGS, "Makes you a CLASS N of level M" },
	{ "money_get", PyObjHandle_MoneyGet, METH_VARARGS, NULL},
	{ "money_adj", PyObjHandle_MoneyAdj, METH_VARARGS, NULL},
	{ "move", PyObjHandle_Move, METH_VARARGS, NULL },

	{ "npc_flags_get", GetFlags<obj_f_npc_flags>, METH_VARARGS, NULL },
	{ "npc_flag_set", SetFlag<obj_f_npc_flags>, METH_VARARGS, NULL },
	{ "npc_flag_unset", ClearFlag<obj_f_npc_flags>, METH_VARARGS, NULL },
	{ "npc_waypoints_get", PyObjHandle_GetWaypoints, METH_VARARGS, "Gets Waypoints list" },
	{ "npc_waypoints_set", PyObjHandle_SetWaypoints, METH_VARARGS, "Sets Waypoints from list of dict" },

	{ "object_event_append", PyObjHandle_ObjectEventAppend, METH_VARARGS, NULL },
	{ "object_event_append_wall", PyObjHandle_WallEventAppend, METH_VARARGS, NULL },
	{ "obj_del_idx_int", PyObjHandle_DelIdxInt, METH_VARARGS, NULL },
	{ "obj_get_int", PyObjHandle_GetInt, METH_VARARGS, NULL },
	{ "obj_get_idx_int", PyObjHandle_GetIdxInt, METH_VARARGS, NULL },
	{ "obj_get_idx_int64", PyObjHandle_GetIdxInt64, METH_VARARGS, NULL },
	{ "obj_get_idx_int_size", PyObjHandle_GetIdxIntSize, METH_VARARGS, NULL },
	{ "obj_get_idx_int64_size", PyObjHandle_GetIdxInt64Size, METH_VARARGS, NULL },
	{ "obj_get_int64", PyObjHandle_GetInt64, METH_VARARGS, "Gets 64 bit field" },
	{ "obj_get_obj", PyObjHandle_GetObj, METH_VARARGS, "Gets Object field" },
	{ "obj_get_string", PyObjHandle_GetString, METH_VARARGS, NULL },
	{ "obj_get_idx_obj", PyObjHandle_GetIdxObj, METH_VARARGS, "Gets Object Array field" },
	{ "obj_get_idx_obj_size", PyObjHandle_GetIdxObjSize, METH_VARARGS, "Gets Object Array field" },
	{ "obj_get_spell", PyObjHandle_GetSpell, METH_VARARGS, NULL },
	{ "obj_remove_from_all_groups", PyObjHandle_RemoveFromAllGroups, METH_VARARGS, "Removes the object from all the groups (GroupList, PCs, NPCs, AI controlled followers, Currently Selected" },
	{ "obj_set_int", PyObjHandle_SetInt, METH_VARARGS, NULL },
	{ "obj_set_float", PyObjHandle_SetFloat, METH_VARARGS, NULL },
	{ "obj_set_obj", PyObjHandle_SetObj, METH_VARARGS, NULL },
	{ "obj_set_string", PyObjHandle_SetString, METH_VARARGS, NULL },
	{ "obj_set_idx_int", PyObjHandle_SetIdxInt, METH_VARARGS, NULL },
	{ "obj_set_int64", PyObjHandle_SetInt64, METH_VARARGS, NULL },
	{ "obj_set_idx_int64", PyObjHandle_SetIdxInt64, METH_VARARGS, NULL },
	{ "object_flags_get", GetFlags<obj_f_flags>, METH_VARARGS, NULL },
	{ "object_flag_set", PyObjHandle_ObjectFlagSet, METH_VARARGS, NULL },
	{ "object_flag_unset", PyObjHandle_ObjectFlagUnset, METH_VARARGS, NULL },
	{ "object_script_execute", PyObjHandle_ObjectScriptExecute, METH_VARARGS, NULL },

	{ "pc_add", PyObjHandle_PCAdd, METH_VARARGS, "Adds object as a PC party member." },
	{ "perform_touch_attack", PyObjHandle_PerformTouchAttack, METH_VARARGS, NULL },
	{ "portal_flags_get", GetFlags<obj_f_portal_flags>, METH_VARARGS, NULL },
	{ "portal_flag_set", SetFlag<obj_f_portal_flags>, METH_VARARGS, NULL },
	{ "portal_flag_unset", ClearFlag<obj_f_portal_flags>, METH_VARARGS, NULL },
	{ "portal_toggle_open", PyObjHandle_PortalToggleOpen, METH_VARARGS, NULL },


	{ "reaction_get", PyObjHandle_ReactionGet, METH_VARARGS, NULL },
	{ "reaction_set", PyObjHandle_ReactionSet, METH_VARARGS, NULL },
	{ "reaction_adj", PyObjHandle_ReactionAdjust, METH_VARARGS, NULL },
	{ "reflex_save_and_damage", PyObjHandle_ReflexSaveAndDamage, METH_VARARGS, NULL },
	{ "refresh_turn", PyObjHandle_RefreshTurn, METH_VARARGS, NULL },
	{ "reputation_has", PyObjHandle_ReputationHas, METH_VARARGS, NULL },
	{ "reputation_add", PyObjHandle_ReputationAdd, METH_VARARGS, NULL },
	{ "remove_from_initiative", PyObjHandle_RemoveFromInitiative, METH_VARARGS, NULL },
	{ "reputation_remove", PyObjHandle_ReputationRemove, METH_VARARGS, NULL },
	{ "resurrect", PyObjHandle_Resurrect, METH_VARARGS, NULL },
	{ "rumor_log_add", PyObjHandle_RumorLogAdd, METH_VARARGS, NULL },
	{ "runoff", PyObjHandle_RunOff, METH_VARARGS, NULL },
	{ "run_to", PyObjHandle_RunTo, METH_VARARGS, NULL },

	{ "saving_throw", PyObjHandle_SavingThrow, METH_VARARGS, NULL },
	{ "saving_throw_with_args", PyObjHandle_SavingThrow, METH_VARARGS, NULL },
	{ "saving_throw_spell", PyObjHandle_SavingThrowSpell, METH_VARARGS, NULL },
	{ "secretdoor_detect", PyObjHandle_SecretdoorDetect, METH_VARARGS, NULL },
	{ "set_hp_damage", PyObjHandle_SetHpDamage, METH_VARARGS, NULL },
	{ "set_initiative", PyObjHandle_SetInitiative, METH_VARARGS, NULL },
	{ "set_subdual_damage", PyObjHandle_SetSubdualDamage, METH_VARARGS, NULL },
	{ "skill_level_get", PyObjHandle_SkillLevelGet, METH_VARARGS, NULL},
	{ "skill_ranks_get", PyObjHandle_SkillRanksGet, METH_VARARGS, NULL },
	{ "skill_ranks_set", PyObjHandle_SkillRanksSet, METH_VARARGS, NULL },
	{ "skill_roll", PyObjHandle_SkillRoll, METH_VARARGS, NULL },
	{ "skill_roll_delta", PyObjHandle_SkillRollDelta, METH_VARARGS, "Does a Skill Roll, but returns the delta from the skill roll DC." },
	{ "soundmap_critter", PyObjHandle_SoundmapCritter, METH_VARARGS, NULL },
	{ "soundmap_item", PyObjHandle_SoundmapItem, METH_VARARGS, NULL },
	{ "sound_play_friendly_fire", PyObjHandle_SoundPlayFriendlyFire, METH_VARARGS, NULL},
	{ "spell_known_add", PyObjHandle_SpellKnownAdd, METH_VARARGS, NULL },
	{ "spell_known_add_to_char_class", PyObjHandle_SpellKnownAddToCharClass, METH_VARARGS, NULL },
	{ "spell_memorized_add", PyObjHandle_SpellMemorizedAdd, METH_VARARGS, NULL },
	{ "spell_damage", PyObjHandle_SpellDamage, METH_VARARGS, NULL },
	{ "spell_damage_with_reduction", PyObjHandle_SpellDamageWithReduction, METH_VARARGS, NULL },
	{ "spell_damage_weaponlike", PyObjHandle_SpellDamageWeaponlike, METH_VARARGS, NULL },
	{ "spell_heal", PyObjHandle_SpellHeal, METH_VARARGS, NULL },
	{ "spells_pending_to_memorized", PyObjHandle_PendingToMemorized, METH_VARARGS, NULL },
	{ "spells_cast_reset", PyObjHandle_SpellsCastReset, METH_VARARGS, NULL },
	{ "spells_memorized_forget", PyObjHandle_MemorizedForget, METH_VARARGS, NULL },
	{ "spontaneous_spell_level_can_cast", PyObjHandle_SpontaneousSpellLevelCanCast, METH_VARARGS, NULL },
	{ "spontaneous_spells_remaining", PyObjHandle_SpontaneousSpellsRemaining, METH_VARARGS, NULL },
	{ "spontaneous_spells_remove", PyObjectHandle_SpontaneousSpellsRemove, METH_VARARGS, NULL },
	{ "standpoint_get", PyObjHandle_StandpointGet, METH_VARARGS, NULL },
	{ "standpoint_set", PyObjHandle_StandpointSet, METH_VARARGS, NULL },
	{"stat_level_get", PyObjHandle_StatLevelGet, METH_VARARGS, NULL},
	{"stat_base_get", PyObjHandle_StatLevelGetBase, METH_VARARGS, NULL},
	{"stat_base_set", PyObjHandle_StatLevelSetBase, METH_VARARGS, NULL},
	{"steal_from", PyObjHandle_StealFrom, METH_VARARGS, NULL },

	{"turn_towards", PyObjHandle_TurnTowards, METH_VARARGS, NULL},
	{"turn_towards_loc", PyObjHandle_TurnTowardsLoc, METH_VARARGS, NULL},
	{"trip_check", PyObjHandle_TripCheck, METH_VARARGS, NULL },

	{ "unconceal", PyObjHandle_Unconceal, METH_VARARGS, NULL },
	{ "use_item", PyObjHandle_UseItem, METH_VARARGS, NULL },

	{NULL, NULL, NULL, NULL}
};

#pragma endregion 

#pragma region Getters and Setters

static PyObject* PyObjHandle_GetArea(PyObject* obj, void*) {
	return PyInt_FromLong(maps.GetCurrentAreaId());
}

static PyObject* PyObjHandle_GetMap(PyObject* obj, void*) {
	return PyInt_FromLong(maps.GetCurrentMapId());
}

static PyObject* PyObjHandle_GetDescription(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyString_FromString(objects.description.getDisplayName(self->handle));
}

static PyObject* PyObjHandle_GetNameId(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	if (!self->handle){
		return PyInt_FromLong(0);
	}
	return PyInt_FromLong(objects.GetNameId(self->handle));
}

static PyObject* PyObjHandle_GetProto(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	if (!self->handle){
		logger->warn("obj.proto called with null handle! Returning 0");
		return PyInt_FromLong(0);
	}
	return PyInt_FromLong(gameSystems->GetObj().GetProtoId(self->handle));
}

static PyObject* PyObjHandle_GetLocation(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	if (!self->handle){
		logger->warn("obj.location called with null handle!");
		return PyLong_FromLongLong(0);
	}
	return PyLong_FromLongLong(objects.GetLocation(self->handle));
}
static PyObject* PyObjHandle_GetLocationFull(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		logger->warn("obj.location called with null handle!");
		return PyLong_FromLongLong(0);
	}
	auto locFull = objects.GetLocationFull(self->handle);
	py::object pyLoc = py::cast(locFull);
	pyLoc.inc_ref();
	return pyLoc.ptr();
}

static PyObject* PyObjHandle_GetType(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	if (!self->handle){
		logger->warn("obj.type called with null handle!");
		return PyInt_FromLong(0);
	}
	return PyInt_FromLong(objects.GetType(self->handle));
}

static PyObject* PyObjHandle_GetRadius(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		logger->warn("obj.radius called with null handle!");
		return PyFloat_FromDouble(0.0);
	}
	return PyFloat_FromDouble(objects.GetRadius(self->handle));
}

static int PyObjHandle_SetRadius(PyObject* obj, PyObject* value, void*) {
	auto self = GetSelf(obj);
	if (!self->handle)
		return 0;
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
	if (!self->handle) {
		logger->warn("obj.offset_x called with null handle!");
		return PyFloat_FromDouble(0.0);
	}
	return PyFloat_FromDouble(objects.GetOffsetX(self->handle));
}

static PyObject* PyObjHandle_GetOffsetY(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	if (!self->handle) {
		logger->warn("obj.offset_y called with null handle!");
		return PyFloat_FromDouble(0.0);
	}
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

static int PyObjHandle_SetSubstituteInventory(PyObject* obj, PyObject* value, void*) {
	auto self = GetSelf(obj);

	if (!self->handle)
		return -1;

	auto subsObj = GetSelf(value);
	auto gameObj = objSystem->GetObject(self->handle);
	gameObj->SetObjHndl(obj_f_npc_substitute_inventory, subsObj->handle);

	return 0;
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

static PyObject* PyObjHandle_GetFactions(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	objHndl objHnd = self->handle;
	
	int factionArray[50] = {0,};
	int numFactions = 0;

	for (int i = 0; i < 50; i++){
		int fac = objects.getArrayFieldInt32(objHnd, obj_f_npc_faction, i);
		if (fac == 0) break;
		factionArray[i] = fac;
		numFactions++;
	}

	auto outTup = PyTuple_New(numFactions);
	for (int i = 0; i < numFactions; i++){
		PyTuple_SET_ITEM(outTup, i, PyInt_FromLong(factionArray[i]));
	};


	return  outTup;
}

static PyObject* PyObjHandle_GetCharacterClasses(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	objHndl objHnd = self->handle;
	if (!objHnd)
		Py_RETURN_NONE;

	auto toEEobj = gameSystems->GetObj().GetObject(objHnd);
	auto charClasses = toEEobj->GetInt32Array(obj_f_critter_level_idx);
	int count = charClasses.GetSize();

	auto outTup = PyTuple_New(count);
	for (int i = 0; i < count; i++) {
		PyTuple_SET_ITEM(outTup, i, PyInt_FromLong(charClasses[i]));
	};
	return  outTup;
}

static PyObject* PyObjHandle_GetHighestArcaneClass(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	objHndl objHnd = self->handle;
	if (!objHnd) {
		return PyInt_FromLong(0);
	}

	auto highestClass = (Stat)0;
	auto highestLvl = 0;

	for (auto it : d20ClassSys.classEnumsWithSpellLists) {
		auto classEnum = (Stat)it;
		if (d20ClassSys.IsArcaneCastingClass(classEnum)) {
			auto lvlThis = objects.StatLevelGet(objHnd, classEnum);
			if (lvlThis > highestLvl) {
				highestLvl = lvlThis;
				highestClass = classEnum;
			}
		}
	}

	return PyInt_FromLong(highestClass);
}


static PyObject* PyObjHandle_GetHighestSpontaneousArcaneClass(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	objHndl objHnd = self->handle;
	if (!objHnd) {
		return PyInt_FromLong(0);
	}

	auto highestClass = (Stat)0;
	auto highestLvl = 0;

	for (auto it : d20ClassSys.classEnumsWithSpellLists) {
		auto classEnum = (Stat)it;
		if (d20ClassSys.IsArcaneCastingClass(classEnum) && d20ClassSys.IsNaturalCastingClass(classEnum)) {
			auto lvlThis = objects.StatLevelGet(objHnd, classEnum);
			if (lvlThis > highestLvl) {
				highestLvl = lvlThis;
				highestClass = classEnum;
			}
		}
	}

	return PyInt_FromLong(highestClass);
}


static PyObject* PyObjHandle_GetHighestVancianArcaneClass(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	objHndl objHnd = self->handle;
	if (!objHnd) {
		return PyInt_FromLong(0);
	}

	auto highestClass = (Stat)0;
	auto highestLvl = 0;

	for (auto it : d20ClassSys.classEnumsWithSpellLists) {
		auto classEnum = (Stat)it;
		if (d20ClassSys.IsArcaneCastingClass(classEnum) && d20ClassSys.IsVancianCastingClass(classEnum)) {
			auto lvlThis = objects.StatLevelGet(objHnd, classEnum);
			if (lvlThis > highestLvl) {
				highestLvl = lvlThis;
				highestClass = classEnum;
			}
		}
	}

	return PyInt_FromLong(highestClass);
}


static PyObject* PyObjHandle_GetHighestDivineClass(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	objHndl objHnd = self->handle;
	if (!objHnd) {
		return PyInt_FromLong(0);
	}

	auto highestClass = (Stat)0;
	auto highestLvl = 0;

	for (auto it : d20ClassSys.classEnumsWithSpellLists) {
		auto classEnum = (Stat)it;
		if (d20ClassSys.IsDivineCastingClass(classEnum)) {
			auto lvlThis = objects.StatLevelGet(objHnd, classEnum);
			if (lvlThis > highestLvl){
				highestLvl = lvlThis;
				highestClass = classEnum;
			}
		}
	}

	return PyInt_FromLong(highestClass);
}


//Gets the highest divine caster level
static PyObject* PyObjHandle_GetHighestDivineCasterLevel(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	objHndl objHnd = self->handle;
	if (!objHnd) {
		return PyInt_FromLong(0);
	}

	int highestCasterLvl = 0;

	for (auto it : d20ClassSys.classEnumsWithSpellLists) {
		auto classEnum = (Stat)it;
		if (d20ClassSys.IsDivineCastingClass(classEnum)) {
			auto currentCasterLevel = objects.StatLevelGet(objHnd, stat_caster_level, classEnum);
			highestCasterLvl = std::max(highestCasterLvl, currentCasterLevel);
		}
	}

	return PyInt_FromLong(highestCasterLvl);
}


//Gets the highest arcane caster level
static PyObject* PyObjHandle_GetHighestArcaneCasterLevel(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	objHndl objHnd = self->handle;
	if (!objHnd) {
		return PyInt_FromLong(0);
	}

	int highestCasterLvl = 0;

	for (auto it : d20ClassSys.classEnumsWithSpellLists) {
		auto classEnum = (Stat)it;
		if (d20ClassSys.IsArcaneCastingClass(classEnum)) {
			auto currentCasterLevel = objects.StatLevelGet(objHnd, stat_caster_level, classEnum);
			highestCasterLvl = std::max(highestCasterLvl, currentCasterLevel);
		}
	}

	return PyInt_FromLong(highestCasterLvl);
}


static PyObject* PyObjHandle_GetSpellsKnown(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	if (!self->handle)
		Py_RETURN_NONE;

	auto gameobj = objSystem->GetObject(self->handle);
	auto numKnown = gameobj->GetSpellArray(obj_f_critter_spells_known_idx).GetSize();

	auto result = PyTuple_New(numKnown);
	for (size_t i = 0; i < numKnown; ++i) {
		PyTuple_SET_ITEM(result, i, PySpellStore_Create(gameobj->GetSpell(obj_f_critter_spells_known_idx, i)));
	}
	return result;
}

static PyObject* PyObjHandle_GetSpellsMemorized(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	if (!self->handle)
		Py_RETURN_NONE;

	auto gameobj = objSystem->GetObject(self->handle);
	auto numMem = gameobj->GetSpellArray(obj_f_critter_spells_memorized_idx).GetSize();

	auto result = PyTuple_New(numMem);
	for (size_t i = 0; i < numMem; ++i) {
		PyTuple_SET_ITEM(result, i, PySpellStore_Create(gameobj->GetSpell(obj_f_critter_spells_memorized_idx, i)));
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

static PyObject* PyObjHandle_GetID(PyObject* obj, void*) {
	auto self = (PyObjHandle*)obj;

	if (!self->handle) {
		return PyString_FromString("OBJ_HANDLE_NULL");
	}
	else {
		auto name = self->id.ToString();
		return PyString_FromString(name.c_str());
	}
}

PyGetSetDef PyObjHandleGetSets[] = {
	{ "area", PyObjHandle_GetArea, NULL, NULL, NULL },
	{"char_classes", PyObjHandle_GetCharacterClasses, NULL, "a tuple containing the character classes array", NULL },
	{ "highest_arcane_class", PyObjHandle_GetHighestArcaneClass, NULL, "Highest Arcane spell casting class", NULL },
	{ "highest_vancian_arcane_class", PyObjHandle_GetHighestVancianArcaneClass, NULL, "Highest Vancian Arcane spell casting class", NULL },
	{ "highest_spontaneous_arcane_class", PyObjHandle_GetHighestSpontaneousArcaneClass, NULL, "Highest Natural Arcane spell casting class", NULL },
	{ "highest_divine_class", PyObjHandle_GetHighestDivineClass, NULL, "Highest Divine spell casting class", NULL },
    { "highest_arcane_caster_level", PyObjHandle_GetHighestArcaneCasterLevel, NULL, "Highest Arcane caster level", NULL },
    { "highest_divine_caster_level", PyObjHandle_GetHighestDivineCasterLevel, NULL, "Highest Divine caster level", NULL },
	{"description", PyObjHandle_GetDescription, NULL, NULL },
	{"name", PyObjHandle_GetNameId, NULL, NULL},
	{"location", PyObjHandle_GetLocation, NULL, NULL},
	{ "location_full", PyObjHandle_GetLocationFull, NULL, NULL },
	{"type", PyObjHandle_GetType, NULL, NULL},
	{"radius", PyObjHandle_GetRadius, PyObjHandle_SetRadius, NULL},
	{"height", PyObjHandle_GetRenderHeight, PyObjHandle_SetRenderHeight, NULL},
	{"rotation", PyObjHandle_GetRotation, PyObjHandle_SetRotation, NULL},
	{"map", PyObjHandle_GetMap, NULL, NULL, NULL},
	{"hit_dice", PyObjHandle_GetHitDice, NULL, NULL},
	{"hit_dice_num", PyObjHandle_GetHitDiceNum, NULL, NULL},
	{"get_size", PyObjHandle_GetSize, NULL, NULL},
	{"off_x", PyObjHandle_GetOffsetX, NULL, NULL},
	{"off_y", PyObjHandle_GetOffsetY, NULL, NULL},
	{"scripts", PyObjHandle_GetScripts, NULL, NULL},
	{"origin", PyObjHandle_GetOriginMapId, PyObjHandle_SetOriginMapId, NULL},
	{"substitute_inventory", PyObjHandle_GetSubstituteInventory, PyObjHandle_SetSubstituteInventory, NULL},
	{"factions", PyObjHandle_GetFactions, NULL, NULL },
	{"feats", PyObjHandle_GetFeats, NULL, NULL},
	{"spells_known", PyObjHandle_GetSpellsKnown, NULL, NULL },
	{"spells_memorized", PyObjHandle_GetSpellsMemorized, NULL, NULL },
	{"loots", PyObjHandle_GetLoots, PyObjHandle_SetLoots, NULL},
	{"proto", PyObjHandle_GetProto, NULL, NULL },
	{"__safe_for_unpickling__", PyObjHandle_SafeForUnpickling, NULL, NULL},
	{"id", PyObjHandle_GetID, NULL, NULL },
	{NULL, NULL, NULL, NULL}
};

#pragma endregion

#pragma region Number Methods

static int PyObjHandle_NonZero(PyObject* obj) {
	auto self = GetSelf(obj);
	if (self->handle) {
		return TRUE;
	} else {
		return FALSE;
	}
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
		self->id.subtype = ObjectIdKind::Null;
	} else {
		// Get GUID from handle
		self->id = objects.GetId(self->handle);

		// The obj handle is invalid
		if (!self->id) {
			auto msg = format("The object handle {} is invalid.", self->handle);
			PyErr_SetString(PyExc_ValueError, msg.c_str());
			// Reset the handle to the null handle
			self->handle = 0;
			self->id.subtype = ObjectIdKind::Null;
			return -1;
		}
	}

	return 0;
}

static PyObject* PyObjHandle_New(PyTypeObject*, PyObject*, PyObject*) {
	auto self = PyObject_New(PyObjHandle, &PyObjHandleType);
	self->handle = 0;
	self->id.subtype = ObjectIdKind::Null;
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

BOOL ConvertObjHndl(PyObject* obj, objHndl* pHandleOut) {
	if (obj == Py_None) {
		*pHandleOut = 0;
		return TRUE;
	}

	if (obj->ob_type != &PyObjHandleType) {
		PyErr_SetString(PyExc_TypeError, "Expected object handle.");
		return FALSE;
	}

	*pHandleOut = GetSelf(obj)->handle;
	return TRUE;
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
	return PyObjHndl_Create(objHndl::null);
}

objHndl PyObjHndl_AsObjHndl(PyObject* obj) {
	assert(PyObjHndl_Check(obj));
	return GetSelf(obj)->handle;
}

bool PyObjHndl_Check(PyObject* obj) {
	return obj->ob_type == &PyObjHandleType;
}
