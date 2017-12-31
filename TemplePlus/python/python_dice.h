
#pragma once

#include <pybind11/pybind11.h>

class Dice;

extern PyTypeObject PyDiceType;
PyObject *PyDice_FromDice(const Dice &dice);

// Use with PyArg_ParseTuple and a O& placeholder
bool ConvertDice(PyObject *obj, Dice *pDiceOut);

void init_dice_class(pybind11::module &module);
