
#pragma once

#include "common.h"
struct ObjListResult;

enum ObjectListFilter {
	OLC_NONE = 0,
	OLC_PORTAL = 2,
	OLC_CONTAINER = 4,
	OLC_SCENERY = 8,
	OLC_PROJECTILE = 0x10,
	OLC_WEAPON = 0x20,
	OLC_AMMO = 0x40,
	OLC_ARMOR = 0x80,
	OLC_MONEY = 0x100,
	OLC_FOOD = 0x200,
	OLC_SCROLL = 0x400,
	OLC_KEY = 0x800,
	OLC_BAG = 0x1000,
	OLC_WRITTEN = 0x2000,
	OLC_GENERIC = 0x4000,
	OLC_ITEMS = 0x7FE0,
	OLC_PC = 0x8000,
	OLC_NPC = 0x10000,
	OLC_CRITTERS = 0x18000,
	OLC_MOBILE = 0x1FFF4,
	OLC_TRAP = 0x20000,
	OLC_IMMOBILE = 0x2000A,
	OLC_ALL = 0x3FFFE,
	OLC_PATH_BLOCKER = 0x18006 // added for pathfinding purposes
};

#pragma pack(push, 1)


#pragma pack(pop)

class ObjList {
public:
	ObjList();
	~ObjList();

	/*
		Searches for everything on a single tile that matches the given search flags.
	*/
	void ListTile(locXY loc, int flags);


	/*
		search within worldspace rect
	*/
	void ListRect(TileRect &trect, ObjectListFilter olcCritters);

	/*
		I believe this searches for all objects that would be visible if the screen was
		centered on the given tile.
	*/
	void ListVicinity(locXY loc, int flags);
	void ListVicinity(objHndl handle, int flags); // using the object's location

	/*
		Lists objects in a radius. This seems to be the radius in the X,Y 3D coordinate
		space.
	*/
	void ListRadius(LocAndOffsets loc, float radius, int flags);

	/*
	Lists objects in a radius + angles. This seems to be the radius in the X,Y 3D coordinate
	space.
	*/
	void ListRange(LocAndOffsets loc, float radius, float angleMin, float angleMax, int flags);


	/*
		Lists objects in a cone. This seems to be the radius in the X,Y 3D coordinate
		space.
	*/
	void ListCone(LocAndOffsets loc, float radius, float coneStartAngleRad, float coneArcRad, int flags);

	/*
		Lists all followers (and their followers).
	*/
	void ListFollowers(objHndl critter);

	int size();
	objHndl get(int idx) {
		auto item = mResult.objects;
		for (int i = 1; i <= idx ; i++) {
			item = item->next;
		}
		return item->handle;
	}
	objHndl operator[](int idx) {
		return get(idx);
	}


private:
	ObjListResult mResult;
	bool mHasToFree = false;
	int mSize;
	bool mSizeValid = false;

	int CountObjects() const;
	void FreeResult();

	// No copy
	ObjList(const ObjList &other) = delete;
	ObjList& operator=(const ObjList &other) = delete;
};
