#pragma once

#include <gsl/string_span.h>
#include <string>

#include "params.h"

namespace particles {

class ParserParams {
public:

	static PartSysParam* Parse(PartSysParamId id,
							   gsl::cstring_span<> value,
	                           float emitterLifespan,
	                           float particleLifespan,
	                           bool& success);

	static PartSysParam* Parse(gsl::cstring_span<> value,
	                           float defaultValue,
	                           float parentLifespan,
	                           bool& success);

	static PartSysParamKeyframes* ParseKeyframes(gsl::cstring_span<> value, float parentLifespan);

	static PartSysParamRandom* ParseRandom(gsl::cstring_span<> value);

	static PartSysParamSpecial* ParseSpecial(gsl::cstring_span<> value);

	static PartSysParamConstant* ParseConstant(gsl::cstring_span<> value, float defaultValue, bool& success);

};

}
