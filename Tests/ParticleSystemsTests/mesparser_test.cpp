#include "stdafx.h"
#include "CppUnitTest.h"

#include "mesparser.h"
#include "vfs.h"
#include <stringutil.h>
#include <format.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ParticleSystemsTests
{		
	TEST_CLASS(MesParserTest)
	{
	public:

		TEST_CLASS_INITIALIZE(InitVfs)
		{
			// Init VFS with mock/dummy code
			vfs.reset(Vfs::CreateStdIoVfs());				
		}
				
		TEST_METHOD(TestParseFile)
		{
			auto fn = "..\\Tests\\ParticleSystemsTests\\data\\meshes.mes";

			auto content = MesFile::ParseFile(fn);
			Assert::AreEqual<size_t>(872, content.size());

			// Some exemplary checks
			Assert::AreEqual(std::string("TempMan\\TempMan"), content[0]); // First entry of file
			Assert::AreEqual(std::string("Weapons\\Sword_Roman1"), content[10042]); // Last entry of file
			Assert::AreEqual(std::string("wands\\white_staff"), content[18019]); // Last entry of file

			// Check ordering
			Assert::AreEqual(0, content.begin()->first);
			Assert::AreEqual(18019, (--content.end())->first);
		}

	};
}
