
#include "stdafx.h"
#include "python_time.h"
#include <temple/dll.h>
#include "gamesystems/timeevents.h"
#include <gamesystems/gamesystems.h>

static PyTypeObject *GetPyTimeStampType();
bool PyTimeStampObject_Check(PyObject* obj);
static int PyTimeStamp_Cmp(PyObject* objA, PyObject* objB);

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


static PyTimeStampObject* GetSelf(PyObject* obj) {
	assert(PyTimeStampObject_Check(obj));
	auto self = (PyTimeStampObject*)obj;
	return self;
}

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

static PyObject* PyTimeStamp_GetDeltaMs(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	
	PyTimeStampObject* ts;
	if (!PyArg_ParseTuple(args, "O!", GetPyTimeStampType(), &ts))
		return nullptr;
	
	auto diffTime = self->time - ts->time;

	return PyInt_FromLong(diffTime.ToMs());
}

static PyObject* PyTimeStamp_AddMs(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);

	int addMs = 0;
	if (!PyArg_ParseTuple(args, "i", &addMs)) {
		return nullptr;
	}
	if (addMs < 0)
		addMs = 0;

	self->time += GameTime(0,addMs);

	Py_RETURN_NONE;
}



static PyMethodDef PyTimeStampMethods[] {
	{ "time_elapsed", PyTimeStamp_GetElapsed, METH_VARARGS, "GameTime_Elapsed (the gameplay time that's frozen while in combat) - this time stamp" },
	{ "time_in_game_in_seconds", PyTimeStamp_GetInGameSeconds, METH_VARARGS, "GameTime_Elapsed in seconds" },
	{ "time_in_game_in_days", PyTimeStamp_GetInGameDays, METH_VARARGS, "Likewise but in days" },
	{ "time_game_in_seconds_elapsed", PyTimeStamp_GetSecondsFloat, METH_VARARGS, "Timestamp in seconds (is offset by starting day of year)" },
	{ "time_game_in_seconds", PyTimeStamp_GetSeconds, METH_VARARGS, "Timestamp in seconds - integer (is offset by starting day of year)" },
	{ "time_game_in_seconds_float", NULL, METH_VARARGS, NULL },
	{ "time_game_in_hours", PyTimeStamp_GetHours, METH_VARARGS, "Timestamp in hours - integer (it is hour of day actually)" },
	{ "time_game_in_hours2", PyTimeStamp_GetHours2, METH_VARARGS, "Timestamp in hours - integer (inc. hours from start of year i.e. + 24*day of year)" },
	{ "time_game_in_minutes", PyTimeStamp_GetMinutes, METH_VARARGS, NULL },
	{ "time_game_in_months", PyTimeStamp_GetMonth, METH_VARARGS, NULL },
	{ "time_game_in_days", PyTimeStamp_GetDays, METH_VARARGS, NULL },
	{ "time_game_in_years", PyTimeStamp_GetYear, METH_VARARGS, NULL },
	{ "delta_ms", PyTimeStamp_GetDeltaMs, METH_VARARGS, "Delta time, in msec"},
	{ "add_ms", PyTimeStamp_AddMs, METH_VARARGS, "Add X ms to timestamp"},
	{ NULL, NULL, NULL, NULL }
};



static PyObject* PyTimeStamp_Repr(PyObject* obj) {
	auto self = GetSelf(obj);
	auto displayName = format("Timestamp ({} days, {} ms)", self->time.timeInDays, self->time.timeInMs);
	return PyString_FromString(displayName.c_str());
}

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
	PyTimeStamp_Cmp,           /*tp_compare*/
	PyTimeStamp_Repr,          /*tp_repr*/
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

static int PyTimeStamp_Cmp(PyObject* objA, PyObject* objB) {
	GameTime handleA(0,0);
	GameTime handleB(0,0);
	if (objA->ob_type == &PyTimeStampType) {
		handleA = ((PyTimeStampObject*)objA)->time;
	}
	if (objB->ob_type == &PyTimeStampType) {
		handleB = ((PyTimeStampObject*)objB)->time;
	}

	if (handleA.timeInDays < handleB.timeInDays) {
		return -1;
	}
	else if (handleA.timeInDays > handleB.timeInDays) {
		return 1;
	}

	// timeInDays is equal
	if (handleA.timeInMs < handleB.timeInMs) {
		return -1;
	}
	else if (handleA.timeInMs > handleB.timeInMs) {
		return 1;
	}

	return 0;
}

static PyTypeObject *GetPyTimeStampType() {
	return &PyTimeStampType;
}

bool PyTimeStampObject_Check(PyObject* obj) {
	return obj->ob_type == &PyTimeStampType;
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
