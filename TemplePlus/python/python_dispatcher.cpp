
#include "stdafx.h"
#include "python_bonus.h"
#include <structmember.h>
#include "../damage.h"
#include "../common.h"
#include "../feat.h"
#include "python_dice.h"
#include "../dice.h"
#include "../dispatcher.h"
#include "../pathfinding.h"
#include "../common.h"
#include <infrastructure/elfhash.h>
#include "python_object.h"
#include "python_dice.h"
#include "python_dispatcher.h"

#include <pybind11/embed.h>
#include <pybind11/cast.h>
#include <pybind11/stl.h>
#include <radialmenu.h>
#include <action_sequence.h>
#include <condition.h>
#include "python_spell.h"
#include "turn_based.h"
#include "combat.h"
#include "location.h"
#include "gamesystems/objects/objsystem.h"
#include "python_integration_spells.h"
#include "temple_functions.h"
#include "rng.h"
#include "float_line.h"

namespace py = pybind11;

template <> class py::detail::type_caster<objHndl> {
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
template <> class py::detail::type_caster<Dice> {
public:
	bool load(handle src, bool) {
		Dice dice;
		ConvertDice(src.ptr(), &dice);
		value = dice;
		success = true;
		return true;
	}

	static handle cast(const Dice &src, return_value_policy /* policy */, handle /* parent */) {
		return PyDice_FromDice(src);
	}

	PYBIND11_TYPE_CASTER(Dice, _("PyDice"));
protected:
	bool success = false;
};

template <> class py::detail::type_caster<SpellStoreData> {
public:
	bool load(handle src, bool) {
		SpellStoreData spData;
		ConvertSpellStore(src.ptr(), &spData);
		value = spData;
		success = true;
		return true;
	}

	static handle cast(const SpellStoreData& src, return_value_policy
	                   /* policy */, handle
	                   /* parent */) {
		return PySpellStore_Create(src);
	}

	PYBIND11_TYPE_CASTER(SpellStoreData, _("PySpellStore"));
protected:
	bool success = false;
};



void AddPyHook(CondStructNew& condStr, uint32_t dispType, uint32_t dispKey, PyObject* pycallback, PyObject* pydataTuple) {
	Expects(condStr.numHooks < 99);
	condStr.subDispDefs[condStr.numHooks++] = { (enum_disp_type)dispType, (D20DispatcherKey)dispKey, PyModHookWrapper, (uint32_t)pycallback, (uint32_t)pydataTuple };
}

PYBIND11_EMBEDDED_MODULE(tpdp, m) {

	m.doc() = "Temple+ Dispatcher module, used for creating modifier extensions.";

	m.def("hash", [](std::string &text){
		return ElfHash::Hash(text);
	});

	m.def("GetModifierFileList", [](){
		auto result = std::vector<std::string>();
		TioFileList flist;
		tio_filelist_create(&flist, "scr\\tpModifiers\\*.py");

		for (auto i=0; i < flist.count; i++){
			auto &f = flist.files[i];
			if (!_strcmpi(f.name, "__init__.py"))
				continue;
			for (auto ch = f.name; *ch; ch++)
			{
				if (*ch == '.')
				{
					*ch = 0;
					break;
				}
					
			}
			result.push_back(f.name);
		}

		tio_filelist_destroy(&flist);
		return result;
	});

	#pragma region Basic Dispatcher stuff

	py::class_<CondStructNew>(m, "ModifierSpec")
		.def(py::init())
		.def(py::init<std::string, int, bool>(), py::arg("name"), py::arg("numArgs"), py::arg("preventDup") = true)
		.def("add_hook", [](CondStructNew &condStr, uint32_t dispType, uint32_t dispKey, py::function &pycallback, py::tuple &pydataTuple) {
				Expects(condStr.numHooks < 99);
				pydataTuple.inc_ref();
				condStr.subDispDefs[condStr.numHooks++] = { (enum_disp_type)dispType, (D20DispatcherKey)dispKey, PyModHookWrapper, (uint32_t)pycallback.ptr(), (uint32_t)pydataTuple.ptr() };
		}, "Add callback hook")
		.def("add_hook", [](CondStructNew &condStr, uint32_t dispType, std::string dispKey, py::function &pycallback, py::tuple &pydataTuple) {
			Expects(condStr.numHooks < 99);
			pydataTuple.inc_ref();
			condStr.subDispDefs[condStr.numHooks++] = { (enum_disp_type)dispType, (D20DispatcherKey)ElfHash::Hash(dispKey), PyModHookWrapper, (uint32_t)pycallback.ptr(), (uint32_t)pydataTuple.ptr() };
		}, "Add callback hook")
		.def("add_to_feat_dict", [](CondStructNew &condStr, int feat_enum, int feat_max, int feat_offset) {
			condStr.AddToFeatDictionary((feat_enums)feat_enum, (feat_enums)feat_max, feat_offset);
		}, py::arg("feat_enum"), py::arg("feat_list_max") = -1, py::arg("feat_list_offset") = 0)
		.def("add_to_feat_dict", [](CondStructNew &condStr, std::string &feat_name, int feat_max, int feat_offset) {
			condStr.AddToFeatDictionary((feat_enums)ElfHash::Hash(feat_name), (feat_enums)feat_max, feat_offset);
		}, py::arg("feat_enum"), py::arg("feat_list_max") = -1, py::arg("feat_list_offset") = 0)
		// .def_readwrite("num_args", &CondStructNew::numArgs) // this is probably something we don't want to expose due to how ToEE saves/loads args
		.def_readwrite("name", &CondStructNew::condName)
		.def("extend_existing", [](CondStructNew &condStr, std::string condName){
				auto cond = (CondStructNew*)conds.GetByName(condName);
				if (cond) {
					for (auto i = 0; i < 100 && cond->subDispDefs[i].dispType != 0; i++) {
						condStr.subDispDefs[condStr.numHooks++] = cond->subDispDefs[i];
					}
					condStr.numArgs = cond->numArgs;
					condStr.condName = cond->condName;
					condStr.Register();
				} else {
					logger->info("Extend Existing Error: Condition {} does not exist!", condName);
				}
			})
		.def("add_item_force_remove_callback", [](CondStructNew &condStr){
			condStr.AddHook(dispTypeItemForceRemove, DK_NONE, temple::GetRef<int(__cdecl)(DispatcherCallbackArgs)>(0x10104410));
		})
		.def("add_spell_countdown_standard", [](CondStructNew &condStr){
			condStr.AddHook(dispTypeBeginRound, DK_NONE, temple::GetRef<int(__cdecl)(DispatcherCallbackArgs)>(0x100DC100));
		})
		.def("add_aoe_spell_ender", [](CondStructNew &condStr) {
			condStr.AddAoESpellRemover();
		})
		.def("add_spell_dismiss_hook", [](CondStructNew &condStr){
			condStr.AddHook(dispTypeConditionAdd, DK_NONE, temple::GetRef<int(__cdecl)(DispatcherCallbackArgs)>(0x100CBD60));
		})
		;

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
		.def("get_param", [](DispatcherCallbackArgs &args, int paramIdx){
			PyObject*tuplePtr = (PyObject*)args.GetData2();
			
			//return tuplePtr;
			if (!tuplePtr || paramIdx >= PyTuple_Size(tuplePtr)){
				return 0;
			}
				
			auto tupItem = PyTuple_GetItem(tuplePtr, paramIdx);
			auto result = (int)PyLong_AsLong(tupItem);
			return result;
		})
		.def_readwrite("evt_obj", &DispatcherCallbackArgs::dispIO)
		.def("condition_remove", [](DispatcherCallbackArgs& args)
		{
			conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		})
		.def("remove_spell_mod", &DispatcherCallbackArgs::RemoveSpellMod)
		.def("remove_spell", &DispatcherCallbackArgs::RemoveSpell)
		;

	#pragma endregion 

	#pragma region useful data types

	#pragma region Location
		py::class_<locXY>(m, "LocXY")
		.def_readwrite("x", &locXY::locx)
		.def_readwrite("y", &locXY::locy)
		;
		py::class_<LocAndOffsets>(m, "LocAndOffsets")
		.def_readwrite("loc_xy", &LocAndOffsets::location)
		.def_readwrite("off_x", &LocAndOffsets::off_x)
		.def_readwrite("off_y", &LocAndOffsets::off_y)
		.def("get_location", [](LocAndOffsets& loc)->int64_t{
			return (int64_t)loc.location;
		})
		.def("distance_to", [](LocAndOffsets& src, LocAndOffsets& dest)-> float {
			return locSys.Distance3d(src, dest) / INCH_PER_FEET;
		})
		.def("get_offset_loc", [](LocAndOffsets& src, float angleRad, float rangeFt)-> LocAndOffsets{
			auto absX = 0.0f, absY = 0.0f;
			locSys.GetOverallOffset(src, &absX, &absY);
			auto vectorAngleRad = 5 * M_PI / 4 - angleRad;
			auto result = src;
			result.off_x += (float)(rangeFt * INCH_PER_FEET * cos(vectorAngleRad));
			result.off_y += (float)(rangeFt * INCH_PER_FEET * sin(vectorAngleRad));
			locSys.RegularizeLoc(&result);
			return result;
		})
		;
		py::class_<LocFull>(m, "LocFull")
		.def_readwrite("loc_and_offsets", &LocFull::location)
		;
	#pragma endregion

	#pragma region Bonuslist etc
	py::class_<BonusList>(m, "BonusList")
			.def(py::init())
			.def("add", [](BonusList & bonlist, int value, int bonType, int mesline) {
				bonlist.AddBonus(value, bonType, mesline);
			}, "Adds a bonus entry. Args are: value, type, and bonus.mes line number")
			.def("add", [](BonusList & bonlist, int value, int bonType, std::string &text) {
				bonlist.AddBonus(value, bonType, text);
			}, "Adds a bonus entry. Args are: value, type, and free text")
			.def("add_from_feat", [](BonusList &bonlist, int value, int bonType, int mesline, int feat)->int
			{
				return bonlist.AddBonusFromFeat(value, bonType, mesline, (feat_enums)feat);
			})
			.def("add_from_feat", [](BonusList &bonlist, int value, int bonType, int mesline, std::string &feat)->int
			{
				return bonlist.AddBonusFromFeat(value, bonType, mesline, feat);
			})
			.def("set_overall_cap", [](BonusList & bonlist, int bonflags, int newCap, int newCapType, int bonusMesline) {
				bonlist.SetOverallCap(bonflags, newCap, newCapType, bonusMesline);
			 })
			.def("modify", &BonusList::ModifyBonus)
			.def("get_sum", &BonusList::GetEffectiveBonusSum)
			.def("get_total", &BonusList::GetEffectiveBonusSum)
			.def("add_zeroed", &BonusList::ZeroBonusSetMeslineNum, "Adds a zero-value bonus (usually to represent nullified bonuses)")
			.def("add_cap", [](BonusList & bonlist, int bonType, int value, int mesline) {
				 bonlist.AddCap(bonType, value, mesline); }, "Adds cap for a particular bonus type")
			.def("add_cap", [](BonusList & bonlist, int bonType, int value, int mesline, std::string &text) {
					 bonlist.AddCapWithCustomDescr(bonType, value, mesline, text);
				 }, "Adds cap for a particular bonus type")
			.def_readwrite("flags", &BonusList::bonFlags)
			;

	 py::class_<AttackPacket>(m, "AttackPacket")
		.def(py::init())
		.def("get_weapon_used", &AttackPacket::GetWeaponUsed)
		.def("is_offhand_attack", &AttackPacket::IsOffhandAttack)
		.def_readwrite("attacker", &AttackPacket::attacker)
		.def_readwrite("target", &AttackPacket::victim)
		.def("get_flags", [](AttackPacket& pkt)->int {	return (int)pkt.flags;	}, "D20CAF flags")
		.def("set_flags", [](AttackPacket& pkt, int flagsNew) {	pkt.flags = (D20CAF)flagsNew;	}, "sets attack packet D20CAF flags to value specified")
		.def_readwrite("action_type", &AttackPacket::d20ActnType)
		.def_readwrite("event_key", &AttackPacket::dispKey)
		;

	py::class_<DamagePacket>(m, "DamagePacket")
		.def(py::init())
		.def("add_dice", [](DamagePacket& damPkt, Dice & dice, int damType, int damageMesLine){
		damPkt.AddDamageDice(dice.ToPacked(), (DamageType)damType, damageMesLine);
		})
		.def("add_dice", [](DamagePacket& damPkt, Dice & dice, int damType, int damageMesLine, const char reason[]) {
		//damPkt.AddDamageDice(dice.ToPacked(), (DamageType)damType, damageMesLine);
			//not implemented yet
		})
		.def("add_physical_damage_res", [](DamagePacket& damPkt, int amount, int bypassingAttackPower, int damMesLine){
			damPkt.AddPhysicalDR(amount, bypassingAttackPower, damMesLine);
		}, "Adds physical (Slashing/Piercing/Crushing) damage resistance.")
		.def("add_damage_bonus", [](DamagePacket& damPkt, int32_t damBonus, int bonType, int bonMesline) {
			damPkt.AddDamageBonus(damBonus, bonType, bonMesline);  //Note:  Description string not supported now
		}, "Adds a damage Bonus.")
		.def("add_damage_resistance", [](DamagePacket& damPkt, int amount, int damType, int damMesLine) {
			auto _damType = (DamageType)damType;
			damPkt.AddDR(amount, _damType, damMesLine);
		}, "Adds damage resistance.")
		.def_readwrite("final_damage", &DamagePacket::finalDamage, "Final Damage Value")
		.def_readwrite("flags", &DamagePacket::flags, "1 - maximized, 2 - empowered")
		.def_readwrite("bonus_list", &DamagePacket::bonuses)
		.def_readwrite("critical_multiplier", &DamagePacket::critHitMultiplier, "1 by default, gets increased by various things")
		.def_readwrite("attack_power", &DamagePacket::attackPowerType, "See D20DAP_");

	py::class_<MetaMagicData>(m, "MetaMagicData")
		.def(py::init<>())
		.def("get_heighten_count", [](MetaMagicData& mmData)->int {	return mmData.metaMagicHeightenSpellCount;	})
		.def("get_enlarge_count", [](MetaMagicData& mmData)->int {	return mmData.metaMagicEnlargeSpellCount;	})
		.def("get_extend_count", [](MetaMagicData& mmData)->int {	return mmData.metaMagicExtendSpellCount;	})
		.def("get_widen_count", [](MetaMagicData& mmData)->int {	return mmData.metaMagicWidenSpellCount;	})
		.def("get_empower_count", [](MetaMagicData& mmData)->int {	return mmData.metaMagicEmpowerSpellCount;	})

		.def("set_heighten_count", [](MetaMagicData& mmData, int newVal) {  mmData.metaMagicHeightenSpellCount = newVal;	})
		.def("set_enlarge_count", [](MetaMagicData& mmData, int newVal) {  mmData.metaMagicEnlargeSpellCount = newVal;	})
		.def("set_extend_count", [](MetaMagicData& mmData, int newVal) {  mmData.metaMagicExtendSpellCount = newVal;	})
		.def("set_widen_count", [](MetaMagicData& mmData, int newVal) {  mmData.metaMagicWidenSpellCount = newVal;	})
		.def("set_empower_count", [](MetaMagicData& mmData, int newVal) {  mmData.metaMagicEmpowerSpellCount = newVal;	})
		.def("get_still", [](MetaMagicData& mmData)->bool {	return (mmData.metaMagicFlags & MetaMagic_Still) != 0;	})
		.def("set_still", [](MetaMagicData& mmData, bool newVal) { 
			if (newVal)
				mmData.metaMagicFlags |= MetaMagic_Still;
			else if (mmData.metaMagicFlags & MetaMagic_Still)
				mmData.metaMagicFlags ^= MetaMagic_Still;
		})
		.def("get_quicken", [](MetaMagicData& mmData)->bool {	return (mmData.metaMagicFlags & MetaMagic_Quicken) != 0;	})
		.def("set_quicken", [](MetaMagicData& mmData, bool newVal) {
			if (newVal)
				mmData.metaMagicFlags |= MetaMagic_Quicken;
			else if (mmData.metaMagicFlags & MetaMagic_Quicken)
				mmData.metaMagicFlags ^= MetaMagic_Quicken;
		})
		.def("get_silent", [](MetaMagicData& mmData)->bool {	return (mmData.metaMagicFlags & MetaMagic_Silent) != 0;	})
		.def("set_silent", [](MetaMagicData& mmData, bool newVal) {
			if (newVal)
				mmData.metaMagicFlags |= MetaMagic_Silent;
			else if (mmData.metaMagicFlags & MetaMagic_Silent)
				mmData.metaMagicFlags ^= MetaMagic_Silent;
		})
		.def("get_maximize", [](MetaMagicData& mmData)->bool {	return (mmData.metaMagicFlags & MetaMagic_Maximize) != 0;	})
		.def("set_maximize", [](MetaMagicData& mmData, bool newVal) {
			if (newVal)
				mmData.metaMagicFlags |= MetaMagic_Maximize;
			else if (mmData.metaMagicFlags & MetaMagic_Maximize)
				mmData.metaMagicFlags ^= MetaMagic_Maximize;
		})
		;

	py::class_<D20SpellData>(m, "D20SpellData")
		.def(py::init<>())
		.def(py::init<int>(), py::arg("spell_enum"))
		.def_readwrite("spell_enum", &D20SpellData::spellEnumOrg)
		.def_readwrite("spell_class", &D20SpellData::spellClassCode)
		.def("set_spell_level", [](D20SpellData& spData, int spLvl){ spData.spellSlotLevel = spLvl;	})
		.def("get_spell_level", [](D20SpellData& spData)->int{	return spData.spellSlotLevel;	})
		.def_readwrite("inven_idx", &D20SpellData::itemSpellData)
		.def("get_spell_store", [](D20SpellData&spData)->SpellStoreData {	return spData.ToSpellStore();	})
		.def("set_spell_class", [](D20SpellData& spData, int spClass) { spData.spellClassCode= spellSys.GetSpellClass(spClass);	})
		.def("get_metamagic_data", [](D20SpellData& spData)->MetaMagicData {return spData.metaMagicData; })
		;
	#pragma endregion

	#pragma region D20Actions
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
		.def_readwrite("spell_data", &D20Actn::d20SpellData)
		.def_readwrite("roll_id_0", &D20Actn::rollHistId0)
		.def_readwrite("roll_id_1", &D20Actn::rollHistId1)
		.def_readwrite("roll_id_2", &D20Actn::rollHistId2)
		.def("query_can_be_affected_action_perform", [](D20Actn& d20a, objHndl handle)->int{
			return d20Sys.D20QueryWithDataDefaultTrue(handle, DK_QUE_CanBeAffected_PerformAction, &d20a, 0);
		})
		.def("query_is_action_invalid", [](D20Actn& d20a, objHndl handle)->int {
			return d20Sys.d20QueryWithData(handle, DK_QUE_CanBeAffected_PerformAction, (int)&d20a, 0);
		})
		.def("to_hit_processing", [](D20Actn& d20a){
			combatSys.ToHitProcessing(d20a);
		})
		.def("filter_spell_targets", [](D20Actn&d20a, SpellPacketBody& pkt){
			return d20a.FilterSpellTargets(pkt);
		})
		.def("create_projectile_and_throw", [](D20Actn& d20a, int protoNum, LocAndOffsets endLoc)->objHndl {
			auto createProjAndThrow = temple::GetRef<objHndl(__cdecl)(locXY, int, int, int, LocAndOffsets, objHndl, objHndl)>(0x100B4D00);
			
			// if missed, randomize the target location a bit
			if (!(d20a.d20Caf & D20CAF_HIT)){
				endLoc.off_x = static_cast<float>(rngSys.GetInt(-30, 30));
				endLoc.off_y = static_cast<float>(rngSys.GetInt(-30, 30));
			}
			auto startLoc = objSystem->GetObject(d20a.d20APerformer)->GetLocationFull();
			return createProjAndThrow(startLoc.location, protoNum, 0,0, endLoc, d20a.d20APerformer, d20a.d20ATarget);
		})
		.def("create_projectile_and_throw", [](D20Actn& d20a, int protoNum, objHndl tgt)->objHndl {
			auto createProjAndThrow = temple::GetRef<objHndl(__cdecl)(locXY, int, int, int, LocAndOffsets, objHndl, objHndl)>(0x100B4D00);

			if (!tgt)
				tgt = d20a.d20ATarget;
			if (!tgt){
				logger->error("create_projectile_and_throw: Target is null! using performer as target.");
				tgt = d20a.d20APerformer;
			}
				

			auto endLoc = objSystem->GetObject(tgt)->GetLocationFull();

			// if missed, randomize the target location a bit
			if (!(d20a.d20Caf & D20CAF_HIT)) {
				endLoc.off_x = static_cast<float>(rngSys.GetInt(-30, 30));
				endLoc.off_y = static_cast<float>(rngSys.GetInt(-30, 30));
				endLoc.Regularize();
			}
			auto startLoc = objSystem->GetObject(d20a.d20APerformer)->GetLocationFull();
			return createProjAndThrow(startLoc.location, protoNum, 0, 0, endLoc, d20a.d20APerformer, d20a.d20ATarget);
		})
		.def("projectile_append", &D20Actn::ProjectileAppend)
		;


	py::enum_<D20ActionType>(m, "D20ActionType")
		.value("FiveFootStep", D20ActionType::D20A_5FOOTSTEP)
		.value("PythonAction", D20ActionType::D20A_PYTHON_ACTION)
		.value("StandardAttack", D20ActionType::D20A_STANDARD_ATTACK)
		.value("FullAttack", D20ActionType::D20A_FULL_ATTACK)
		.value("StandardRangedAttack", D20ActionType::D20A_STANDARD_RANGED_ATTACK)
		.value("StandUp", D20ActionType::D20A_STAND_UP)
		.value("CastSpell", D20ActionType::D20A_CAST_SPELL)
		.value("UseItem", D20ActionType::D20A_USE_ITEM)
		.value("UsePotion", D20ActionType::D20A_USE_POTION)
		.export_values()
		;

	py::class_<TurnBasedStatus>(m, "TurnBasedStatus")
		.def(py::init())
		.def_readwrite("hourglass_state", &TurnBasedStatus::hourglassState)
		.def_readwrite("num_bonus_attacks", &TurnBasedStatus::numBonusAttacks)
		.def_readwrite("surplus_move_dist", &TurnBasedStatus::surplusMoveDistance)
		.def_readwrite("flags", &TurnBasedStatus::tbsFlags)
		.def_readwrite("attack_mode_code", &TurnBasedStatus::attackModeCode, "0 - normal main hand, 99 - dual wielding, 999 - natural attack")
		;
	py::class_<ActionCostPacket>(m, "ActionCost")
		.def_readwrite("action_cost", &ActionCostPacket::hourglassCost)
		.def_readwrite("move_distance_cost", &ActionCostPacket::moveDistCost)
		;
	#pragma endregion

	#pragma region Radial Menu Enums
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

	#pragma endregion 

	#pragma region Radial Menu Entries
	py::class_<RadialMenuEntry>(m, "RadialMenuEntry")
		.def(py::init())
		.def("add_as_child", &RadialMenuEntry::AddAsChild, "Adds this node as a child to a specified node ID, and returns the newly created node ID (so you may give it other children, etc.)")
		.def("add_child_to_standard", &RadialMenuEntry::AddChildToStandard, "Adds this node as a child to a Standard Node (one of several hardcoded root nodes such as class, inventory etc.), and returns the newly created node ID (so you may give it other children, etc.)")
		.def_readwrite("flags", &RadialMenuEntry::flags)
		.def_readwrite("min_arg", &RadialMenuEntry::minArg)
		.def_readwrite("max_arg", &RadialMenuEntry::maxArg)
		;

	py::class_<RadialMenuEntryAction, RadialMenuEntry>(m, "RadialMenuEntryAction")
		.def(py::init<int, int, int, const char*>(), py::arg("combesMesLine"), py::arg("action_type"), py::arg("data1"), py::arg("helpTopic"))
		.def(py::init<std::string&, int, int, std::string&>(), py::arg("radialText"), py::arg("action_type"), py::arg("data1"), py::arg("helpTopic"))
		.def(py::init<SpellStoreData&>(), py::arg("spell_data"))
		.def("set_spell_data", [](RadialMenuEntryAction & entry, D20SpellData& spellData){
			entry.d20SpellData.Set(spellData.spellEnumOrg, spellData.spellClassCode, spellData.spellSlotLevel, spellData.itemSpellData, spellData.metaMagicData);
		})
		;

	py::class_<RadialMenuEntryPythonAction, RadialMenuEntryAction>(m, "RadialMenuEntryPythonAction")
		.def(py::init<int, int, int, int, const char*>(), py::arg("combatMesLine"), py::arg("action_type"), py::arg("action_id"), py::arg("data1"), py::arg("helpTopic"))
		.def(py::init<int, int, const char*, int, const char*>(), py::arg("combatMesLine"), py::arg("action_type"), py::arg("action_name"), py::arg("data1"), py::arg("helpTopic"))
		.def(py::init<std::string&, int, int, int, const char*>(), py::arg("radialText"), py::arg("action_type"), py::arg("action_id"), py::arg("data1"), py::arg("helpTopic"))
		.def(py::init<SpellStoreData&, int, int, int, const char*>(), py::arg("spell_store"), py::arg("action_type"), py::arg("action_id"), py::arg("data1"), py::arg("helpTopic")="")
		;

	py::class_<RadialMenuEntryToggle, RadialMenuEntry>(m, "RadialMenuEntryToggle")
		.def(py::init<std::string&, const char*>(), py::arg("radialText"), py::arg("helpTopic"))
		.def("link_to_args", [](RadialMenuEntryToggle & entry, DispatcherCallbackArgs &args, int argIdx)
		{
			entry.actualArg = (int)args.GetCondArgPtr(argIdx);
		})
		;

	py::class_<RadialMenuEntryParent>(m, "RadialMenuEntryParent")
		.def(py::init<int>(), py::arg("combesMesLine"))
		.def(py::init<std::string&>(), py::arg("radialText"))
		.def("add_as_child", &RadialMenuEntryParent::AddAsChild, "Adds this node as a child to a specified node ID, and returns the newly created node ID (so you may give it other children, etc.)")
		.def("add_child_to_standard", &RadialMenuEntryParent::AddChildToStandard, "Adds this node as a child to a Standard Node (one of several hardcoded root nodes such as class, inventory etc.), and returns the newly created node ID (so you may give it other children, etc.)")
		;

	py::class_<SpellEntryLevelSpec>(m, "SpellEntryLevelSpec")
		.def(py::init())
		.def_readwrite("spell_class_code", &SpellEntryLevelSpec::spellClass)
		.def_readwrite("spell_level", &SpellEntryLevelSpec::slotLevel)
		.def("casting_class", [](SpellEntryLevelSpec &spec)->int {
			if (spellSys.isDomainSpell(spec.spellClass))
				return -( (int)spec.spellClass );
			else
				return (int)spellSys.GetCastingClass(spec.spellClass);
			}, "Returns casting class. Domain spell specs will return negative numbers.")
		;
	#pragma endregion

	#pragma region Spell stuff
	py::class_<SpellEntry>(m, "SpellEntry")
		.def(py::init())
		.def(py::init<uint32_t>(), py::arg("spell_enum"))
		.def_readwrite("spell_enum", &SpellEntry::spellEnum)
		.def_readwrite("casting_time", &SpellEntry::castingTimeType)
		.def_readwrite("saving_throw_type", &SpellEntry::savingThrowType)
		.def_readwrite("min_target", &SpellEntry::minTarget)
		.def_readwrite("max_target", &SpellEntry::maxTarget)
		.def_readwrite("mode_target", &SpellEntry::modeTargetSemiBitmask)
		.def("is_base_mode_target", &SpellEntry::IsBaseModeTarget)
		.def("get_level_specs", [](SpellEntry &spEntry)->std::vector<SpellEntryLevelSpec> {
			auto result = std::vector<SpellEntryLevelSpec>();
			if (!spEntry.spellEnum)
				return result;
			for (auto i = 0u; i < spEntry.spellLvlsNum; i++){
				result.push_back(spEntry.spellLvls[i]);
			}
			return result;
		})
		.def("level_for_spell_class", [](SpellEntry &spEntry, int spellClass)->int
		{
			return spellSys.GetSpellLevelBySpellClass(spEntry.spellEnum, spellClass);
		})
		;



		py::class_<SpellPacketBody>(m, "SpellPacket")
			.def(py::init<uint32_t>(), py::arg("spell_id"))
			.def_readwrite("spell_enum", &SpellPacketBody::spellEnum)
			.def_readwrite("inventory_idx", &SpellPacketBody::invIdx)
			.def_readwrite("picker_result", &SpellPacketBody::pickerResult)
			.def_readwrite("spell_class", &SpellPacketBody::spellClass)
			.def_readwrite("spell_id", &SpellPacketBody::spellId)
			.def_readwrite("caster_level", &SpellPacketBody::casterLevel)
			.def_readwrite("loc", &SpellPacketBody::aoeCenter)
			.def_readwrite("caster", &SpellPacketBody::caster)
			.def("get_metamagic_data", [](SpellPacketBody&pkt) {
				return pkt.metaMagicData;
			})
			.def("get_target",[](SpellPacketBody &pkt, int idx)->objHndl
			{
				if (idx < (int)pkt.targetCount)
					return pkt.targetListHandles[idx];
				return objHndl::null;
			} )
			.def("set_projectile", [](SpellPacketBody &pkt, int idx, objHndl projectile){
				if (idx >=0 && idx < 5){
					spellSys.GetSpellPacketBody(pkt.spellId, &pkt);
					pkt.projectiles[idx] = projectile;
					if (pkt.projectileCount <= (uint32_t) idx)
						pkt.projectileCount = idx+1;

					// update the spell repositories
					spellSys.UpdateSpellPacket(pkt);
					pySpellIntegration.UpdateSpell(pkt.spellId);
				}
			})
			.def("is_divine_spell", &SpellPacketBody::IsDivine)
			.def("debit_spell", &SpellPacketBody::Debit)
			.def("update_registry", [](SpellPacketBody &pkt){
				spellSys.UpdateSpellPacket(pkt);
				pySpellIntegration.UpdateSpell(pkt.spellId);
			}, "Updates the changes made in this local copy in the active spell registry.")
			.def("set_spell_object", [](SpellPacketBody&pkt, int idx,  objHndl spellObj, int partsysId){
				pkt.spellObjs[idx].obj = spellObj;
				pkt.spellObjs[idx].partySysId = partsysId;
			})
			.def("add_spell_object", [](SpellPacketBody&pkt, objHndl spellObj, int partsysId) {
				auto idx = pkt.numSpellObjs;
				if (idx >= 128)
					return;
				pkt.spellObjs[idx].obj = spellObj;
				pkt.spellObjs[idx].partySysId = partsysId;
				pkt.numSpellObjs++;
			})
			.def("add_target",[](SpellPacketBody&pkt, objHndl handle, int partsysId)->bool{
				return pkt.AddTarget(handle, partsysId, false);
			})
			.def("end_target_particles", [](SpellPacketBody&pkt, objHndl handle){
				pkt.EndPartsysForTgtObj(handle);
			})
			.def("remove_target", [](SpellPacketBody&pkt, objHndl handle){
				pkt.RemoveObjFromTargetList(handle);
			})
			.def("check_spell_resistance", [](SpellPacketBody&pkt, objHndl tgt){
				return pkt.CheckSpellResistance(tgt);
			})
			.def("trigger_aoe_hit", [](SpellPacketBody&pkt) {
				if (!pkt.spellEnum)
					return;
				pySpellIntegration.SpellTrigger(pkt.spellId, SpellEvent::AreaOfEffectHit);
			})
			.def("float_spell_line", [](SpellPacketBody& pkt, objHndl handle, int lineId, int color){
				auto color_ = (FloatLineColor)color;
				floatSys.FloatSpellLine(handle, lineId, color_);
			})
			;

	#pragma endregion 

	#pragma endregion 

	

	py::class_<DispIoCondStruct, DispIO>(m, "EventObjModifier", "Used for checking modifiers before applying them")
		.def(py::init())
		.def_readwrite("return_val", &DispIoCondStruct::outputFlag)
		.def_readwrite("arg1", &DispIoCondStruct::arg1, "First modifier argument")
		.def_readwrite("arg2", &DispIoCondStruct::arg2, "Second modifier argument")
		.def_readwrite("modifier_spec", &DispIoCondStruct::condStruct, "Modifier Spec (DO NOT ADD HOOKS FROM HERE! Due to different format for vanilla ToEE and Temple+ modifier specs.)")
		.def("is_modifier", [](DispIoCondStruct& evtObj, std::string& s)->int {
			auto cond = conds.GetByName(s);
			if (!cond){
				logger->error("Unknown Modifier specified: {}", s);
				return 0;
			}
			auto isEqual = evtObj.condStruct == cond;
			return isEqual ? TRUE : FALSE;
		}, "Used for checking condition equality. The condition to be checked against is specified by its string ID")
		;


	py::class_<DispIoBonusList, DispIO>(m, "EventObjBonusList", "Used for fetching ability score levels and cur/max HP")
		.def(py::init())
		.def_readwrite("flags", &DispIoBonusList::flags)
		.def_readwrite("bonus_list", &DispIoBonusList::bonlist);

	py::class_<DispIoSavingThrow, DispIO>(m, "EventObjSavingThrow", "Used for fetching saving throw bonuses")
		.def_readwrite("bonus_list", &DispIoSavingThrow::bonlist)
		.def_readwrite("return_val", &DispIoSavingThrow::returVal)
		.def_readwrite("obj", &DispIoSavingThrow::obj)
		.def_readwrite("flags", &DispIoSavingThrow::flags);

	py::class_<DispIoDamage, DispIO>(m, "EventObjDamage", "Used for damage dice and such")
		.def_readwrite("attack_packet", &DispIoDamage::attackPacket)
		.def_readwrite("damage_packet", &DispIoDamage::damage)
		;


	py::class_<DispIoAttackBonus, DispIO>(m, "EventObjAttack", "Used for fetching attack or AC bonuses")
		.def(py::init())
		.def_readwrite("bonus_list", &DispIoAttackBonus::bonlist)
		.def_readwrite("attack_packet", &DispIoAttackBonus::attackPacket);

	py::class_<DispIoD20Signal, DispIO>(m, "EventObjD20Signal")
		.def(py::init())
		.def_readwrite("return_val", &DispIoD20Signal::return_val)
		.def_readwrite("data1", &DispIoD20Signal::data1)
		.def_readwrite("data2", &DispIoD20Signal::data2)
		;

	py::class_<DispIoD20Query, DispIO>(m, "EventObjD20Query")
		.def(py::init())
		.def_readwrite("return_val", &DispIoD20Query::return_val)
		.def_readwrite("data1", &DispIoD20Query::data1)
		.def_readwrite("data2", &DispIoD20Query::data2)
		.def("get_spell_packet", [](DispIoD20Query& evtObj)->SpellPacketBody& {
			SpellPacketBody* spPkt = (SpellPacketBody*)evtObj.data1;
			return *spPkt;
		}, "Used for CasterLevelMod callbacks to get a spellpacket from the data1 field")
		.def("get_d20_action", [](DispIoD20Query& evtObj)->D20Actn& {
			D20Actn* d20a= (D20Actn*)evtObj.data1;
			return *d20a;
		}, "Used for Q_IsActionInvalid_CheckAction callbacks to get a D20Action from the data1 field")
		;

	py::class_<DispIOTurnBasedStatus, DispIO>(m, "EventObjTurnBasedStatus")
		.def(py::init())
		.def_readwrite("tb_status", &DispIOTurnBasedStatus::tbStatus);


	py::class_<DispIoTooltip, DispIO>(m, "EventObjTooltip", "Tooltip event for mouse-overed objects.")
		.def("append", &DispIoTooltip::Append, "Appends a string")
		.def_readwrite("num_strings", &DispIoTooltip::numStrings);

	py::class_<DispIoObjBonus, DispIO>(m, "EventObjObjectBonus", "Used for Item Bonuses, initiative modifiers and others.")
		.def_readwrite("bonus_list", &DispIoObjBonus::bonOut)
		.def_readwrite("flags", &DispIoObjBonus::flags)
		.def_readwrite("return_val", &DispIoObjBonus::flags) // I think that field is also used for return_val somewhere... not 100% sure though. also leaving it for backward compatibility
		.def_readwrite("obj", &DispIoObjBonus::obj)
		;

	py::class_<DispIoDispelCheck, DispIO>(m, "EventObjDispelCheck", "Dispel Check Event")
		.def_readwrite("return_val", &DispIoDispelCheck::returnVal)
		.def_readwrite("flags", &DispIoDispelCheck::flags)
		.def_readwrite("spell_id", &DispIoDispelCheck::spellId);

	py::class_<DispIoD20ActionTurnBased, DispIO>(m, "EventObjD20Action", "Used for D20 Action Checks/Performance events and obtaining number of attacks (base/bonus/natural)")
		.def_readwrite("return_val", &DispIoD20ActionTurnBased::returnVal)
		.def_readwrite("d20a", &DispIoD20ActionTurnBased::d20a)
		.def_readwrite("turnbased_status", &DispIoD20ActionTurnBased::tbStatus);

	py::class_<DispIoMoveSpeed, DispIO>(m, "EventObjMoveSpeed", "Used for getting move speed, and also for model size scaling with Temple+.")
		.def_readwrite("factor", &DispIoMoveSpeed::factor)
		.def_readwrite("bonus_list", &DispIoMoveSpeed::bonlist)
		;

	py::class_<DispIOBonusListAndSpellEntry, DispIO>(m, "EventObjSpellEntry", "Used for Spell DC and Spell Resistance Mod")
		.def_readwrite("bonus_list", &DispIOBonusListAndSpellEntry::bonList)
		.def_readwrite("spell_entry", &DispIOBonusListAndSpellEntry::spellEntry)
		;

	py::class_<DispIoReflexThrow, DispIO>(m, "EventObjReflexSaveThrow", "Used for Reflex Save throws that reduce damage")
		.def_readwrite("attack_type", &DispIoReflexThrow::attackType)
		.def_readwrite("effective_reduction", &DispIoReflexThrow::effectiveReduction)
		.def_readwrite("reduction", &DispIoReflexThrow::reduction, "0,1,2 for None,Half,Quarter respectively")
		.def_readwrite("damage_mesline", &DispIoReflexThrow::damageMesLine, "Line no. from damage.mes to be used")
		.def_readwrite("attack_power", &DispIoReflexThrow::attackPower, "D20DAP_")
		.def_readwrite("flags", &DispIoReflexThrow::flags, "D20STD_ flags")
		;

	py::class_<DispIoObjEvent, DispIO>(m, "EventObjObjectEvent", "Used for Object Events (triggered by entering/leaveing AoE)")
		.def_readwrite("target", &DispIoObjEvent::tgt, "The critter affected by the AoE")
		.def_readwrite("aoe_obj", &DispIoObjEvent::aoeObj, "The origin of the AoE effect")
		.def_readwrite("evt_id", &DispIoObjEvent::evtId)
		;


	py::class_<DispIoAbilityLoss, DispIO>(m, "EventObjGetAbilityLoss", "Used for Ability Loss status")
		.def_readwrite("flags", &DispIoAbilityLoss::flags)
		.def_readwrite("spell_id", &DispIoAbilityLoss::spellId)
		.def_readwrite("stat_damaged", &DispIoAbilityLoss::statDamaged)
		.def_readwrite("result", &DispIoAbilityLoss::result)
		;

	py::class_<DispIoAttackDice, DispIO>(m, "EventObjGetAttackDice", "Used for getting the critter's attack dice")
		.def_readwrite("flags", &DispIoAttackDice::flags, "D20CAF_ ")
		.def_readwrite("damage_type", &DispIoAttackDice::attackDamageType, "D20DT_")
		.def_readwrite("bonus_list", &DispIoAttackDice::bonlist)
		.def_readwrite("dice_packed", &DispIoAttackDice::dicePacked)
		.def_readwrite("weapon", &DispIoAttackDice::weapon)
		.def_readwrite("wielder", &DispIoAttackDice::wielder)
		;

	py::class_<DispIoTypeImmunityTrigger, DispIO>(m, "EventObjImmunityTrigger", "Used for triggering the immunity handling query")
		.def_readwrite("should_perform_immunity_check", &DispIoTypeImmunityTrigger::interrupt)
		.def_readwrite("immunity_key", &DispIoTypeImmunityTrigger::SDDKey1)
		//.def()
		; // TODO

	py::class_<DispIoImmunity, DispIO>(m, "EventObjImmunityQuery", "Used for performing the immunity handling")
		.def_readwrite("spell_entry", &DispIoImmunity::spellEntry)
		.def_readwrite("spell_packet", &DispIoImmunity::spellPkt)
		;

	py::class_<DispIoEffectTooltip, DispIO>(m, "EventObjEffectTooltip", "Used for tooltips when hovering over the status effect indicators in the party portrait row")
		.def("append", &DispIoEffectTooltip::Append)
		;

	py::class_<EvtObjSpellCaster>(m, "EventObjSpellCaster", "Used for retrieving spell caster specs. New for Temple+!")
		.def_readwrite("bonus_list", &EvtObjSpellCaster::bonlist)
		.def_readwrite("arg0", &EvtObjSpellCaster::arg0)
		.def_readwrite("arg1", &EvtObjSpellCaster::arg1)
		.def_readwrite("spell_packet", &EvtObjSpellCaster::spellPkt)
		;

	py::class_<EvtObjActionCost, DispIO>(m, "EventObjActionCost", "Used for modifying action cost")
		.def_readwrite("cost_orig", &EvtObjActionCost::acpOrig)
		.def_readwrite("cost_new", &EvtObjActionCost::acpCur)
		.def_readwrite("d20a", &EvtObjActionCost::d20a)
		.def_readwrite("turnbased_status", &EvtObjActionCost::tbStat)
		;

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
	case dispTypeAcModifyByAttacker:
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
	case dispTypePythonSignal:
	case dispTypeBeginRound:
	case dispTypeDestructionDomain:
		pbEvtObj = py::cast(static_cast<DispIoD20Signal*>(args.dispIO));
		break;

	case dispTypeD20Query:
	case dispTypePythonQuery:
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
	case dispTypePythonActionPerform:
	case dispTypePythonActionAdd:
	case dispTypePythonActionCheck:
	case dispTypePythonActionFrame:
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

	case dispTypeSpellListExtension:
	case dispTypeGetBaseCasterLevel:
	case dispTypeLevelupSystemEvent:
		pbEvtObj = py::cast(static_cast<EvtObjSpellCaster*>(args.dispIO));
		break;

	case dispTypeActionCostMod:
		pbEvtObj = py::cast(static_cast<EvtObjActionCost*>(args.dispIO));
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
	return PyDispatchEventObject_Create(self->args.dispIO);
}

PyObject* PyEventArgs_GetAttachee(PyObject *obj, void *) {
	auto self = (PyEventArgs*)obj;
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
