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

// Fix for duplicate particle systems
static class SectorLoadLightFix : TempleFix {
public:
	void apply() override;

private:

	static int ReadLight(TioFile* file, SectorLight** lightOut);
	static int ReadLightFromDiff(TioFile* file, SectorLight** lightOut);
	static int (*OrgReadLightFromDiff)(TioFile*, SectorLight**);
	static int (*OrgReadLight)(TioFile*, SectorLight**);

} sectorCacheFix;

int (*SectorLoadLightFix::OrgReadLightFromDiff)(TioFile*, SectorLight**);
int (*SectorLoadLightFix::OrgReadLight)(TioFile*, SectorLight**);

void SectorLoadLightFix::apply() {
	OrgReadLightFromDiff = replaceFunction(0x100A8050, ReadLightFromDiff);
	OrgReadLight = replaceFunction(0x100A6890, ReadLight);

}

int SectorLoadLightFix::ReadLight(TioFile* file, SectorLight** lightOut) {
	auto &particles = gameSystems->GetParticleSys();
	
	SectorLight light;
	if (tio_fread(&light, 0x40, 1u, file) != 1)
		return 0;

	auto lightPos = light.position.ToInches3D(light.offsetZ);

	SectorLight *realLight;
	if (light.flags & 0x10) {
		realLight = (SectorLight *)malloc(0x48u);
		memcpy(realLight, &light, 0x40);
		if (tio_fread(&realLight->partSys, 8u, 1u, file) != 1)
			return 0;

		if (realLight->partSys.hashCode) {
			realLight->partSys.handle = particles.CreateAt(realLight->partSys.hashCode, lightPos);
		} else {
			realLight->partSys.handle = 0;
		}			
	} else if (light.flags & 0x40) {
		realLight = (SectorLight *)malloc(0xB0u);
		memcpy(realLight, &light, 0x40);
		if (tio_fread(&realLight->partSys, 8u, 1u, file) != 1)
			return 0;
		if (tio_fread(&realLight->light2, 0x24u, 1u, file) != 1)
			return 0;

		// reset the handles so whatever the on-disk sector may say, the particle systems are not running
		realLight->partSys.handle = 0;
		realLight->light2.partSys.handle = 0;

		static auto& sIsNight = temple::GetRef<BOOL>(0x10B5DC80);
		SectorLightPartSys *partSys;
		if (sIsNight) {
			partSys = &realLight->light2.partSys;
		} else {
			partSys = &realLight->partSys;
		}

		if (partSys->hashCode) {
			partSys->handle = particles.CreateAt(partSys->hashCode, lightPos);
		}
	} else {
		realLight = (SectorLight *)malloc(0x40u);
		memcpy(realLight, &light, 0x40);
	}
	*lightOut = realLight;
	return 1;
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

	return ReadLight(file, lightOut);
}
