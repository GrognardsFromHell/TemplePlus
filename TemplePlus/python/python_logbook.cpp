#include "stdafx.h"
#include "common.h"
#include "pybind11/pybind11.h"
#include "pybind11/cast.h"
#include "python_object.h"
#include <pybind11/embed.h>
#include "ui/ui_logbook.h"

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


PYBIND11_EMBEDDED_MODULE(logbook, m) {
	m.def("inc_hits", [](objHndl handle)->void { // logbook consecutive hits
		uiLogbook.IncreaseHits(handle);
		});
	m.def("inc_misses", [](objHndl handle) { // logbook consecutive misse
		uiLogbook.IncreaseMisses(handle);
		});
	m.def("inc_criticals", [](objHndl handle) {
			uiLogbook.IncreaseCritHits(handle);
		});
	m.def("record_highest_damage", [](BOOL isWeaponDamage, int damTot, objHndl attacker, objHndl tgt) {
		uiLogbook.RecordHighestDamage(isWeaponDamage, damTot, attacker, tgt);
		});

	m.attr("is_weapon_damage") = temple::GetRef<BOOL>(0x10BCA8AC);
	
}