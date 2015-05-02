
#pragma once
#include <obj.h>

struct PySpell;
BOOL ConvertTargetArray(PyObject *obj, PySpell **pySpellOut);

PyObject *PySpell_Create(int spellId);
void PySpell_Update(int spellId);
void PySpell_UpdatePacket(PyObject *pySpell);
objHndl PySpell_GetTargetHandle(PyObject *spell, int targetIdx);
