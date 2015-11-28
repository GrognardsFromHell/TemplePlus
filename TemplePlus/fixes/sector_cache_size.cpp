#include "stdafx.h"
#include "util/fixes.h"
#include "config/config.h"

/*
	Overrides the sector cache size which defaults to 16 with the size from
	the TemplePlus configuration file (min. 8, max. 128).

	This makes better use of modern computer's memory and greatly improves
	the performance when scrolling around a large map, because ToEE will no
	longer have to unload/load sectors constantly.
*/
static class SectorCacheSizeFix : TempleFix {
public:
	const char* name() override {
		return "Sector Cache Size Fix";
	}

	void apply() override {
		OrgInitSectorCache = replaceFunction(0x10084300, InitSectorCache);
	}

private:

	static int (*OrgInitSectorCache)(int);

	static int InitSectorCache(int /*cacheSize*/) {
		return OrgInitSectorCache(config.sectorCacheSize);
	}

} sectorCacheSizeFix;

int (*SectorCacheSizeFix::OrgInitSectorCache)(int);
