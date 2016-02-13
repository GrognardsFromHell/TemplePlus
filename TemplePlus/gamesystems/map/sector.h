#pragma once


#include "common.h"
#include <util/fixes.h>
#include "gamesystems/timeevents.h"
#include "../gamesystem.h"

struct Sector;

#pragma pack(push, 1)

struct SectorLoc
{
	uint64_t raw;

	uint64_t x()
	{
		return raw & 0x3ffFFFF;
	}

	uint64_t y()
	{
		return raw >> 26;
	}

	uint64_t ToField() {
		return this->raw;
	}

	operator uint64_t() const {
		return this->raw;
	}

	operator int64_t() const {
		return (int64_t)this->raw;
	}

	void GetFromLoc(locXY loc)
	{
		raw = loc.locx / SECTOR_SIDE_SIZE
			+ ((loc.locy / SECTOR_SIDE_SIZE) << 26);
	}

	SectorLoc()
	{
		raw = 0;
	}

	SectorLoc(locXY loc)
	{
		raw = loc.locx / SECTOR_SIDE_SIZE
			+ ((loc.locy / SECTOR_SIDE_SIZE) << 26);
	}

	SectorLoc(uint64_t sectorLoc) {
		raw = sectorLoc;
	}

	SectorLoc(int sectorX, int sectorY) {
		raw = (sectorX & 0x3ffFFFF) | (sectorY << 26);
	}

	locXY GetBaseTile()
	{
		locXY loc;
		loc.locx = (int)x() * SECTOR_SIDE_SIZE;
		loc.locy = (int)y() * SECTOR_SIDE_SIZE;
		return loc;
	}

	bool operator ==(SectorLoc secLoc) {
		return raw == secLoc.raw;
	}

};

#define TILES_PER_SECTOR SECTOR_SIDE_SIZE*SECTOR_SIDE_SIZE
struct SectorObjectsNode {
	objHndl handle;
	SectorObjectsNode *next;
};

struct SectorLightPartSys {
	int hashCode;
	int handle;
};

struct SectorLightNight {
	int type;
	uint32_t color;
	D3DVECTOR direction;
	float phi;
	SectorLightPartSys partSys;
};

struct SectorLight {
	objHndl obj;
	int flags; // 0x40 -> light2 is present
	int type;
	uint32_t color;
	int field14;
	LocAndOffsets position;
	float offsetZ;
	D3DVECTOR direction;
	float range;
	float phi;
	SectorLightPartSys partSys;
	SectorLightNight light2;
};

struct SectorLightNode {
	SectorLight *light;
	SectorLightNode *next;
};

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

struct SectorLights {
	SectorLightNode* listHead;
	BOOL enabled;
};

enum TileFlags : uint32_t
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
	TF_80000000   =   0x80000000
};

enum class TileMaterial : uint8_t {
	ReservedBlocked = 0,
	ReservedFlyOver = 1,
	Dirt = 2,
	Grass = 3, // Default
	Water = 4,
	DeepWater = 5,
	Ice = 6,
	Fire = 7,
	Wood = 8,
	Stone = 9,
	Metal = 10,
	Marsh = 11
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

struct SectorTile {
	TileMaterial material; // for footsteps
	uint8_t padding[3];
	TileFlags flags;
	uint32_t padding1;
	uint32_t padding2;
};

struct SectorTilePacket{
	SectorTile tiles[TILES_PER_SECTOR];
	uint8_t unk10000[TILES_PER_SECTOR / 8]; // this is probably a 64x64 bitmap, designating some tile state (changed? valid?)
	int changedFlagMaybe; // probably a worlded thing
	
};

struct SectorObjects {
	SectorObjectsNode* tiles[SECTOR_SIDE_SIZE][SECTOR_SIDE_SIZE];
	BOOL staticObjsDirty;
	int objectsRead;
};

struct Sector
{
	int flags; // 1 - townmapinfo  2 - aptitude  4 - lightscheme
	int field4;
	SectorLoc secLoc;
	GameTime timeElapsed;
	SectorLights lights;
	SectorTilePacket tilePkt;
	SectorTileScripts tileScripts;
	SectorScript sectorScript;
	int townmapinfo;
	int aptitudeAdj;
	int lightScheme;
	SectorSoundList soundList;
	SectorObjects objects;
	int field1425C;
	/*
	return an offset for getting a proper index in the TilePacket
	*/
	int GetTileOffset(LocAndOffsets* loc);
	TileFlags GetTileFlags(LocAndOffsets* loc);
};

const int testSizeofSector = sizeof(Sector); // should be 82528 (0x14260)

#pragma pack(pop)

class SectorLightIterator {
public:
	SectorLightIterator(SectorLightNode *first) : mCurrent(first) {}

	bool HasNext() const {
		return !!mCurrent;
	}

	SectorLight& Next() {
		auto& result = *mCurrent->light;
		mCurrent = mCurrent->next;
		return result;
	}

private:
	SectorLightNode *mCurrent;
};

class LockedMapSector {
public:
	LockedMapSector(int secX, int secY);
	LockedMapSector(SectorLoc loc);
	~LockedMapSector();

	SectorObjectsNode* GetObjectsAt(int x, int y) const;

	LockedMapSector(LockedMapSector&) = delete;
	LockedMapSector(LockedMapSector&&) = delete;
	LockedMapSector& operator=(LockedMapSector&) = delete;
	LockedMapSector& operator=(LockedMapSector&&) = delete;

	SectorLightIterator GetLights();

	void AddObject(objHndl handle);
private:
	Sector* mSector = nullptr;
};

class MapSectorSystem : public GameSystem, public BufferResettingGameSystem, public ResetAwareGameSystem, public MapCloseAwareGameSystem {
public:
	static constexpr auto Name = "MapSector";
	MapSectorSystem(const GameSystemConf &config);
	~MapSectorSystem();
	void Reset() override;
	void ResetBuffers(const RebuildBufferInfo& rebuildInfo) override;
	void CloseMap() override;
	const std::string &GetName() const override;

	void Clear();
	void SetDirectories(const std::string &dataDir, const std::string &saveDir);

	bool IsSectorLoaded(SectorLoc location);

	void RemoveSectorLight(objHndl handle);


};

struct PointAlongSegment;
struct RaycastPointSearchPacket;

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


class LegacySectorSystem : TempleFix
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


	static BOOL(__cdecl * orgSectorCacheFind)(SectorLoc secLoc, int * secCacheIdx);

	void apply() override {
		orgSectorCacheFind = replaceFunction(0x10081FA0, SectorCacheFind);
	//	replaceFunction(0x10082700, SectorLock);
	};
};

extern LegacySectorSystem sectorSys;