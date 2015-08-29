#pragma once

#include <string>

#include "params.h"

class ParamsParser {
public:
		
	static PartSysParam* Parse(const std::string& value,
	                           float defaultValue,
	                           float parentLifespan,
							   bool &success);

	static PartSysParamKeyframes* ParseKeyframes(const std::string &value, float parentLifespan);

	static PartSysParamRandom* ParseRandom(const std::string &value);

};
