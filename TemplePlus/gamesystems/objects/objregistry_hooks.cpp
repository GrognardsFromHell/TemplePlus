#include "stdafx.h"

#include <temple/dll.h>

#include "util/fixes.h"
#include "obj_structs.h"

#include "objregistry.h"
#include "objsystem.h"
#include "../gamesystems.h"

#include <obj.h>
#include <objlist.h>
#include <maps.h>

static class ObjRegistryHooks : public TempleFix {
public:
	const char* name() override {
		return "Object Registry Hooks";
	}

	static void Object_Tables_Init(int nToEEObject_BodySize, BOOL editor);
	static ObjectId* Object_Tables_GetIdByHandle(ObjectId* idOut, objHndl handle);
	static void Object_Tables_CompactIdx();
	static void Object_Tables_Destroy();
	static GameObjectBody* Object_Tables_GetItem(objHndl handle);
	static void Object_Tables_AddToIndex(ObjectId ObjectId, objHndl handle);

	static BOOL Object_Tables_FirstHandle(objHndl* handleOut, int* iteratorOut);
	static BOOL Object_Tables_NextHandle(objHndl* handleOut, int* iteratorInOut);

	static BOOL Object_Tables_IsValidHandle(objHndl handle);
	static GameObjectBody* Object_Tables_Add(objHndl* handleOut);
	static void Object_Tables_Remove(objHndl handle);
	static objHndl Object_Tables_perm_lookup(ObjectId id);

	static ObjRegistry::It it;

	void apply() override {
		replaceFunction(0x100c2580, Object_Tables_Init);
		replaceFunction(0x100c2660, Object_Tables_GetIdByHandle);
		replaceFunction(0x100c26c0, Object_Tables_CompactIdx);
		replaceFunction(0x100c2960, Object_Tables_Destroy);
		replaceFunction(0x100c2a70, Object_Tables_GetItem);
		replaceFunction(0x100c2ac0, Object_Tables_AddToIndex);
		replaceFunction(0x100c2ba0, Object_Tables_FirstHandle);
		replaceFunction(0x100c2c50, Object_Tables_NextHandle);
		replaceFunction(0x100c2d00, Object_Tables_IsValidHandle);
		replaceFunction(0x100c2e90, Object_Tables_Add);
		replaceFunction(0x100c3030, Object_Tables_Remove);
		replaceFunction(0x100C3050, Object_Tables_perm_lookup);

		// The following internal functions of the module are not replaced:
		// replaceFunction(0x100c2790, Object_Tables_AddBucket);
		// replaceFunction(0x100c2810, _Object_Tables_AddToFreeList);
		// replaceFunction(0x100c2870, _Object_Tables_FindInIdx);
		// replaceFunction(0x100c2db0, _Object_Tables_GetFreeIdx);
	}
} hooks;

ObjRegistry::It ObjRegistryHooks::it;

void ObjRegistryHooks::Object_Tables_Init(int nToEEObject_BodySize, BOOL editor) {
	Expects(nToEEObject_BodySize == sizeof(GameObjectBody));
	Expects(editor == FALSE);
}

ObjectId* ObjRegistryHooks::Object_Tables_GetIdByHandle(ObjectId* idOut, objHndl handle) {
	*idOut = gameSystems->GetObj().GetIdByHandle(handle);
	return idOut;

}

void ObjRegistryHooks::Object_Tables_CompactIdx() {
	gameSystems->GetObj().CompactIndex();
}

void ObjRegistryHooks::Object_Tables_Destroy() {
	// NOOP
}

GameObjectBody* ObjRegistryHooks::Object_Tables_GetItem(objHndl handle) {
	throw TempleException("Should not be called.");
}

void ObjRegistryHooks::Object_Tables_AddToIndex(ObjectId id, objHndl handle) {
	gameSystems->GetObj().AddToIndex(id, handle);
}

/**
	NOTE: ToEE will remove items from the obj registy while using this iterator function.
	For that reason, the global iterator object will always point to the *next* object
	instead of the current object. unordered_map guarantees that iterators for items 
	will be unaffected by item removal unless they point to the removed item.
*/
BOOL ObjRegistryHooks::Object_Tables_FirstHandle(objHndl* handleOut, int* iteratorOut) {
	it = gameSystems->GetObj().mObjRegistry->begin();
	if (it == gameSystems->GetObj().mObjRegistry->end()) {
		*handleOut = 0;
		return FALSE;
	}

	*handleOut = it->first;
	++it;
	return TRUE;

}

BOOL ObjRegistryHooks::Object_Tables_NextHandle(objHndl* handleOut, int* iteratorIn) {

	if (it == gameSystems->GetObj().mObjRegistry->end()) {
		*handleOut = 0;
		return FALSE;
	}

	*handleOut = it->first;
	++it;
	return TRUE;

}

BOOL ObjRegistryHooks::Object_Tables_IsValidHandle(objHndl handle) {
	return gameSystems->GetObj().IsValidHandle(handle) ? TRUE : FALSE;
}

GameObjectBody* ObjRegistryHooks::Object_Tables_Add(objHndl* handleOut) {
	throw TempleException("Should not be called.");
}

void ObjRegistryHooks::Object_Tables_Remove(objHndl handle) {
	gameSystems->GetObj().mObjRegistry->Remove(handle);
}

objHndl ObjRegistryHooks::Object_Tables_perm_lookup(ObjectId id) {

	return gameSystems->GetObj().GetHandleById(id);

}
