#include "stdafx.h"
#include "CppUnitTest.h"

#include "meshes.h"
#include "particles/parser.h"
#include "vfs.h"
#include <stringutil.h>
#include <format.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ParticleSystemsTests
{		
	TEST_CLASS(ParserTest)
	{
	public:
		ParticleSystemParser parser;

		ParserTest() {
			materials.reset(new MaterialManager);

			meshes.reset(new MeshesManager);
			meshes->LoadMapping("..\\Tests\\ParticleSystemsTests\\data\\meshes.mes");
			
			parser.ParseFile("..\\Tests\\ParticleSystemsTests\\data\\partsys0.tab");
			parser.ParseFile("..\\Tests\\ParticleSystemsTests\\data\\partsys1.tab");
			parser.ParseFile("..\\Tests\\ParticleSystemsTests\\data\\partsys2.tab");
		}

		TEST_CLASS_INITIALIZE(InitVfs)
		{
			// Init VFS with mock/dummy code
			vfs.reset(Vfs::CreateStdIoVfs());				
		}
				
		TEST_METHOD(TestSingleEmitter)
		{

			// Tests that the max particles are calculated correctly for all particle systems
			auto flamingAxeEmitter = parser.GetSpec("ef-flaming axe");
			Assert::IsNotNull(flamingAxeEmitter.get());
			Assert::AreEqual((size_t) 6, flamingAxeEmitter->GetEmitters().size());
		}

		TEST_METHOD(TestMaxParticleCalculations)
		{
			char line[4096];

			auto fh = fopen("..\\Tests\\ParticleSystemsTests\\data\\partsysdump.txt", "rt");
			while (fgets(line, sizeof(line), fh)) {
				auto parts = split(line, '|');
				auto partSysName = parts[0];
				size_t emitterIdx = stoi(parts[1]);

				auto partSys = parser.GetSpec(partSysName);
				if (!partSys) {
					Assert::Fail(fmt::format(L"Could not find particle system '{}'", partSysName).c_str());
				}
				if (emitterIdx >= partSys->GetEmitters().size()) {
					Assert::Fail(fmt::format(L"Particle System '{}' is missing emitter {}", partSysName, emitterIdx).c_str());
				}
				auto emitter = partSys->GetEmitters()[emitterIdx];

				auto msg = fmt::format(L"Sys: {} Emitter: {}", partSysName, emitter->GetName());
				
				auto maxParticles = stoi(parts[2]);
				Assert::AreEqual(maxParticles, emitter->GetMaxParticles(), msg.c_str());

				auto particlesPerStep = stof(parts[3]);
				Assert::AreEqual(particlesPerStep, emitter->GetParticleRate(), msg.c_str());

				auto particlesPerStepSec = stof(parts[4]);
				Assert::AreEqual(particlesPerStepSec, emitter->GetParticleRateSecondary(), msg.c_str());
			}
			fclose(fh);
		}

	};
}
