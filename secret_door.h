#pragma once
#include "stdafx.h"
#include "common.h"


struct SecretDoorSys : AddressTable
{
	bool isSecretDoor(objHndl obj);
	int getSecretDoorDC(objHndl obj);
	uint32_t secretDoorIsRevealed(objHndl secDoor);

	SecretDoorSys();

};


extern SecretDoorSys secretdoorSys;


bool _isSecretDoor(objHndl obj);
int _getSecretDoorDC(objHndl obj);
uint32_t _secretDoorIsRevealed(objHndl secDoor);