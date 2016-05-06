#pragma once

class DamagePacket;

extern PyTypeObject PyDamagePacketType;
PyObject *PyDamagePacket_FromDamagePacket(const DamagePacket & dam);

// Use with PyArg_ParseTuple and a O& placeholder
//bool ConvertDamagePacket(PyObject *obj, DamagePacket **pDamOut);