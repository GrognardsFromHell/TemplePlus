
#pragma once

#include <obj.h>

struct Sector;

#pragma pack(push, 1)
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


