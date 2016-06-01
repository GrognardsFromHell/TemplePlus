
#include "stdafx.h"
#include "tig_mes.h"

MesFuncs mesFuncs;

MesLine::MesLine()
{
	key = 0;
	value = nullptr;
}

MesLine::MesLine(uint32_t Key){
	key = Key;
	value = nullptr;
}

MesLine::MesLine(uint32_t Key, const char* Line){
	key = Key;
	value = Line;
}

int MesFuncs::GetNumLines(MesHandle mesHandle){
	return temple::GetRef<int(__cdecl)(MesHandle)>(0x101E62F0)(mesHandle);
}

BOOL MesFuncs::ReadLineDirect(MesHandle mesHandle, int lineCount, MesLine* mesLine){
	return temple::GetRef<BOOL(__cdecl)(MesHandle, int, MesLine*)>(0x101E6310)(mesHandle, lineCount, mesLine);
}
