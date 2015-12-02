
#include "stdafx.h"
#include "python_debug.h"
#include "python_object.h"
#include "../condition.h"
#include "../radialmenu.h"
#include "feat.h"
#include <ai.h>
#include <path_node.h>
#include <pathfinding.h>
#include <gamesystems/map/sector.h>
#include <location.h>

static std::string GetNiceName(void* ptr) {
	
	auto startAddr = temple::GetPointer<char>(0x10000001) - 1;
	// Heuristic to try to determine whether it's a valid ptr
	if (ptr < startAddr || ptr > (startAddr + 0x10000000))
		return std::string();

	auto cap = conds.mCondStructHashtable->numItems;
	auto result = PyList_New(cap);

	for (size_t i = 0; i < cap; ++i) {
		auto idx = conds.mCondStructHashtable->idxArray[i];
		auto data = conds.mCondStructHashtable->dataArray[idx];

		if (data == ptr) {
			return fmt::format("{} (Cond Ref)", data->condName);
		}
	}

	// Can we read a string?
	char* charPtr = (char*)ptr;
	for (int i = 0; i < 64; ++i) {
		if (charPtr[i] < 0 || !isalnum(charPtr[i]) && !isspace(charPtr[i]) && charPtr[i]) {
			return std::string();
		}
		if (!charPtr) {
			return charPtr;
		}
	}

	return std::string(); // Pointed to string longer than 64 chars

}

/*
	Dumps all conditions from the global hashtable to a Wiki article.
*/
PyObject *PyDebug_DumpConds() {
	
	auto cap = conds.mCondStructHashtable->numItems;
	auto result = PyList_New(cap);

	for (size_t i = 0; i < cap; ++i) {
		auto idx = conds.mCondStructHashtable->idxArray[i];
		auto data = conds.mCondStructHashtable->dataArray[idx];
		
		auto hooks = PyList_New(0);
		auto hook = data->subDispDefs;
		while (hook->dispType) {
			std::string nameData1(GetNiceName(reinterpret_cast<void*>(hook->data1)));
			std::string nameData2(GetNiceName(reinterpret_cast<void*>(hook->data2)));
			
			PyObject* v;
			if (!nameData1.empty() && !nameData2.empty()) {
				v = Py_BuildValue("IssII", hook->dispCallback, &nameData1[0], &nameData2[0], hook->dispType, hook->dispKey);
			} else if (!nameData1.empty()) {
				v = Py_BuildValue("IsIII", hook->dispCallback, &nameData1[0], hook->data2, hook->dispType, hook->dispKey);
			} else if (!nameData2.empty()) {
				v = Py_BuildValue("IIsII", hook->dispCallback, hook->data1, &nameData2[0], hook->dispType, hook->dispKey);
			} else {
				v = Py_BuildValue("IIIII", hook->dispCallback, hook->data1, hook->data2, hook->dispType, hook->dispKey);
			}

			PyList_Append(hooks, v);
			hook++;
		}

		auto c = Py_BuildValue("sIO", data->condName, data->numArgs, hooks);
		Py_DecRef(hooks);
		PyList_SET_ITEM(result, i, c);
	}

	return result;
}

static void SetDictItem(PyObject *dict, const char *keyName, PyObject *val) {
	PyDict_SetItemString(dict, keyName, val);
	Py_DECREF(val);
}

static void SetDictItem(PyObject *dict, const char *keyName, int value) {
	auto val = PyInt_FromLong(value);
	PyDict_SetItemString(dict, keyName, val);
	Py_DECREF(val);
}

static void SetDictItem(PyObject *dict, const char *keyName, const char *text) {
	auto val = PyString_FromString(text);
	PyDict_SetItemString(dict, keyName, val);
	Py_DECREF(val);
}

PyObject* DumpRadialNode(const RadialMenuNode &node) {
	auto result = PyDict_New();

	// Dump the entry
	auto entry = node.entry;	
	SetDictItem(result, "text", entry.text);
	SetDictItem(result, "field4", entry.field4);
	SetDictItem(result, "textHash", entry.textHash);
	SetDictItem(result, "fieldc", entry.fieldc);
	SetDictItem(result, "type", (int) entry.type);
	SetDictItem(result, "minArg", entry.minArg);
	SetDictItem(result, "maxArg", entry.maxArg);
	SetDictItem(result, "actualArg", entry.actualArg);
	SetDictItem(result, "d20ActionType", entry.d20ActionType);
	SetDictItem(result, "d20ActionData1", entry.d20ActionData1);
	SetDictItem(result, "d20Caf", entry.d20Caf);
	SetDictItem(result, "spellEnumOrg", entry.d20SpellData.spellEnumOrg);
	SetDictItem(result, "spellMetaMagic", entry.d20SpellData.metaMagicData.metaMagicFlags);
	SetDictItem(result, "dispKey", entry.dispKey);
	SetDictItem(result, "callback", (uint32_t) entry.callback);
	SetDictItem(result, "flags", (uint32_t) entry.flags);
	SetDictItem(result, "helpId", (uint32_t) entry.helpId);
	SetDictItem(result, "field44", (uint32_t) entry.field44);

	auto children = PyList_New(node.childCount);
	for (int i = 0; i < node.childCount; ++i) {
		PyList_SET_ITEM(children, i, PyInt_FromLong(node.children[i]));
	}
	SetDictItem(result, "children", children);
	SetDictItem(result, "morphsTo", PyInt_FromLong(node.morphsTo));
	SetDictItem(result, "parent", PyInt_FromLong(node.parent));

	return result;
}

PyObject *PyDebug_DumpRadial() {	
	auto result = PyList_New(0);

	for (auto &radial : radialMenus.GetAll()) {
		auto item = PyDict_New();

		// Set owner of the radial menu
		SetDictItem(item, "owner", PyObjHndl_Create(radial->obj));
		SetDictItem(item, "field8", PyInt_FromLong(radial->field8));

		auto nodes = PyList_New(radial->nodeCount);
		for (int i = 0; i < radial->nodeCount; ++i) {
			PyList_SET_ITEM(nodes, i, DumpRadialNode(radial->nodes[i]));
		}
		SetDictItem(item, "nodes", nodes);

		PyList_Append(result, item);
		Py_DECREF(item);
	}

	return result;
}

/*
	dumps feat data tables so they can finally be externalized!
*/
PyObject *PyDebug_DumpFeats() {
	auto cap = NUM_FEATS;
	auto result = PyList_New(cap);
	uint32_t * featPropertiesTable = feats.m_featPropertiesTable;
	FeatPrereqRow * featPreReqTable = feats.m_featPreReqTable;

	for (int i = 0; i < cap; ++i) {
		feat_enums feat = (feat_enums)i;
		char * featName = feats.GetFeatName(feat);

		auto featPrereqs = PyList_New(0);
		for (int j = 0; j < 8; j++)
		{
			auto v = Py_BuildValue("II", featPreReqTable[i].featPrereqs[j].featPrereqCode, 
				featPreReqTable[i].featPrereqs[j].featPrereqCodeArg);
			PyList_Append(featPrereqs, v);
		}

		auto c = Py_BuildValue("sIIO", featName, feat, featPropertiesTable[i], featPrereqs);
		Py_DecRef(featPrereqs);
		PyList_SET_ITEM(result, i, c);
	}

	return result;
}

PyObject *PyDebug_DumpD20Actions() {
	auto cap = D20A_NUMACTIONS;
	auto result = PyList_New(cap);
	D20ActionDef * d20Defs = d20Sys.d20Defs;
	
	for (int i = 0; i < cap; ++i) {

		auto c = Py_BuildValue("IIIIIIIIIIIII", i, 
			d20Defs[i].addToSeqFunc, d20Defs[i].aiCheckMaybe, d20Defs[i].actionCheckFunc,
			d20Defs[i].tgtCheckFunc, d20Defs[i].locCheckFunc, d20Defs[i].performFunc,
			d20Defs[i].actionFrameFunc, d20Defs[i].projectilePerformFunc, d20Defs[i].pad_apparently,
			d20Defs[i].actionCost, d20Defs[i].pickerFuncMaybe, d20Defs[i].flags);
		
		PyList_SET_ITEM(result, i, c);
	}

	return result;
}

PyObject *PyDebug_DumpAiTactics() {
	auto cap = 44;
	auto result = PyList_New(cap);
	AiTacticDef * aiTacDefs = aiSys.aiTacticDefs;

	for (int i = 0; i < cap; ++i) {

		auto c = Py_BuildValue("sII", 
			aiTacDefs[i].name, aiTacDefs[i].aiFunc, aiTacDefs[i].onInitiativeAdd );

		PyList_SET_ITEM(result, i, c);
	}

	return result;
}


PyObject *PyDebug_RecalculatePathNodeNeighbours()
{
	pathNodeSys.RecalculateAllNeighbours();
	return PyLong_FromLongLong(1);
}

PyObject *PyDebug_FlushNodes()
{
	pathNodeSys.FlushNodes();
	return PyLong_FromLongLong(1);
}

PyObject *PyDebug_ReciprocityDebug()
{
	pathNodeSys.RecipDebug();
	return PyLong_FromLongLong(1);
}

PyObject *PyDebug_GenerateClearanceFile()
{
	pathNodeSys.GenerateClearanceFile();
	return PyLong_FromLongLong(1);
}

/*
 check if destination is considered clear for inhabitation by a critter
*/
PyObject *PyDebug_DestClear(PyObject*, PyObject* args)
{
	objHndl dude = 0;
	LocAndOffsets loc ;
	loc.location.locx = 0;
	loc.location.locy = 0;
	loc.off_x = 0;
	loc.off_y = 0;

	if (!PyArg_ParseTuple(args, "|O&iiff:destclear", &ConvertObjHndl, &dude, &loc.location.locx, &loc.location.locy, &loc.off_x, &loc.off_y)) {
		return 0;
	}

	PathQuery pathQ;
	pathQ.flags = (PathQueryFlags)0;
	int result = pathfindingSys.PathDestIsClear(&pathQ, dude, loc);
	return PyLong_FromLongLong(result);
}


PyObject *PyDebug_GetTileFlags(PyObject*, PyObject* args)
{
	TileFlags flags = TileFlags::TILEFLAG_NONE;
	LocAndOffsets loc = {};
	if (!PyArg_ParseTuple(args, "ii|ff:tileflags", &loc.location.locx, &loc.location.locy, &loc.off_x, &loc.off_y)) {
		return 0;
	}
	flags = sectorSys.GetTileFlags(loc);
	return PyLong_FromLongLong(flags);
}

PyObject *PyDebug_GetLocClearance(PyObject*, PyObject* args)
{
	if (!PathNodeSys::hasClearanceData)
		return PyInt_FromLong(-1);

	LocAndOffsets loc = {};
	if (!PyArg_ParseTuple(args, "ii|ff:tileflags", &loc.location.locx, &loc.location.locy, &loc.off_x, &loc.off_y)) {
		return 0;
	}

	SectorLoc secLoc;
	secLoc.GetFromLoc(loc.location);
	auto secAddr = PathNodeSys::clearanceData.clrIdx.clrAddr[secLoc.y()][secLoc.x()];
	auto baseTile = secLoc.GetBaseTile();
	float result = PathNodeSys::clearanceData.secClr[secAddr].val[3*(loc.location.locy - baseTile.locy)][3*(loc.location.locx - baseTile.locx)];
	return PyFloat_FromDouble(result);
}

PyObject *PyDebug_PathTo(PyObject*, PyObject* args)
{
	objHndl dude = 0;
	LocAndOffsets loc;
	loc.location.locx = 0;
	loc.location.locy = 0;
	loc.off_x = 0;
	loc.off_y = 0;

	if (!PyArg_ParseTuple(args, "|O&iiff:pathto", &ConvertObjHndl, &dude, &loc.location.locx, &loc.location.locy, &loc.off_x, &loc.off_y)) {
		return 0;
	}

	PathQuery pathQ;
	pathQ.flags = (PathQueryFlags)0;
	int result = pathfindingSys.PathDestIsClear(&pathQ, dude, loc);
	if (!result)
		return PyInt_FromLong(0);
	PathQueryResult pqr;
	pathQ.flags = PathQueryFlags::PQF_HAS_CRITTER;
	pathQ.from = objects.GetLocationFull(dude);
	pathQ.to = loc;
	pathQ.critter = dude;
	result = pathfindingSys.FindPath(&pathQ, &pqr);
	return PyLong_FromLongLong(result);
}


static PyMethodDef PyDebug_Methods[] = {
	{ "dump_conds", (PyCFunction) PyDebug_DumpConds, METH_NOARGS, NULL },
	{ "dump_radial", (PyCFunction) PyDebug_DumpRadial, METH_NOARGS, NULL },
	{ "dump_feats", (PyCFunction)PyDebug_DumpFeats, METH_NOARGS, NULL },
	{ "dump_d20actions", (PyCFunction)PyDebug_DumpD20Actions, METH_NOARGS, NULL },
	{ "dump_ai_tactics", (PyCFunction)PyDebug_DumpAiTactics, METH_NOARGS, NULL },
	{ "recalc_neighbours", (PyCFunction)PyDebug_RecalculatePathNodeNeighbours, METH_NOARGS, NULL },
	{ "flush_nodes", (PyCFunction)PyDebug_FlushNodes, METH_NOARGS, NULL },
	{ "recip", (PyCFunction)PyDebug_ReciprocityDebug, METH_NOARGS, NULL },
	{ "destclear", (PyCFunction)PyDebug_DestClear, METH_VARARGS, NULL },
	{ "tileflags", (PyCFunction)PyDebug_GetTileFlags, METH_VARARGS, NULL },
	{ "genclearance", (PyCFunction)PyDebug_GenerateClearanceFile, METH_VARARGS, NULL },
	{ "genclr", (PyCFunction)PyDebug_GenerateClearanceFile, METH_VARARGS, NULL },
	{ "getclr", (PyCFunction)PyDebug_GetLocClearance, METH_VARARGS, NULL },
	{ "pathto", (PyCFunction)PyDebug_PathTo, METH_VARARGS, NULL },
	{ NULL, }
};

void PyDebug_Init() {
	Py_InitModule("debug", PyDebug_Methods);
}
