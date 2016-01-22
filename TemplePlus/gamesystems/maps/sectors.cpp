#include "stdafx.h"
#include "sectors.h"

#include <temple/dll.h>

#include "gamesystems/timeevents.h"

static int64_t MakeSectorLoc(int x, int y) {
	return ((int64_t)(y & 0x3FFFFFF) << 26) | (x & 0x3FFFFFF);
}

struct SectorTile {
	int maybeFootstepSound;
	int field04;
	int field08;
	int field0c;
};

struct SectorTiles {
	SectorTile tiles[64][64];
	int unk[128];
	int unk1Count;
};

struct SectorLights {
	SectorLightNode* head;
	BOOL enabled;
};

struct SectorTileScript {
	int field00;
	int field04;
	int field08;
	int field0c;
	int field10;
	SectorTileScript* next;
};

struct SectorTileScripts {
	int field0;
	SectorTileScript* listHead;
};

struct SectorScript {
	int field0;
	int data1;
	int data2;
	int data3;
};

struct SectorSoundList {
	int field00;
	int field04;
	int field08;
};

struct SectorObjects {
	SectorObjectsNode* tiles[64][64];
	BOOL staticObjsDirty;
	int objectsRead;
};

struct Sector {
	int field_0;
	int field_4;
	int64_t loc;
	GameTime time;
	SectorLights lights;
	SectorTiles tiles;
	SectorTileScripts tileScripts;
	SectorScript sectorScript;
	int townmapInfo;
	int aptitudeAdj;
	int lightscheme;
	SectorSoundList sectorSounds;
	SectorObjects objects;

	/*
	0001022C sectorscript    sector_sectorscript ?
	0001023C townmapInfo     dd ?
	00010240 aptitudeAdj     dd ?
	00010244 lightScheme     dd ?
	00010248 soundlist       sector_soundlist ?
	00010254 objects         sector_objects ?
	0001425C field_1425C     dd ?*/
};

static struct SectorAddresses : public temple::AddressTable {
public:

	BOOL (*MapSectorLock)(uint64_t sectorLoc, Sector** pSectorOut);
	BOOL (*MapSectorUnlock)(uint64_t sectorLoc);

	SectorAddresses() {
		rebase(MapSectorLock, 0x10082700);
		rebase(MapSectorUnlock, 0x10082B40);
	}
} addresses;

LockedMapSector::LockedMapSector(int secX, int secY) {

	auto sectorLoc = MakeSectorLoc(secX, secY);

	if (!addresses.MapSectorLock(sectorLoc, &mSector)) {
		logger->warn("Unable to lock sector {}, {}", secX, secY);
		mSector = nullptr;
	}

}

LockedMapSector::LockedMapSector(SectorLoc loc) : LockedMapSector(loc.x(), loc.y())
{
}

LockedMapSector::~LockedMapSector() {

	if (mSector) {
		addresses.MapSectorUnlock(mSector->loc);
	}

}

SectorObjectsNode* LockedMapSector::GetObjectsAt(int x, int y) const {
	Expects(x >= 0 && x < 64);
	Expects(y >= 0 && y < 64);

	return mSector->objects.tiles[x][y];
}

SectorLightIterator LockedMapSector::GetLights() {
	return SectorLightIterator(mSector->lights.head);
}

void LockedMapSector::AddObject(objHndl handle)
{
	static auto map_sector_objects_add = temple::GetPointer<BOOL(SectorObjects*objects, objHndl ObjHnd)>(0x100c1ad0);
	map_sector_objects_add(&mSector->objects, handle);
}
