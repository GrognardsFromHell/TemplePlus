
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
