#pragma once

#include <string>

#include "params.h"

class ParserParams {
public:

	static PartSysParam* Parse(PartSysParamId id,
	                           const std::string& value,
	                           float emitterLifespan,
	                           float particleLifespan,
	                           bool& success);

	static PartSysParam* Parse(const std::string& value,
	                           float defaultValue,
	                           float parentLifespan,
	                           bool& success);

	static PartSysParamKeyframes* ParseKeyframes(const std::string& value, float parentLifespan);

	static PartSysParamRandom* ParseRandom(const std::string& value);

	static PartSysParamSpecial* ParseSpecial(const std::string& value);

	static PartSysParamConstant* ParseConstant(const std::string& value, float defaultValue, bool& success);

};
