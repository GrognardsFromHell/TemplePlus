#include "stdafx.h"
#include "sector.h"
#include <temple/dll.h>
#include <raycast.h>
#include <maps.h>


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
	int *sectorLockSthg_10AB7458;
	int * sectorCacheIdx;
	int ** sectorCacheIndices;
	BOOL * sectorLocking;
	SectorCacheEntry ** sectorCache;
	int * sectorLockSerial;
	int * secCountSthg_10AB74AC;

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

		rebase(sectorLockSerial,		0x10AB7410);
		rebase(sectorCacheIdx,			0x10AB7430);
		rebase(sectorLockSthg_10AB7458, 0x10AB7458);
		rebase(sectorCacheIndices,		0x10AB7478);
		rebase(sectorLocking,			0x10AB747C);
		rebase(secCountSthg_10AB74AC,	0x10AB74AC);
	}
} addresses;

SectorSystem  sectorSys;

BOOL SectorSystem::BuildTileListFromRect(TileRect* tileRect, TileListEntry* tle)
{
	return addresses.BuildTileListFromRect(tileRect, tle);
}

uint64_t SectorSystem::GetSectorLimitX()
{
	return temple::GetRef<uint64_t>(0x10AB7470);
}

uint64_t SectorSystem::GetSectorLimitY()
{
	return temple::GetRef<uint64_t>(0x10AB7448);
}

BOOL SectorSystem::SectorCacheFind(SectorLoc secLoc, int* sectorCacheIdx)
{
	return addresses.SectorCacheFind(secLoc, sectorCacheIdx);
}

int SectorSystem::SectorLock(SectorLoc secLoc, Sector** sectorOut)
{
/*	int	unlockedSecIdx = -1;
	if ( *addresses.sectorLocking )
	{
	logger->warn("Warning: recursive sector lock detected\n");
	return 0;
	}
	if ( *addresses.sectorLockSthg_10AB7458 )
	return 0;

	if ( !maps.IsMapOpen() )
	{
	logger->error("ERROR: Attempt to lock a sector when the map is not valid!!!\n");
	return 0;
	}
	if (secLoc.x() >= GetSectorLimitX() || secLoc.y() >= GetSectorLimitY())
		return 0;

	
	int * secIndices = *addresses.sectorCacheIndices;
	auto secCahceIdx = secIndices[*addresses.sectorCacheIdx];
	auto secCache = *addresses.sectorCache;
	SectorCacheEntry * secCacheEntry = &secCache[secCahceIdx];

	// begin locking!
	*addresses.sectorLocking = 1;
	int lockSerial = ++*addresses.sectorLockSerial;

	if (secCache[secCahceIdx].isUsed && secCacheEntry->sector.secLoc == secLoc)
	{
		++secCacheEntry->lockCount;
		secCacheEntry->lastLockId = lockSerial;
		*sectorOut = &secCacheEntry->sector;
		++*addresses.secCountSthg_10AB74AC;
		*addresses.sectorLocking = false;
		return 1;
	}

	if (SectorCacheFind(secLoc,addresses.sectorCacheIdx))
	{
		
	}
	*addresses.sectorLocking = 0;
	*/
	return addresses.SectorLock(secLoc, sectorOut);
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