#include "stdafx.h"
#include "common.h"
#include "raycast.h"
#include <temple/dll.h>

struct RaycastAddresses: temple::AddressTable
{
	
	int(__cdecl * Raycast)(RaycastPacket* objIt);
	int(__cdecl * RaycastShortRange)(RaycastPacket* objIt);
	void(__cdecl* RaycastPacketClear)(RaycastPacket * objIt);
	RaycastAddresses()
	{
		rebase(RaycastPacketClear, 0x100BABE0);
		rebase(Raycast, 0x100BACE0);
		rebase(RaycastShortRange, 0x100BC750);
	}
} addresses;

int RaycastPacket::Raycast()
{
	return addresses.Raycast(this);
}

int RaycastPacket::RaycastShortRange()
{
	return addresses.RaycastShortRange(this);
}

void RaycastPacket::RaycastPacketFree()
{
	addresses.RaycastPacketClear(this);
}

RaycastPacket::~RaycastPacket()
{
	addresses.RaycastPacketClear(this);
}
