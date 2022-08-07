
#pragma once
#include <obj.h>

struct PySpell;
struct PySpellStore;
struct SpellPacketBody;
void PySpell_Reset();
BOOL ConvertTargetArray(PyObject *obj, PySpell **pySpellOut);

PyObject *PySpell_Create(int spellId);
void PySpell_Update(int spellId);
void PySpell_UpdatePacket(PyObject *pySpell);
void PySpell_UpdateFromPacket(PySpell* self, const SpellPacketBody& spell);
objHndl PySpell_GetTargetHandle(PyObject *spell, int targetIdx);

// PySpellStore
extern PyTypeObject PySpellStoreType;
BOOL ConvertSpellStore(PyObject *obj, SpellStoreData *pSpellStoreOut);
PyObject *PySpellStore_Create(const SpellStoreData &spellId);
SpellStoreData PySpellStore_AsSpellStore(PyObject *obj);