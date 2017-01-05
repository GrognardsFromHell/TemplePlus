
#include "stdafx.h"
#include "widget_styles.h"

#include <infrastructure/json11.hpp>
#include <infrastructure/vfs.h>

namespace std {
	ostream &operator <<(ostream &out, const eastl::string &text) {
		out << text.c_str();
		return out;
	}
}

WidgetTextStyles* widgetTextStyles = nullptr;

WidgetTextStyles::WidgetTextStyles()
{
	Expects(!widgetTextStyles);
	widgetTextStyles = this;

	LoadStyles("templeplus/text_styles.json");
}

WidgetTextStyles::~WidgetTextStyles()
{
	if (widgetTextStyles == this) {
		widgetTextStyles = nullptr;
	}
}

void WidgetTextStyles::AddStyle(const eastl::string & id, const gfx::TextStyle & textStyle)
{
	auto it = mTextStyles.find(id);
	if (it != mTextStyles.end()) {
		throw TempleException("Duplicate text style defined: {}", id);
	}
	mTextStyles.emplace(id, std::move(textStyle));

	if (mTextStyles.size() == 1) {
		// The very first style is used as the default style
		mDefaultStyle = mTextStyles[id];
	}
}

void WidgetTextStyles::SetDefaultStyle(const gfx::TextStyle & textStyle)
{
	mDefaultStyle = textStyle;
}

const gfx::TextStyle & WidgetTextStyles::GetTextStyle(const eastl::string & id) const
{
	auto it = mTextStyles.find(id);
	if (it != mTextStyles.end()) {
		return it->second;
	}
	return mDefaultStyle;
}

bool WidgetTextStyles::HasStyle(const eastl::string & id) const
{
	return mTextStyles.find(id) != mTextStyles.end();
}

static XMCOLOR ParseColor(const std::string &def) {
	XMCOLOR color(1, 1, 1, 1);

	if (def.size() != 7 && def.size() != 9) {
		logger->warn("Color definition '{}' has to be #RRGGBB or #RRGGBBAA.");
		return color;
	}

	if (def.find("#") != 0) {
		logger->warn("Color definition '{}' has to start with # sign.", def);
		return color;
	}

	color.r = std::stoi(def.substr(1, 2), 0, 16);
	color.g = std::stoi(def.substr(3, 2), 0, 16);
	color.b = std::stoi(def.substr(5, 2), 0, 16);
	if (def.length() >= 9) {
		color.a = std::stoi(def.substr(7, 2), 0, 16);
	}

	return color;
}

static gfx::Brush ParseBrush(const json11::Json &jsonVal) {

	if (jsonVal.is_array()) {
		if (jsonVal.array_items().size() != 2) {
			throw TempleException("Brush specification {} has to have 2 elements for a gradient!", jsonVal.dump());
		}

		gfx::Brush gradient;
		gradient.gradient = true;
		gradient.primaryColor = ParseColor(jsonVal.array_items()[0].string_value());
		gradient.secondaryColor = ParseColor(jsonVal.array_items()[1].string_value());
		return gradient;
	}

	gfx::Brush brush;
	brush.gradient = false;
    brush.primaryColor = ParseColor(jsonVal.string_value());
	brush.secondaryColor = brush.primaryColor;
	return brush;

}

void WidgetTextStyles::LoadStylesFile(const std::string & path)
{
	std::string error;
	json11::Json json = json.parse(vfs->ReadAsString(path), error);

	if (json.is_null()) {
		throw TempleException("Unable to parse text styles from {}: {}", path, error);
	}

	if (!json.is_array()) {
		throw TempleException("Text style files must start with an array at the root");
	}

	LoadStyles(json);

}

void WidgetTextStyles::LoadStyles(const json11::Json & jsonStyleArray)
{
	for (auto &item : jsonStyleArray.array_items()) {
		if (!item.is_object()) {
			logger->warn("Skipping text style that is not an object.");
			continue;
		}

		auto idNode = item["id"];
		if (!idNode.is_string()) {
			logger->warn("Skipping text style that is missing 'id' attribute.");
			continue;
		}
		auto id = eastl::string(idNode.string_value().c_str());

		// Process the inherit attribute (what is the base style)
		auto inherit = item["inherit"];
		gfx::TextStyle style = mDefaultStyle;
		if (inherit.is_string()) {
			auto inheritId = inherit.string_value();
			if (!HasStyle(eastl::string(inheritId.c_str()))) {
				logger->warn("Style {} inherits from unknown style {}", id, inheritId);
			}
			style = GetTextStyle(inheritId.c_str());
		}

		// Every other attribute from here on out is optional
		if (item["fontFamily"].is_string()) {
			style.fontFace = item["fontFamily"].string_value().c_str();
		}

		if (item["pointSize"].is_number()) {
			style.pointSize = (float)item["pointSize"].number_value();
		}

		if (item["bold"].is_bool()) {
			style.bold = item["bold"].bool_value();
		}

		if (item["italic"].is_bool()) {
			style.italic = item["italic"].bool_value();
		}

		if (item["align"].is_string()) {
			auto align = item["align"].string_value();
			if (align == "left") {
				style.align = gfx::TextAlign::Left;
			}
			else if (align == "center") {
				style.align = gfx::TextAlign::Center;
			}
			else if (align == "right") {
				style.align = gfx::TextAlign::Right;
			}
			else if (align == "justified") {
				style.align = gfx::TextAlign::Justified;
			}
			else {
				logger->warn("Invalid text alignment: '{}'", align);
			}
		}

		if (item["paragraphAlign"].is_string()) {
			auto align = item["paragraphAlign"].string_value();
			if (align == "near") {
				style.paragraphAlign = gfx::ParagraphAlign::Near;
			}
			else if (align == "far") {
				style.paragraphAlign = gfx::ParagraphAlign::Far;
			}
			else if (align == "center") {
				style.paragraphAlign = gfx::ParagraphAlign::Center;
			}
			else {
				logger->warn("Invalid paragraph alignment: '{}'", align);
			}
		}

		if (!item["foreground"].is_null()) {
			style.foreground = ParseBrush(item["foreground"]);
		}

		if (item["uniformLineHeight"].is_bool()) {
			style.uniformLineHeight = item["uniformLineHeight"].bool_value();
		}

		if (item["lineHeight"].is_number()) {
			style.lineHeight = (float)item["lineHeight"].number_value();
		}

		if (item["baseLine"].is_number()) {
			style.baseLine = (float)item["baseLine"].number_value();
		}

		if (item["dropShadow"].is_bool()) {
			style.dropShadow = item["dropShadow"].bool_value();
		}

		if (!item["dropShadowBrush"].is_null()) {
			style.dropShadowBrush = ParseBrush(item["dropShadowBrush"]);
		}

		AddStyle(id, style);
	}
}

WidgetButtonStyles* widgetButtonStyles = nullptr;

WidgetButtonStyles::WidgetButtonStyles()
{
	Expects(!widgetButtonStyles);
	widgetButtonStyles = this;
	LoadStylesFile("templeplus/button_styles.json");
}

WidgetButtonStyles::~WidgetButtonStyles()
{
	if (widgetButtonStyles == this) {
		widgetButtonStyles = nullptr;
	}
}

void WidgetButtonStyles::AddStyle(const eastl::string & id, const WidgetButtonStyle &style)
{
	auto it = mStyles.find(id);
	if (it != mStyles.end()) {
		throw TempleException("Duplicate button style defined: {}", id);
	}
	mStyles.emplace(id, std::move(style));
}

bool WidgetButtonStyles::HasStyle(const eastl::string & id) const
{
	return mStyles.find(id) != mStyles.end();
}

void WidgetButtonStyles::LoadStylesFile(const std::string & path)
{
	std::string error;
	json11::Json json = json.parse(vfs->ReadAsString(path), error);

	if (json.is_null()) {
		throw TempleException("Unable to parse button styles from {}: {}", path, error);
	}

	if (!json.is_array()) {
		throw TempleException("Button style files must start with an array at the root");
	}

	LoadStyles(json);
}

void WidgetButtonStyles::LoadStyles(const json11::Json & jsonStyleArray)
{
	for (auto &style : jsonStyleArray.array_items()) {

		eastl::string id(style["id"].string_value().c_str());
		if (id.empty()) {
			throw TempleException("Found button style without id!");
		}

		// Process the inherit attribute (what is the base style)
		auto inherit = style["inherit"];
		WidgetButtonStyle buttonStyle;
		if (inherit.is_string()) {
			auto inheritId = inherit.string_value();
			if (!HasStyle(eastl::string(inheritId.c_str()))) {
				logger->warn("Style {} inherits from unknown style {}", id, inheritId);
			}
			buttonStyle = GetStyle(inheritId.c_str());
		}

		buttonStyle.textStyleId = style["textStyle"].string_value();
		buttonStyle.hoverTextStyleId = style["hoverTextStyle"].string_value();
		buttonStyle.pressedTextStyleId = style["pressedTextStyle"].string_value();
		buttonStyle.disabledTextStyleId = style["disabledTextStyle"].string_value();
		buttonStyle.disabledImagePath = style["disabledImage"].string_value();
		buttonStyle.normalImagePath = style["normalImage"].string_value();
		buttonStyle.hoverImagePath = style["hoverImage"].string_value();
		buttonStyle.pressedImagePath = style["pressedImage"].string_value();
		if (style["soundEnter"].is_number()) {
			buttonStyle.soundEnter = (int) style["soundEnter"].number_value();
		}
		if (style["soundLeave"].is_number()) {
			buttonStyle.soundLeave = (int)style["soundLeave"].number_value();
		}
		if (style["soundDown"].is_number()) {
			buttonStyle.soundDown = (int)style["soundDown"].number_value();
		}
		if (style["soundClick"].is_number()) {
			buttonStyle.soundClick = (int)style["soundClick"].number_value();
		}

		if (mStyles.find(id) != mStyles.end()) {
			throw TempleException("Duplicate button style: {}", id);
		}
		
		mStyles[id] = buttonStyle;

	}
}

const WidgetButtonStyle& WidgetButtonStyles::GetStyle(const eastl::string & id) const
{
	static WidgetButtonStyle sDefaultStyle;
	auto it = mStyles.find(id);
	if (it != mStyles.end()) {
		return it->second;
	}
	return sDefaultStyle;
}
