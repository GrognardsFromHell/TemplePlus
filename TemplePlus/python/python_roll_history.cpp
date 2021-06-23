#include "stdafx.h"
#include "common.h"
#include "pybind11/pybind11.h"
#include "pybind11/cast.h"
#include "python_object.h"
#include <pybind11/embed.h>
#include "../history.h"
#include "../damage.h"

namespace py = pybind11;

template <> class py::detail::type_caster<objHndl> {
public:
	bool load(handle src, bool) {
		value = PyObjHndl_AsObjHndl(src.ptr());
		success = true;
		return true;
	}

	static handle cast(const objHndl& src, return_value_policy /* policy */, handle /* parent */) {
		return PyObjHndl_Create(src);
	}

	PYBIND11_TYPE_CASTER(objHndl, _("objHndl"));
protected:
	bool success = false;
};


PYBIND11_EMBEDDED_MODULE(roll_history, m) {
	
	m.def("add_from_pattern", [](int historyMesline, objHndl handle, objHndl handle2) {
		histSys.CreateRollHistoryLineFromMesfile(historyMesline, handle, handle2);
		});

	m.def("add_damage_roll", [](objHndl attacker, objHndl tgt, DamagePacket& dmg) ->int{
		auto id = histSys.RollHistoryAddType1DamageRoll(attacker, tgt, &dmg);
		return id;
		});

	m.def("add_percent_chance_roll", [](objHndl performer, objHndl tgt, int failChance, int combatMesTitle, int rollResult, int combatMeslineResultText, int combatMeslineCheckType ) {
		auto id = histSys.RollHistoryAddType5PercentChanceRoll(performer, tgt, failChance, combatMesTitle, rollResult, combatMeslineResultText, combatMeslineCheckType);
		return id;
		});
}