#include "stdafx.h"

#include <infrastructure/meshes.h>
#include <particles/parser.h>
#include <infrastructure/vfs.h>
#include <infrastructure/stringutil.h>
#include <fmt/format.h>

using namespace particles;

/*
	This is a test fixture for particle system parser tests that only parses all particle systems once
	to speed up the tests.
*/
class PartSysParserTest : public testing::Test {
protected:

	static PartSysParser &GetParser() {	
		static PartSysParser sParser;
		return sParser;
	}

	static void SetUpTestCase() {

		// Init VFS with mock/dummy code
		vfs.reset(Vfs::CreateStdIoVfs());

		GetParser().ParseFile("data\\partsys0.tab");
		GetParser().ParseFile("data\\partsys1.tab");
		GetParser().ParseFile("data\\partsys2.tab");

		ASSERT_NE(GetParser().begin(), GetParser().end()) << "No particle systems have been loaded";
	}

};

TEST_F(PartSysParserTest, TestBasicSystem) {

	// Tests that the max particles are calculated correctly for all particle systems
	auto flamingAxeEmitter = GetParser().GetSpec("ef-flaming axe");
	ASSERT_TRUE(flamingAxeEmitter);
	ASSERT_EQ(6, flamingAxeEmitter->GetEmitters().size());
}

TEST_F(PartSysParserTest, TestModelParsing) {
	/*CheckModelId("sp-Bullstrength", 0, 14008); // Particle\bullstrength
	CheckModelId("sp-Chaos Hammer", 0, 14005); // Particle\Chaos-Hammer
	CheckModelId("sp-cool stuff", 0, 14007); // Particle\Tree
	CheckModelId("sp-Leomunds Secret Chest", 0, 11000); // Scenery\Containers\TreasureChest
	CheckModelId("sp-Minor Globe of Invulnerability", 5, 14004); // Particle\MinorGlobe
	CheckModelId("sp-Tree Shape", 0, 14007); // Particle\Tree*/
}

TEST_F(PartSysParserTest, TestKeyFrameParsing) {
	char line[4096];

	auto fh = fopen("data\\keyframedump.txt", "rt");
	while (fgets(line, sizeof(line), fh)) {
		auto parts = split(std::string(line), '|');
		auto partSysName = parts[0];
		size_t emitterIdx = stoi(parts[1]);
		PartSysParamId paramIdx = (PartSysParamId) stoi(parts[2]);

		auto partSys = GetParser().GetSpec(partSysName);
		if (!partSys) {
			FAIL() << "Could not find particle system" << partSysName;
		}
		if (emitterIdx >= partSys->GetEmitters().size()) {
			FAIL() << "Particle System" << partSysName << "is missing emitter" << emitterIdx;
		}
		auto emitter = partSys->GetEmitters()[emitterIdx];
		auto param = emitter->GetParam(paramIdx);
		float lifespan = (paramIdx >= part_accel_X) ? emitter->GetParticleLifespan() : emitter->GetLifespan();
		auto msg = fmt::format(L"Sys: {} Emitter: {} Param: {} (Lifespan: {})", partSysName, emitter->GetName(), paramIdx, lifespan);

		ASSERT_NE(nullptr, param) << msg;
		ASSERT_EQ((int) PSPT_KEYFRAMES, (int) param->GetType()) << msg;
		auto frameParam = (const PartSysParamKeyframes*)param;

		size_t frameCount = stoi(parts[3]);
		ASSERT_EQ(frameCount, frameParam->GetFrames().size()) << msg;

		for (size_t i = 0; i < frameCount; ++i) {
			auto frameStr = parts[4 + i];
			auto frameParts = split(frameStr, ';');
			auto frameStarti = (uint32_t) stoll(frameParts[0], 0, 16);
			auto frameValuei = (uint32_t) stoll(frameParts[1], 0, 16);
			auto frameDeltai = (uint32_t) stoll(frameParts[2], 0, 16);
			auto frameStart = *(float*)&frameStarti;
			auto frameValue = *(float*)&frameValuei;
			auto frameDelta = *(float*)&frameDeltai;

			auto actual = frameParam->GetFrames()[i];
			ASSERT_NEAR(frameStart, actual.start, 0.0001f) << msg;
			ASSERT_NEAR(frameValue, actual.value, 0.0001f) << msg;
			ASSERT_NEAR(frameDelta, actual.deltaPerSec, fabs(frameDelta * 0.0001f)) << msg;
		}
	}
	fclose(fh);
}

TEST_F(PartSysParserTest, TestMaxParticleCalculations) {
	char line[4096];

	auto fh = fopen("data\\partsysdump.txt", "rt");
	while (fgets(line, sizeof(line), fh)) {
		auto parts = split(std::string(line), '|');
		auto partSysName = parts[0];
		size_t emitterIdx = stoi(parts[1]);

		auto partSys = GetParser().GetSpec(partSysName);
		if (!partSys) {
			FAIL() << "Could not find particle system" << partSysName;
		}
		if (emitterIdx >= partSys->GetEmitters().size()) {
			FAIL() << "Particle System" << partSysName << "is missing emitter" << emitterIdx;
		}
		auto emitter = partSys->GetEmitters()[emitterIdx];

		auto msg = fmt::format(L"Sys: {} Emitter: {}", partSysName, emitter->GetName());

		auto maxParticles = stoi(parts[2]);
		ASSERT_EQ(maxParticles, emitter->GetMaxParticles()) << msg;

		auto particlesPerStep = stof(parts[3]);
		ASSERT_EQ(particlesPerStep, emitter->GetParticleRate()) << msg;

		auto particlesPerStepSec = stof(parts[4]);
		ASSERT_EQ(particlesPerStepSec, emitter->GetParticleRateMin()) << msg;
	}
	fclose(fh);
}
