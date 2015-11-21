#include <infrastructure/stringutil.h>
#include <infrastructure/logging.h>

#include "particles/parser_keyframes.h"
#include "particles/params.h"

namespace particles {

	bool ParserKeyframes::ParseKeyframe(gsl::cstring_view<> value, float lifespan, PartSysParamKeyframe& frame) {

		// percentage based on lifespan
		if (std::find(value.begin(), value.end(), '%') != value.end()) {
			float percentage;
			if (_snscanf_s(value.data(), value.size(), "%f(%f%%)", &frame.value, &percentage) == 2) {
				frame.start = lifespan * percentage / 100.0f;
				if (frame.start < 0) {
					frame.start += lifespan;
				}
				return true;
			}
		}

		int tokenCount = _snscanf_s(value.data(), value.size(), "%f(%f)", &frame.value, &frame.start);

		// Got a frame count in the 2nd argument
		if (tokenCount == 2) {
			frame.start /= 30.0f; // convert from particle system frames to seconds

			// Relative to the end of the lifespan if its negative
			if (frame.start < 0) {
				frame.start += lifespan;
			}
		}

		return (tokenCount == 1 || tokenCount == 2);

	}

	/*
	Validates that the keyframes are defined in ascending order.
*/
	bool ParserKeyframes::IsStartTimeAscending(const std::vector<PartSysParamKeyframe>& frames) {

		for (size_t i = 1; i < frames.size(); ++i) {
			if (frames[i].start <= frames[i - 1].start) {
				return false;
			}
		}

		return true;

	}

	void ParserKeyframes::PostprocessFrames(std::vector<PartSysParamKeyframe>& frames) {

		// Calculate delta to next
		for (size_t j = 1; j < frames.size(); ++j) {
			float valueDelta = (frames[j].value - frames[j - 1].value);
			float timeDelta = (frames[j].start - frames[j - 1].start);
			frames[j - 1].deltaPerSec = valueDelta / timeDelta;
		}
		frames[frames.size() - 1].deltaPerSec = 0; // No delta for last

	}

	PartSysParamKeyframes* ParserKeyframes::Parse(gsl::cstring_view<> value, float parentLifespan) {

		auto frameDefs = split(value, ',', true, true);

		PartSysParamKeyframe preFrame, postFrame;
		bool havePreFrame = false, havePostFrame = false;

		int frameCount = frameDefs.size();

		// Check if we need to insert a pre-frame to get a frame at time 0
		preFrame.start = 0;
		if (ParseKeyframe(frameDefs[0], parentLifespan, preFrame)) {
			if (preFrame.start > 0) {
				preFrame.start = 0;
				frameCount++;
				havePreFrame = true;
			}
		}

		// Check if we need to insert a post-frame to get a frame at the end of the lifespan
		postFrame.start = parentLifespan;
		if (ParseKeyframe(frameDefs[frameDefs.size() - 1], parentLifespan, postFrame)) {
			if (postFrame.start < parentLifespan) {
				postFrame.start = parentLifespan;
				frameCount++;
				havePostFrame = true;
			}
		}

		std::vector<PartSysParamKeyframe> frames;
		frames.reserve(frameCount);

		/*
	Pre-Frames are bugged in ToEE. The whole animation is shifted forward
	if the first keyframe has a non-zero start-time and the last frame is
	extended.
	*/
		if (havePreFrame) {
			// frames.push_back(preFrame);
		}

		// Increase in the frame time between frames
		float timeStep = parentLifespan / (frameCount - 1);

		int i = 0;
		float curTime = 0.0f;
		PartSysParamKeyframe frame;
		for (auto frameDef : frameDefs) {
			frame.start = curTime;
			curTime += timeStep;
			if (!ParseKeyframe(frameDef, parentLifespan, frame)) {
				logger->warn("Unable to parse particle system keyframes: {}", value);
				return nullptr;
			}
			if (i++ == 0 && havePreFrame) {
				frame.start = 0;
			}
			frames.push_back(frame);
		}

		// See above for the ToEE bug we're trying to replicate here...
		if (havePreFrame) {
			if (curTime <= parentLifespan) {
				auto lastFrame = frames[frames.size() - 1];

				lastFrame.start = curTime;
				curTime += timeStep;

				frames.push_back(lastFrame);
			}
		}

		if (havePostFrame) {
			frames.push_back(postFrame);
		}

		PostprocessFrames(frames);

		// Validate monotonically increasing start times
		if (!IsStartTimeAscending(frames)) {
			logger->warn("Animated keyframes '{}' are not in ascending time.", value);
			return nullptr;
		}

		return new PartSysParamKeyframes(frames);

	}

}
