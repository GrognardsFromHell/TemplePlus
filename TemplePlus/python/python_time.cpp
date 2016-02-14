
#include "stdafx.h"
#include "python_time.h"
#include <temple/dll.h>
#include "gamesystems/timeevents.h"
#include <gamesystems/gamesystems.h>

static PyTypeObject *GetPyTimeStampType();

static struct PyTimeStampAddresses : temple::AddressTable {

	// Returns the number of years (respecting the module starting year) in the given game time
	int (__cdecl *GameTimeGetYear)(const GameTime *gameTime);

	// Returns the day of year (respecting the module starting day) in the given game time
	int (__cdecl *GameTimeGetDay)(const GameTime *gameTime);

	// Returns the month of year (respecting the module starting month) in the given game time
	int (__cdecl *GameTimeGetMonth)(const GameTime *gameTime);
	
	// Returns the minutes of the day in the given game time
	int (__cdecl *GameTimeGetMinutes)(const GameTime *gameTime);
	
	// Returns the hour of the day in the given game time
	int (__cdecl *GameTimeGetHours)(const GameTime *gameTime);

	// Returns the overall hours that elapsed
	int (__cdecl *GameTimeGetHours2)(const GameTime *gameTime);

	float (__cdecl *GameTimeInSecondsFloat)(const GameTime *gameTime);

	int (__cdecl *GameTimeInSeconds)(const GameTime *gameTime);

	int (__cdecl *GameInSecondsElapsed)(const GameTime *gameTime);

	int (__cdecl *GameInDays)();

	int (__cdecl *GameInSeconds)();
	
	uint64_t (__cdecl *GameElapsed)(const GameTime *gameTime);

	PyTimeStampAddresses() {
		rebase(GameElapsed, 0x1005FCA0);
		rebase(GameInSeconds, 0x100612B0);
		rebase(GameInDays, 0x1005FDE0);
		rebase(GameInSecondsElapsed, 0x100612E0);
		rebase(GameTimeInSeconds, 0x1005FD50);
		rebase(GameTimeInSecondsFloat, 0x1005FD10);
		rebase(GameTimeGetHours, 0x1005FD80);
		rebase(GameTimeGetHours2, 0x1005FDA0);
		rebase(GameTimeGetMinutes, 0x1005FDC0);
		rebase(GameTimeGetDay, 0x1005FDF0);
		rebase(GameTimeGetMonth, 0x1005FE10);
		rebase(GameTimeGetYear, 0x1005FE40);
	}
} pyTimeStampAddresses;

struct PyTimeStampObject {
	PyObject_HEAD;
	GameTime time;
};

// Generalized function that returns a transformed integer of the time stamps time
#define PYTIMESTAMP_GET_TRANSFORMED(NAME, FUNC) PyObject* PyTimeStamp_ ## NAME(PyObject *, PyObject *args) { \
	PyTimeStampObject *ts; \
	if (!PyArg_ParseTuple(args, "O!", GetPyTimeStampType(), &ts)) \
		return nullptr; \
\
	return PyInt_FromLong(FUNC(&ts->time));\
}

PYTIMESTAMP_GET_TRANSFORMED(GetYear, pyTimeStampAddresses.GameTimeGetYear);
PYTIMESTAMP_GET_TRANSFORMED(GetMonth, pyTimeStampAddresses.GameTimeGetMonth);
PYTIMESTAMP_GET_TRANSFORMED(GetDays, pyTimeStampAddresses.GameTimeGetDay);
PYTIMESTAMP_GET_TRANSFORMED(GetMinutes, pyTimeStampAddresses.GameTimeGetMinutes);
PYTIMESTAMP_GET_TRANSFORMED(GetHours, pyTimeStampAddresses.GameTimeGetHours);
PYTIMESTAMP_GET_TRANSFORMED(GetHours2, pyTimeStampAddresses.GameTimeGetHours2);
PYTIMESTAMP_GET_TRANSFORMED(GetSeconds, pyTimeStampAddresses.GameTimeInSeconds);

static PyObject *PyTimeStamp_GetSecondsFloat(PyObject*, PyObject *args) {
	PyTimeStampObject *ts;
	if (!PyArg_ParseTuple(args, "O!", GetPyTimeStampType(), &ts))
		return nullptr;
	return PyFloat_FromDouble(pyTimeStampAddresses.GameTimeInSecondsFloat(&ts->time));
}

static PyObject *PyTimeStamp_GetElapsed(PyObject *obj, PyObject *args) {
	PyTimeStampObject *ts;
	if (!PyArg_ParseTuple(args, "O!", GetPyTimeStampType(), &ts))
		return nullptr;
	
	return PyTimeStamp_Create(pyTimeStampAddresses.GameElapsed(&ts->time));
}

static PyObject *PyTimeStamp_GetInGameSeconds(PyObject*, PyObject*) {
	return PyInt_FromLong(pyTimeStampAddresses.GameInSeconds());
}

static PyObject *PyTimeStamp_GetInGameDays(PyObject*, PyObject*) {
	return PyInt_FromLong(pyTimeStampAddresses.GameInDays());
}

static PyMethodDef PyTimeStampMethods[] {
	{ "time_elapsed", PyTimeStamp_GetElapsed, METH_VARARGS, NULL },
	{ "time_in_game_in_seconds", PyTimeStamp_GetInGameSeconds, METH_VARARGS, NULL },
	{ "time_in_game_in_days", PyTimeStamp_GetInGameDays, METH_VARARGS, NULL },
	{ "time_game_in_seconds_elapsed", PyTimeStamp_GetSecondsFloat, METH_VARARGS, NULL },
	{ "time_game_in_seconds", PyTimeStamp_GetSeconds, METH_VARARGS, NULL },
	{ "time_game_in_seconds_float", NULL, METH_VARARGS, NULL },
	{ "time_game_in_hours", PyTimeStamp_GetHours, METH_VARARGS, NULL },
	{ "time_game_in_hours2", PyTimeStamp_GetHours2, METH_VARARGS, NULL },
	{ "time_game_in_minutes", PyTimeStamp_GetMinutes, METH_VARARGS, NULL },
	{ "time_game_in_months", PyTimeStamp_GetMonth, METH_VARARGS, NULL },
	{ "time_game_in_days", PyTimeStamp_GetDays, METH_VARARGS, NULL },
	{ "time_game_in_years", PyTimeStamp_GetYear, METH_VARARGS, NULL },
	{ NULL, NULL, NULL, NULL }
};

static PyTypeObject PyTimeStampType = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"PyTimeStamp",             /*tp_name*/
	sizeof(PyTimeStampObject), /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor) PyObject_Del, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	PyObject_GenericGetAttr,   /*tp_getattro*/
	PyObject_GenericSetAttr,   /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,        /*tp_flags*/
	0,			               /* tp_doc */
	0,						   /* tp_traverse */
	0,						   /* tp_clear */
	0,						   /* tp_richcompare */
	0,						   /* tp_weaklistoffset */
	0,						   /* tp_iter */
	0,						   /* tp_iternext */
	PyTimeStampMethods,        /* tp_methods */
	0,                         /* tp_members */
	0,						   /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	0,						   /* tp_init */
	0,                         /* tp_alloc */
	0,						   /* tp_new */
};

static PyTypeObject *GetPyTimeStampType() {
	return &PyTimeStampType;
}

PyObject* PyTimeStamp_Create() {
	auto obj = PyObject_New(PyTimeStampObject, &PyTimeStampType);	
	obj->time = gameSystems->GetTimeEvent().GetTime();
	return (PyObject*)obj;
}

PyObject *PyTimeStamp_Create(const GameTime &gameTime) {
	auto obj = PyObject_New(PyTimeStampObject, &PyTimeStampType);
	obj->time = gameTime;
	return (PyObject*) obj;
}
