
#include "stdafx.h"
#include "python_game.h"
#include "python_object.h"
#include "python_dice.h"
#include "python_bonus.h"
#include "python_damage.h"
#include "python_dispatcher.h"
#include "python_spell.h"

#include <pybind11/embed.h>

namespace py = pybind11;
using namespace pybind11::literals;

PYBIND11_EMBEDDED_MODULE(toee, m) {

	m.def("anyone", [](const py::iterable &target_objs, const char *method_name, const py::object &method_arg) {
		for (auto &obj : target_objs) {
			auto method = obj.attr(method_name);
			auto result = method.call(method_arg);
			if (result.cast<bool>()) {
				return true;
			}
		}
		return false;
	}, "target_objs"_a, "method_name"_a, "method_arg"_a, R"(
	Calls the method 'method_name' on each object in the target_objs list with the given method_arg and 
    returns true as soon as the method returns true for any of the objects.

    Example:
    toee.anyone(game.party, "has_follower", 123)
)");

	init_dice_class(m);
	init_objhndl_class(m);

	auto module = Py_InitModule("toee", nullptr); // Borrowed ref
	auto dict = PyModule_GetDict(module); // Borrowed ref

	// The null object (pointless by the way, since it implements IsTrue)
	/*auto nullHandle = PyObjHndl_CreateNull();
	PyDict_SetItemString(dict, "OBJ_HANDLE_NULL", nullHandle);
	Py_DECREF(nullHandle);*/

	// The game object, which behaves more like a module, but has getter/setter based properties
	auto pyGame = PyGame_Create(); // New ref
	PyDict_SetItemString(dict, "game", pyGame);
	Py_DECREF(pyGame);

	if (PyType_Ready(&PySpellStoreType)) {
		PyErr_Print();
	}
	if (PyType_Ready(&PyBonusListType)) {
		PyErr_Print();
	}
	if (PyType_Ready(&PyDamagePacketType)) {
		PyErr_Print();
	}
	if (PyType_Ready(&PyModifierSpecType)) {
		PyErr_Print();
	}

	// This is critical for unpickling object handles stored in timed events
	// PyDict_SetItemString(dict, "PyObjHandle", (PyObject*)&PyObjHandleType);
	PyDict_SetItemString(dict, "PySpellStore", (PyObject*)&PySpellStoreType);
	PyDict_SetItemString(dict, "PyBonusList", (PyObject*)&PyBonusListType);
	PyDict_SetItemString(dict, "PyDamagePacket", (PyObject*)&PyDamagePacketType);
	PyDict_SetItemString(dict, "PyModifierSpec", (PyObject*)&PyModifierSpecType);

	// Copy all constants into toee for legacy support
	auto constantsMod = PyImport_ImportModule("templeplus.constants"); // New ref
	if (!constantsMod) {
		throw TempleException("TemplePlus Python module templeplus.constants is missing.");
	}
	PyDict_Merge(dict, PyModule_GetDict(constantsMod), 0);
	Py_DECREF(constantsMod);

}
