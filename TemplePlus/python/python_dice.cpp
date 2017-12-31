
#include "stdafx.h"
#include "python_dice.h"
#include "../dice.h"

namespace py = pybind11;
using namespace pybind11::literals;

PyObject* PyDice_FromDice(const Dice& dice) {
	auto dice_obj = py::cast(dice);
	dice_obj.inc_ref();
	return dice_obj.ptr();
}

bool ConvertDice(PyObject* obj, Dice* pDiceOut) {
	try {
		auto dice = py::cast<Dice>(obj);
		*pDiceOut = dice;
		return true;
	} catch (py::cast_error&) {
		return false;
	}
}

void init_dice_class(pybind11::module &m)
{

	auto dice_class = py::class_<Dice>(m, "Dice", R"(Represents a dice roll template (i.e. 2d6+5)")
		
		.def(py::init<int, int, int>(), "dice"_a, "sides"_a, "bonus"_a=0,
			R"(Creates a dice object that represents a dice roll with 'dice' (the number of dice to roll), 'sides' (the number of sides that the dice have), and an optional bonus that is added to the dice roll result.)")
		
		.def(py::init([](const char *diceStr){
			int count, sides, modifier = 0;
			Dice::Parse(diceStr, count, sides, modifier);
			return std::make_unique<Dice>(count, sides, modifier);
		}), "dice_spec"_a, R"(Creates a new dice object from a standard D&D dice specification. Such as: 2d6+2 or 2d6-1 or 1d4)")
		
		.def_property("number", &Dice::GetCount, &Dice::SetCount, R"(The number of dice to roll.)")
		.def_property("size", &Dice::GetSides, &Dice::SetSides, R"(The type of dice to roll, expressed as it's sides.)")
		.def_property("bonus", &Dice::GetModifier, &Dice::SetModifier, R"(The bonus to add to the final result. Can be negative.)")
		
		// because a few spell scripts use this, guess it's accepted because originally it did an _strnicmp using the input length
		.def_property("num", &Dice::GetCount, &Dice::SetCount, R"(Deprecated and only provided for backwards compatibility with vanilla scripts.)")

		.def("roll", [](const Dice &dice) {
			return dice.Roll();
		}, R"(Rolls the dice and returns the result of the roll.)")
		
		.def("clone", [](const Dice &dice) {
			return dice;
		}, R"(Returns an independent copy of this dice object.)")

		.def("__repr__", &Dice::ToString);

	// Scripts usually create dice objects using the dice_new alias.
	m.attr("dice_new") = dice_class;

}
