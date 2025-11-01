
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
#include "history.h"
#include "bonus.h"
#include "config/config.h"
#include <mod_support.h>
#include "gamesystems/gamesystems.h"
#include "gamesystems/d20/d20stats.h"

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

	m.def("hash", [](std::string& text) {
		return ElfHash::Hash(text);
	});

	m.def("cache_name", [](std::string & text) {
		bonusSys.CacheCustomText(text);
		return ElfHash::Hash(text);
	}, "Caches the given string for reference by hash. Returns the hash.");

	m.def("class_enum_to_casting_class", [](int class_enum) {
		return spellSys.GetSpellClass(class_enum);
	});

	m.def("register_metamagic_feat", [](std::string &text) {
		auto feat = ElfHash::Hash(text);
		feats.AddMetamagicFeat(static_cast<feat_enums>(feat));
	});

	m.def("register_bard_song_stopping_python_action", [](int action) {
		actSeqSys.RegisterBardSongStoppingPythonAction(action);
	});

	m.def("get_metamagic_feats", []() {
		std::vector<int> result;
		auto mmFeats = feats.GetMetamagicFeats();
		for (auto &&feat : mmFeats) {
			result.push_back(static_cast<int>(feat));
		}
		return result;
	});

	m.def("get_stat_name", [](int stat)->py::bytes {
		return py::bytes(d20Stats.GetStatName((Stat)stat));
	});

	m.def("get_stat_short_name", [](int stat)->py::bytes {
		return py::bytes(d20Stats.GetStatShortName((Stat)stat));
	});

	m.def("is_temple_module", []() {
		return !modSupport.IsZMOD() && !modSupport.IsKotB() && !modSupport.IsPalCove() && !modSupport.IsIWD();
	});

	m.def("config_set_string", [](std::string& configItem, std::string& value) {
		auto configItemLower(tolower(configItem));
		if (configItemLower == "hpfornpchd") {
			config.HpForNPCHd = value;
			config.maxHpForNpcHitdice = false;  //Turn off if we are setting the enum flag
		}
		else if (configItemLower == "hponlevelup") {
			config.hpOnLevelup = value;
		}
		else {
			logger->warn("Can't set config item {}.", configItem);
		}
	});

	m.def("config_get_string", [](std::string& configItem) {
		auto configItemLower(tolower(configItem));
		if (configItemLower == "hpfornpchd") {
			//Check the old param
			if (config.maxHpForNpcHitdice) {
				return std::string("max");
			}
			return config.HpForNPCHd;
		}
		else if (configItemLower == "hponlevelup") {
			return config.hpOnLevelup;
		}
		else if (configItemLower == "defaultmodule") {
			return config.defaultModule;
		}

		logger->warn("Can't get config item {}.", configItem);
		return std::string("");
		});

	m.def("config_set_bool", [](std::string& configItem, bool value) {
		auto configItemLower(tolower(configItem));
		if (configItemLower == "preferuse5footstep") {
		    config.preferUse5FootStep = value;
		}
		else if (configItemLower == "slowerlevelling") {
			config.slowerLevelling = value;
		}
		else if (configItemLower == "disabletargetsurrounded") {
			config.disableTargetSurrounded = value;
		}
		else if (configItemLower == "disablechooserandomspell_regardinvulnerablestatus") {
			config.disableChooseRandomSpell_RegardInvulnerableStatus = value;
		}
		else if (configItemLower == "stricterrulesenforcement") {
			config.stricterRulesEnforcement = value;
		}
		else if (configItemLower == "iszmod") {
			modSupport.SetIsZMOD(value);
		}
		else {
			logger->warn("Can't set config item {}.", configItem);
		}
	});

	m.def("config_get_bool", [](std::string& configItem) {
		auto configItemLower(tolower(configItem));
		if (configItemLower == "preferuse5footstep") {
			return config.preferUse5FootStep;
		}
		else if (configItemLower == "slowerlevelling") {
			return config.slowerLevelling;
		}
		else if (configItemLower == "disabletargetsurrounded") {
			return config.disableTargetSurrounded;
		}
		else if (configItemLower == "disablechooserandomspell_regardinvulnerablestatus") {
			return config.disableChooseRandomSpell_RegardInvulnerableStatus;
		}
		else if (configItemLower == "stricterrulesenforcement") {
			return config.stricterRulesEnforcement;
		}
		else if (configItemLower == "iszmod") {
			return modSupport.IsZMOD();
		}
		logger->warn("Can't get config item {}.", configItem);
		return false;
	});

	m.def("config_set_int", [](std::string& configItem, int value) {
		auto configItemLower(tolower(configItem));
		if (configItemLower == "pointbuypoints") {
			config.pointBuyPoints = value;
		}
		else {
			logger->warn("Can't set config item {}.", configItem);
		}
		});

	m.def("config_get_int", [](std::string& configItem) {
		auto configItemLower(tolower(configItem));
		if (configItemLower == "pointbuypoints") {
			return config.pointBuyPoints;
		}

		logger->warn("Can't get config item {}.", configItem);
		return 0;
		});

	m.def("cur_seq_get_turn_based_status_flags", []() {
		int res = 0;
		auto tbStatus = actSeqSys.curSeqGetTurnBasedStatus();
		if (tbStatus) {
			res = tbStatus->tbsFlags;
		}
		return res;
	});

	m.def("cur_seq_set_turn_based_status_flags", [](int flags) {
		auto tbStatus = actSeqSys.curSeqGetTurnBasedStatus();
		if (tbStatus) {
			tbStatus->tbsFlags = flags;
		}
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

	m.def("dispatch_skill", [](objHndl obj, uint32_t skill_enum, BonusList& bonList, objHndl obj2 = objHndl::null, uint32_t flag = 1)-> int {
		auto skillLevel = dispatch.dispatch1ESkillLevel(obj, (SkillEnum)skill_enum, (BonusList*)&bonList, obj2, flag);
		return skillLevel;
	});

	m.def("dispatch_stat", [](objHndl obj, uint32_t stat, BonusList& bonList)-> int {
		DispIoBonusList evtObjAbScore;
		evtObjAbScore.flags |= 1; // effect unknown??
		evtObjAbScore.bonlist = bonList;
		auto result = objects.abilityScoreLevelGet(obj, (Stat)stat, &evtObjAbScore);
		bonList = evtObjAbScore.bonlist;
		return result;
	});

	m.def("dispatch_ability_check", [](objHndl handle, objHndl opponent, int statUsed, BonusList& bonlist, int flags)->int { // used in Trip attempts
		auto obj = objSystem->GetObject(handle);
		if (!obj) {
			return 0;
		}
		
		auto dispatcher = obj->GetDispatcher();
		DispIoObjBonus dispIo;
		if (!dispatcher->IsValid())
			return 0;
		dispIo.bonOut = &bonlist;
		dispIo.flags = flags;
		dispIo.obj = opponent;
		dispatch.DispatcherProcessor(dispatcher, dispTypeAbilityCheckModifier, DK_STAT_STRENGTH + statUsed, &dispIo);
		return dispIo.bonOut->GetEffectiveBonusSum();
	});

	m.def("create_history_type6_opposed_check", [](objHndl performer, objHndl defender, int performerRoll, int defenderRoll
		, BonusList& performerBonList, BonusList& defenderBonList, uint32_t combatMesLineTitle, uint32_t combatMesLineResult, uint32_t flag)-> int
	{
		auto rollHistId = histSys.RollHistoryAddType6OpposedCheck(performer, defender, performerRoll, defenderRoll
			, (BonusList*)&performerBonList, (BonusList*)&defenderBonList, combatMesLineTitle, combatMesLineResult, flag);
		return rollHistId;
	});

	m.def("create_history_dc_roll", [](objHndl performer, int dc, Dice& dice, int roll, std::string& text, BonusList& bonlist)-> int
	{
		auto ptext = bonusSys.CacheCustomText(text);
		auto rollHistId = histSys.RollHistoryAddType4MiscCheckRoll(performer, dc, ptext, dice.ToPacked(), roll, (BonusList*)&bonlist);
		return rollHistId;
	});
	
	m.def("create_history_attack_roll", [](objHndl performer, objHndl target, int roll, BonusList& bonlistAttacker, BonusList& bonlistTarget, uint32_t flags, int critHitRoll = -1)-> int
	{
		auto rollHistId = histSys.RollHistoryAddType0AttackRoll(roll, critHitRoll, performer, target, (BonusList*)&bonlistAttacker, (BonusList*)&bonlistTarget, (D20CAF)flags);
		return rollHistId;
	}, py::arg("performer"), py::arg("target"), py::arg("roll_result"), py::arg("bonus_list_attacker"), py::arg("bonus_list_defender"), py::arg("flags"), py::arg("critical_hit_roll") = -1);

	m.def("get_condition_ref", [](std::string& text)-> int
	{
		auto cond = conds.GetByName(text);
		if (!cond) {
			return 0;
		}
		return (uint32_t)cond;
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
		.def("replace_hook",
			[](CondStructNew & condStr, uint32_t ix, uint32_t dispType, uint32_t dispKey, py::function &pycallback, py::tuple &pydataTuple) {
			Expects(ix < condStr.numHooks);
			Expects(condStr.subDispDefs[ix].dispCallback != PyModHookWrapper);

			pydataTuple.inc_ref();

			condStr.subDispDefs[ix] =
				{ static_cast<enum_disp_type>(dispType)
				, static_cast<D20DispatcherKey>(dispKey)
				, PyModHookWrapper
				, reinterpret_cast<uint32_t>(pycallback.ptr())
				, reinterpret_cast<uint32_t>(pydataTuple.ptr())
				};
		}, "Replace an existing legacy hook with a python hook.")
		.def("add_to_feat_dict", [](CondStructNew &condStr, int feat_enum, int feat_max, int feat_offset) {
			condStr.AddToFeatDictionary((feat_enums)feat_enum, (feat_enums)feat_max, feat_offset);
		}, py::arg("feat_enum"), py::arg("feat_list_max") = -1, py::arg("feat_list_offset") = 0)
		.def("add_to_feat_dict", [](CondStructNew &condStr, std::string &feat_name, int feat_max, int feat_offset) {
			condStr.AddToFeatDictionary((feat_enums)ElfHash::Hash(feat_name), (feat_enums)feat_max, feat_offset);
		}, py::arg("feat_enum"), py::arg("feat_list_max") = -1, py::arg("feat_list_offset") = 0)
		// .def_readwrite("num_args", &CondStructNew::numArgs) // this is probably something we don't want to expose due to how ToEE saves/loads args
		.def_readwrite("name", &CondStructNew::condName)
		.def("extend_existing", [](CondStructNew &condStr, std::string condName){
			condStr.ExtendExisting(condName);
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
		.def("add_spell_teleport_prepare_standard", [](CondStructNew & condStr) {
			condStr.AddHook(dispTypeD20Signal, DK_SIG_Teleport_Prepare, temple::GetRef<int(__cdecl)(DispatcherCallbackArgs)>(0x100dbec0));
		})
		.def("add_spell_teleport_reconnect_standard", [](CondStructNew& condStr) {
			condStr.AddHook(dispTypeD20Signal, DK_SIG_Teleport_Reconnect, temple::GetRef<int(__cdecl)(DispatcherCallbackArgs)>(0x10262530));
		})
		.def("add_spell_dismiss_hook", [](CondStructNew &condStr){
			condStr.AddHook(dispTypeConditionAdd, DK_NONE, temple::GetRef<int(__cdecl)(DispatcherCallbackArgs)>(0x100CBD60));
		})
		.def("add_spell_dispell_check_hook", [](CondStructNew &condStr) {
		    condStr.AddHook(dispTypeDispelCheck, DK_NONE, temple::GetRef<int(__cdecl)(DispatcherCallbackArgs)>(0x100DB690));
		})
		.def("add_spell_touch_attack_discharge_radial_menu_hook", [](CondStructNew &condStr) {
			condStr.AddHook(dispTypeRadialMenuEntry, DK_NONE, temple::GetRef<int(__cdecl)(DispatcherCallbackArgs)>(0x100C3450));
		})
		.def("add_spell_dispel_check_standard", [](CondStructNew& condStr) {
			condStr.AddHook(dispTypeDispelCheck, DK_NONE, DispelCheck);
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
		.def("get_cond_name", [](DispatcherCallbackArgs& args) -> std::string {
			return std::string(args.subDispNode->condNode->condStruct->condName);
			})
		.def("get_arg", &DispatcherCallbackArgs::GetCondArg)
		.def("set_arg", &DispatcherCallbackArgs::SetCondArg)
		.def("get_obj_from_args", &DispatcherCallbackArgs::GetCondArgObjHndl)
		.def("set_args_from_obj", &DispatcherCallbackArgs::SetCondArgObjHndl)
		.def("get_param", [](DispatcherCallbackArgs &args, int paramIdx, int dflt){
			PyObject*tuplePtr = (PyObject*)args.GetData2();
			
			//return tuplePtr;
			if (!tuplePtr || paramIdx >= PyTuple_Size(tuplePtr)){
				return dflt;
			}
				
			auto tupItem = PyTuple_GetItem(tuplePtr, paramIdx);
			auto result = (int)PyLong_AsLong(tupItem);
			return result;
		}, py::arg("index"), py::arg("default") = 0)
		.def_readwrite("evt_obj", &DispatcherCallbackArgs::dispIO)
		.def("condition_remove", [](DispatcherCallbackArgs& args)
		{
			conds.ConditionRemove(args.objHndCaller, args.subDispNode->condNode);
		})
		.def("remove_spell_mod", &DispatcherCallbackArgs::RemoveSpellMod)
		.def("remove_spell_with_key", [](DispatcherCallbackArgs& args, int dispKey) {
			// Remove a spell using a different key to work around the mess that is removeSpell
			auto newArgs = args;
			newArgs.dispKey = dispKey;
			newArgs.RemoveSpell();
		})
		.def("remove_spell", [](DispatcherCallbackArgs& args) {
		   args.RemoveSpell();
		})
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
			.def("__copy__", [](const BonusList& src) {
					BonusList bonlist = src;
					return bonlist;
				})
			.def("reset", &BonusList::Reset)
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
			.def("set_cap_with_custom_descr", [](BonusList& bonlist, int newCap, int newCapType, int bonusMesline, std::string& textArg) {
				bonlist.AddCapWithCustomDescr(newCap, newCapType, bonusMesline, textArg);
			})
			.def("set_overall_cap", [](BonusList & bonlist, int bonflags, int newCap, int newCapType, int bonusMesline) {
				bonlist.SetOverallCap(bonflags, newCap, newCapType, bonusMesline);
			 })
			.def("modify", &BonusList::ModifyBonus)
			.def("get_sum",   &BonusList::GetEffectiveBonusSum)
			.def("get_total", &BonusList::GetEffectiveBonusSum)
			.def("get_highest", &BonusList::GetHighestBonus)
			.def("add_zeroed", &BonusList::ZeroBonusSetMeslineNum, "Adds a zero-value bonus (usually to represent nullified bonuses)")
			.def("add_cap", [](BonusList & bonlist, int bonType, int value, int mesline) {
				 bonlist.AddCap(bonType, value, mesline); }, "Adds cap for a particular bonus type")
			.def("add_cap", [](BonusList & bonlist, int bonType, int value, int mesline, std::string &text) {
					 bonlist.AddCapWithCustomDescr(bonType, value, mesline, text);
				 }, "Adds cap for a particular bonus type")
			.def_readwrite("flags", &BonusList::bonFlags)
			.def("saving_throw", [](BonusList & bonlist, objHndl tgt, objHndl atk, int dc, int ty, int flags)->bool {
					auto typ = static_cast<SavingThrowType>(ty);
					return damage.SavingThrow(tgt, atk, dc, typ, flags, &bonlist);
				}, "Roll a saving throw using this bonus list as a basis")
			;

	 py::class_<AttackPacket>(m, "AttackPacket")
		.def(py::init())
		.def("get_weapon_used", &AttackPacket::GetWeaponUsed, "gets used weapon, subject to manipulation via D20CAF flags")
		.def("set_weapon_used", [](AttackPacket& pkt, objHndl wpn) ->void { 
			pkt.weaponUsed = wpn; 
			})
		.def("is_offhand_attack", &AttackPacket::IsOffhandAttack)
		.def_readwrite("attacker", &AttackPacket::attacker)
		.def_readwrite("target", &AttackPacket::victim)
		.def("get_flags", [](AttackPacket& pkt)->int {	return (int)pkt.flags;	}, "D20CAF flags")
		.def("set_flags", [](AttackPacket& pkt, int flagsNew) {	pkt.flags = (D20CAF)flagsNew;	}, "sets attack packet D20CAF flags to value specified")
		.def_readwrite("action_type", &AttackPacket::d20ActnType)
		.def_readwrite("event_key", &AttackPacket::dispKey)
		.def_readwrite("ammo_item", &AttackPacket::ammoItem)
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
		.def("add_mod_factor", [](DamagePacket& damPkt, float factor, int damType, int damMesLine) {
			auto _damType = (DamageType)damType;
			damPkt.AddModFactor(factor, _damType, damMesLine);
		}, "Adds a modification factor to damage.")
		.def("get_overall_damage_by_type", [](DamagePacket& damPkt, int damType) {
			const auto _damType = static_cast<DamageType>(damType);
			return damPkt.GetOverallDamageByType(_damType);
		}, "Gets the total damage of a particular type.")
		.def("get_overall_damage", &DamagePacket::GetOverallDamage)
		.def("calc_final_damage", &DamagePacket::CalcFinalDamage)
		.def("play_pfx", &DamagePacket::PlayPfx)
		.def_readwrite("final_damage", &DamagePacket::finalDamage, "Final Damage Value")
		.def_readwrite("flags", &DamagePacket::flags, "1 - maximized, 2 - empowered")
		.def_readwrite("bonus_list", &DamagePacket::bonuses)
		.def_readwrite("critical_multiplier", &DamagePacket::critHitMultiplier, "1 by default, gets increased by various things")
		.def("critical_multiplier_apply", &DamagePacket::CriticalMultiplierApply)
		.def_readwrite("attack_power", &DamagePacket::attackPowerType, "See D20DAP_")
	;

	py::class_<MetaMagicData>(m, "MetaMagicData")
		.def(py::init<>())
		.def(py::init<unsigned int>(), py::arg("value"))
		.def("get_raw_value", [](MetaMagicData& mmData)->unsigned int {	return mmData;	})
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
		.def("get_spell_name", [](D20SpellData& spData)->py::bytes {
			auto spellName = spellSys.GetSpellName(spData.spellEnumOrg); 
				if (spellName == nullptr) {
					return py::bytes("");
				}
				else {
					return py::bytes(spellName);
				}
			})
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
		.value("UnspecifiedAttack", D20ActionType::D20A_UNSPECIFIED_ATTACK)
		.value("TouchAttack", D20ActionType::D20A_TOUCH_ATTACK)
		.value("StandUp", D20ActionType::D20A_STAND_UP)
		.value("TurnUndead", D20ActionType::D20A_TURN_UNDEAD)
		.value("ClassAbility", D20ActionType::D20A_CLASS_ABILITY_SA)
		.value("CastSpell", D20ActionType::D20A_CAST_SPELL)
		.value("UseItem", D20ActionType::D20A_USE_ITEM)
		.value("UsePotion", D20ActionType::D20A_USE_POTION)
		.value("Feint", D20ActionType::D20A_FEINT)
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
		.value("Tactical", Tactical)
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

	py::class_<RadialMenuEntrySlider, RadialMenuEntry>(m, "RadialMenuEntrySlider")
		.def(py::init<const std::string&, const std::string&, int, int, const std::string&>(), py::arg("radialText"), py::arg("titleText"), py::arg("min_val"), py::arg("max_val"), py::arg("help_topic") )
		.def("link_to_args", [](RadialMenuEntrySlider& entry, DispatcherCallbackArgs& args, int argIdx)
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
		.def_readwrite("spell_school_enum", &SpellEntry::spellSchoolEnum)
		.def_readwrite("spell_subschool_enum", &SpellEntry::spellSubSchoolEnum)
		.def_readwrite("spell_component_flags", &SpellEntry::spellComponentBitmask)
		.def_readwrite("descriptor", &SpellEntry::spellDescriptorBitmask)
		.def_readwrite("casting_time", &SpellEntry::castingTimeType)
		.def_readwrite("saving_throw_type", &SpellEntry::savingThrowType)
		.def_readwrite("min_target", &SpellEntry::minTarget)
		.def_readwrite("max_target", &SpellEntry::maxTarget)
		.def_readwrite("mode_target", &SpellEntry::modeTargetSemiBitmask)
		.def("is_base_mode_target", [](SpellEntry &spEntry, int type)->bool {
		    return spEntry.IsBaseModeTarget(static_cast<UiPickerType>(type));
		})
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
		.def("get_lowest_spell_level", [](SpellEntry& spEntry)->int
		{
			return spEntry.GetLowestSpellLevel(spEntry.spellEnum);
		})
		.def_readwrite("spellRangeType", &SpellEntry::spellRangeType)
		.def("get_spell_range_exact", [](SpellEntry &spEntry, int casterLevel, objHndl &caster)->int
		{
			return spellSys.GetSpellRangeExact(spEntry.spellRangeType, casterLevel, caster);
		})
		;

		py::enum_<SpellRangeType>(m, "SpellRangeType")
			.value("SRT_Specified", SpellRangeType::SRT_Specified)
			.value("SRT_Personal", SpellRangeType::SRT_Personal)
			.value("SRT_Touch", SpellRangeType::SRT_Touch)
			.value("SRT_Close", SpellRangeType::SRT_Close)
			.value("SRT_Medium", SpellRangeType::SRT_Medium)
			.value("SRT_Long", SpellRangeType::SRT_Long)
			.value("SRT_Unlimited", SpellRangeType::SRT_Unlimited)
			.value("SRT_Special_Inivibility_Purge", SpellRangeType::SRT_Special_Inivibility_Purge)
			;

		py::class_<SpellPacketBody>(m, "SpellPacket")
			.def(py::init<objHndl, D20SpellData>(), py::arg("caster"), py::arg("spell_data"))
			.def(py::init<uint32_t>(), py::arg("spell_id"))
			.def_readwrite("spell_enum", &SpellPacketBody::spellEnum)
			.def_readwrite("spell_known_slot_level", &SpellPacketBody::spellKnownSlotLevel)
			.def_readwrite("inventory_idx", &SpellPacketBody::invIdx)
			.def_readwrite("picker_result", &SpellPacketBody::pickerResult)
			.def_readwrite("spell_class", &SpellPacketBody::spellClass)
			.def_readwrite("spell_id", &SpellPacketBody::spellId)
			.def_readwrite("caster_level", &SpellPacketBody::casterLevel)
			.def_readwrite("dc", &SpellPacketBody::dc)
			.def_readwrite("loc", &SpellPacketBody::aoeCenter)
			.def_readwrite("caster", &SpellPacketBody::caster)
			.def_readwrite("duration_remaining", &SpellPacketBody::durationRemaining)
			.def("get_spell_casting_class", [](SpellPacketBody&pkt) {
				return static_cast<int>(spellSys.GetCastingClass(pkt.spellClass));
				})
			.def("get_metamagic_data", [](SpellPacketBody&pkt) -> MetaMagicData {
				return MetaMagicData(pkt.metaMagicData);
			})
			.def("get_spell_component_flags", [](SpellPacketBody& pkt) {
				auto result = (uint32_t)pkt.GetSpellComponentFlags();
				return result;
			})
			.def_readonly("target_count", &SpellPacketBody::targetCount)
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
			.def("is_arcane_spell", &SpellPacketBody::IsArcane)
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
			.def("check_spell_resistance_force", [](SpellPacketBody& pkt, objHndl tgt) {
				return pkt.CheckSpellResistance(tgt, true);  //Force the check even if the spell wouldn't normally allow it
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
	.def("is_modifier_hash", [](DispIoCondStruct& evtObj, int key)->int {
			auto cond = conds.GetById(key);
			if (!cond) {
				logger->error("Unknown Modifier specified: {}", key);
				return 0;
			}
			auto isEqual = evtObj.condStruct == cond;
			return isEqual ? TRUE : FALSE;
		}, "Used for checking condition equality. Use tpdp.hash on the condition name to get the right key for a condition")
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
		.def(py::init())
		.def_readwrite("attack_packet", &DispIoDamage::attackPacket)
		.def_readwrite("damage_packet", &DispIoDamage::damage)
		.def("dispatch", [](DispIoDamage& evtObj, objHndl handle, int dispType, int dispKey ) {
			auto dispType_ = (enum_disp_type)dispType;
			auto dispKey_ = (D20DispatcherKey)dispKey;
			dispatch.DispatchDamage(handle, &evtObj, dispType_, dispKey_);
			})
		.def("dispatch_spell_damage", [](DispIoDamage& evtObj, objHndl handle, objHndl target, SpellPacketBody & pkt) {
				dispatch.DispatchSpellDamage(handle, &evtObj.damage, target, &pkt);
			})
		.def("send_signal", [](DispIoDamage& evtObj, objHndl handle, int signalCode) {
				auto dispKey = (D20DispatcherKey)(signalCode + DK_SIG_HP_Changed);
				d20Sys.d20SendSignal(handle, dispKey, (int)&evtObj, 0);
			})
		;


	py::class_<DispIoAttackBonus, DispIO>(m, "EventObjAttack", "Used for fetching attack or AC bonuses")
		.def(py::init())
		.def("__copy__", [](const DispIoAttackBonus& self) {
			DispIoAttackBonus copy = self;
			return copy;
			})
		.def_readwrite("bonus_list", &DispIoAttackBonus::bonlist)
		.def_readwrite("attack_packet", &DispIoAttackBonus::attackPacket)
		.def("dispatch", [](DispIoAttackBonus& evtObj, objHndl handle, objHndl target, int disp_type, int disp_key)->int {
			auto result = evtObj.Dispatch(handle, target, (enum_disp_type)disp_type, (D20DispatcherKey)disp_key);
			return result;
			}, "")
		;

	py::class_<DispIoD20Signal, DispIO>(m, "EventObjD20Signal")
		.def(py::init())
		.def_readwrite("return_val", &DispIoD20Signal::return_val)
		.def_readwrite("data1", &DispIoD20Signal::data1)
		.def_readwrite("data2", &DispIoD20Signal::data2)
		.def("get_obj_from_args", [](DispIoD20Query& evtObj)->objHndl {
			
			auto handle = objHndl::FromUpperAndLower(evtObj.data2, evtObj.data1);
			if (!gameSystems->GetObj().IsValidHandle(handle))
				handle = objHndl::null;
			return handle;
			}, "Used for python signals that have a handle as the parameter.")
		.def("set_args_from_obj", [](DispIoD20Query& evtObj, objHndl & handle)->void {

				if (objSystem->IsValidHandle(handle)) {
					evtObj.data1 = handle.GetHandleLower();
					evtObj.data2 = handle.GetHandleUpper();
				}
				else {
					logger->error("EventObjD20Signal::set_args_from_obj - invalid handle specified, ignoring.");
				}

			}, "Used for python signals that have a handle as the parameter.")
		.def("get_d20_action", [](DispIoD20Signal& evtObj)->D20Actn& {
			D20Actn* d20a = (D20Actn*)evtObj.data1;
			return *d20a;
			}, "Used for S_TouchAttack to get a D20Action from the data1 field")
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
		.def("get_d20_spell_data", [](DispIoD20Query& evtObj)->D20SpellData& {
			D20SpellData* d20sd = (D20SpellData*)evtObj.data1;
			return *d20sd;
		}, "Used for Q_SpellInterrupted callbacks to get a D20SpellData from the data1 field")
		.def("get_obj_from_args", [](DispIoD20Query& evtObj)->objHndl {
			objHndl handle{ ((((uint64_t)evtObj.data2) << 32) | evtObj.data1) };
			if (!gameSystems->GetObj().IsValidHandle(handle))
				handle = objHndl::null;
			return handle;
		}, "Used for python queries that have a handle as the parameter.")
		.def("set_args_from_obj", [](DispIoD20Query& evtObj, objHndl& handle)->void {

			if (objSystem->IsValidHandle(handle)) {
				evtObj.data1 = handle.GetHandleLower();
				evtObj.data2 = handle.GetHandleUpper();
			}
			else {
				logger->error("EventObjD20Signal::set_args_from_obj - invalid handle specified, ignoring.");
			}

		}, "Used for python signals that have a handle as the parameter.")
		;

	py::class_<DispIOTurnBasedStatus, DispIO>(m, "EventObjTurnBasedStatus")
		.def(py::init())
		.def_readwrite("tb_status", &DispIOTurnBasedStatus::tbStatus);


	py::class_<DispIoTooltip, DispIO>(m, "EventObjTooltip", "Tooltip event for mouse-overed objects.")
		.def("append", &DispIoTooltip::Append, "Appends a string")
		.def("append_distinct", &DispIoTooltip::AppendDistinct, "Appends a string if not already in the list")
		.def_readwrite("num_strings", &DispIoTooltip::numStrings);

	py::class_<DispIoObjBonus, DispIO>(m, "EventObjObjectBonus", "Used for Item Bonuses, initiative modifiers and others.")
		.def(py::init())
		.def_readwrite("bonus_list", &DispIoObjBonus::bonOut)
		.def_readwrite("flags", &DispIoObjBonus::flags)
		.def_readwrite("return_val", &DispIoObjBonus::flags) // I think that field is also used for return_val somewhere... not 100% sure though. also leaving it for backward compatibility
		.def_readwrite("obj", &DispIoObjBonus::obj)
		.def("dispatch", [](DispIoObjBonus& evtObj, objHndl handle, int disp_type, int disp_key)->void{
				auto obj = objSystem->GetObject(handle);
				if (!obj) return;
				auto disp = obj->GetDispatcher();
				if (!dispatch.dispatcherValid(disp)) return;
				auto dispType = (enum_disp_type)disp_type;
				dispatch.DispatcherProcessor(disp, dispType, disp_key, &evtObj);
				return;
			}, "")
		;

	py::class_<DispIoDispelCheck, DispIO>(m, "EventObjDispelCheck", "Dispel Check Event")
		.def_readwrite("return_val", &DispIoDispelCheck::returnVal)
		.def_readwrite("flags", &DispIoDispelCheck::flags)
		.def_readwrite("spell_id", &DispIoDispelCheck::spellId);

	py::class_<DispIoD20ActionTurnBased, DispIO>(m, "EventObjD20Action", "Used for D20 Action Checks/Performance events and obtaining number of attacks (base/bonus/natural)")
		.def_readwrite("return_val", &DispIoD20ActionTurnBased::returnVal)
		.def_readwrite("d20a", &DispIoD20ActionTurnBased::d20a)
		.def_readwrite("bonus_list", &DispIoD20ActionTurnBased::bonlist)
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
		.def_readwrite("return_val", &DispIoImmunity::returnVal)
		;

	py::class_<DispIoEffectTooltip, DispIO>(m, "EventObjEffectTooltip", "Used for tooltips when hovering over the status effect indicators in the party portrait row")
		.def("append", &DispIoEffectTooltip::Append)
		.def("append_distinct", &DispIoEffectTooltip::AppendDistinct)
		;

	py::class_<EvtObjSpellCaster>(m, "EventObjSpellCaster", "Used for retrieving spell caster specs. New for Temple+!")
		.def_readwrite("bonus_list", &EvtObjSpellCaster::bonlist)
		.def_readwrite("arg0", &EvtObjSpellCaster::arg0)
		.def_readwrite("arg1", &EvtObjSpellCaster::arg1)
		.def_readwrite("spell_packet", &EvtObjSpellCaster::spellPkt)
		;

	py::class_<DispIoSpellsPerDay>(m, "EventObjSpellsPerDay", "Used for retrieving spells per day mods. Resurrected in Temple+!")
		.def_readwrite("bonus_list", &DispIoSpellsPerDay::bonList)
		.def_readwrite("caster_class", &DispIoSpellsPerDay::classCode)
		.def_readwrite("spell_level", &DispIoSpellsPerDay::spellLvl)
		.def_readwrite("base_caster_level", &DispIoSpellsPerDay::casterEffLvl)
		;

	py::class_<EvtObjActionCost, DispIO>(m, "EventObjActionCost", "Used for modifying action cost")
		.def_readwrite("cost_orig", &EvtObjActionCost::acpOrig)
		.def_readwrite("cost_new", &EvtObjActionCost::acpCur)
		.def_readwrite("d20a", &EvtObjActionCost::d20a)
		.def_readwrite("turnbased_status", &EvtObjActionCost::tbStat)
		;

	py::class_<EvtObjMetaMagic, DispIO>(m, "EvtObjMetaMagic", "Used for modifying metamagic data")
		.def_readwrite("meta_magic", &EvtObjMetaMagic::mmData)
		.def_readwrite("spell_enum", &EvtObjMetaMagic::spellEnum)
		.def_readwrite("spell_level", &EvtObjMetaMagic::spellLevel)
		.def_readwrite("spell_class", &EvtObjMetaMagic::spellClass)
		.def("is_divine_spell", [](EvtObjMetaMagic mm)->bool {
			if (spellSys.isDomainSpell(mm.spellClass)) {
				return true;
			}

			auto castingClass = spellSys.GetCastingClass(mm.spellClass);
			if (d20ClassSys.IsDivineCastingClass(castingClass)) {
				return true;
			}

			return false;
		}, "")
		.def("is_arcane_spell", [](EvtObjMetaMagic mm)->bool {
			if (spellSys.isDomainSpell(mm.spellClass)) {
				return false;
			}

			auto castingClass = spellSys.GetCastingClass(mm.spellClass);
			if (d20ClassSys.IsArcaneCastingClass(castingClass)) {
				return true;
			}

			return false;
			}, "")
		.def("get_spell_casting_class", [](EvtObjMetaMagic mm)->int {
			return static_cast<int>(spellSys.GetCastingClass(mm.spellClass));
		}, "")
		;

	py::class_<EvtObjSpecialAttack, DispIO>(m, "EvtObjSpecialAttack", "Used for applying effects")
		.def_readwrite("attack", &EvtObjSpecialAttack::attack)
		.def_readwrite("target", &EvtObjSpecialAttack::target)
		;

	py::class_<EvtObjRangeIncrementBonus, DispIO>(m, "EvtObjRangeIncrementBonus", "Add a bonus to the range increment.")
		.def_readwrite("weapon_used", &EvtObjRangeIncrementBonus::weaponUsed, "Weapon Being Used")
		.def_readwrite("range_bonus", &EvtObjRangeIncrementBonus::rangeBonus, "The Bonus to add to the weapon's range increment")
		;

	py::class_<EvtObjDealingSpellDamage, DispIO>(m, "EvtObjDealingSpellDamage", "Dealing damage from a spell.")
		.def_readwrite("damage_packet", &EvtObjDealingSpellDamage::damage, "Damage Packet.")
		.def_readwrite("target", &EvtObjDealingSpellDamage::target, "The target of the spell.")
		.def_readwrite("spell_packet", &EvtObjDealingSpellDamage::spellPkt, "Spell packet.")
		;

	py::class_<EvtObjSpellTargetBonus, DispIO>(m, "EvtObjSpellPacketTargetBonusList", "Spell Packet, Target and Bonus List.")
		.def_readwrite("bonus_list", &EvtObjSpellTargetBonus::bonusList, "Bonus list.")
		.def_readwrite("target", &EvtObjSpellTargetBonus::target, "The target of the spell.")
		.def_readwrite("spell_packet", &EvtObjSpellTargetBonus::spellPkt, "Spell packet.")
		;

	py::class_<EvtIgnoreDruidOathCheck, DispIO>(m, "EvtIgnoreDruidOathCheck", "Check if the Druid oath chan be ignored.")
		.def_readwrite("item", &EvtIgnoreDruidOathCheck::item, "The item being checked.")
		.def_readwrite("ignore_druid_oath", &EvtIgnoreDruidOathCheck::ignoreDruidOath, "True if the druid oath should be ignored for this item, false otherwise.")
		;

	py::class_<EvtObjAddMesh, DispIO>(m, "EvtObjAddMesh", "Adds addmeshes via modifiers.")
		.def_readwrite("obj", &EvtObjAddMesh::handle, "An object")
		.def_readonly("addmeshes", &EvtObjAddMesh::addmeshes, "The addmesh list")
		.def("append", &EvtObjAddMesh::Append, "Adds an addmesh to the list")
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

	case dispTypeDealingDamageWeaponlikeSpell:
	case dispTypeDealingDamage:
	case dispTypeTakingDamage:
	case dispTypeDealingDamage2:
	case dispTypeTakingDamage2:
		pbEvtObj = py::cast(static_cast<DispIoDamage*>(args.dispIO));
		break;

	
	case dispConfirmCriticalBonus:
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
	case dispTypeArmorCheckPenalty:
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
	case dispTypeSpellCasterGeneral:
		pbEvtObj = py::cast(static_cast<EvtObjSpellCaster*>(args.dispIO));
		break;

	case dispType58SpellsPerDayMod:
		pbEvtObj = py::cast(static_cast<DispIoSpellsPerDay*>(args.dispIO));
		break;

	case dispTypeActionCostMod:
		pbEvtObj = py::cast(static_cast<EvtObjActionCost*>(args.dispIO));
		break;

	case dispTypeSpecialAttack:
		pbEvtObj = py::cast(static_cast<EvtObjSpecialAttack*>(args.dispIO));
		break;

	case dispRangeIncrementBonus:
		pbEvtObj = py::cast(static_cast<EvtObjRangeIncrementBonus*>(args.dispIO));
		break;

	case dispTypeDealingDamageSpell:
		pbEvtObj = py::cast(static_cast<EvtObjDealingSpellDamage*>(args.dispIO));
		break;

	case dispTypeMetaMagicMod:
		pbEvtObj = py::cast(static_cast<EvtObjMetaMagic*>(args.dispIO));
		break;

	case dispTypeSpellResistanceCasterLevelCheck:
	case dispTypeTargetSpellDCBonus:
		pbEvtObj = py::cast(static_cast<EvtObjSpellTargetBonus*>(args.dispIO));
		break;

	case dispTypeIgnoreDruidOathCheck:
		pbEvtObj = py::cast(static_cast<EvtIgnoreDruidOathCheck*>(args.dispIO));
		break;

	case dispTypeAddMesh:
		pbEvtObj = py::cast(static_cast<EvtObjAddMesh*>(args.dispIO));
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
