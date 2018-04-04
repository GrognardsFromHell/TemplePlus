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

RaycastPointSearchPacket::RaycastPointSearchPacket(const XMFLOAT2& origin, const XMFLOAT2& endPt){
	originAbsX = origin.x;
	originAbsY = origin.y;
	targetAbsX = endPt.x;
	targetAbsY = endPt.y;
	auto deltaX = targetAbsX - originAbsX;
	auto deltaY = targetAbsY - originAbsY;
	this->rangeInch = sqrt(deltaX*deltaX + deltaY * deltaY);
	auto rangeInverse = 1.0 / rangeInch;
	this->ux = deltaX * rangeInverse;
	this->uy = deltaY * rangeInverse;
	this->absOdotU = ux * originAbsX + uy * originAbsY;
}

bool RaycastPointSearchPacket::IsPointInterceptedBySegment(const XMFLOAT2& v, float objRadiusInch){
	auto radiusAdj = objRadiusInch + this->radius;
	auto radiusAdjSquare = radiusAdj* radiusAdj;
	// find the closest point on the segment by projecting the vector OV onto u
	auto projection = v.x * this->ux + v.y * this->uy - this->absOdotU;
	// There are now 3 cases:
	// 1. projection < 0
	//  means the closest point to v is at the segment origin
	if (projection < 0){
		return false;// for raycasting purposes, the point is considered "behind" the segment. Hence it is not symmetrical with the next case.
	}
	// 2. projection > segment length
	// means the closest point to v is the segment end
	if (projection > this->rangeInch){
		auto deltaX = targetAbsX - v.x;
		auto deltaY = targetAbsY - v.y;
		auto result = deltaX * deltaX + deltaY * deltaY < radiusAdjSquare;
		return result;
	}
	// 3. otherwise:
	// the closest point is O + u * projection
	auto deltaX = projection * ux + originAbsX - v.x;
	auto deltaY = projection * uy + originAbsY - v.y;
	auto result = deltaX * deltaX + deltaY * deltaY < radiusAdjSquare;
	return result;
}
