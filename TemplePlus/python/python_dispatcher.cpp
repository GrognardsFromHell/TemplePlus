
#include "stdafx.h"
#include "python_bonus.h"
#include <structmember.h>
#include "../damage.h"
#include "../common.h"
#include "../feat.h"
#include "python_dice.h"
#include "../dice.h"
#include "../dispatcher.h"
#include "../common.h"
#include <infrastructure/elfhash.h>
#include "python_object.h"
#include "python_dispatcher.h"

#undef HAVE_ROUND
#include <pybind11/pybind11.h>
#include <pybind11/common.h>
#include <pybind11/cast.h>
#include <radialmenu.h>
#include <action_sequence.h>

namespace py = pybind11;
using namespace pybind11;
using namespace pybind11::detail;

template <> class type_caster<objHndl> {
public:
	bool load(handle src, bool) {
		value = PyObjHndl_AsObjHndl(src.ptr());
		success = true;
		return true;
	}

	static handle cast(const objHndl &src, return_value_policy /* policy */, handle /* parent */) {
		return PyObjHndl_Create(src);
	}

	PYBIND11_TYPE_CASTER(objHndl, _("objHndl"));
protected:
	bool success = false;
};



void AddPyHook(CondStructNew& condStr, uint32_t dispType, uint32_t dispKey, PyObject* pycallback, PyObject* pydataTuple) {
	Expects(condStr.numHooks < 99);
	condStr.subDispDefs[condStr.numHooks++] = { (enum_disp_type)dispType, (D20DispatcherKey)dispKey, PyModHookWrapper, (uint32_t)pycallback, (uint32_t)pydataTuple };
}

PYBIND11_PLUGIN(tp_dispatcher){
	py::module m("tpdp", "Temple+ Dispatcher module, used for creating modifier extensions.");

	py::class_<CondStructNew>(m, "ModifierSpec")
		.def(py::init())
		.def(py::init<std::string, int, bool>(), py::arg("name"), py::arg("numArgs"), py::arg("preventDup") = true)
		.def("add_hook", [](CondStructNew &condStr, uint32_t dispType, uint32_t dispKey, py::function &pycallback, py::tuple &pydataTuple) {
				Expects(condStr.numHooks < 99);
				condStr.subDispDefs[condStr.numHooks++] = { (enum_disp_type)dispType, (D20DispatcherKey)dispKey, PyModHookWrapper, (uint32_t)pycallback.ptr(), (uint32_t)pydataTuple.ptr() };
		}, "Add callback hook")
		.def("add_to_feat_dict", &CondStructNew::AddToFeatDictionary)
		// .def_readwrite("num_args", &CondStructNew::numArgs) // this is probably something we don't want to expose due to how ToEE saves/loads args
		.def_readwrite("name", &CondStructNew::condName);
		
		
	py::class_<DispIO>(m, "EventObj", "The base Event Object")
		.def_readwrite("evt_obj_type", &DispIO::dispIOType)
		.def("__repr__", [](const DispIO& dispIo)->std::string {
		return fmt::format("EventObj: Type {}", dispIo.dispIOType);
		})
		;


	py::class_<DispatcherCallbackArgs>(m, "EventArgs")
		.def(py::init())
		.def("__repr__", [](DispatcherCallbackArgs &args)->std::string{
			return fmt::format("EventArgs: Type = {} , Key = {}", args.dispType, args.dispKey);
		})
		.def("get_arg", &DispatcherCallbackArgs::GetCondArg)
		.def("set_arg", &DispatcherCallbackArgs::SetCondArg)
		.def_readwrite("evt_obj", &DispatcherCallbackArgs::dispIO)
		;

	#pragma region useful data types

	py::class_<BonusList>(m, "BonusList")
			.def(py::init())
			.def("add", &BonusList::AddBonus, "Adds a bonus entry. Args are: value, type, and bonus.mes line number")
			.def("add_from_feat", &BonusList::AddBonusFromFeat)
			.def("set_overall_cap", &BonusList::SetOverallCap)
			.def("get_sum", &BonusList::GetEffectiveBonusSum)
			.def("get_total", &BonusList::GetEffectiveBonusSum)
			.def("add_zeroed", &BonusList::ZeroBonusSetMeslineNum, "Adds a zero-value bonus (usually to represent nullified bonuses)")
			.def("add_cap", &BonusList::AddCap, "Adds cap for a particular bonus type");

	py::class_<AttackPacket>(m, "AttackPacket")
		.def(py::init())
		.def("get_weapon_used", &AttackPacket::GetWeaponUsed)
		.def("is_offhand_attack", &AttackPacket::IsOffhandAttack)
		.def_readwrite("attacker", &AttackPacket::attacker)
		.def_readwrite("target", &AttackPacket::victim);

	py::class_<DamagePacket>(m, "DamagePacket")
		.def(py::init())
		.def("add_dice", &DamagePacket::AddDamageDice)
		.def_readwrite("flags", &DamagePacket::flags, "1 - maximized, 2 - empowered")
		.def_readwrite("bonus_list", &DamagePacket::bonuses)
		.def_readwrite("critical_multiplier", &DamagePacket::critHitMultiplier, "1 by default, gets increased by various things")
		.def_readwrite("attack_power", &DamagePacket::attackPowerType, "See D20DAP_");



	py::class_<D20Actn>(m, "D20Action")
		.def(py::init())
		.def(py::init<D20ActionType>())
		.def_readwrite("performer", &D20Actn::d20APerformer)
		.def_readwrite("target", &D20Actn::d20ATarget)
		.def_readwrite("spell_id", &D20Actn::spellId)
		.def_readwrite("data1", &D20Actn::data1 , "Generic piece of data used for various things depending on context.")
		.def_readwrite("flags", &D20Actn::d20Caf, "D20CAF_ flags")
		.def_readwrite("path", &D20Actn::path)
		.def_readwrite("action_type", &D20Actn::d20ActType, "See D20A_ constants")
		.def_readwrite("loc", &D20Actn::destLoc, "Location")
		.def_readwrite("anim_id", &D20Actn::animID)
		
		.def_readwrite("attack_mode_code", &TurnBasedStatus::attackModeCode, "0 - normal main hand, 99 - dual wielding, 999 - natural attack")
		;

	py::class_<TurnBasedStatus>(m, "TurnBasedStatus")
		.def(py::init())
		.def_readwrite("hourglass_state", &TurnBasedStatus::hourglassState)
		.def_readwrite("num_bonus_attacks", &TurnBasedStatus::numBonusAttacks)
		.def_readwrite("surplus_move_dist", &TurnBasedStatus::surplusMoveDistance)
		.def_readwrite("flags", &TurnBasedStatus::tbsFlags)
		.def_readwrite("attack_mode_code", &TurnBasedStatus::attackModeCode, "0 - normal main hand, 99 - dual wielding, 999 - natural attack")
		;



	py::enum_<RadialMenuStandardNode>(m, "RadialMenuStandardNode")
		.value("Root", Root)
		.value("Spells", Spells)
		.value("Skills", Skills)
		.value("Feats", Feats)
		.value("Class", Class)
		.value("Combat", Combat)
		.value("Items", Items)
		.value("Alchemy", Alchemy)
		.value("Movement", Movement)
		.value("Offense", Offense)
		.value("Tacitical", Tactical)
		.value("Options", Options)
		.value("Potions", Potions)
		.value("Wands", Wands)
		.value("Scrolls", Scrolls)
		.export_values()
		;


	py::class_<RadialMenuEntry>(m, "RadialMenuEntry")
		.def(py::init())
		.def("add_as_child", &RadialMenuEntry::AddAsChild, "Adds this node as a child to a specified node ID, and returns the newly created node ID (so you may give it other children, etc.)")
		.def("add_child_to_standard", &RadialMenuEntry::AddChildToStandard, "Adds this node as a child to a Standard Node (one of several hardcoded root nodes such as class, inventory etc.), and returns the newly created node ID (so you may give it other children, etc.)");

	

	py::class_<RadialMenuEntryAction>(m, "RadialMenuEntryAction", py::base<RadialMenuEntry>())
		.def(py::init<int, int, int, const char[]>(), py::arg("combesMesLine"), py::arg("action_type"), py::arg("data1"), py::arg("helpTopic"))
	;

	py::class_<RadialMenuEntryParent>(m, "RadialMenuEntryParent")
		.def(py::init<int>(), py::arg("combesMesLine"))
		.def("add_as_child", &RadialMenuEntryParent::AddAsChild, "Adds this node as a child to a specified node ID, and returns the newly created node ID (so you may give it other children, etc.)")
		.def("add_child_to_standard", &RadialMenuEntryParent::AddChildToStandard, "Adds this node as a child to a Standard Node (one of several hardcoded root nodes such as class, inventory etc.), and returns the newly created node ID (so you may give it other children, etc.)")
		;

	#pragma endregion 

	

	py::class_<DispIoCondStruct>(m, "EventObjModifier", "Used for checking modifiers before applying them", py::base<DispIO>())
		.def(py::init())
		.def_readwrite("retun_val", &DispIoCondStruct::outputFlag)
		.def_readwrite("arg1", &DispIoCondStruct::arg1, "First modifier argument")
		.def_readwrite("arg2", &DispIoCondStruct::arg2, "Second modifier argument")
		.def_readwrite("modifier_spec", &DispIoCondStruct::condStruct, "Modifier Spec (DO NOT ADD HOOKS FROM HERE! Due to different format for vanilla ToEE and Temple+ modifier specs.)");


	py::class_<DispIoBonusList>(m, "EventObjBonusList", "Used for fetching ability score levels and cur/max HP", py::base<DispIO>())
		.def(py::init())
		.def_readwrite("flags", &DispIoBonusList::flags)
		.def_readwrite("bonus_list", &DispIoBonusList::bonlist);

	py::class_<DispIoSavingThrow>(m, "EventObjSavingThrow", "Used for fetching saving throw bonuses", py::base<DispIO>())
		.def_readwrite("bonus_list", &DispIoSavingThrow::bonlist)
		.def_readwrite("return_val", &DispIoSavingThrow::returVal)
		.def_readwrite("obj", &DispIoSavingThrow::obj);

	py::class_<DispIoDamage>(m, "EventObjDamage", "Used for damage dice and such", py::base<DispIO>())
		.def_readwrite("attack_packet", &DispIoDamage::attackPacket)
		.def_readwrite("damage_packet", &DispIoDamage::damage)
		;


	py::class_<DispIoAttackBonus>(m, "EventObjAttack", "Used for fetching attack or AC bonuses", py::base<DispIO>())
		.def(py::init())
		.def_readwrite("bonus_list", &DispIoAttackBonus::bonlist)
		.def_readwrite("attack_packet", &DispIoAttackBonus::attackPacket);

	py::class_<DispIoD20Signal>(m, "EventObjD20Signal", py::base<DispIO>())
		.def(py::init())
		.def_readwrite("return_val", &DispIoD20Signal::return_val)
		.def_readwrite("data1", &DispIoD20Signal::data1)
		.def_readwrite("data2", &DispIoD20Signal::data2);

	py::class_<DispIoD20Query>(m, "EventObjD20Query", py::base<DispIO>())
		.def(py::init())
		.def_readwrite("return_val", &DispIoD20Query::return_val)
		.def_readwrite("data1", &DispIoD20Query::data1)
		.def_readwrite("data2", &DispIoD20Query::data2);

	py::class_<DispIOTurnBasedStatus>(m, "EventObjTurnBasedStatus", py::base<DispIO>())
		.def(py::init())
		.def_readwrite("tb_status", &DispIOTurnBasedStatus::tbStatus);


	py::class_<DispIoTooltip>(m, "EventObjTooltip", "Tooltip event for mouse-overed objects.", py::base<DispIO>())
		.def("append", &DispIoTooltip::Append, "Appends a string")
		.def_readwrite("num_strings", &DispIoTooltip::numStrings);

	py::class_<DispIoObjBonus>(m, "EventObjObjectBonus", "Used for Item Bonuses, initiative modifiers and others.", py::base<DispIO>())
		.def_readwrite("bonus_list", &DispIoObjBonus::bonOut)
		.def_readwrite("return_val", &DispIoObjBonus::returnVal);

	py::class_<DispIoDispelCheck>(m, "EventObjDispelCheck", "Dispel Check Event", py::base<DispIO>())
		.def_readwrite("return_val", &DispIoDispelCheck::returnVal)
		.def_readwrite("flags", &DispIoDispelCheck::flags)
		.def_readwrite("spell_id", &DispIoDispelCheck::spellId);

	py::class_<DispIoD20ActionTurnBased>(m, "EventObjD20Action", "Used for D20 Action Checks/Performance events and obtaining number of attacks (base/bonus/natural)", py::base<DispIO>())
		.def_readwrite("return_val", &DispIoD20ActionTurnBased::returnVal)
		.def_readwrite("d20a", &DispIoD20ActionTurnBased::d20a)
		.def_readwrite("turnbased_status", &DispIoD20ActionTurnBased::tbStatus);

	py::class_<DispIoMoveSpeed>(m, "EventObjMoveSpeed", "Used for getting move speed, and also for model size scaling with Temple+.", py::base<DispIO>())
		.def_readwrite("factor", &DispIoMoveSpeed::factor)
		.def_readwrite("bonus_list", &DispIoMoveSpeed::bonlist)
		;

	py::class_<DispIOBonusListAndSpellEntry>(m, "EventObjSpellEntry", "Used for Spell DC and Spell Resistance Mod", py::base<DispIO>())
		.def_readwrite("bonus_list", &DispIOBonusListAndSpellEntry::bonList)
		.def_readwrite("spell_entry", &DispIOBonusListAndSpellEntry::spellEntry)
		;

	py::class_<DispIoReflexThrow>(m, "EventObjReflexSaveThrow", "Used for Reflex Save throws that reduce damage", py::base<DispIO>())
		.def_readwrite("attack_type", &DispIoReflexThrow::attackType)
		.def_readwrite("effective_reduction", &DispIoReflexThrow::effectiveReduction)
		.def_readwrite("reduction", &DispIoReflexThrow::reduction, "0,1,2 for None,Half,Quarter respectively")
		.def_readwrite("damage_mesline", &DispIoReflexThrow::damageMesLine, "Line no. from damage.mes to be used")
		.def_readwrite("attack_power", &DispIoReflexThrow::attackPower, "D20DAP_")
		.def_readwrite("flags", &DispIoReflexThrow::flags, "D20STD_ flags")
		;

	py::class_<DispIoObjEvent>(m, "EventObjObjectEvent", "Used for Object Events (triggered by entering/leaveing AoE)", py::base<DispIO>())
		.def_readwrite("target", &DispIoObjEvent::tgt, "The critter affected by the AoE")
		.def_readwrite("aoe_obj", &DispIoObjEvent::aoeObj, "The origin of the AoE effect")
		.def_readwrite("evt_id", &DispIoObjEvent::evtId)
		;


	py::class_<DispIoAbilityLoss>(m, "EventObjGetAbilityLoss", "Used for Ability Loss status", py::base<DispIO>())
		.def_readwrite("flags", &DispIoAbilityLoss::flags)
		.def_readwrite("spell_id", &DispIoAbilityLoss::spellId)
		.def_readwrite("stat_damaged", &DispIoAbilityLoss::statDamaged)
		.def_readwrite("result", &DispIoAbilityLoss::result)
		;

	py::class_<DispIoAttackDice>(m, "EventObjGetAttackDice", "Used for getting the critter's attack dice", py::base<DispIO>())
		.def_readwrite("flags", &DispIoAttackDice::flags, "D20CAF_ ")
		.def_readwrite("damage_type", &DispIoAttackDice::attackDamageType, "D20DT_")
		.def_readwrite("bonus_list", &DispIoAttackDice::bonlist)
		.def_readwrite("dice_packed", &DispIoAttackDice::dicePacked)
		.def_readwrite("weapon", &DispIoAttackDice::weapon)
		.def_readwrite("wielder", &DispIoAttackDice::wielder)
		;

	py::class_<DispIoTypeImmunityTrigger>(m, "EventObjImmunityTrigger", "Used for triggering the immunity handling query", py::base<DispIO>())
		.def_readwrite("should_perform_immunity_check", &DispIoTypeImmunityTrigger::interrupt)
		;

	py::class_<DispIoImmunity>(m, "EventObjImmunityQuery", "Used for performing the immunity handling", py::base<DispIO>())
		.def_readwrite("spell_entry", &DispIoImmunity::spellEntry)
		.def_readwrite("spell_packet", &DispIoImmunity::spellPkt)
		;

	py::class_<DispIoEffectTooltip>(m, "EventObjEffectTooltip", "Used for tooltips when hovering over the status effect indicators in the party portrait row", py::base<DispIO>())
		.def("append", &DispIoEffectTooltip::Append)
		;

	//py::class_<DispatcherCallbackArgs>(m, "EventArgsD20Signal")
	//	.def(py::init())
	//	.def("__repr__", [](DispatcherCallbackArgs &args)->std::string {
	//	return fmt::format("EventArgsD20Signal: Type = {} , Key = {}", args.dispType, args.dispKey);
	//})
	//	.def("get_arg", &DispatcherCallbackArgs::GetCondArg)
	//	.def("set_arg", &DispatcherCallbackArgs::SetCondArg)
	//	//	.def_readwrite("evt_obj", &DispatcherCallbackArgs::dispIO)
	//	;
	
	return m.ptr();
}



struct PyModifier {
	PyObject_HEAD;
	CondNode *cond;
};

struct PyModifierSpec{
	PyObject_HEAD;
	CondStructNew condSpec;
	int condId;
	char condName[512];
};

struct PyDispatchEventObject {
	PyObject_HEAD;
	DispIO *evtObj;
};

struct PyDispatchEventObjectD20Signal : PyDispatchEventObject {

};

struct PyEventArgs {
	PyObject_HEAD;
	DispatcherCallbackArgs args;
};



PyObject* PyEventArgs_Create(DispatcherCallbackArgs args) {
	auto result = PyObject_New(PyEventArgs, &PyEventArgsType);
	result->args= args;
	return (PyObject*)result;
}

PyObject* PyDispatchEventObject_Create(DispIO* dispIo) {
	auto result = PyObject_New(PyDispatchEventObject, &PyDispatchEventObjectType);
	result->evtObj = dispIo;
	return (PyObject*)result;
}



/******************************************************************************
* PyModifierSpec
******************************************************************************/
#pragma region PyModifierSpec


PyObject *PyModifierSpec_Repr(PyObject *obj) {
	auto self = (PyModifierSpec*)obj;
	string text;

	text = format("Modifier Spec: {}", self->condSpec.condName);

	return PyString_FromString(text.c_str());
}



int PyModifierSpec_Init(PyObject *obj, PyObject *args, PyObject *kwds) {
	auto self = (PyModifierSpec*)obj;
	CondStructNew condSpecNew;
	self->condSpec = condSpecNew;
	self->condId = 0;
	memset(self->condName, 0, sizeof self->condName);
	return 0;
}

PyObject *PyModifierSpec_New(PyTypeObject *subtype, PyObject *args, PyObject *kwds) {
	auto self = PyObject_New(PyModifierSpec, &PyModifierSpecType);
	return (PyObject*)self;
}


int PyModHookWrapper(DispatcherCallbackArgs args){
	auto callback = (PyObject*)args.GetData1();

	
	//auto dispPyArgs = PyTuple_New(3);
	
	py::object pbArgs = py::cast(args);
	py::object pbEvtObj;

	auto attachee = PyObjHndl_Create(args.objHndCaller);

	switch (args.dispType) {
	
	case dispTypeConditionAddPre:
		pbEvtObj = py::cast(static_cast<DispIoCondStruct*>(args.dispIO));
		break;

	case dispTypeAbilityScoreLevel:
	case dispTypeCurrentHP:
	case dispTypeMaxHP:
	case dispTypeStatBaseGet:
		pbEvtObj = py::cast(static_cast<DispIoBonusList*>(args.dispIO));
		break;


	case dispTypeSaveThrowLevel:
	case dispTypeSaveThrowSpellResistanceBonus:
	case dispTypeCountersongSaveThrow:
		pbEvtObj = py::cast(static_cast<DispIoSavingThrow*>(args.dispIO));
		break;


	case dispTypeDealingDamage:
	case dispTypeTakingDamage:
	case dispTypeDealingDamage2:
	case dispTypeTakingDamage2:
		pbEvtObj = py::cast(static_cast<DispIoDamage*>(args.dispIO));
		break;

	

	case dispTypeGetAC:
	case dispTypeGetACBonus2:
	case dispTypeToHitBonusBase:
	case dispTypeToHitBonus2:
	case dispTypeToHitBonusFromDefenderCondition:
	case dispTypeGetCriticalHitRange:
	case dispTypeGetCriticalHitExtraDice:
	case dispTypeGetDefenderConcealmentMissChance:
	case dispTypeDeflectArrows:
	case dispTypeProjectileCreated:
	case dispTypeProjectileDestroyed:
	case dispTypeBucklerAcPenalty:
		pbEvtObj = py::cast(static_cast<DispIoAttackBonus*>(args.dispIO));
		break;



	case dispTypeD20AdvanceTime:
	case dispTypeD20Signal:
	case dispTypeBeginRound:
	case dispTypeDestructionDomain:
		pbEvtObj = py::cast(static_cast<DispIoD20Signal*>(args.dispIO));
		break;

	case dispTypeD20Query:
	case dispTypeBaseCasterLevelMod:
	case dispTypeWeaponGlowType:
	case dispTypeGetSizeCategory:
		pbEvtObj = py::cast(static_cast<DispIoD20Query*>(args.dispIO));
		break;

	case dispTypeTurnBasedStatusInit:
		pbEvtObj = py::cast(static_cast<DispIOTurnBasedStatus*>(args.dispIO));
		break;


	case dispTypeTooltip:
		pbEvtObj = py::cast(static_cast<DispIoTooltip*>(args.dispIO));
		break;

	case dispTypeInitiativeMod:
	case dispTypeSkillLevel:
	case dispTypeAbilityCheckModifier:
	case dispTypeGetAttackerConcealmentMissChance:
	case dispTypeGetLevel:
	case dispTypeMaxDexAcBonus:
		pbEvtObj = py::cast(static_cast<DispIoObjBonus*>(args.dispIO));
		break;


	case dispTypeDispelCheck:
		pbEvtObj = py::cast(static_cast<DispIoDispelCheck*>(args.dispIO));
		break;

	case dispTypeD20ActionCheck:
	case dispTypeD20ActionPerform:
	case dispTypeD20ActionOnActionFrame:
	case dispTypeGetNumAttacksBase:
	case dispTypeGetBonusAttacks:
	case dispTypeGetCritterNaturalAttacksNum:
		pbEvtObj = py::cast(static_cast<DispIoD20ActionTurnBased*>(args.dispIO));
		break;


	case dispTypeGetMoveSpeedBase:
	case dispTypeGetMoveSpeed:
	case dispTypeGetModelScale:
		pbEvtObj = py::cast(static_cast<DispIoMoveSpeed*>(args.dispIO));
		break;

	case dispTypeSpellResistanceMod:
	case dispTypeSpellDcBase:
	case dispTypeSpellDcMod:
		pbEvtObj = py::cast(static_cast<DispIOBonusListAndSpellEntry*>(args.dispIO));
		break;

	case dispTypeReflexThrow:
		pbEvtObj = py::cast(static_cast<DispIoReflexThrow*>(args.dispIO));
		break;

	case dispTypeObjectEvent:
		pbEvtObj = py::cast(static_cast<DispIoObjEvent*>(args.dispIO));
		break;

	case dispTypeGetAbilityLoss:
		pbEvtObj = py::cast(static_cast<DispIoAbilityLoss*>(args.dispIO));
		break;

	case dispTypeGetAttackDice:
		pbEvtObj = py::cast(static_cast<DispIoAttackDice*>(args.dispIO));
		break;

	case dispTypeImmunityTrigger:
	case dispType63:
		pbEvtObj = py::cast(static_cast<DispIoTypeImmunityTrigger*>(args.dispIO));
		break;

	case dispTypeSpellImmunityCheck:
		pbEvtObj = py::cast(static_cast<DispIoImmunity*>(args.dispIO));
		break;

	case dispTypeEffectTooltip:
		pbEvtObj = py::cast(static_cast<DispIoEffectTooltip*>(args.dispIO));
		break;

	case dispTypeConditionAdd: // these are actually null
	case dispTypeConditionRemove:
	case dispTypeConditionRemove2:
	case dispTypeConditionAddFromD20StatusInit:
	case dispTypeInitiative:
	case dispTypeNewDay:
	case dispTypeRadialMenuEntry:
	case dispTypeItemForceRemove:
	default:
		pbEvtObj = py::cast(args.dispIO);
	}
	
	auto dispPyArgs = Py_BuildValue("OOO", attachee, pbArgs.ptr(), pbEvtObj.ptr());

	if ( !PyObject_CallObject(callback, dispPyArgs))	{
		int dummy = 1;
	}

	Py_DECREF(attachee);
	Py_DECREF(dispPyArgs);

	return 0;
}




PyObject *PyModifierSpec_AddHook(PyObject *obj, PyObject *args){
	auto self = (PyModifierSpec*)obj;

	enum_disp_type dispType = dispType0;
	D20DispatcherKey dispKey = DK_NONE;
	PyObject *callback = nullptr;

	if (!PyArg_ParseTuple(args, "iiO:PyModifierSpec.__add_hook__", &dispType, &dispKey, &callback)) {
		return PyInt_FromLong(0);
	}

	if (!callback || !PyCallable_Check(callback)) {
		logger->error("PyModifierSpec {} attached with a bad hook.", self->condSpec.condName);
		return PyInt_FromLong(0);
	}


	self->condSpec.AddHook(dispType, dispKey, PyModHookWrapper, (uint32_t)callback, 0);
	return PyInt_FromLong(1);
};


PyObject *PyModifierSpec_RegisterMod(PyObject *obj, PyObject *args) {
	auto self = (PyModifierSpec*)obj;

	const char *name;
	int numArgs = 0, preventDup = 1;

	if (!PyArg_ParseTuple(args, "sii:PyModifierSpec.__register_mod__", &name, &numArgs, &preventDup)) {
		return PyInt_FromLong(0);
	}

	if (!name)
		return PyInt_FromLong(0);

	auto strSize = strlen(name);
	if (strSize > 512)
		strSize = 511;

	self->condSpec.numArgs = numArgs;
	self->condSpec.condName = self->condName;
	memcpy(self->condName, name, strSize);
	self->condId = ElfHash::Hash(name);

	self->condSpec.Register();
	return PyInt_FromLong(1);
};


PyObject *PyModifierSpec_AddHookPreventDup(PyObject *obj, PyObject *args) {
	// todo
	return PyInt_FromLong(1);
};

static PyMemberDef PyModifierSpec_Members[] = {
	{ "num_args", T_INT, offsetof(PyModifierSpec, condSpec.numArgs), 0, NULL },
	{ "num_hooks", T_INT, offsetof(PyModifierSpec, condSpec.numHooks), 0, NULL },
	{ NULL, NULL, NULL, NULL, NULL }
};

static PyMethodDef PyModifierSpec_Methods[] = {
	{ "__add_hook__", PyModifierSpec_AddHook, METH_VARARGS, NULL },
	{ "__add_hook_prevent_dup__", PyModifierSpec_AddHookPreventDup, METH_VARARGS, NULL },
	{ "__register_mod__", PyModifierSpec_RegisterMod, METH_VARARGS, NULL },
	{ NULL, NULL, NULL, NULL }
};

PyTypeObject PyModifierSpecType = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"PyModifierSpec",                  /*tp_name*/
	sizeof(CondStructNew)+sizeof(int),     	   /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)PyObject_Del,  /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	PyModifierSpec_Repr,               /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	PyObject_GenericGetAttr,   /*tp_getattro*/
	PyObject_GenericSetAttr,   /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,			               /* tp_doc */
	0,						   /* tp_traverse */
	0,						   /* tp_clear */
	0,						   /* tp_richcompare */
	0,						   /* tp_weaklistoffset */
	0,						   /* tp_iter */
	0,						   /* tp_iternext */
	PyModifierSpec_Methods,            /* tp_methods */
	PyModifierSpec_Members,            /* tp_members */
	0,                   	   /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	PyModifierSpec_Init,               /* tp_init */
	0,                         /* tp_alloc */
	PyModifierSpec_New,                /* tp_new */
};




PyObject* PyModifierSpec_FromCondStructNew(const CondStructNew& condSpec) {
	auto self = PyObject_New(PyModifierSpec, &PyModifierSpecType);
	self->condSpec = condSpec;
	self->condId = ElfHash::Hash(condSpec.condName);
	return (PyObject*)self;
}

bool ConvertDamagePacket(PyObject* obj, CondStructNew **pCondStructNew) {
	if (obj->ob_type != &PyModifierSpecType) {
		PyErr_SetString(PyExc_TypeError, "Expected a PyModifierSpec object.");
		return false;
	}

	auto pyMod = (PyModifierSpec*)obj;
	*pCondStructNew = &pyMod->condSpec;
	return true;
}

#pragma endregion




/******************************************************************************
* PyEventArgs
******************************************************************************/

#pragma region PyEventArgs

PyObject *PyEventArgs_Repr(PyObject *obj) {
	auto self = (PyEventArgs*)obj;
	string text;
	text = format("Event Args [{}][{}]", self->args.dispType, self->args.dispKey);
	return PyString_FromString(text.c_str());
}

int PyEventArgs_Init(PyObject *obj, PyObject *args, PyObject *kwds) {
	auto self = (PyEventArgs*)obj;
	memset(&self->args, 0, sizeof self->args);
	return 0;
}

PyObject *PyEventArgs_New(PyTypeObject *subtype, PyObject *args, PyObject *kwds) {
	auto self = PyObject_New(PyEventArgs, &PyEventArgsType);
	memset(&self->args, 0, sizeof self->args);
	return (PyObject*)self;
}



PyObject* PyEventArgs_GetEventObject(PyObject *obj, void *) {
	auto self = (PyEventArgs*)obj;
	switch (self->args.dispKey) {
		
	}
	return PyDispatchEventObject_Create(self->args.dispIO);
}

PyObject* PyEventArgs_GetAttachee(PyObject *obj, void *) {
	auto self = (PyEventArgs*)obj;
	switch (self->args.dispKey) {

	}
	return PyObjHndl_Create(self->args.objHndCaller);
}

static PyGetSetDef PyEventArgsGetSet[] = {
	{ "event_obj", PyEventArgs_GetEventObject, NULL, NULL },
	{ "attachee", PyEventArgs_GetAttachee, NULL, NULL },
	{ NULL, NULL, NULL, NULL }
};

static PyMethodDef PyEventArgs_Methods[] = {
	//{ "__add_hook__", PyModifierSpec_AddHook, METH_VARARGS, NULL },
	//{ "__add_hook_prevent_dup__", PyModifierSpec_AddHookPreventDup, METH_VARARGS, NULL },
	//{ "__register_mod__", PyModifierSpec_RegisterMod, METH_VARARGS, NULL },
	{ NULL, NULL, NULL, NULL }
};

PyTypeObject PyEventArgsType = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"PyEventArgs",                  /*tp_name*/
	sizeof(CondStructNew) + sizeof(int),     	   /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)PyObject_Del,  /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	PyEventArgs_Repr,               /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	PyObject_GenericGetAttr,   /*tp_getattro*/
	PyObject_GenericSetAttr,   /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,			               /* tp_doc */
	0,						   /* tp_traverse */
	0,						   /* tp_clear */
	0,						   /* tp_richcompare */
	0,						   /* tp_weaklistoffset */
	0,						   /* tp_iter */
	0,						   /* tp_iternext */
	PyEventArgs_Methods,            /* tp_methods */
	0,						   /* tp_members */
	PyEventArgsGetSet,     	   /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	PyEventArgs_Init,               /* tp_init */
	0,                         /* tp_alloc */
	PyEventArgs_New,                /* tp_new */
};


#pragma endregion

/******************************************************************************
* PyDispatchEventObject
******************************************************************************/

#pragma region PyDispatchEventObject

PyObject *PyDispatchEventObject_Repr(PyObject *obj) {
	auto self = (PyDispatchEventObject*)obj;
	string text;
	text = format("EventObject [{}]", self->evtObj->dispIOType);
	return PyString_FromString(text.c_str());
}

int PyDispatchEventObject_Init(PyObject *obj, PyObject *args, PyObject *kwds) {
	// not sure how relevant this is
	return 0;
}

PyObject *PyDispatchEventObject_New(PyTypeObject *subtype, PyObject *args, PyObject *kwds) {
	auto self = PyObject_New(PyDispatchEventObject, &PyDispatchEventObjectType);
	// not sure how relevant this is
	return (PyObject*)self;
}


//
//static PyGetSetDef PyDispatchEventObject_GetSet[] = {
//	{ "event_obj", PyEventArgs_GetEventObject, NULL, NULL },
//	{ NULL, NULL, NULL, NULL }
//};

//static PyMethodDef PyEventArgs_Methods[] = {
//	{ NULL, NULL, NULL, NULL }
//};

PyTypeObject PyDispatchEventObjectType = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"PyDispatchEventObject",                  /*tp_name*/
	sizeof(PyDispatchEventObject),     	   /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)PyObject_Del,  /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	PyDispatchEventObject_Repr,               /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	PyObject_GenericGetAttr,   /*tp_getattro*/
	PyObject_GenericSetAttr,   /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,			               /* tp_doc */
	0,						   /* tp_traverse */
	0,						   /* tp_clear */
	0,						   /* tp_richcompare */
	0,						   /* tp_weaklistoffset */
	0,						   /* tp_iter */
	0,						   /* tp_iternext */
	0,            /* tp_methods */
	0,						   /* tp_members */
	0,     					   /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	PyDispatchEventObject_Init,               /* tp_init */
	0,                         /* tp_alloc */
	PyDispatchEventObject_New,                /* tp_new */
};


#pragma endregion
