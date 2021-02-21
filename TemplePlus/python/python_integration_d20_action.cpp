#include "stdafx.h"
#include "python_integration_d20_action.h"
#include <gamesystems/gamesystems.h>
#include <gamesystems/objects/objsystem.h>
#include "python_object.h"
#include "python/python_integration_spells.h"

#include <pybind11/embed.h>
#include <pybind11/cast.h>

#include "d20.h"
#include "action_sequence.h"
#include "float_line.h"
#include "ui/ui_picker.h"

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
PythonD20ActionIntegration pythonD20ActionIntegration;


PYBIND11_EMBEDDED_MODULE(tpactions, m) {
	m.doc() = "Temple+ D20 Actions module, used for handling of D20 actions & action sequencing.";

	py::class_<ActnSeq>(m, "ActionSequence", R"(Describes a D20 action sequence.)")
		.def_readwrite("cur_idx", &ActnSeq::d20aCurIdx)
		.def_readwrite("performer", &ActnSeq::performer)
		.def_readwrite("tb_status", &ActnSeq::tbStatus)
		.def_readwrite("target", &ActnSeq::targetObj)
		.def_readwrite("spell_packet", &ActnSeq::spellPktBody)
		.def_readwrite("spell_action", &ActnSeq::d20Action)
		.def("add_action", [](ActnSeq & actSeq, D20Actn & d20a){
			actSeq.d20ActArray[actSeq.d20ActArrayNum++] = d20a;
		})
		;

	py::class_<PickerArgs>(m, "PickerArgs")
		.def_readwrite("spell_enum", &PickerArgs::spellEnum)
		.def_readwrite("caster", &PickerArgs::caster)
		.def_readwrite("mode_target", &PickerArgs::modeTarget)
		.def("get_base_mode_target", &PickerArgs::GetBaseModeTarget)
		.def("set_mode_target_flag", &PickerArgs::SetModeTargetFlag)
		.def("is_mode_target_flag_set", &PickerArgs::IsModeTargetFlagSet)
			;

	py::enum_<UiPickerType>(m, "ModeTarget", py::module_local())
		.value("Single", UiPickerType::Single)
		.value("Cone", UiPickerType::Cone)
		.value("Area", UiPickerType::Area)
		.value("Personal", UiPickerType::Personal)
		.value("Ray", UiPickerType::Ray)
		.value("Wall", UiPickerType::Wall)
		.value("EndEarlyMulti", UiPickerType::EndEarlyMulti)
		.value("OnceMulti", UiPickerType::OnceMulti)
		.value("Any30Feet", UiPickerType::Any30Feet)
		.value("PickOrigin", UiPickerType::PickOrigin)
		;

	m.def("add_to_seq", [](D20Actn & d20a, ActnSeq & actSeq){
		actSeq.d20ActArray[actSeq.d20ActArrayNum++] = d20a;
	});

	m.def("get_new_spell_id", []()->int{
		return spellSys.GetNewSpellId();
	});

	m.def("register_spell_cast", [](SpellPacketBody &spellPkt, int spellId){
		spellSys.RegisterSpell(spellPkt, spellId);
	});

	m.def("trigger_spell_effect", [](int spellId){
		pySpellIntegration.SpellTrigger(spellId, SpellEvent::SpellEffect);
	});
	m.def("trigger_spell_projectile", [](int spellId, objHndl projectile) {

		auto projectileIdx = -1;
		
		SpellPacketBody pkt(spellId);
		if (!pkt.spellEnum){
			logger->debug("trigger_spell_projectile: Unable to retrieve spell packet!");
			return FALSE;
		}

		int spellEnum = pkt.spellEnum;

		SpellEntry spEntry(spellEnum);
		if (!spEntry.projectileFlag)
			return FALSE;

		pySpellIntegration.SpellSoundPlay(&pkt, SpellEvent::SpellStruck);

		// get the projectileIdx
		for (auto i = 0; i< 5; i++) {
			if (pkt.projectiles[i] == projectile) {
				projectileIdx = i;
				break;
			}
		}

		if (projectileIdx < 0) {
			logger->error("ProjectileHitSpell: Projectile not found!");
			return FALSE;
		}

		pySpellIntegration.SpellTriggerProjectile(spellId, SpellEvent::EndProjectile, projectile, projectileIdx);

		spellSys.GetSpellPacketBody(spellId, &pkt); // update spell if altered by the above

		spellSys.UpdateSpellPacket(pkt);
		pySpellIntegration.UpdateSpell(pkt.spellId);

		return TRUE;
	});

	m.def("get_cur_seq", []()->ActnSeq &{
		return **actSeqSys.actSeqCur;
	});
	
	m.def("action_cost_from_spell_casting_time", [](int castingTimeType)->std::tuple<int, int> {
		int hourglassCost = 0;
		auto result = d20Sys.CombatActionCostFromSpellCastingTime(castingTimeType, hourglassCost);
		std::tuple<int, int> ret( {(int)result, hourglassCost} ) ;
		return ret;
		}
	);
}

PythonD20ActionIntegration::PythonD20ActionIntegration()
	:PythonIntegration("rules\\d20_actions\\action*.py", "(action(\\d{3,}).*)\\.py"){
}

void PythonD20ActionIntegration::GetActionEnums(std::vector<int>& actionEnums){

	for (auto it : mScripts){
		actionEnums.push_back(it.first);
	}
}

std::string PythonD20ActionIntegration::GetActionName(int actionEnum){
	auto actionSpecEntry = mScripts.find(actionEnum);
	if (actionSpecEntry == mScripts.end())
		return fmt::format(""); 

	return RunScriptStringResult(actionSpecEntry->second.id, (EventId)D20ActionSpecFunc::GetActionName, nullptr);
}



int PythonD20ActionIntegration::GetInt(int actionEnum, D20ActionSpecFunc specType, int defaultVal){
	auto actionSpecEntry = mScripts.find(actionEnum);
	if (actionSpecEntry == mScripts.end())
		return defaultVal; 

	return RunScriptDefault0(actionSpecEntry->second.id, (EventId)specType, nullptr);
}

int PythonD20ActionIntegration::GetActionDefinitionFlags(int actionEnum){
	return GetInt(actionEnum, D20ActionSpecFunc::GetActionDefinitionFlags);
}

int PythonD20ActionIntegration::GetTargetingClassification(int actionEnum){
	return GetInt(actionEnum, D20ActionSpecFunc::GetTargetingClassification);
}

void PythonD20ActionIntegration::ModifyPicker(int actionEnum, PickerArgs * pickArgs){
	auto actionSpecEntry = mScripts.find(actionEnum);
	if (actionSpecEntry == mScripts.end()) {
		return;
	}

	py::object pbPicker = py::cast(pickArgs);

	auto dispPyArgs = Py_BuildValue("(O)", pbPicker.ptr());

	auto result = (ActionErrorCode)RunScriptDefault0(actionSpecEntry->second.id, (EventId)D20ActionSpecFunc::ModifyPicker, dispPyArgs);

	Py_DECREF(dispPyArgs);

	return;
}

ActionCostType PythonD20ActionIntegration::GetActionCostType(int actionEnum){
	return (ActionCostType)GetInt(actionEnum, D20ActionSpecFunc::GetActionCostType);
}

ActionErrorCode PythonD20ActionIntegration::PyAddToSeq(int actionEnum, D20Actn * d20a, ActnSeq * actSeq, TurnBasedStatus * tbStat){

	auto actionSpecEntry = mScripts.find(actionEnum);
	if (actionSpecEntry == mScripts.end()){
		if (d20a->d20APerformer){
			floatSys.floatMesLine(d20a->d20APerformer, 1, FloatLineColor::Red, fmt::format("Could not find script file {}", actionEnum).c_str());
		}
		return AEC_INVALID_ACTION;
	}
		

	py::object pbD20A = py::cast(d20a);
	py::object pbActSeq = py::cast(actSeq);
	py::object pbTbStat = py::cast(tbStat);

	
	auto dispPyArgs = Py_BuildValue("(OOO)", pbD20A.ptr(), pbActSeq.ptr(), pbTbStat.ptr());

	
	auto result = (ActionErrorCode)RunScriptDefault0(actionSpecEntry->second.id, (EventId)D20ActionSpecFunc::AddToSequence, dispPyArgs);

	Py_DECREF(dispPyArgs);

	return result;

}

BOOL PythonD20ActionIntegration::PyProjectileHit(int actionEnum, D20Actn * d20a, objHndl projectile, objHndl obj2)
{
	auto actionSpecEntry = mScripts.find(actionEnum);
	if (actionSpecEntry == mScripts.end())
		return FALSE;

	py::object pbD20A = py::cast(d20a);
	py::object pbActSeq = py::cast(projectile);
	py::object pbTbStat = py::cast(obj2);
	auto pyProjectile = PyObjHndl_Create(projectile);
	auto pyObj2 = PyObjHndl_Create(obj2);


	auto dispPyArgs = Py_BuildValue("(OOO)", pbD20A.ptr(), pyProjectile, pyObj2);

	Py_DECREF(pyProjectile);
	Py_DECREF(pyObj2);

	auto result = RunScriptDefault0(actionSpecEntry->second.id, (EventId)D20ActionSpecFunc::ProjectileHit, dispPyArgs);

	Py_DECREF(dispPyArgs);

	return result;
}


static std::map<D20ActionSpecFunc, std::string> D20ActionSpecFunctions = {

	{ D20ActionSpecFunc::GetActionDefinitionFlags,"GetActionDefinitionFlags"},
	{ D20ActionSpecFunc::GetActionName,"GetActionName" },
	{ D20ActionSpecFunc::GetTargetingClassification,"GetTargetingClassification" },
	{ D20ActionSpecFunc::GetActionCostType,"GetActionCostType" },
	{ D20ActionSpecFunc::AddToSequence,"AddToSequence" },
	{ D20ActionSpecFunc::ModifyPicker,"ModifyPicker" },
	{ D20ActionSpecFunc::ProjectileHit,"ProjectileHit" },
	
	
};

const char* PythonD20ActionIntegration::GetFunctionName(EventId evt) {
	auto _evt = (D20ActionSpecFunc)evt;
	return D20ActionSpecFunctions[_evt].c_str();
}
