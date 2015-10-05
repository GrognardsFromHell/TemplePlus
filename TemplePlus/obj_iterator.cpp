#include "stdafx.h"
#include "common.h"
#include "obj_iterator.h"
#include <temple/dll.h>

struct ObjIteratorAddresses: temple::AddressTable
{
	
	int(__cdecl * TargettingSthg_100BACE0)(ObjIterator* objIt);
	int(__cdecl * TargettingSthg_100BC750)(ObjIterator* objIt);
	void(__cdecl* ObjIteratorClear)(ObjIterator * objIt);
	ObjIteratorAddresses()
	{
		rebase(ObjIteratorClear, 0x100BABE0);
		rebase(TargettingSthg_100BACE0, 0x100BACE0);
		rebase(TargettingSthg_100BC750, 0x100BC750);
	}
} addresses;

int ObjIterator::TargettingSthg_100BACE0()
{
	return addresses.TargettingSthg_100BACE0(this);
}

int ObjIterator::TargettingSthg_100BC750()
{
	return addresses.TargettingSthg_100BC750(this);
}

ObjIterator::~ObjIterator()
{
	addresses.ObjIteratorClear(this);
}