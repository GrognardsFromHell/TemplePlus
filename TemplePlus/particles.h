
#pragma once

#include "obj.h"

/*
	Wraps particle system related functions.
*/
class LegacyParticles {
public:	

	/*
		Creates a particle system attached to an object.
	*/
	int CreateAtObj(const char *name, objHndl atObj);

	/*
		Creates a particle system at a fixed 3D location.
	*/
	int CreateAt3dPos(const char *name, vector3f pos);

	/*
	Creates a particle system at a tile location.
	*/
	int CreateAtTile(const char *name, locXY tile) {
		return CreateAt3dPos(name, tile.ToInches3D());
	}

	/*
		Not quite sure what the difference is here.
		Guess: one also destroys all existing particles, while the other only stops spawning new ones.
	*/
	void Kill(int partSysId);
	void End(int partSysId);

	// Special effects
	void CallLightning(LocAndOffsets location);
	void LightningBolt(objHndl caster, LocAndOffsets target);
	void ChainLightning(objHndl caster, const vector<objHndl> &targets);
};

extern LegacyParticles legacyParticles;

