
#pragma once

#include <EASTL/string.h>
#include <EASTL/map.h>
#include <graphics/textengine.h>

class FontsMapping {
public:
	using container = eastl::map<eastl::string, gfx::TextStyle>;
	using iterator = container::const_iterator;

	FontsMapping();
	~FontsMapping();

	iterator find(const char *fontId) const {
		return mMappings.find(fontId);
	}
	iterator end() const {
		return mMappings.end();
	}
	
private:
	eastl::map<eastl::string, gfx::TextStyle> mMappings;

	void LoadMappings();
};
