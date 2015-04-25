#pragma once
#include "stdafx.h"
#include "common.h"
#include "tig\tig_mes.h"


struct TabFileStatus
{
	const char * fileName; // path relatives to data
	uint32_t numLines;
	uint32_t numTabsMax; //per line
	uint32_t numLinesParsed;
	const char * fileContentsRaw;
	const char * endOfString;
	uint32_t(__cdecl * lineParser)(TabFileStatus * tab, uint32_t numLinesParsed, const char ** tokens);
};

struct TabFileSystem : AddressTable
{

	void(__cdecl * tabFileStatusInit)(TabFileStatus * tab, uint32_t(__cdecl* tabLineParser)(TabFileStatus *, uint32_t, const char**));
	uint32_t(__cdecl * tabFileStatusBasicFormatter)(TabFileStatus* tab, const char * fileName);
	uint32_t(__cdecl * tabFileGetNumLines)(TabFileStatus * tab);
	void(__cdecl * tabFileParseLines)(TabFileStatus * tab);
	void(__cdecl * tabFileStatusDealloc)(TabFileStatus * tab);

	TabFileSystem();
};

extern TabFileSystem tabSys;