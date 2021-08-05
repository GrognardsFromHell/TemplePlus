#pragma once

#include <string>

#include "params.h"

namespace particles {

class ParserParams {
public:

	static PartSysParam* Parse(PartSysParamId id,
							   std::string_view value,
	                           float emitterLifespan,
	                           float particleLifespan,
	                           bool& success);

	static PartSysParam* Parse(std::string_view value,
	                           float defaultValue,
	                           float parentLifespan,
	                           bool& success);

	static PartSysParamKeyframes* ParseKeyframes(std::string_view value, float parentLifespan);

	static PartSysParamRandom* ParseRandom(std::string_view value);

	static PartSysParamSpecial* ParseSpecial(std::string_view value);

	static PartSysParamConstant* ParseConstant(std::string_view value, float defaultValue, bool& success);

};

}
