#include "stdafx.h"

#include <particles/parser_params.h>

/*
	Check the default value calculation of PartSysParam for a 
	few examples
*/
TEST(ParserParamsTest, GetDefaultValue) {
	ASSERT_EQ(0, PartSysParam::GetDefaultValue(part_accel_X));
	ASSERT_EQ(255, PartSysParam::GetDefaultValue(emit_init_alpha));
	ASSERT_EQ(1, PartSysParam::GetDefaultValue(emit_scale_X));
}

TEST(ParserParamsTest, ParseConstant) {

	bool success;
	auto param = ParserParams::Parse(emit_accel_X, "1.5", 0, 0, success);

	ASSERT_NE(nullptr, param);
	ASSERT_TRUE(success);
	ASSERT_EQ(PSPT_CONSTANT, param->GetType());
	ASSERT_EQ(1.5f, param->GetValue());

}

/*
For constants with a default value, the parser will return null, but
set success to true.
*/
TEST(ParserParamsTest, ParseConstantDefault) {
	
	bool success;
	auto param = ParserParams::Parse(emit_accel_X, "0", 0, 0, success);

	ASSERT_EQ(nullptr, param) << "No parameter should have been returned for a default value.";
	ASSERT_TRUE(success) << "The parser should still indicate success, however.";

	// Try it for a param with another default value than 1

	param = ParserParams::Parse(emit_init_alpha, "255", 0, 0, success);

	ASSERT_EQ(nullptr, param) << "No parameter should have been returned for a default value.";
	ASSERT_TRUE(success) << "The parser should still indicate success, however.";


}
