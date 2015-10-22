#include "stdafx.h"
#include "sector.h"
#include <temple/dll.h>
#include <raycast.h>



struct SectorAddresses : temple::AddressTable
{
	
	BOOL(__cdecl*BuildTileListFromRect)(TileRect * tileRect, TileListEntry* tle);
	int(__cdecl*SectorLock)(SectorLoc secLoc, Sector** sectOut);
	int(__cdecl*SectorCacheFindUnusedIdx)(int*secCacheIdx);
	int(__cdecl*SectorCacheFind)(SectorLoc secLoc, int* secCacheIdx);
	BOOL(__cdecl*SectorCacheLocExists)(SectorLoc secLoc);
	int(__cdecl*SectorUnlock)(SectorLoc secLoc);
	void(__cdecl* SectorCacheUnlockAll)();
	int(__cdecl* SectorSystemInit)();
	BOOL(__cdecl* GetPointAlongSegmentNearestToOriginDistanceRfromV)(float absVx, float absVy, float radiusAdjAmt, RaycastPointSearchPacket * srchPkt, PointAlongSegment* pnt);
	BOOL(__cdecl* IsPointCloseToSegment)(float absVx, float absVy, float radiusAdjAmt, RaycastPointSearchPacket * srchPkt);
	SectorAddresses()
	{
		rebase(SectorCacheFind,		  0x10081FA0);
		rebase(BuildTileListFromRect, 0x100824D0);
		rebase(SectorCacheFindUnusedIdx, 0x10082030);
		rebase(SectorCacheLocExists, 0x100826B0);
		rebase(SectorLock,			  0x10082700);
		rebase(SectorUnlock,		  0x10082B40);
		rebase(SectorCacheUnlockAll, 0x10082B90);
		rebase(SectorSystemInit, 0x10082DB0);
		rebase(IsPointCloseToSegment, 0x100BA980);
		rebase(GetPointAlongSegmentNearestToOriginDistanceRfromV,0x100BAA30);
	}
} addresses;

SectorSystem  sectorSys;

BOOL SectorSystem::BuildTileListFromRect(TileRect* tileRect, TileListEntry* tle)
{
	return addresses.BuildTileListFromRect(tileRect, tle);
}

int SectorSystem::SectorLock(SectorLoc secLoc, Sector** sect)
{
	return addresses.SectorLock(secLoc, sect);
}

int SectorSystem::SectorUnlock(SectorLoc secLoc)
{
	return addresses.SectorUnlock(secLoc);	
}

BOOL SectorSystem::GetSegmentInterception(float absVx, float absVy, float radiusAdjAmt, RaycastPointSearchPacket* srchPkt, PointAlongSegment* pnt)
{
	return addresses.GetPointAlongSegmentNearestToOriginDistanceRfromV(absVx, absVy, radiusAdjAmt, srchPkt, pnt);
}

BOOL SectorSystem::IsPointInterceptedBySegment(float absVx, float absVy, float radiusAdjAmt, RaycastPointSearchPacket* srchPkt)
{
	return addresses.IsPointCloseToSegment(absVx, absVy, radiusAdjAmt, srchPkt);
}

TileFlags SectorSystem::GetTileFlags(LocAndOffsets loc)
{
	TileFlags flags = TileFlags::TILEFLAG_NONE;
	SectorLoc secLoc(loc.location);
	Sector * sect;
	int lockStatus = SectorLock(secLoc, &sect);
	if (!lockStatus)
	{
		return (TileFlags)(-1);
	} 

	locXY baseTile = secLoc.GetBaseTile();
	int deltaX = loc.location.locx - baseTile.locx;
	int deltaY = loc.location.locy - baseTile.locy;

	int tileIdx = deltaX + SECTOR_SIDE_SIZE * deltaY;

	flags = sect->tilePkt.tiles[tileIdx].flags;
	int flags3 = sect->tilePkt.tiles[tileIdx].flags3;

	SectorUnlock(secLoc);
	return flags;
}

bool SectorSystem::GetTileFlagsArea(TileRect * tileRect, Subtile * out, int * count)
{
	TileListEntry tle[25];
	assert(abs(tileRect->y2 - tileRect->y1) < 200);
	assert(abs(tileRect->x2 - tileRect->x1) < 200);
	BuildTileListFromRect(tileRect, tle);


	return 1;
}