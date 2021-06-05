#pragma once
#include "common.h"
#include <temple/dll.h>
#include "d20.h"

struct SecretDoorSys : temple::AddressTable
{
	bool isSecretDoor(objHndl obj);
	int getSecretDoorDC(objHndl obj);
	uint32_t secretDoorIsRevealed(objHndl secDoor);
	int SearchConcentratedPerformFunc(D20Actn *d20a);
	int SecretDoorSkillCheck(objHndl seeker, objHndl *foundObject);
	int SearchEventSecretDoorDetect(objHndl sd, objHndl seeker);
	int SecretDoorRollAndReveal(objHndl secdoor, objHndl seeker, BonusList* bonList);
	BOOL SecretDoorDetect(objHndl sd, objHndl seeker);
	int SecretDoorGetDCAutoDetect(objHndl secdoor);

	SecretDoorSys();

};


extern SecretDoorSys secretdoorSys;


bool _isSecretDoor(objHndl obj);
int _getSecretDoorDC(objHndl obj);
uint32_t _secretDoorIsRevealed(objHndl secDoor);