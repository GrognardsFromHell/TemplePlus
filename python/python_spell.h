
#pragma once

PyObject *PySpell_Create(int spellId);
void PySpell_Update(int spellId);
void PySpell_UpdatePacket(PyObject *pySpell);

