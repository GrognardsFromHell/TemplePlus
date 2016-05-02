#pragma once

class BonusList;

extern PyTypeObject PyBonusListType;
PyObject *PyBonusList_FromBonusList(const BonusList & bonlist);

// Use with PyArg_ParseTuple and a O& placeholder
bool ConvertBonusList(PyObject *obj, BonusList *pDiceOut);