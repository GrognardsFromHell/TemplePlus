#include <regex>

#include <stringutil.h>
#include <logging.h>

#include "particles/parser_keyframes.h"
#include "particles/parser_params.h"
#include "particles/params.h"

PartSysParam* ParserParams::Parse(PartSysParamId id, const std::string& value, float emitterLifespan, float particleLifespan, bool& success) {

	// Look up the default value
	auto defaultValue = PartSysParam::GetDefaultValue(id);

	// Do we have to use the particle or emitter lifespan as reference for keyframes?
	auto lifespan = (id >= part_accel_X) ? particleLifespan : emitterLifespan;

	return Parse(value, defaultValue, lifespan, success);

}

PartSysParam* ParserParams::Parse(const std::string& value, float defaultValue, float parentLifespan, bool& success) {
	success = false;

	if (parentLifespan == 0) {
		parentLifespan = 1.0f;
	}

	if (value.find(',') != std::string::npos) {
		auto result = ParseKeyframes(value, parentLifespan);
		success = !!result;
		return result;
	}

	if (value.find('?') != std::string::npos) {
		auto result = ParseRandom(value);
		success = !!result;
		return result;
	}

	if (value.find('#') != std::string::npos) {
		return ParseSpecial(value);
	}

	return ParseConstant(value, defaultValue, success);
}

PartSysParamKeyframes* ParserParams::ParseKeyframes(const std::string& value, float parentLifespan) {
	return ParserKeyframes::Parse(value, parentLifespan);
}

PartSysParamRandom* ParserParams::ParseRandom(const std::string& value) {
#define FLOAT_PATTERN "(-?\\d*\\.\\d+|-?\\d+)"
	static const std::regex re(FLOAT_PATTERN "\\?" FLOAT_PATTERN);
	std::smatch match;
	if (std::regex_search(value, match, re)) {
		auto lower = std::stof(match[1]);
		auto upper = std::stof(match[2]);
		auto variance = upper - lower;
		return new PartSysParamRandom(lower, variance);
	}
	return nullptr;
}

PartSysParamSpecial* ParserParams::ParseSpecial(const std::string& value) {
	if (value == "#radius") {
		return new PartSysParamSpecial(PSPST_RADIUS);
	}
	return nullptr;
}

PartSysParamConstant* ParserParams::ParseConstant(const std::string& value, float defaultValue, bool &success) {
	// Try to parse it as a floating point constant
	float floatValue;
	try {
		floatValue = std::stof(value);
	} catch (const std::exception&) {
		return nullptr;
	}

	success = true; // At this point it's a valid floating point number

	// Save some memory by not allocating a param if we're using the default value anyway
	if (fabs(floatValue - defaultValue) < 0.000001f) {
		return nullptr;
	}

	return new PartSysParamConstant(floatValue);
}
