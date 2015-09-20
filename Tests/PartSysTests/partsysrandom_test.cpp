
#include "stdafx.h"

#include <particles/parser.h>
#include <particles/parser_params.h>

using namespace particles;

static std::unique_ptr<PartSysParamRandom> Parse(const std::string &spec) {
	bool success;
	std::unique_ptr<PartSysParamRandom> result(
		(PartSysParamRandom*)ParserParams::Parse(spec, 0.0f, 0.0f, success)
		);
	if (!success) {
		throw TempleException("Unable to parse...");
	}
	
	return result;
}

TEST(PartSysRandom, SimpleCase) {
	auto param = Parse("0?360");
	ASSERT_EQ(0, param->GetBase());
	ASSERT_EQ(360, param->GetVariance());
}

TEST(PartSysRandom, NegativeVariance) {
	auto param = Parse("255?200");
	ASSERT_EQ(255, param->GetBase());
	ASSERT_EQ(-55, param->GetVariance());
}

TEST(PartSysRandom, NegativeUpperBound) {
	auto param = Parse("75?-75");
	ASSERT_EQ(75, param->GetBase());
	ASSERT_EQ(-150, param->GetVariance());
}

TEST(PartSysRandom, NegativeLowerBound) {
	auto param = Parse("-75?75");
	ASSERT_EQ(-75, param->GetBase());
	ASSERT_EQ(150, param->GetVariance());
}

TEST(PartSysRandom, NegativeLowerBoundNegativeVariance) {
	auto param = Parse("-75?-150");
	ASSERT_EQ(-75, param->GetBase());
	ASSERT_EQ(-75, param->GetVariance());
}

TEST(PartSysRandom, FloatBoundaries) {
	auto param = Parse("0.5?-1.5");
	ASSERT_EQ(0.5f, param->GetBase());
	ASSERT_EQ(-2.0f, param->GetVariance());
}
