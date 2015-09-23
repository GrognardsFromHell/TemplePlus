#include "stdafx.h"

#include <infrastructure/mesparser.h>
#include <infrastructure/vfs.h>
#include <infrastructure/stringutil.h>

TEST(MesFileTest, TestParseFile)
{
	// Init VFS with mock/dummy code
	vfs.reset(Vfs::CreateStdIoVfs());

	auto fn = "data\\meshes.mes";

	auto content = MesFile::ParseFile(fn);
	ASSERT_EQ(872, content.size());

	// Some exemplary checks
	ASSERT_EQ(std::string("TempMan\\TempMan"), content[0]); // First entry of file
	ASSERT_EQ(std::string("Weapons\\Sword_Roman1"), content[10042]); // Last entry of file
	ASSERT_EQ(std::string("wands\\white_staff"), content[18019]); // Last entry of file

	// Check ordering
	ASSERT_EQ(0, content.begin()->first);
	ASSERT_EQ(18019, (--content.end())->first);
}
