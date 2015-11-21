#include <regex>

#include <gsl/string_view.h>
#include <infrastructure/logging.h>

#include "particles/parser_keyframes.h"
#include "particles/parser_params.h"
#include "particles/params.h"

namespace particles {

	PartSysParam* ParserParams::Parse(PartSysParamId id, gsl::cstring_view<> value, float emitterLifespan, float particleLifespan, bool& success) {

		// Look up the default value
		auto defaultValue = PartSysParam::GetDefaultValue(id);

		// Do we have to use the particle or emitter lifespan as reference for keyframes?
		auto lifespan = (id >= part_accel_X) ? particleLifespan : emitterLifespan;

		return Parse(value, defaultValue, lifespan, success);

	}

	PartSysParam* ParserParams::Parse(gsl::cstring_view<> value, float defaultValue, float parentLifespan, bool& success) {
		success = false;

		if (parentLifespan == 0) {
			parentLifespan = 1.0f;
		}

		if (std::find(value.begin(), value.end(), ',') != value.end()) {
			auto result = ParseKeyframes(value, parentLifespan);
			success = !!result;
			return result;
		}

		if (std::find(value.begin(), value.end(), '?') != value.end()) {
			auto result = ParseRandom(value);
			success = !!result;
			return result;
		}

		if (std::find(value.begin(), value.end(), '#') != value.end()) {
			auto result = ParseSpecial(value);
			success = !!result;
			return result;
		}

		return ParseConstant(value, defaultValue, success);
	}

	PartSysParamKeyframes* ParserParams::ParseKeyframes(gsl::cstring_view<> value, float parentLifespan) {
		return ParserKeyframes::Parse(value, parentLifespan);
	}

	PartSysParamRandom* ParserParams::ParseRandom(gsl::cstring_view<> value) {
		float lower, upper;
		if (_snscanf_s(value.data(), value.size(), "%f?%f", &lower, &upper) == 2) {
			auto variance = upper - lower;
			return new PartSysParamRandom(lower, variance);
		}
		return nullptr;
	}

	PartSysParamSpecial* ParserParams::ParseSpecial(gsl::cstring_view<> value) {
		if (!_strnicmp(value.data(), "#radius", value.size())) {
			return new PartSysParamSpecial(PSPST_RADIUS);
		}
		return nullptr;
	}

	PartSysParamConstant* ParserParams::ParseConstant(gsl::cstring_view<> value, float defaultValue, bool& success) {
		// Try to parse it as a floating point constant
		float floatValue;
		if (_snscanf_s(value.data(), value.size(), "%f", &floatValue) != 1) {
			return nullptr;
		}

		success = true; // At this point it's a valid floating point number

		// Save some memory by not allocating a param if we're using the default value anyway
		if (fabs(floatValue - defaultValue) < 0.000001f) {
			return nullptr;
		}

		return new PartSysParamConstant(floatValue);
	}

}
