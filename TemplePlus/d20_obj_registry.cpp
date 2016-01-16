#include "stdafx.h"
#include "common.h"
#include "d20_obj_registry.h"
#include "description.h"
#include "util/fixes.h"
#include "d20.h"


class D20ObjRegistrySystem  d20ObjRegistrySys;

struct D20ObjRegistrySystemAddresses : temple::AddressTable{

	objHndl **d20ObjRegistry; // room for 1024
	int * d20ObjRegistryNumItems; 
	D20ObjRegistrySystemAddresses()
	{
		rebase(d20ObjRegistry,			0x10BCAD94);
		rebase(d20ObjRegistryNumItems,	0x10BCAD98);
		rebase(InitiativeRefresh, 0x100DFE40);
	}

	int (*InitiativeRefresh)(int initiative, int initiativeNext);
}addresses;


class D20ObjRegistryReplacements : TempleFix
{
public: 
	const char* name() override { return "D20ObjRegistry" "Function Replacements";} 
	void apply() override 
	{
		replaceFunction(0x100DFAD0, _D20ObjRegistryAppend);
		replaceFunction(0x100DFB00, _D20ObjRegistryFind);
		replaceFunction(0x100DFB50, _D20ObjRegistryRemove);
	}
} fix;

int D20ObjRegistrySystem::GetNum()
{
	return *addresses.d20ObjRegistryNumItems;
}

void D20ObjRegistrySystem::IncNum()
{
	(*addresses.d20ObjRegistryNumItems)++;
}

void D20ObjRegistrySystem::DecNum()
{
	(*addresses.d20ObjRegistryNumItems)--;
}

void D20ObjRegistrySystem::Set(int idx, objHndl objHnd)
{
	(*addresses.d20ObjRegistry)[idx] = objHnd;
}

objHndl D20ObjRegistrySystem::Get(int idx)
{
	return (*addresses.d20ObjRegistry)[idx];
}

void D20ObjRegistrySystem::Append(objHndl objHnd)
{
	auto numItems = GetNum();
	//logger->debug("Adding to d20 registry: {}, at slot {}", description.getDisplayName(objHnd), numItems);
	if (numItems >= D20_OBJ_REGISTRY_MAX)
	{
		logger->debug("D20 Obj Registry limit exceeded!");
		assert(numItems < D20_OBJ_REGISTRY_MAX);
	}
	Set( numItems, objHnd) ;
	IncNum();
}

int D20ObjRegistrySystem::Find(objHndl objHnd)
{
	auto numItems = GetNum();
	if ( numItems<= 0)
		return 0;
	for (int i = 0; i < numItems; i++)
	{
		if ( Get(i) == objHnd)
			return 1;
	}
	return 0;
}

void D20ObjRegistrySystem::Remove(objHndl objHnd)
{
	auto numItems = GetNum();
	if (numItems <= 0) return;

	for (int i = 0; i < numItems; i++)
	{
		if (Get(i) == objHnd)
		{
			//logger->debug("Removing from d20 registry: {}, at slot {}", description.getDisplayName(objHnd), i);
			Set(i, Get(numItems - 1));
			DecNum();
			return;
		}
	}
	return;
}

void D20ObjRegistrySystem::D20ObjRegistrySendSignalAll(D20DispatcherKey dispKey, D20Actn* d20a, int32_t arg2)
{
	for (int i = 0; i < *addresses.d20ObjRegistryNumItems; i++)
	{
		d20Sys.d20SendSignal( (*addresses.d20ObjRegistry)[i], dispKey, d20a, arg2);
	}
}

int D20ObjRegistrySystem::InitiativeRefresh(int initiative, int initiativeNext)
{
	return addresses.InitiativeRefresh(initiative, initiativeNext);
		
}

void _D20ObjRegistryAppend(objHndl objHnd)
{
	d20ObjRegistrySys.Append(objHnd);
}

int _D20ObjRegistryFind(objHndl objHnd)
{
	return d20ObjRegistrySys.Find(objHnd);
}

void _D20ObjRegistryRemove(objHndl objHnd)
{
	d20ObjRegistrySys.Remove(objHnd);
}