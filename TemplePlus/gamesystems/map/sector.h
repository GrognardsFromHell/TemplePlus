#pragma once


#include "common.h"
#include <util/fixes.h>
#include <timeevents.h>




#define TILES_PER_SECTOR SECTOR_SIDE_SIZE*SECTOR_SIDE_SIZE

struct PointAlongSegment;
struct RaycastPointSearchPacket;

#pragma pack(push,1)

struct TileListEntry
{
	union
	{
		uint32_t Ny; // number of sectors spanned in the Y direction minus one ( -1 ) 
					// is zero when only a single sector is invovled
		int yExtent; // the delta Y for the y-axis sector span; can be between 0 and 64
	};
	int pad0;
	int32_t Nx; // same as above but for 
	int pad1;
	locXY tileLocs[4]; 
	SectorLoc sectorLocs[4]; 
	int sectorTileCoords[4]; // used as an index of a 2D array in the sectorTiles array
	int xExtents[4]; // the delta X's for the x-axis sector span from the BuildTileList function; is 64 when a whole sector is spanned, otherwise it denotes the internal sector X coord span
};

struct SectorLightPartSys {
	int hashCode;
	int id;
};

struct SectorLight2 {
	int field0;
	int field4;
	int field8;
	int fieldc;
	int field10;
	int field14;
	int field18;
	SectorLightPartSys partSys;
};

struct SectorLight {
	objHndl obj;
	int flags;
	int fieldc;
	int field10;
	int field14;
	int field18;
	int field1c;
	int field20;
	int field24;
	float offsetz;
	int field2c;
	int field30;
	int field34;
	int field38;
	int field3c;
	SectorLightPartSys partSys;
	SectorLight2 light2;
};

struct SectorLightListEntry {
	SectorLight* light;
	SectorLightListEntry* next;
};

struct SectorLights {
	SectorLightListEntry* listHead;
	BOOL enabled;
};

enum TileFlags : uint64_t
{
	TILEFLAG_NONE = 0,
	TF_1 = 1,
	TF_2 = 2,
	TF_4 = 4,
	TF_8 = 8,
	TF_10 = 0x10,
	TF_20 = 0x20,
	TF_40 = 0x40,
	TF_80 = 0x80,
	TF_100 = 0x100,
	BlockX0Y0 = 0x200,
	BlockX1Y0 = 0x400,
	BlockX2Y0 = 0x800,
	BlockX0Y1 = 0x1000,
	BlockX1Y1 = 0x2000,
	BlockX2Y1 = 0x4000,
	BlockX0Y2 = 0x8000,
	BlockX1Y2 = 0x10000,
	BlockX2Y2 = 0x20000,
	FlyOverX0Y0 = 0x40000, //indices denote the subtile locations (using the same axis directions as the normal tiles)
	FlyOverX1Y0 = 0x80000,
	FlyOverX2Y0 = 0x100000,
	FlyOverX0Y1 = 0x200000,
	FlyOverX1Y1 = 0x400000,
	FlyOverX2Y1 = 0x800000,
	FlyOverX0Y2 = 0x1000000,
	FlyOverX1Y2 = 0x2000000,
	FlyOverX2Y2 = 0x4000000,
	ProvidesCover = 0x8000000, //applies to the whole tile apparently
	TF_10000000 = 0x10000000,
	TF_20000000 = 0x20000000,
	TF_40000000 = 0x40000000,
	TF_80000000   =   0x80000000,

	TF_100000000  =  0x100000000,
	TF_200000000  =  0x200000000,
	TF_400000000  =  0x400000000,
	TF_800000000  =  0x800000000,
	TF_1000000000 = 0x1000000000,
	TF_2000000000 = 0x2000000000,
	TF_4000000000 = 0x4000000000,
	TF_8000000000  =  0x8000000000,
	TF_10000000000 = 0x10000000000,
	TF_20000000000 = 0x20000000000,
	TF_40000000000 = 0x40000000000,
	TF_80000000000 = 0x80000000000,
	TF_100000000000 = 0x100000000000,
	TF_200000000000 = 0x200000000000,
	TF_400000000000 = 0x400000000000,
	TF_800000000000 = 0x800000000000
};


struct SectorTile{
	int maybeFootstepSound;
	TileFlags flags;
	int flags3;
};

struct SectorTilePacket{
	SectorTile tiles[TILES_PER_SECTOR];
	char unk10000[TILES_PER_SECTOR / 8]; // this is probably a 64x64 bitmap, designating some tile state (changed? valid?)
	int changedFlagMaybe; // probably a worlded thing
	
};

const int testSizeofSectorTilePacket = sizeof(SectorTilePacket); // should be 66052 (0x10204)
#pragma pack(pop)

const int testSizeofTileListEntry = sizeof(TileListEntry); // should be 112 (0x70)


struct TileScript
{
	int field0;
	int field4;
	int field8;
	int fieldC;
	int field10;
	TileScript* next;
};

struct SectorTileScriptPacket
{
	int field0;
	TileScript * tilescriptlist;
};

struct SectorScriptPacket
{
	int field0;
	int data1;
	int data2;
	int data3;
};

struct SectorSoundListPacket
{
	int field0;
	int field4;
	int field8;
};

struct ObjectNode
{
	objHndl obj;
	ObjectNode * next;
	int fieldc; //pad?
};

struct SectorObjectsPacket
{
	ObjectNode * objNodes[TILES_PER_SECTOR];
	int field4000;
	int objectsRead;
};
const int testSizeofSectorObjectsPacket = sizeof(SectorObjectsPacket); // should be 16392 (0x4008)

struct Sector
{
	int flags; // 1 - townmapinfo  2 - aptitude  4 - lightscheme
	int field4;
	SectorLoc secLoc;
	GameTime timeElapsed;
	SectorLights lights;
	SectorTilePacket tilePkt;
	SectorTileScriptPacket tileScripts;
	SectorScriptPacket sectorScripts;
	int townmapinfo;
	int aptitudeAdj;
	int lightScheme;
	SectorSoundListPacket soundList;
	SectorObjectsPacket objects;
	int field1425C;
	/*
	return an offset for getting a proper index in the TilePacket
	*/
	int GetTileOffset(LocAndOffsets* loc);
	TileFlags GetTileFlags(LocAndOffsets* loc);
};

const int testSizeofSector = sizeof(Sector); // should be 82528 (0x14260)


struct SectorCacheEntry
{
	int isUsed;
	int lockCount;
	int lastLockId;
	int field_C;
	Sector sector;
};

struct SectorTime // used as a timestap for sector locking
{
	SectorLoc secLoc;
	GameTime gameTime;
};


class SectorSystem : TempleFix
{
public:
	const char* name() override
	{
		return "Sector Function Replacements";
	}

	/*
	builds a list of TileListEntry's for every sector contained in the  TileRect (including partially contained sectors)
	*/
	static BOOL BuildTileListFromRect(TileRect* tileRect, TileListEntry* tle);

	static uint64_t GetSectorLimitX();
	static uint64_t GetSectorLimitY();
	static BOOL SectorCacheFind(SectorLoc secLoc, int* sectorCacheIdx);
	/* 
	save sector to hdd; uses a different function in editor mode 
	*/
	static void SectorSave(Sector* sect);
	/*
	load sector from hdd; uses a different function in editor mode
	NOTE: will return TRUE for non-existant file! (it just won't load any actual data)
	*/
	static BOOL SectorLoad(SectorLoc secLoc, Sector* sect);
	static bool SectorFileExists(SectorLoc secLoc);

	static void SectorCacheEntryFree(Sector* sect);
	static BOOL SectorCacheFindUnusedIdx(int * idxUnused);
	/*
	loads from file into a cache (with a recency information so old sector get unlocked first)
	*/
	static int SectorLock(SectorLoc secLoc, Sector** sect); // 
	static int SectorUnlock(SectorLoc secLoc);

	/*
	checks if the point V (given in gamespace absolute coordinates absVx, absVy)
	is within the radius + radiusAdjAmt of the specified segment
	if so desired, it will also return the first point where this occurs (i.e. the intersection point)
	at the PointAlongSegment output
	*/
	static BOOL GetSegmentInterception(float absVx, float absVy, float radiusAdjAmt, RaycastPointSearchPacket * srchPkt, PointAlongSegment* pnt);
	/*
	Simplified versio of the above, that does not return the intersection point
	*/
	static BOOL IsPointInterceptedBySegment(float absVx, float absVy, float radiusAdjAmt, RaycastPointSearchPacket * srchPkt);

	TileFlags GetTileFlags(LocAndOffsets loc);

	bool GetTileFlagsArea(TileRect * tileRect, Subtile *out, int * count);


	static BOOL(__cdecl * orgSectorCacheFind)(SectorLoc secLoc, int * secCacheIdx);

	void apply() override {
		orgSectorCacheFind = replaceFunction(0x10081FA0, SectorCacheFind);
	//	replaceFunction(0x10082700, SectorLock);
	};
};

extern SectorSystem sectorSys;