#include "stdafx.h"

#include <particles/parser.h>
#include <particles/params_parser.h>
#include <format.h>

static std::string BuildFrameString(const std::vector<PartSysParamKeyframe> &frames) {
	std::string debugOut = "";
	for (auto frame : frames) {
		if (!debugOut.empty()) {
			debugOut.append(" -> ");
		}
		debugOut.append(fmt::format("{}@{}", frame.value, frame.start));
	}
	return debugOut;
}

static std::vector<PartSysParamKeyframe> Parse(const std::string &spec, float lifetime) {
	bool success;
	std::unique_ptr<PartSysParamKeyframes> keyframes(
		(PartSysParamKeyframes*) ParamsParser::Parse(spec, 0.0f, lifetime, success)
	);
	if (!success) {
		throw new TempleException("Unable to parse...");
	}
	auto result = keyframes->GetFrames();

	// Build nice debug output
	std::string debugOut = BuildFrameString(result);
	std::cout << "Parsed " << spec << " as:" << std::endl;
	std::cout << debugOut << std::endl;

	return result;
}

/*
	This test checks our implementation of the primary parsing bug that 
	the ToEE particle systems parser has. If the first frame is offset 
	from the actual start of the animation, the system needs to insert
	a frame with the same value as the actual first frame at time code 0.
	This somewhat works, but while it modifies the time code of the first
	specified frame, it also skips it too early. This leads to the entire 
	animation being shifted towards the front of the animation and
	an incorrect frame being inserted at the end.

	This particular example comes from the Brasier Main Fire emitter.
	The actual expected result i'd guess would be:
	255@0 -> 255@2/30 -> 255@3/30-> 197@0.5
*/
TEST(PartSysKeyframes, TestOldBugOnlyPreFrame) {
	auto frames = Parse("255(2),255(3),197", 0.5f);
	auto actual = BuildFrameString(frames);
	auto expected = "255@0 -> 255@0.1 -> 197@0.333333 -> 197@0.5";
	ASSERT_EQ(expected, actual);
}

/*
	This tests our implementation of the ToEE frame parsing bug
	when the last frame is offset from the end of the animation.
	This particular (pointless since the value is constant, by the way)
	example comes from sp-Burning Hands
*/
TEST(PartSysKeyframes, TestOldBugWithPostFrame) {
	
	auto frames = Parse("255(6),255(10)", 1);
	auto actual = BuildFrameString(frames);
	auto expected = "255@0 -> 255@0.333333 -> 255@0.666667 -> 255@1";
	ASSERT_EQ(expected, actual);

}

/*
	This tests that keyframe specs that are actually unsupported by the old
	engine but still parsed correctly are supported in the same way by TemplePlus.
	In particular this is the mixing of random and keyframe parameters. In reality,
	the sscanf operation used by ToEE will just read the 200 of "200?300" when the
	frame is parsed by the keyframe parser.
	This particular example comes from the Brasier Smoke emitter.
*/
TEST(PartSysKeyframes, TestOldUnsupportedBehaviour) {

	auto frames = Parse("200?300,-100", 1.5f);
	auto actual = BuildFrameString(frames);
	auto expected = "200@0 -> -100@1.5";
	ASSERT_EQ(expected, actual);

}
