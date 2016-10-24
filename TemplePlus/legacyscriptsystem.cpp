#include "stdafx.h"
#include "legacyscriptsystem.h"
#include "temple\dll.h"

LegacyScriptSystem scriptSys;


int LegacyScriptSystem::GetGlobalFlag(int flagIdx){
	return temple::GetRef<int(__cdecl)(int)>(0x10006790)(flagIdx);
}

int LegacyScriptSystem::GetGlobalVar(int varIdx){
	return temple::GetRef<int(__cdecl)(int)>(0x10006760)(varIdx);
}

void LegacyScriptSystem::SetGlobalFlag(int flagIdx, int value){
	if (value)
		temple::GetRef<void(__cdecl)(int, int)>(0x100067C0)(flagIdx, 1);
	else
		temple::GetRef<void(__cdecl)(int, int)>(0x100067C0)(flagIdx, 0);
}

void LegacyScriptSystem::SetGlobalVar(int varIdx, int value){
	
	temple::GetRef<void(__cdecl)(int, int)>(0x10006770)(varIdx, value);
}

int LegacyScriptSystem::GetStoryState(){
	return temple::GetRef<int(__cdecl)()>(0x10006A20)();
}
