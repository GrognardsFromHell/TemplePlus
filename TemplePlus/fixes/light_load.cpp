#include "stdafx.h"
#include "util/fixes.h"
#include <obj.h>
#include <temple_functions.h>
#include <timeevents.h>
#include <particles.h>

/*
	This is the fix for bug #104, which causes particle systems to be
	duplicated when a diff file for a sector is loaded alongside the
	sector.

	ToEE first reads the light from the original sector @ 0x101063C3,
	apparently to get it's position. Then it reads the light from the
	diff file @ 0x10106452, *without* freeing the first light. This
	means the memory of the first light leaks (miniscule) and 
	the particle systems created in the load function will also leak (major issue).

	The correct solution would be to a) free the light and b) not even create
	the particle systems in the first load function. This fix will simply
	free the light and kill the particle systems in the load-light-from-diff 
	function as a quick fix.
*/

struct TioFile;

#pragma pack(push, 1)
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

struct SectorTile {
	int maybeFootstepSound;
	int field4;
	int field8;
	int fieldc;
};

struct SectorLightListEntry {
	SectorLight* light;
	SectorLightListEntry* next;
};

struct SectorLights {
	SectorLightListEntry* listHead;
	BOOL enabled;
};

struct SectorTiles {
	SectorTile tiles[4096]; // 64x64
	int unk[128];
	int unkCount;
};

struct SectorTileScripts {

};

struct SectorScript {

};

struct Sector {
	int unknown;
	int unknown4;
	locationSec location;
	GameTime time;
	SectorLights lights;
	SectorTiles tiles;
	/*		0001023C townmapInfo     dd ?
		00010240 aptitudeAdj     dd ?
		00010244 lightScheme     dd ?
		00010248 soundlist       sector_soundlist ?
		00010254 objects         sector_objects ?
		0001425C field_1425C     dd ?*/
};

#pragma pack(pop)

static class SectorLoadLightFix : TempleFix {
public:
	const char* name() override {
		return "Fix for duplicate particle systems";
	}

	void apply() override;

private:

	static int ReadLightFromDiff(TioFile* file, SectorLight** lightOut);
	static int (*OrgReadLightFromDiff)(TioFile*, SectorLight**);

} sectorCacheFix;

int (*SectorLoadLightFix::OrgReadLightFromDiff)(TioFile*, SectorLight**);

void SectorLoadLightFix::apply() {
	OrgReadLightFromDiff = replaceFunction(0x100A8050, ReadLightFromDiff);
}

int SectorLoadLightFix::ReadLightFromDiff(TioFile* file, SectorLight** lightOut) {

	/*
		ToEE would let the light read from the original sector file leak,
		so we take care of it here and destroy the light and the particle
		systems that might have been created.
	*/
	auto lightOrg = *lightOut;
	if (lightOrg) {
		if (lightOrg->flags & 0x50) {
			auto partSys1 = lightOrg->partSys;
			if (partSys1.id) {
				particles.Kill(partSys1.id);
			}
		}
		if (lightOrg->flags & 0x40) {
			auto partSys2 = lightOrg->light2.partSys;
			if (partSys2.id) {
				particles.Kill(partSys2.id);
			}
		}

		free(lightOrg);
	}

	return OrgReadLightFromDiff(file, lightOut);
}
