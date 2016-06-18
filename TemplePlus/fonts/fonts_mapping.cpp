
#include "stdafx.h"
#include "fonts_mapping.h"

#include <infrastructure/vfs.h>
#include <infrastructure/json11.hpp>
#include "python/python_debug.h"

FontsMapping::FontsMapping()
{
	LoadMappings();

	// Register a callback that allows the mapping to be reloaded
	RegisterDebugFunction("reload_fonts", [this]() {
		mMappings.clear();
		LoadMappings();
	});
}

FontsMapping::~FontsMapping() = default;

void FontsMapping::LoadMappings()
{
	auto mappingData = vfs->ReadAsString("fonts/mapping.json");

	std::string parsingErr;
	auto json = json11::Json::parse(mappingData.c_str(), parsingErr);
	if (!parsingErr.empty()) {
		throw TempleException("Unable to parse fonts/mapping.json: {}", parsingErr);
	}

	if (!json.is_array()) {
		throw TempleException("Expected an array on the top-level of fonts/mapping.json");
	}

	for (auto &record : json.array_items()) {
		auto id = eastl::string(record["id"].string_value().c_str());

		gfx::TextStyle style;
		style.fontFace = eastl::string(record["fontFace"].string_value().c_str());
		style.pointSize = (float)record["size"].number_value();
		style.bold = record["bold"].bool_value();
		style.italic = record["italic"].bool_value();
		auto uniformLineHeight = record["uniformLineHeight"];
		if (uniformLineHeight.is_object()) {
			style.uniformLineHeight = true;
			style.lineHeight = (float)uniformLineHeight["lineHeight"].number_value();
			style.baseLine = (float)uniformLineHeight["baseline"].number_value();
		}
		mMappings[id] = style;
	}
}
