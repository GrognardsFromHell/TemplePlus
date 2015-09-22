
#include "stdafx.h"
#include <windows.h>

int main(int argc, char **argv)
{
	TCHAR path[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, path);
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
