#pragma once

struct Dispatcher;
struct CondStructNew;
struct CondNode;

extern PyTypeObject PyModifierSpecType;
extern PyTypeObject PyModifierType;
extern PyTypeObject PyDispatchEventObjectType;
extern PyTypeObject PyEventArgsType;

PyObject *PyModifierSpec_FromCondStruct(const CondStructNew & cond);
int PyModHookWrapper(DispatcherCallbackArgs args);

