#pragma once



class LegacyScriptSystem
{
public:
	int GetGlobalFlag(int flagIdx);
	int GetGlobalVar(int varIdx);
	void SetGlobalFlag(int flagIdx, int value);
	void SetGlobalVar(int flagIdx, int value);

	int GetStoryState();
};


extern LegacyScriptSystem scriptSys;