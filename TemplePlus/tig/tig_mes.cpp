
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

void MesFuncs::AddToMap(MesHandle openedMesHandle, std::map<int, std::string>& mesMap, int *highestKey){
	auto mh = openedMesHandle;
	auto numLines = mesFuncs.GetNumLines(mh);
	for (auto i=0; i < numLines; i++){
		MesLine line;
		mesFuncs.ReadLineDirect(mh, i, &line);
		mesMap[line.key] = line.value;
		if (highestKey && *highestKey < line.key)
			*highestKey = line.key;
	}
}

void MesFuncs::AddToMap(std::string & mesFilename, std::map<int, std::string>& mesMap, int *highestKey){
	MesHandle mh;
	mesFuncs.Open(mesFilename.c_str(), &mh);
	AddToMap(mh, mesMap, highestKey);
	mesFuncs.Close(mh);
}

int MesFuncs::GetNumLines(MesHandle mesHandle){
	return temple::GetRef<int(__cdecl)(MesHandle)>(0x101E62F0)(mesHandle);
}

BOOL MesFuncs::ReadLineDirect(MesHandle mesHandle, int lineCount, MesLine* mesLine){
	return temple::GetRef<BOOL(__cdecl)(MesHandle, int, MesLine*)>(0x101E6310)(mesHandle, lineCount, mesLine);
}

bool MesFuncs::GetFirstLine(MesHandle mesHandle, MesLine* lineOut){
	// todo
	return temple::GetRef<BOOL(__cdecl)(MesHandle, MesLine*)>(0x101E62B0)(mesHandle, lineOut) != FALSE;
}
