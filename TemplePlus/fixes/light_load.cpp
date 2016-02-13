#include "stdafx.h"
#include "util/fixes.h"
#include <obj.h>
#include <temple_functions.h>
#include "gamesystems/timeevents.h"
#include <particles.h>
#include <gamesystems/gamesystems.h>
#include <gamesystems/map/sector.h>
#include <gamesystems/particlesystems.h>

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

	auto& particles = gameSystems->GetParticleSys();

	/*
		ToEE would let the light read from the original sector file leak,
		so we take care of it here and destroy the light and the particle
		systems that might have been created.
	*/
	auto lightOrg = *lightOut;
	if (lightOrg) {
		if (lightOrg->flags & 0x50) {
			auto partSys1 = lightOrg->partSys;
			if (partSys1.handle) {
				particles.Remove(partSys1.handle);
			}
		}
		if (lightOrg->flags & 0x40) {
			auto partSys2 = lightOrg->light2.partSys;
			if (partSys2.handle) {
				particles.Remove(partSys2.handle);
			}
		}

		free(lightOrg);
	}

	return OrgReadLightFromDiff(file, lightOut);
}
