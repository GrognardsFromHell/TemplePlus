
#pragma once

#include "obj.h"

/**
* Info used to persistently and safely save a reference to an object.
*/
#pragma pack(push, 1)
struct FrozenObjRef {
	ObjectId guid;
	locXY location;
	int mapNumber;
	int padding;


	static bool Save(const objHndl &handle, const FrozenObjRef *ref, void* fh);
	static bool Load(objHndl *handleOut, FrozenObjRef *ref, void *fh);
	static FrozenObjRef Freeze(objHndl handle);
	static bool Unfreeze(const FrozenObjRef &ref, objHndl *handleOut);
};
#pragma pack(pop)
