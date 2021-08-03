#include "stdafx.h"
#include "animgoal.h"

std::string_view GetAnimGoalPriorityText(AnimGoalPriority priority)
{
    using namespace std::literals;

	switch (priority) {
	case 0:
		return "AGP_NONE"sv;
	case 1:
		return "AGP_1"sv;
	case 2:
		return "AGP_2"sv;
	case 3:
		return "AGP_3"sv;
	case 4:
		return "AGP_4"sv;
	case 5:
		return "AGP_5"sv;
	case 6:
		return "AGP_HIGHEST"sv;
	case 7:
		return "AGP_7"sv;
	case 8:
		return "AGP_MAX"sv;
	default:
		return "AGP_UNKNOWN"sv;
	}
}
