#include "stdafx.h"
#include "python_object.h"
#include "python_support.h"
#include "../maps.h"
#include "../inventory.h"
#include "../timeevents.h"
#include "python_dice.h"
#include "python_objectscripts.h"

static struct PyObjHandleAddresses : AddressTable {
	PyObjHandleAddresses() {
	}
} addresses;

struct PyObjHandle {
	PyObject_HEAD;
	ObjectId id;
	objHndl handle;
};

static PyObjHandle* GetSelf(PyObject* self) {
	// TODO refresh handle from guid if necessary
	return (PyObjHandle*)self;
}

static PyObject* PyObjHandle_Repr(PyObject* obj) {
	auto self = (PyObjHandle*) obj;

	if (!self->handle) {
		return PyString_FromString("OBJ_HANDLE_NULL");
	} else {
		auto name = objects.GetDisplayName(self->handle, self->handle);		
		auto displayName = format("{}({})", name, self->handle);
		
		return PyString_FromString(displayName.c_str());
	}
}

static int PyObjHandle_Cmp(PyObject* objA, PyObject* objB) {
	objHndl handleA = 0;
	objHndl handleB = 0;
	if (objA->ob_type == &PyObjHandleType) {
		handleA = ((PyObjHandle*)objA)->handle;
	}
	if (objB->ob_type == &PyObjHandleType) {
		handleB = ((PyObjHandle*)objB)->handle;
	}
	
	if (handleA < handleB) {
		return -1;
	} else if (handleA > handleB) {
		return 1;
	} else {
		return 0;
	}
}

#pragma region Pickle Protocol 

// Only the object GUID is pickled
// The format has to be compatible with old ToEE to be save compatible
static PyObject* PyObjHandle_getstate(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);
	// Mobile GUID
	if (self->id.subtype == 2) {
		const auto &guid = self->id.guid;
		return Py_BuildValue("i(iii(iiiiiiii))",
			2,
			guid.Data1,
			guid.Data2,
			guid.Data3,
			guid.Data4[0],
			guid.Data4[1],
			guid.Data4[2],
			guid.Data4[3],
			guid.Data4[4],
			guid.Data4[5],
			guid.Data4[6],
			guid.Data4[7]);
	}
	return Py_BuildValue("(i)", self->id.subtype);
}

static PyObject* PyObjHandle_reduce(PyObject* obj, PyObject* args) {
	auto constructor = (PyObject*) &PyObjHandleType;
	auto unpickleArgs = PyTuple_New(0);
	auto pickledValue = PyObjHandle_getstate(obj, args);

	auto result = Py_BuildValue("OOO", constructor, unpickleArgs, pickledValue);
	Py_DECREF(unpickleArgs);
	Py_DECREF(pickledValue);
	return result;
}

static PyObject* PyObjHandle_setstate(PyObject* obj, PyObject* args) {
	auto self = (PyObjHandle*) obj;
	PyObject *pickledData;

	// Expected is one argument, which is the tuple we returned from __getstate__
	if (!PyArg_ParseTuple(args, "O!:PyObjHandle.__setstate__", &PyTuple_Type, &pickledData)) {
		return 0;
	}

	PyObject *guidContent = 0;
	if (!PyArg_ParseTuple(pickledData, "i|O:PyObjHandle.__setstate__", &self->id.subtype, &guidContent)) {
		self->handle = 0;
		self->id.subtype = 0;
		return 0;
	}

	if (self->id.subtype == 0) {
		// This is the null obj handle
		self->handle = 0;
		Py_RETURN_NONE;
	}

	if (guidContent == 0) {
		PyErr_SetString(PyExc_ValueError, "GUID type other than 0 is given, but GUID is missing.");
		self->handle = 0;
		self->id.subtype = 0;
		return 0;
	}

	// Try parsing the GUID tuple
	auto &guid = self->id.guid;
	if (!PyArg_ParseTuple(guidContent, "iii(iiiiiiii)", &guid.Data1,
		&guid.Data2,
		&guid.Data3,
		&guid.Data4[0],
		&guid.Data4[1],
		&guid.Data4[2],
		&guid.Data4[3],
		&guid.Data4[4],
		&guid.Data4[5],
		&guid.Data4[6],
		&guid.Data4[7])) {
		self->handle = 0;
		self->id.subtype = 0;
		return 0;
	}

	// Finally look up the handle for the GUID
	self->handle = objects.GetHandle(self->id);

	Py_RETURN_NONE;
}

#pragma endregion

#pragma region Methods

/*
	Schedules a Python dialog time event in 1ms. Probably so the rest of the calling
	script executes before the Python dialog UI is created.
	self: PC
	target: NPC
	line: The initial line in the NPCs dialog to show
*/
static PyObject* PyObjHandle_BeginDialog(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);

	objHndl target;
	int line;
	if (!PyArg_ParseTuple(args, "O&i:begin_dialog", &ConvertObjHndl, &target, &line)) {
		return 0;
	}
	
	TimeEvent evt;
	evt.system = TimeEventSystem::PythonDialog;
	evt.params[0].handle = self->handle;
	evt.params[1].handle = target;
	evt.params[2].int32 = line;
	timeEvents.Schedule(evt, 1);

	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_ReactionGet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);

	objHndl towards;
	if (!PyArg_ParseTuple(args, "O&:reaction_get", &ConvertObjHndl, &towards)) {
		return 0;
	}

	auto reaction = objects.GetReaction(self->handle, towards);
	return PyInt_FromLong(reaction);
}

static PyObject* PyObjHandle_ReactionSet(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);

	objHndl towards;
	int desiredReaction;
	if (!PyArg_ParseTuple(args, "O&i:reaction_set", &ConvertObjHndl, &towards, &desiredReaction)) {
		return 0;
	}

	auto currentReaction = objects.GetReaction(self->handle, towards);
	auto adjustment = desiredReaction - currentReaction;
	objects.AdjustReaction(self->handle, towards, adjustment);

	Py_RETURN_NONE;
}

static PyObject* PyObjHandle_ReactionAdjust(PyObject* obj, PyObject* args) {
	auto self = GetSelf(obj);

	objHndl towards;
	int adjustment;
	if (!PyArg_ParseTuple(args, "O&i:reaction_adjust", &ConvertObjHndl, &towards, &adjustment)) {
		return 0;
	}

	objects.AdjustReaction(self->handle, towards, adjustment);
	Py_RETURN_NONE;
}

static PyMethodDef PyObjHandleMethods[] = {
	{ "__getstate__", PyObjHandle_getstate, METH_VARARGS, NULL },
	{ "__reduce__", PyObjHandle_reduce, METH_VARARGS, NULL },
	{ "__setstate__", PyObjHandle_setstate, METH_VARARGS, NULL },
	{ "begin_dialog", PyObjHandle_BeginDialog, METH_VARARGS, NULL },
	{ "reaction_get", PyObjHandle_ReactionGet, METH_VARARGS, NULL },
	{ "reaction_set", PyObjHandle_ReactionSet, METH_VARARGS, NULL },
	{ "reaction_adj", PyObjHandle_ReactionAdjust, METH_VARARGS, NULL },
	{ "item_find", NULL, METH_VARARGS, NULL },
	{ "item_transfer_to", NULL, METH_VARARGS, NULL },
	{ "item_find_by_proto", NULL, METH_VARARGS, NULL },
	{ "item_transfer_to_by_proto", NULL, METH_VARARGS, NULL },
	{ "money_get", NULL, METH_VARARGS, NULL },
	{ "money_adj", NULL, METH_VARARGS, NULL },
	{ "cast_spell", NULL, METH_VARARGS, NULL },
	{ "skill_level_get", NULL, METH_VARARGS, NULL },
	{ "has_met", NULL, METH_VARARGS, NULL },
	{ "hasMet", NULL, METH_VARARGS, NULL },
	{ "has_follower", NULL, METH_VARARGS, NULL },
	{ "group_list", NULL, METH_VARARGS, NULL },
	{ "stat_level_get", NULL, METH_VARARGS, NULL },
	{ "stat_base_get", NULL, METH_VARARGS, NULL },
	{ "stat_base_set", NULL, METH_VARARGS, NULL },
	{ "follower_add", NULL, METH_VARARGS, NULL },
	{ "follower_remove", NULL, METH_VARARGS, NULL },
	{ "follower_atmax", NULL, METH_VARARGS, NULL },
	{ "ai_follower_add", NULL, METH_VARARGS, NULL },
	{ "ai_follower_remove", NULL, METH_VARARGS, NULL },
	{ "ai_follower_atmax", NULL, METH_VARARGS, NULL },
	{ "leader_get", NULL, METH_VARARGS, NULL },
	{ "can_see", NULL, METH_VARARGS, NULL },
	{ "has_wielded", NULL, METH_VARARGS, NULL },
	{ "has_item", NULL, METH_VARARGS, NULL },
	{ "item_worn_at", NULL, METH_VARARGS, NULL },
	{ "attack", NULL, METH_VARARGS, NULL },
	{ "turn_towards", NULL, METH_VARARGS, NULL },
	{ "float_line", NULL, METH_VARARGS, NULL },
	{ "damage", NULL, METH_VARARGS, NULL },
	{ "damage_with_reduction", NULL, METH_VARARGS, NULL },
	{ "heal", NULL, METH_VARARGS, NULL },
	{ "healsubdual", NULL, METH_VARARGS, NULL },
	{ "steal_from", NULL, METH_VARARGS, NULL },
	{ "reputation_has", NULL, METH_VARARGS, NULL },
	{ "reputation_add", NULL, METH_VARARGS, NULL },
	{ "reputation_remove", NULL, METH_VARARGS, NULL },
	{ "item_condition_add_with_args", NULL, METH_VARARGS, NULL },
	{ "condition_add_with_args", NULL, METH_VARARGS, NULL },
	{ "condition_add_with_args", NULL, METH_VARARGS, NULL },
	{ "condition_add", NULL, METH_VARARGS, NULL },
	{ "is_friendly", NULL, METH_VARARGS, NULL },
	{ "fade_to", NULL, METH_VARARGS, NULL },
	{ "move", NULL, METH_VARARGS, NULL },
	{ "float_mesfile_line", NULL, METH_VARARGS, NULL },
	{ "object_flags_get", NULL, METH_VARARGS, NULL },
	{ "object_flag_set", NULL, METH_VARARGS, NULL },
	{ "object_flag_unset", NULL, METH_VARARGS, NULL },
	{ "portal_flags_get", NULL, METH_VARARGS, NULL },
	{ "portal_flag_set", NULL, METH_VARARGS, NULL },
	{ "portal_flag_unset", NULL, METH_VARARGS, NULL },
	{ "container_flags_get", NULL, METH_VARARGS, NULL },
	{ "container_flag_set", NULL, METH_VARARGS, NULL },
	{ "container_flag_unset", NULL, METH_VARARGS, NULL },
	{ "portal_toggle_open", NULL, METH_VARARGS, NULL },
	{ "container_toggle_open", NULL, METH_VARARGS, NULL },
	{ "container_toggle_open", NULL, METH_VARARGS, NULL },
	{ "item_flags_get", NULL, METH_VARARGS, NULL },
	{ "item_flag_set", NULL, METH_VARARGS, NULL },
	{ "item_flag_unset", NULL, METH_VARARGS, NULL },
	{ "critter_flags_get", NULL, METH_VARARGS, NULL },
	{ "critter_flag_set", NULL, METH_VARARGS, NULL },
	{ "critter_flag_unset", NULL, METH_VARARGS, NULL },
	{ "npc_flags_get", NULL, METH_VARARGS, NULL },
	{ "npc_flag_set", NULL, METH_VARARGS, NULL },
	{ "npc_flag_unset", NULL, METH_VARARGS, NULL },
	{ "saving_throw", NULL, METH_VARARGS, NULL },
	{ "saving_throw_with_args", NULL, METH_VARARGS, NULL },
	{ "saving_throw_spell", NULL, METH_VARARGS, NULL },
	{ "reflex_save_and_damage", NULL, METH_VARARGS, NULL },
	{ "soundmap_critter", NULL, METH_VARARGS, NULL },
	{ "footstep", NULL, METH_VARARGS, NULL },
	{ "secretdoor_detect", NULL, METH_VARARGS, NULL },
	{ "has_spell_effects", NULL, METH_VARARGS, NULL },
	{ "critter_kill", NULL, METH_VARARGS, NULL },
	{ "critter_kill_by_effect", NULL, METH_VARARGS, NULL },
	{ "destroy", NULL, METH_VARARGS, NULL },
	{ "item_get", NULL, METH_VARARGS, NULL },
	{ "perform_touch_attack", NULL, METH_VARARGS, NULL },
	{ "add_to_initiative", NULL, METH_VARARGS, NULL },
	{ "remove_from_initiative", NULL, METH_VARARGS, NULL },
	{ "get_initiative", NULL, METH_VARARGS, NULL },
	{ "set_initiative", NULL, METH_VARARGS, NULL },
	{ "d20_query", NULL, METH_VARARGS, NULL },
	{ "d20_query_has_spell_condition", NULL, METH_VARARGS, NULL },
	{ "d20_query_with_data", NULL, METH_VARARGS, NULL },
	{ "d20_query_test_data", NULL, METH_VARARGS, NULL },
	{ "d20_query_get_data", NULL, METH_VARARGS, NULL },
	{ "critter_get_alignment", NULL, METH_VARARGS, NULL },
	{ "distance_to", NULL, METH_VARARGS, NULL },
	{ "anim_callback", NULL, METH_VARARGS, NULL },
	{ "anim_goal_interrupt", NULL, METH_VARARGS, NULL },
	{ "d20_status_init", NULL, METH_VARARGS, NULL },
	{ "object_script_execute", NULL, METH_VARARGS, NULL },
	{ "standpoint_set", NULL, METH_VARARGS, NULL },
	{ "runoff", NULL, METH_VARARGS, NULL },
	{ "get_category_type", NULL, METH_VARARGS, NULL },
	{ "is_category_type", NULL, METH_VARARGS, NULL },
	{ "is_category_subtype", NULL, METH_VARARGS, NULL },
	{ "obj_set_initiative", NULL, METH_VARARGS, NULL },
	{ "rumor_log_add", NULL, METH_VARARGS, NULL },
	{ "obj_set_int", NULL, METH_VARARGS, NULL },
	{ "obj_get_int", NULL, METH_VARARGS, NULL },
	{ "has_feat", NULL, METH_VARARGS, NULL },
	{ "spell_known_add", NULL, METH_VARARGS, NULL },
	{ "spell_memorized_add", NULL, METH_VARARGS, NULL },
	{ "spell_damage", NULL, METH_VARARGS, NULL },
	{ "spell_damage_with_reduction", NULL, METH_VARARGS, NULL },
	{ "spell_heal", NULL, METH_VARARGS, NULL },
	{ "identify_all", NULL, METH_VARARGS, NULL },
	{ "ai_flee_add", NULL, METH_VARARGS, NULL },
	{ "get_deity", NULL, METH_VARARGS, NULL },
	{ "item_wield_best_all", NULL, METH_VARARGS, NULL },
	{ "award_experience", NULL, METH_VARARGS, NULL },
	{ "has_los", NULL, METH_VARARGS, NULL },
	{ "has_atoned", NULL, METH_VARARGS, NULL },
	{ "sweep_for_cover", NULL, METH_VARARGS, NULL },
	{ "d20_send_signal", NULL, METH_VARARGS, NULL },
	{ "d20_send_signal_ex", NULL, METH_VARARGS, NULL },
	{ "balor_death", NULL, METH_VARARGS, NULL },
	{ "concealed_set", NULL, METH_VARARGS, NULL },
	{ "ai_shitlist_add", NULL, METH_VARARGS, NULL },
	{ "ai_shitlist_remove", NULL, METH_VARARGS, NULL },
	{ "unconceal", NULL, METH_VARARGS, NULL },
	{ "spells_pending_to_memorized", NULL, METH_VARARGS, NULL },
	{ "spells_memorized_forget", NULL, METH_VARARGS, NULL },
	{ "ai_stop_attacking", NULL, METH_VARARGS, NULL },
	{ "resurrect", NULL, METH_VARARGS, NULL },
	{ "dominate", NULL, METH_VARARGS, NULL },
	{ "is_unconscious", NULL, METH_VARARGS, NULL },
	{NULL, NULL, NULL, NULL}
};

#pragma endregion 

#pragma region Getters and Setters

static PyObject* PyObjHandle_GetNameId(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyInt_FromLong(objects.GetNameId(self->handle));
}

static PyObject* PyObjHandle_GetLocation(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyLong_FromLongLong(objects.GetLocation(self->handle));
}

static PyObject* PyObjHandle_GetType(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyInt_FromLong(objects.GetType(self->handle));
}

static PyObject* PyObjHandle_GetRadius(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyFloat_FromDouble(objects.GetRadius(self->handle));
}

static int PyObjHandle_SetRadius(PyObject* obj, PyObject* value, void*) {
	auto self = GetSelf(obj);
	float radius;
	if (!GetFloatLenient(value, radius)) {
		return -1;
	}

	// I think without setting the OBJ_F radius_set this pretty much does nothing
	objects.SetRadius(self->handle, radius);
	return 0;
}

static PyObject* PyObjHandle_GetRenderHeight(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyFloat_FromDouble(objects.GetRenderHeight(self->handle));
}

static int PyObjHandle_SetRenderHeight(PyObject* obj, PyObject* value, void*) {
	auto self = GetSelf(obj);
	float renderHeight;
	if (!GetFloatLenient(value, renderHeight)) {
		return -1;
	}

	objects.SetRenderHeight(self->handle, renderHeight);
	return 0;
}

static PyObject* PyObjHandle_GetRotation(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyFloat_FromDouble(objects.GetRotation(self->handle));
}

static int PyObjHandle_SetRotation(PyObject* obj, PyObject* value, void*) {
	auto self = GetSelf(obj);
	float rotation;
	if (!GetFloatLenient(value, rotation)) {
		return -1;
	}
	objects.SetRotation(self->handle, rotation);
	return 0;
}

static PyObject* PyObjHandle_GetHitDice(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyDice_FromDice(objects.GetHitDice(self->handle));
}

static PyObject* PyObjHandle_GetHitDiceNum(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyInt_FromLong(objects.GetHitDiceNum(self->handle));
}

static PyObject* PyObjHandle_GetSize(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyInt_FromLong(objects.GetSize(self->handle));
}

static PyObject* PyObjHandle_GetOffsetX(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyFloat_FromDouble(objects.GetOffsetX(self->handle));
}

static PyObject* PyObjHandle_GetOffsetY(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyFloat_FromDouble(objects.GetOffsetY(self->handle));
}

static PyObject* PyObjHandle_GetScripts(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyObjScripts_Create(self->handle);
}

static PyObject* PyObjHandle_GetOriginMapId(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	return PyInt_FromLong(objects.GetOriginMapId(self->handle));
}

static int PyObjHandle_SetOriginMapId(PyObject* obj, PyObject* value, void*) {
	auto self = GetSelf(obj);
	int mapId;
	if (!GetInt(value, mapId)) {
		return -1;
	}
	if (!maps.IsValidMapId(mapId)) {
		auto msg = format("Map id {} is invalid.", mapId);
		PyErr_SetString(PyExc_ValueError, msg.c_str());
		return -1;
	}
	objects.SetOriginMapId(self->handle, mapId);
	return 0;
}

static PyObject* PyObjHandle_GetSubstituteInventory(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	auto handle = inventory.GetSubstituteInventory(self->handle);
	return PyObjHndl_Create(handle);
}

static PyObject* PyObjHandle_GetFeats(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	auto feats = objects.feats.GetFeats(self->handle);
	auto result = PyTuple_New(feats.size());
	for (size_t i = 0; i < feats.size(); ++i) {
		PyTuple_SET_ITEM(result, i, PyInt_FromLong(feats[i]));
	}
	return result;
}

// This is the NPC looting behaviour
static PyObject* PyObjHandle_GetLoots(PyObject* obj, void*) {
	auto self = GetSelf(obj);
	auto loots = objects.GetInt32(self->handle, obj_f_npc_pad_i_3);
	return PyInt_FromLong(loots & 0xF);
}

static int PyObjHandle_SetLoots(PyObject* obj, PyObject* value, void*) {
	auto self = GetSelf(obj);
	int loots;
	if (!GetInt(value, loots)) {
		return -1;
	}
	objects.SetInt32(self->handle, obj_f_npc_pad_i_3, loots);
	return 0;
}

static PyObject* PyObjHandle_SafeForUnpickling(PyObject*, void*) {
	Py_RETURN_TRUE;
}

static PyGetSetDef PyObjHandleGetSets[] = {
	PY_INT_PROP_RO("area", maps.GetCurrentMapId, NULL),
	{"name", PyObjHandle_GetNameId, NULL, NULL},
	{"location", PyObjHandle_GetLocation, NULL, NULL},
	{"type", PyObjHandle_GetType, NULL, NULL},
	{"radius", PyObjHandle_GetRadius, PyObjHandle_SetRadius, NULL},
	{"height", PyObjHandle_GetRenderHeight, PyObjHandle_SetRenderHeight, NULL},
	{"rotation", PyObjHandle_GetRotation, PyObjHandle_SetRotation, NULL},
	PY_INT_PROP_RO("map", maps.GetCurrentMapId, NULL),
	{"hit_dice", PyObjHandle_GetHitDice, NULL, NULL},
	{"hit_dice_num", PyObjHandle_GetHitDiceNum, NULL, NULL},
	{"get_size", PyObjHandle_GetSize, NULL, NULL},
	{"off_x", PyObjHandle_GetOffsetX, NULL, NULL},
	{"off_y", PyObjHandle_GetOffsetY, NULL, NULL},
	{"scripts", PyObjHandle_GetScripts, NULL, NULL},
	{ "origin", PyObjHandle_GetOriginMapId, PyObjHandle_SetOriginMapId, NULL },
	{"substitute_inventory", PyObjHandle_GetSubstituteInventory, NULL, NULL},
	{"feats", PyObjHandle_GetFeats, NULL, NULL},
	{ "loots", PyObjHandle_GetLoots, PyObjHandle_SetLoots, NULL },
	{"__safe_for_unpickling__", PyObjHandle_SafeForUnpickling, NULL, NULL},
};

#pragma endregion

#pragma region Number Methods

static int PyObjHandle_NonZero(PyObject* obj) {
	auto self = GetSelf(obj);
	return self->handle != 0;
}

static PyNumberMethods PyObjHandleNumberMethods = {
	0, // nb_add
	0, // nb_subtract
	0, // nb_multiply
	0, // nb_divide
	0, // nb_remainder
	0, // nb_divmod
	0, // nb_power
	0, // nb_negative
	0, // nb_positive
	0, // nb_absolute
	PyObjHandle_NonZero, // nb_nonzero
	0,
};

#pragma endregion

#pragma region Initialization and New Methods
static int PyObjHandle_Init(PyObject *obj, PyObject *args, PyObject *kwargs) {
	auto self = (PyObjHandle*)obj;

	if (!PyArg_ParseTuple(args, "|L:PyObjHandle", &self->handle)) {
		return -1;
	}

	if (!self->handle) {
		self->id.subtype = 0;
	} else {
		// Get GUID from handle
		self->id = objects.GetId(self->handle);

		// The obj handle is invalid
		if (!self->id.subtype) {
			auto msg = format("The object handle {} is invalid.", self->handle);
			PyErr_SetString(PyExc_ValueError, msg.c_str());
			// Reset the handle to the null handle
			self->handle = 0;
			self->id.subtype = 0;
			return -1;
		}
	}

	return 0;
}

static PyObject *PyObjHandle_New(PyTypeObject*, PyObject*, PyObject*) {
	auto self = PyObject_New(PyObjHandle, &PyObjHandleType);
	self->handle = 0;
	self->id.subtype = 0;
	return (PyObject*)self;
}
#pragma endregion

PyTypeObject PyObjHandleType = {
	PyObject_HEAD_INIT(NULL)
	0, /*ob_size*/
	"toee.PyObjHandle", /*tp_name*/
	sizeof(PyObjHandle), /*tp_basicsize*/
	0, /*tp_itemsize*/
	(destructor) PyObject_Del, /*tp_dealloc*/
	0, /*tp_print*/
	0, /*tp_getattr*/
	0, /*tp_setattr*/
	PyObjHandle_Cmp, /*tp_compare*/
	PyObjHandle_Repr, /*tp_repr*/
	&PyObjHandleNumberMethods, /*tp_as_number*/
	0, /*tp_as_sequence*/
	0, /*tp_as_mapping*/
	0, /*tp_hash */
	0, /*tp_call*/
	0, /*tp_str*/
	PyObject_GenericGetAttr, /*tp_getattro*/
	PyObject_GenericSetAttr, /*tp_setattro*/
	0, /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT, /*tp_flags*/
	0, /* tp_doc */
	0, /* tp_traverse */
	0, /* tp_clear */
	0, /* tp_richcompare */
	0, /* tp_weaklistoffset */
	0, /* tp_iter */
	0, /* tp_iternext */
	PyObjHandleMethods, /* tp_methods */
	0, /* tp_members */
	PyObjHandleGetSets, /* tp_getset */
	0, /* tp_base */
	0, /* tp_dict */
	0, /* tp_descr_get */
	0, /* tp_descr_set */
	0, /* tp_dictoffset */
	PyObjHandle_Init, /* tp_init */
	0, /* tp_alloc */
	PyObjHandle_New, /* tp_new */
};

bool ConvertObjHndl(PyObject* obj, objHndl* pHandleOut) {
	if (obj == Py_None) {
		*pHandleOut = 0;
		return true;
	}

	if (obj->ob_type != &PyObjHandleType) {
		PyErr_SetString(PyExc_TypeError, "Expected object handle.");
		return false;
	}
	
	*pHandleOut = GetSelf(obj)->handle;
	return true;
}

PyObject* PyObjHndl_Create(objHndl handle) {

	auto result = PyObject_New(PyObjHandle, &PyObjHandleType);
	result->handle = handle;
	if (handle) {
		result->id = objects.GetId(handle);
	} else {
		memset(&result->id, 0, sizeof(result->id));
	}

	return (PyObject*) result;

}

PyObject* PyObjHndl_CreateNull() {
	return PyObjHndl_Create(0);
}

objHndl PyObjHndl_AsObjHndl(PyObject* obj) {
	assert(PyObjHndl_Check(obj));
	return GetSelf(obj)->handle;
}

bool PyObjHndl_Check(PyObject* obj) {
	return obj->ob_type == &PyObjHandleType;
}
