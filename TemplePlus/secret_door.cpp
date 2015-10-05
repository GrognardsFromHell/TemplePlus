
#include "stdafx.h"
#include "secret_door.h"
#include "obj.h"
#include "util/fixes.h"

class SecretDoorReplacements : public TempleFix
{
public:
	const char* name() override {
		return "SecretDoor Replacements";
	}
	void apply() override{
		macReplaceFun(10046470, _isSecretDoor)
		macReplaceFun(10046510, _getSecretDoorDC)
		macReplaceFun(100464A0, _secretDoorIsRevealed)
	}
} secDoorReplacements;


SecretDoorSys secretdoorSys;

bool SecretDoorSys::isSecretDoor(objHndl obj)
{
	return (objects.getInt32(obj, obj_f_secretdoor_flags) & OSDF_SECRET_DOOR) != 0;
}

int SecretDoorSys::getSecretDoorDC(objHndl obj)
{
	if (obj != 0i64)
	{
		if (isSecretDoor(obj))
		{
			return (objects.getInt32(obj, obj_f_secretdoor_dc) & 0x7F);
		}
	}
	return 0;
}

uint32_t SecretDoorSys::secretDoorIsRevealed(objHndl secDoor)
{	// this function replacement should fix the motherfucking secretdoor bug
	// it worked on Aquinas' save!
	// The problem was that there's some sort of "revealed scenery" list which for some reason the secret door appears there
	// so I just removed that - there's the secret_door_found flag anyway
	// come to think of it, it may have supposed to prevent the player from searching again and again, but fuckit
	// they take 20 anyway (well maybe not in KotB >_>)
	if (!secDoor) return 0;
	uint32_t secDoorFlags = objects.getInt32(secDoor, obj_f_secretdoor_flags);
	if ( (secDoorFlags & OSDF_SECRET_DOOR) == 0 )  return 0;
	return (secDoorFlags & OSDF_SECRET_DOOR_FOUND) != 0;
	// there was also a look up in some name table, but that seems to be bugged (the door would already appear there in the secret door bug case, thus preventing its revelation)
}

SecretDoorSys::SecretDoorSys()
{
}

bool _isSecretDoor(objHndl obj)
{
	return secretdoorSys.isSecretDoor(obj);
}

int _getSecretDoorDC(objHndl obj)
{
	return secretdoorSys.getSecretDoorDC(obj);
}

uint32_t _secretDoorIsRevealed(objHndl secDoor)
{
	return secretdoorSys.secretDoorIsRevealed(secDoor);
}