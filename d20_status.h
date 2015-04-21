#pragma once
#include "stdafx.h"
#include "common.h"

class D20StatusSystem
{
public:
	void initRace(objHndl objHnd);
	void initClass(objHndl objHnd);
	void D20StatusInit(objHndl objHnd);
	void initDomains(objHndl objHnd);
	void initFeats(objHndl objHnd);
	void initItemConditions(objHndl objHnd);
};

extern D20StatusSystem d20StatusSys;