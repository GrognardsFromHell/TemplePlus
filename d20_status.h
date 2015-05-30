#pragma once
#include "stdafx.h"
#include "common.h"

struct Dispatcher;

class D20StatusSystem
{
public:
	void initRace(objHndl objHnd);
	void initClass(objHndl objHnd);
	void D20StatusInit(objHndl objHnd);
	void D20StatusRefresh(objHndl objHnd);
	void initDomains(objHndl objHnd);
	void initFeats(objHndl objHnd);
	void initItemConditions(objHndl objHnd);
	void D20StatusInitFromInternalFields(objHndl objHnd, Dispatcher *dispatcher);
};

extern D20StatusSystem d20StatusSys;