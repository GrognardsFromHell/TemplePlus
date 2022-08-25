#include "stdafx.h"
#include "sector.h"
#include <temple/dll.h>
#include <raycast.h>
#include <maps.h>
#include <temple/vfs.h>
#include <tio/tio.h>

#include "gamesystems/gamesystems.h"
#include "gamesystems/mapsystem.h"
#include "gamesystems/timeevents.h"
#include "gamesystems/objects/objsystem.h"

static_assert(temple::validate_size<SectorTilePacket, 66052>::value, "SectorTilePacket has incorrect size");
static_assert(temple::validate_size<TileListEntry, 112>::value, "TileListEntry has incorrect size");
static_assert(temple::validate_size<SectorObjects, 16392>::value, "SectorObjects has incorrect size");
static_assert(temple::validate_size<Sector, 0x14260>::value, "Sector has incorrect size");

struct LegacySectorAddresses : temple::AddressTable
{
	
	BOOL(__cdecl*BuildTileListFromRect)(TileRect * tileRect, TileListEntry* tle);
	int(__cdecl*SectorLock)(SectorLoc secLoc, Sector** sectOut);
	int(__cdecl*SectorCacheFindUnusedIdx)(int*secCacheIdx);
	int(__cdecl*SectorCacheFind)(SectorLoc secLoc, int* secCacheIdx);
	BOOL(__cdecl*SectorCacheLocExists)(SectorLoc secLoc);
	int(__cdecl*SectorUnlock)(SectorLoc secLoc);
	void(__cdecl* SectorCacheUnlockAll)();
	void(*SectorCacheEntryFree)(); // Sector* @<esi>

	int(__cdecl* SectorSystemInit)();

	BOOL(_cdecl**SectorSaveFunc)(Sector* sect);
	BOOL(_cdecl**SectorLoadFunc)(SectorLoc secLoc, Sector* sect);
	BOOL(_cdecl*SectorLoadFunc_GameMode)(SectorLoc secLoc, Sector* sect);
	BOOL(_cdecl*SectorLoadFunc_EditorMode)(SectorLoc secLoc, Sector* sect);

	
	BOOL(__cdecl* GetPointAlongSegmentNearestToOriginDistanceRfromV)(float absVx, float absVy, float radiusAdjAmt, RaycastPointSearchPacket * srchPkt, PointAlongSegment* pnt);
	BOOL(__cdecl* IsPointCloseToSegment)(float absVx, float absVy, float radiusAdjAmt, RaycastPointSearchPacket * srchPkt);

	GameTime * sectorLockTimeBase;

	SectorTime ** sectorTimes;
	char ** sectorFolder;
	int * sectorTimesCount;
	int *sectorLockSthg_10AB7458;
	int * sectorCacheIndicesCurIdx;
	int ** sectorCacheIndices;
	BOOL * sectorLocking;
	SectorCacheEntry ** sectorCache;
	int * sectorLockSerial;
	int * secCountSthg_10AB74AC;
	int * sectorCacheLockedCount;
	int * sectorCacheSize;

	LegacySectorAddresses()
	{
		rebase(SectorCacheEntryFree,  0x10081E10);
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

		rebase(sectorLockTimeBase,		0x102CC130);

		rebase(sectorTimes,				0x10AB73F8);
		rebase(sectorFolder,			0x10AB73FC);
		rebase(sectorCache,				0x10AB7408);
		rebase(sectorLockSerial,		0x10AB7410);
		rebase(SectorLoadFunc,			0x10AB7420);
		rebase(SectorLoadFunc_GameMode, 0x10083210);
		rebase(SectorLoadFunc_EditorMode, 0x10082E00);
		
		rebase(sectorCacheIndicesCurIdx,	0x10AB7430);
		rebase(sectorCacheLockedCount,	0x10AB7434);
		rebase(sectorCacheSize, 		0x10AB7450);
		rebase(sectorTimesCount,		0x10AB7454);
		rebase(sectorLockSthg_10AB7458, 0x10AB7458);
		rebase(sectorCacheIndices,		0x10AB7478);
		rebase(sectorLocking,			0x10AB747C);
		rebase(SectorSaveFunc,			0x10AB74A0);
		rebase(secCountSthg_10AB74AC,	0x10AB74AC);
	}
} addresses;

LegacySectorSystem  sectorSys;
BOOL(__cdecl * LegacySectorSystem::orgSectorCacheFind)(SectorLoc secLoc, int * secCacheIdx);


class SectorHooks : TempleFix
{
public:
	static int SectorTileIsBlocking_OldVersion(locXY loc, int isFlag8);
	static int HookedBuildClampedTileList(int64_t, int64_t, int , int64_t*);
	void apply() override {

		replaceFunction(0x100AC570, SectorTileIsBlocking_OldVersion);
		// replaces BuildClampedTileList in case there is a bug with the input coordinates (will probably crash elsewhere now...)
		redirectCall(0x100824FA, HookedBuildClampedTileList);
		redirectCall(0x1008252A, HookedBuildClampedTileList);
		static SectorList * (__cdecl*orgBuildSectorList)(TileRect *) = 
			replaceFunction<SectorList * (__cdecl)(TileRect *)>(0x10084650, [](TileRect * tiles)->SectorList*{
			auto result = sectorSys.BuildSectorList(tiles);
			//auto result = orgBuildSectorList(tiles);
			return result;
		});

		replaceFunction<void(__cdecl)(SectorList*)>(0x10081A30, [](SectorList* list){
			sectorSys.SectorListReturnToPool(list);
		});


		static BOOL(__cdecl * orgAddObject)(SectorObjects*, objHndl) =
			replaceFunction<BOOL(__cdecl)(SectorObjects*, objHndl)>(0x100C1AD0, [](SectorObjects* objs, objHndl handle)->BOOL {
				static auto objlistInsertInternal = temple::GetRef<BOOL(__cdecl)(SectorObjects*, objHndl)>(0x100C1A20);
				if (objlistInsertInternal(objs, handle)) {
					if (objects.IsStatic(handle))
						objs->staticObjsDirty = 1;
					return TRUE;
				}
				return FALSE;
				//return orgAddObject(objs, handle);
				});

		static BOOL(__cdecl * orgObjlistInsertInternal)(SectorObjects*, objHndl) = 
			replaceFunction<BOOL(__cdecl)(SectorObjects*, objHndl)>(0x100C1A20, [](SectorObjects*objs, objHndl handle)->BOOL {
			return orgObjlistInsertInternal(objs, handle);
			});

		static BOOL(__cdecl*orgSectorLoadObjs)(SectorObjects*, TioFile *, SectorLoc)
		=replaceFunction<BOOL(__cdecl)(SectorObjects* , TioFile* , SectorLoc )>(0x100C1B20,
			[](SectorObjects* secObjs, TioFile* file, SectorLoc sectorLoc) {
			//return orgSectorLoadObjs(secObjs, file, sectorLoc);
			return sectorSys.SectorLoadObjects(secObjs, file, sectorLoc);
		});
		
	}
} sectorHooks;

int Sector::GetTileOffset(LocAndOffsets* loc)
{
	auto baseLoc = secLoc.GetBaseTile();
	return loc->location.locx - baseLoc.locx + SECTOR_SIDE_SIZE * (loc->location.locy - baseLoc.locy);
}

TileFlags Sector::GetTileFlags(LocAndOffsets* loc)
{
	int tileOffset = GetTileOffset(loc);
	return tilePkt.tiles[tileOffset].flags;
}

BOOL LegacySectorSystem::BuildTileListFromRect(TileRect* tileRect, TileListEntry* tle)
{
	return addresses.BuildTileListFromRect(tileRect, tle);
}

SectorList* LegacySectorSystem::BuildSectorList(TileRect* tileRect){
	auto xs = temple::GetRef<int64_t*>(0x10AB7488);
	auto ys = temple::GetRef<int64_t*>(0x10AB7404);
	auto Nx = SectorHooks::HookedBuildClampedTileList(tileRect->x1, tileRect->x2 + 1, 64, xs);
	if (Nx <= 1)
		return nullptr;
	auto Ny = SectorHooks::HookedBuildClampedTileList(tileRect->y1, tileRect->y2 + 1, 64, ys);
	if (Ny <= 1)
		return nullptr;

	SectorList *prevEntry =nullptr, *entry= nullptr;
	SectorList * result = nullptr;
	auto &sectorList = temple::GetRef<SectorList*>(0x10AB7424);
	entry = sectorList;

	for (auto i_y = 0; i_y < Ny-1; i_y++){

		for (auto i_x = 0; i_x < Nx - 1; i_x++) {

			if (!entry) {
				// prepend 4 new nodes
				for (auto i = 0; i < 4; i++) {
					entry = new SectorList;
					entry->next = sectorList;
					sectorList = entry;
				}
			}
			sectorList = entry->next;
			entry->next = prevEntry; //pops the head node of sectorList
			result = entry;
			locXY loc;
			loc.locy = ys[i_y];
			loc.locx = xs[i_x];
			entry->cornerTile = loc;
			SectorLoc secLoc;
			secLoc.GetFromLoc(loc);
			entry->sector = secLoc;
			entry->extent.locx = xs[i_x + 1] - xs[i_x];
			entry->extent.locy = ys[i_y + 1] - ys[i_y];
			prevEntry = entry;
			entry = sectorList;
			
		}
	}
	

	return result;
}

void LegacySectorSystem::SectorListReturnToPool(SectorList* secList){
	auto &sectorListPool = temple::GetRef<SectorList*>(0x10AB7424);

	auto next = &secList->next;
	SectorList * lastNode = secList;

	while (*next){
		lastNode = *next;
		next = &lastNode->next;
	}
	lastNode->next = sectorListPool;
	sectorListPool = secList;
	
}

uint64_t LegacySectorSystem::GetSectorLimitX()
{
	return temple::GetRef<uint64_t>(0x10AB7470);
}

uint64_t LegacySectorSystem::GetSectorLimitY()
{
	return temple::GetRef<uint64_t>(0x10AB7448);
}

BOOL LegacySectorSystem::SectorCacheFind(SectorLoc secLoc, int* secCacheIdx)
{

	int result = orgSectorCacheFind(secLoc, secCacheIdx);

	SectorCacheEntry * secCache = *addresses.sectorCache;
	int * secCacheIndices = *addresses.sectorCacheIndices;


	static SectorLoc lastSecLoc;
	static int idxMid;
	//static int searchDepthHist[128];
	if (secLoc == lastSecLoc)
	{
		if (secCache[secCacheIndices[idxMid]].sector.secLoc == secLoc)
		{
			*secCacheIdx = idxMid;
			//searchDepthHist[0]++;
			return 1;
		}
	}

	int idxLo=0;
	int idxHi = *addresses.sectorCacheLockedCount - 1;

	if (idxHi < 0)
	{
		*secCacheIdx = idxLo;
		return 0;
	}

	int searchCount = 0;
	while(idxHi >= idxLo)
	{
		searchCount++;
		idxMid = (idxLo + idxHi) / 2;
		int idx = secCacheIndices[idxMid];
		if ( secCache[idx].sector.secLoc.raw >= secLoc.raw)
		{
			if (secCache[idx].sector.secLoc.raw == secLoc.raw)
			{
				if (idxMid != *secCacheIdx)
				{
					int dummy = 1;
				}
				*secCacheIdx = idxMid;
				lastSecLoc = secLoc;
				//searchDepthHist[searchCount]++;
				return 1;
				
			} else
			{
				idxHi = idxMid - 1;
			}
		} else
		{
			idxLo = idxMid + 1;
		}
	}
	*secCacheIdx = idxLo;
	return 0;
	// return addresses.SectorCacheFind(secLoc, secCacheIdx);
}

void LegacySectorSystem::SectorSave(Sector* sect)
{
	(*addresses.SectorSaveFunc)(sect);
}

BOOL LegacySectorSystem::SectorLoad(SectorLoc secLoc, Sector* sect)
{
	return (*addresses.SectorLoadFunc)(secLoc, sect);
}

BOOL LegacySectorSystem::SectorLoadObjects(SectorObjects* secObjs, TioFile* file, SectorLoc sectorLoc){

	auto initPos = tio_ftell(file);
	if (initPos == -1)
		return 0;
	int32_t objCount = 0;
	tio_fseek(file, -4, 2);
	if ( tio_fread(&objCount, 4, 1, file) != 1)
		return 0;
	tio_fseek(file, initPos, 0);

	
	secObjs->objectsRead = 0;
	if((int64_t)sectorLoc.raw == -1){
		
	}else{
		auto dummy=- 1;
	}
	auto numRead = 0;
	for (auto i=0; i < objCount; i++){
		objHndl handle;
		try{
		 handle = objSystem->LoadFromFile(file);
		}
		catch ( TempleException &e) {
			break;
		}
		if ((int64_t)sectorLoc.raw!= -1){
			auto loc = objects.getInt64(handle, obj_f_location);
		/*	v12 = GetSectorTileCoords(v11, SBYTE4(v11));
			BYTE4(v7) = v12;
			v13 = _longint_mul(__PAIR__(v9, v10) + (v12 >> 6), 0x100000000i64);
			v17 = HIDWORD(v13);
			v14 = BYTE4(v7);
			HIDWORD(v7) = v20;
			setInt64(handle, obj_f_location, (v7 + (v14 & 0x3F)) | v13);
			PropCollectionReset(handle);
			v6 = sectorLoc;
			*/
		}
		objSystem->UnfreezeIds(handle);
		objects.setInt32(handle, obj_f_temp_id, secObjs->objectsRead);
		auto ObjlistInsertInternal = temple::GetRef<BOOL(__cdecl)(SectorObjects*, objHndl)>(0x100C1A20);
		if (!ObjlistInsertInternal(secObjs, handle))
		{
			break;
		}
			
		++secObjs->objectsRead;
	}
	if (tio_fread(&objCount, 4, 1, file) != 1)
	{
		return 0;
	}
		

	if (secObjs->objectsRead != objCount){
		auto destryObjList=temple::GetRef<void(__cdecl)(SectorObjects*)>(0x100C1360);
		destryObjList(secObjs);
		return FALSE;
	}
	secObjs->staticObjsDirty = 0;
	return TRUE;
}

bool LegacySectorSystem::SectorFileExists(SectorLoc secLoc)
{
	char sectorFileName[256]={0,};
	
	strcpy(sectorFileName, *addresses.sectorFolder);
	int strLength = strlen(sectorFileName);
	sectorFileName[strLength++] = '\\';
	_ui64toa(secLoc.raw, &sectorFileName[strLength], 10);
	strLength = strlen(sectorFileName);
	strcpy(&sectorFileName[strLength], ".sec");
	
	if (tio_fileexists(sectorFileName))
		return true;

	return false;
}

void __declspec(naked) LegacySectorSystem::SectorCacheEntryFree(Sector* sect)
{
	{  __asm push esi  __asm push edi};
	__asm {
		mov esi, sect;
		mov edi, addresses.SectorCacheEntryFree;
		call edi;
	}
	{ __asm pop edi  __asm pop esi } 

}

BOOL LegacySectorSystem::SectorCacheFindUnusedIdx(int* idxUnused)
{
	int secCacheSize = *addresses.sectorCacheSize;
	auto cacheEntry = *addresses.sectorCache;
	for (int i = 0; i < secCacheSize;i++)
	{
		if (cacheEntry[i].isUsed == 0)
		{
			*idxUnused = i;
			return true;
		}
	}
	return false;
	// return addresses.SectorCacheFindUnusedIdx(idxUnused);
}

int LegacySectorSystem::SectorLock(SectorLoc secLoc, Sector** sectorOut)
{
	int	unlockedSecIdx = -1;
	static int sectorCacheIndicesCurIdx = 0;

	if ( *addresses.sectorLocking )
	{
	logger->warn("Warning: recursive sector lock detected\n");
	return 0;
	}
	if ( *addresses.sectorLockSthg_10AB7458 )
	return 0;

	if ( !gameSystems->GetMap().IsMapOpen() )
	{
	logger->error("ERROR: Attempt to lock a sector when the map is not valid!!!\n");
	return 0;
	}
	if (secLoc.x() >= GetSectorLimitX() || secLoc.y() >= GetSectorLimitY())
		return 0;

	
	int * secIndices = *addresses.sectorCacheIndices;
	auto secCacheIdx = secIndices[sectorCacheIndicesCurIdx];
	auto secCache = *addresses.sectorCache;
	SectorCacheEntry * secCacheEntry = &secCache[secCacheIdx];

	// begin locking!
	*addresses.sectorLocking = 1;
	int lockSerial = ++*addresses.sectorLockSerial;

	if (secCache[secCacheIdx].isUsed && secCacheEntry->sector.secLoc == secLoc)
	{
		++secCacheEntry->lockCount;
		secCacheEntry->lastLockId = lockSerial;
		*sectorOut = &secCacheEntry->sector;
		++*addresses.secCountSthg_10AB74AC;
		*addresses.sectorLocking = false;
		return 1;
	}

	if (SectorCacheFind(secLoc , &sectorCacheIndicesCurIdx))
	{
		auto secEntry = &secCache[secIndices[sectorCacheIndicesCurIdx]];
		secEntry->lockCount++;
		secEntry->lastLockId = lockSerial;
		*sectorOut = &secCache[secIndices[sectorCacheIndicesCurIdx]].sector;
		++*addresses.secCountSthg_10AB74AC;
		*addresses.sectorLocking = false;
		return 1;
	}

	if (*addresses.sectorCacheLockedCount >= *addresses.sectorCacheSize){
		for (int i = 0; i < *addresses.sectorCacheSize; i++)
		{
			int _secIdx = secIndices[i];
			int secCacheEntryLockCnt = secCache[_secIdx].lockCount;
			if (secCacheEntryLockCnt != 0) continue;
			if (unlockedSecIdx == -1)
			{
				unlockedSecIdx = i;
				continue;
			}
			if (secCache[_secIdx].lastLockId < secCache[secIndices[unlockedSecIdx]].lastLockId )
				unlockedSecIdx = i;
		}
		if (unlockedSecIdx == -1 || *addresses.sectorCacheSize == 0)
		{
			logger->error("Warning: attempt to lock sector in cache failed due to lack of unlocked slots available.  This is bad.  Help.");
			*addresses.sectorLocking = false;
			return 0;
		}
		logger->info("Sector Cache full, removing oldest unlocked entry (%I64u)...",secCache[secIndices[unlockedSecIdx]].sector.secLoc.raw);
		auto unlockedEntry = &secCache[secIndices[unlockedSecIdx]];
		SectorSave(&unlockedEntry->sector);
		SectorCacheEntryFree(&unlockedEntry->sector);
		unlockedEntry->isUsed = 0;
		int * idxToRemove = addresses.sectorCacheIndices[unlockedSecIdx];
		memcpy(idxToRemove, idxToRemove + 1, sizeof(int) * (*addresses.sectorCacheLockedCount - unlockedSecIdx - 1) );
		--*addresses.sectorCacheLockedCount;
		SectorCacheFind(secLoc, &sectorCacheIndicesCurIdx);
	}

	int idxUnused;
	if ( !SectorCacheFindUnusedIdx(&idxUnused))
	{
		logger->error("SectorCacheFindUnused failed to find an available slot in the art cache!\n");
		*addresses.sectorLocking = false;
		return 0;
	}

	GameTime gameTimeElapsed = *addresses.sectorLockTimeBase;
	SectorTime * sectorTimes = *addresses.sectorTimes;
	for (int i = 0; i < *addresses.sectorTimesCount; i++)
	{
		auto time = &sectorTimes[i];
		if ( time->secLoc == secLoc )
		{
			gameTimeElapsed = gameTimeSys.ElapsedGetDelta(&sectorTimes[i].gameTime);
			memcpy(&sectorTimes[i], &sectorTimes[i + 1], sizeof(SectorTime) * (*addresses.sectorTimesCount - i - 1));
			--*addresses.sectorTimesCount;
			break;
		}
	}


	secCache[idxUnused].sector.timeElapsed = gameTimeElapsed;
	if (!SectorLoad(secLoc, &secCache[idxUnused].sector))
	{
		logger->error("attempt to lock sector %I64u in cache failed due to error in load.  This is bad.  Help.\n", secLoc.raw);
		*addresses.sectorLocking = false;
		return false;
	}


	memcpy(&(*addresses.sectorCacheIndices)[sectorCacheIndicesCurIdx+1],
		&(*addresses.sectorCacheIndices)[sectorCacheIndicesCurIdx],
		sizeof(int) * (*addresses.sectorCacheLockedCount - sectorCacheIndicesCurIdx));
	secIndices[sectorCacheIndicesCurIdx] = idxUnused;
	secCacheEntry = &secCache[idxUnused];
	secCacheEntry->lockCount = 1;
	secCacheEntry->isUsed = 1;
	secCacheEntry->lastLockId = lockSerial;
	secCacheEntry->sector.secLoc = secLoc;
	++*addresses.sectorCacheLockedCount;
	*sectorOut = &secCacheEntry->sector;
	++*addresses.secCountSthg_10AB74AC;
	*addresses.sectorLocking = false;
	return true;
	
	// return addresses.SectorLock(secLoc, sectorOut);
}

int LegacySectorSystem::SectorUnlock(SectorLoc secLoc)
{
	return addresses.SectorUnlock(secLoc);	
}

BOOL LegacySectorSystem::GetSegmentInterception(float absVx, float absVy, float radiusAdjAmt, RaycastPointSearchPacket* srchPkt, PointAlongSegment* pnt)
{
	return addresses.GetPointAlongSegmentNearestToOriginDistanceRfromV(absVx, absVy, radiusAdjAmt, srchPkt, pnt);
}

BOOL LegacySectorSystem::IsPointInterceptedBySegment(float absVx, float absVy, float radiusAdjAmt, RaycastPointSearchPacket* srchPkt)
{
	//return addresses.IsPointCloseToSegment(absVx, absVy, radiusAdjAmt, srchPkt);
	return srchPkt->IsPointInterceptedBySegment({ absVx, absVy }, radiusAdjAmt);
}

TileFlags LegacySectorSystem::GetTileFlags(LocAndOffsets loc)
{
	auto flags = TileFlags::TILEFLAG_NONE;
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

	SectorUnlock(secLoc);
	return flags;
}

bool LegacySectorSystem::SetTileFlags(LocAndOffsets loc, TileFlags flags)
{
	SectorLoc secLoc(loc.location);
	Sector* sect;
	int lockStatus = SectorLock(secLoc, &sect);
	if (!lockStatus){
		return false;
	}

	locXY baseTile = secLoc.GetBaseTile();
	int deltaX = loc.location.locx - baseTile.locx;
	int deltaY = loc.location.locy - baseTile.locy;

	int tileIdx = deltaX + SECTOR_SIDE_SIZE * deltaY;

	sect->tilePkt.tiles[tileIdx].flags = flags;

	SectorUnlock(secLoc);
	return true;
}

static int64_t MakeSectorLoc(int x, int y) {
	return ((int64_t)(y & 0x3FFFFFF) << 26) | (x & 0x3FFFFFF);
}

LockedMapSector::LockedMapSector(int secX, int secY) {

	if (!addresses.SectorLock({ secX, secY }, &mSector)) {
		logger->warn("Unable to lock sector {}, {}", secX, secY);
		mSector = nullptr;
	}

}

LockedMapSector::LockedMapSector(SectorLoc loc) : LockedMapSector((int) loc.x(), (int) loc.y())
{
}

LockedMapSector::~LockedMapSector() {

	if (mSector) {
		addresses.SectorUnlock(mSector->secLoc);
	}

}

SectorObjectsNode* LockedMapSector::GetObjectsAt(int x, int y) const {
	Expects(x >= 0 && x < 64);
	Expects(y >= 0 && y < 64);

	return mSector->objects.tiles[x + y * SECTOR_SIDE_SIZE];
}

SectorLightIterator LockedMapSector::GetLights() {
	return SectorLightIterator(mSector->lights.listHead);
}

void LockedMapSector::AddObject(objHndl handle)
{
	static auto map_sector_objects_add = temple::GetPointer<BOOL(SectorObjects*objects, objHndl ObjHnd)>(0x100c1ad0);
	map_sector_objects_add(&mSector->objects, handle);
}


//*****************************************************************************
//* MapSector
//*****************************************************************************

MapSectorSystem::MapSectorSystem(const GameSystemConf &config) {
	auto startup = temple::GetPointer<int(const GameSystemConf*)>(0x10084930);
	if (!startup(&config)) {
		throw TempleException("Unable to initialize game system MapSector");
	}
}
MapSectorSystem::~MapSectorSystem() {
	auto shutdown = temple::GetPointer<void()>(0x10084130);
	shutdown();
}
void MapSectorSystem::ResetBuffers(const RebuildBufferInfo& rebuildInfo) {
	auto resetBuffers = temple::GetPointer<void(const RebuildBufferInfo*)>(0x10081560);
	resetBuffers(&rebuildInfo);
}
void MapSectorSystem::Reset() {
	auto reset = temple::GetPointer<void()>(0x10084120);
	reset();
}
void MapSectorSystem::CloseMap() {
	auto mapClose = temple::GetPointer<void()>(0x100842e0);
	mapClose();
}
const std::string &MapSectorSystem::GetName() const {
	static std::string name("MapSector");
	return name;
}

void MapSectorSystem::Clear()
{
	static auto clear = temple::GetPointer<void()>(0x10082B90);
	clear();
}

void MapSectorSystem::SetDirectories(const std::string & dataDir, const std::string & saveDir)
{
	static auto set_sector_data_dir = temple::GetPointer<void(const char *dataDir, const char *saveDir)>(0x10082670);
	set_sector_data_dir(dataDir.c_str(), saveDir.c_str());
}

bool MapSectorSystem::IsSectorLoaded(SectorLoc location)
{
	static auto sector_is_loaded = temple::GetPointer<BOOL(SectorLoc)>(0x100826b0);
	return sector_is_loaded(location) == TRUE;
}

void MapSectorSystem::RemoveSectorLight(objHndl handle)
{
	// Not clear if this has *any* effect
	static auto map_sector_reset_sectorlight = temple::GetPointer<void(objHndl)>(0x100a8590);
	map_sector_reset_sectorlight(handle);
}

int SectorHooks::SectorTileIsBlocking_OldVersion(locXY loc, int regardSinks){
	// should always return 0 in ToEE...
	auto tileFlags = sectorSys.GetTileFlags({ loc, 0,0 });
	if (!regardSinks){
		if (tileFlags & (TileFlags::TF_Blocks | TileFlags::TF_CanFlyOver)){
			int dummy = 1;
		}
		return tileFlags & (TileFlags::TF_Blocks | TileFlags::TF_CanFlyOver);
	} else
	{
		int dummy = 1;
	}
		

	return 0;
}

int SectorHooks::HookedBuildClampedTileList(int64_t startLoc, int64_t endLoc, int increment, int64_t* locsOut){
	locsOut[0] = startLoc;
	int64_t nextLoc = (startLoc / increment + 1)*increment;
	auto locsOutIterator = locsOut+1;
	int startLocInt = (int)(startLoc & 0xFFFFffff), endLocInt = (int)(endLoc & 0xFFFFffff);

	if (endLoc > startLoc + 3*increment){ // fixes buffer overflow issue due to output buffer size being 5. This caps it to: { startLoc, [startLoc/64]+64, [startLoc/64]+128, endLoc }
		logger->error("Large endLoc detected! (start: {} ({}) end: {} ({})). Truncating.",startLoc,startLocInt, endLoc, endLocInt);
		endLoc = startLoc + 3 * increment;
		logger->info("new endLoc: {}", endLoc);
	}

	int count = 0;
	while (nextLoc < endLoc){
		if (++count >= 4) {
			logger->error("HookedBuildClampedTileList: too much! Halting at {}.", nextLoc);
			break;
		}
		*locsOutIterator = nextLoc;
		locsOutIterator++;
		nextLoc += increment;
		
	}
	*locsOutIterator = endLoc;
	auto diff = (int)locsOutIterator - (int)locsOut + 8;
	auto tileListSize = diff / 8;
	Expects(tileListSize <= 5);
	return tileListSize;
}
