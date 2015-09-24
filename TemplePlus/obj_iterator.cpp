#include "stdafx.h"
#include "common.h"
#include "obj_iterator.h"


struct ObjIteratorAddresses: temple::AddressTable
{
	int(__cdecl * TargettingSthg_100BACE0)(ObjIterator* objIt);
	void(__cdecl* ObjIteratorClear)(ObjIterator * objIt);
	ObjIteratorAddresses()
	{
		rebase(ObjIteratorClear, 0x100BABE0);
		rebase(TargettingSthg_100BACE0, 0x100BACE0);
	}
} addresses;

int ObjIterator::TargettingSthg_100BACE0()
{
	return addresses.TargettingSthg_100BACE0(this);
}

ObjIterator::~ObjIterator()
{
	addresses.ObjIteratorClear(this);
}