
#include "stdafx.h"
#include <windows.h>

#include <filesystem>
using namespace std::filesystem;

static void ChangeToDataDirectory() {
	path p = current_path();

	while (p.has_parent_path()) {
		if (exists(p / "TemplePlus.sln")) {
			current_path(p / "Tests" / "PartSysTests");
			return;
		}

		p = p.parent_path();
	}
}

int main(int argc, char **argv)
{
	ChangeToDataDirectory();
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
