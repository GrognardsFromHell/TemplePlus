#pragma once

#include <gsl/string_view.h>
#include <string>

#include "params.h"

namespace particles {

class ParserParams {
public:

	static PartSysParam* Parse(PartSysParamId id,
							   gsl::cstring_view<> value,
	                           float emitterLifespan,
	                           float particleLifespan,
	                           bool& success);

	static PartSysParam* Parse(gsl::cstring_view<> value,
	                           float defaultValue,
	                           float parentLifespan,
	                           bool& success);

	static PartSysParamKeyframes* ParseKeyframes(gsl::cstring_view<> value, float parentLifespan);

	static PartSysParamRandom* ParseRandom(gsl::cstring_view<> value);

	static PartSysParamSpecial* ParseSpecial(gsl::cstring_view<> value);

	static PartSysParamConstant* ParseConstant(gsl::cstring_view<> value, float defaultValue, bool& success);

};

}
