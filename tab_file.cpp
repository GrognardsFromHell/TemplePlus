#include "stdafx.h"
#include "common.h"
#include "tab_file.h"

TabFileSystem tabSys;





#pragma region TabFileSystem Implementation

TabFileSystem::TabFileSystem()
{
	macRebase(tabFileStatusInit, 101F2C10)
	macRebase(tabFileStatusDealloc, 101F2C30)
	macRebase(tabFileParseLines, 101F2C70)
	macRebase(tabFileGetNumLines, 101F2D40)
	macRebase(tabFileStatusBasicFormatter,101F2E40)
}

#pragma endregion

