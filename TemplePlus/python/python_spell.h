
#pragma once
#include <obj.h>

struct PySpell;
struct SpellPacketBody;
BOOL ConvertTargetArray(PyObject *obj, PySpell **pySpellOut);

PyObject *PySpell_Create(int spellId);
void PySpell_Update(int spellId);
void PySpell_UpdatePacket(PyObject *pySpell);
void PySpell_UpdateFromPacket(PySpell* self, const SpellPacketBody& spell);
objHndl PySpell_GetTargetHandle(PyObject *spell, int targetIdx);
