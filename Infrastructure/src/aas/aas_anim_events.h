
#pragma once

#include <string_view>

namespace aas {

	enum class AnimEventType {
		Script,
		End,
		Action
	};

	class IAnimEventHandler {
	public:
		virtual ~IAnimEventHandler() = default;
		virtual void HandleEvent(int frame, float frameTime, AnimEventType type, std::string_view args) = 0;
	};

}
