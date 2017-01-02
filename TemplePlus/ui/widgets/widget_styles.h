
#pragma once

#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/hash_map.h>

#include <graphics/textengine.h>

#include "widgets.h"

namespace json11 {
	class Json;
}

/**
 * Serves as a registry for Widget text styles.
 */
class WidgetTextStyles {
public:

	WidgetTextStyles();
	~WidgetTextStyles();

	void AddStyle(const eastl::string &id, const gfx::TextStyle &textStyle);

	const gfx::TextStyle &GetDefaultStyle() const {
		return mDefaultStyle;
	}
	void SetDefaultStyle(const gfx::TextStyle &textStyle);

	const gfx::TextStyle &GetTextStyle(const eastl::string &id) const;

	bool HasStyle(const eastl::string &id) const;

	void LoadStylesFile(const std::string &path);
	
	void LoadStyles(const json11::Json &jsonStyleArray);

private:
	gfx::TextStyle mDefaultStyle;
	eastl::hash_map<eastl::string, gfx::TextStyle> mTextStyles;
};

extern WidgetTextStyles* widgetTextStyles;


/**
* Serves as a registry for Widget text styles.
*/
class WidgetButtonStyles {
public:

	WidgetButtonStyles();
	~WidgetButtonStyles();

	void AddStyle(const eastl::string &id, const WidgetButtonStyle &textStyle);

	const WidgetButtonStyle &GetStyle(const eastl::string &id) const;

	bool HasStyle(const eastl::string &id) const;

	void LoadStylesFile(const std::string &path);

	void LoadStyles(const json11::Json &jsonStyleArray);

private:
	eastl::hash_map<eastl::string, WidgetButtonStyle> mStyles;
};

extern WidgetButtonStyles* widgetButtonStyles;

