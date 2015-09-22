
#pragma once

class Dice;

extern PyTypeObject PyDiceType;
PyObject *PyDice_FromDice(const Dice &dice);

// Use with PyArg_ParseTuple and a O& placeholder
bool ConvertDice(PyObject *obj, Dice *pDiceOut);
