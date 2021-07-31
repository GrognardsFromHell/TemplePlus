#pragma once

#include <string>

namespace particles {

	struct PartSysParamKeyframe;
	class PartSysParamKeyframes;

	/*
		Extracted logic for parsing animated particle system values into
		this class, since the initial parsing is the most complicated
		of all the parameters.

		Keyframe animations in particle values can be spotted by looking for
		commas. I.e. 0,255 would be a keyframed animation that goes from 0 -> 255
		with two keyframes at the start and end.
	*/
	class ParserKeyframes {
	public:

		static PartSysParamKeyframes* Parse(std::string_view value, float parentLifespan);

	private:
		static bool ParseKeyframe(std::string_view value, float lifespan, PartSysParamKeyframe& frame);
		static bool IsStartTimeAscending(const std::vector<PartSysParamKeyframe>& frames);
		static void PostprocessFrames(std::vector<PartSysParamKeyframe>& frames);
	};
}
