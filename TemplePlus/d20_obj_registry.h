#pragma once
#include "common.h"
#include "combat.h"

#define D20_OBJ_REGISTRY_MAX 1024

class D20ObjRegistrySystem
{
public:

	void Set(int idx, objHndl objHnd);
	objHndl Get(int idx);
	void Append(objHndl objHnd);
	int Find(objHndl objHnd);
	void Remove(objHndl objHnd);
	void D20ObjRegistrySendSignalAll(D20DispatcherKey dispKey, D20Actn* d20a, int32_t arg2);
private:
	int GetNum(); // get number of items
	void IncNum();
	void DecNum();
};

extern D20ObjRegistrySystem d20ObjRegistrySys;

void __cdecl _D20ObjRegistryAppend(objHndl objHnd);
int __cdecl _D20ObjRegistryFind(objHndl objHnd);
void __cdecl _D20ObjRegistryRemove(objHndl objHnd);