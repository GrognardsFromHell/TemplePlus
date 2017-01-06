
#include "stdafx.h"
#include "widget_doc.h"
#include "widgets.h"
#include "widget_styles.h"

#include <infrastructure/vfs.h>
#include <infrastructure/json11.hpp>

using WidgetPtr = std::unique_ptr<WidgetBase>;

struct WidgetLoadingContext {
	const std::string &sourceUri;
	WidgetRegistry &widgetRegistry;
};

static std::unique_ptr<WidgetBase> LoadWidgetTree(const json11::Json &jsonObj, WidgetLoadingContext &context);
static void LoadWidgetBase(const json11::Json &jsonObj, WidgetBase &widget, WidgetLoadingContext &context);

static void LoadContent(const json11::Json &contentList, WidgetBase &widget) {
	for (auto &contentJson : contentList.array_items()) {
		auto type = contentJson["type"].string_value();
		
		std::unique_ptr<WidgetContent> content;
		if (type == "image") {
			auto path = contentJson["path"].string_value();
			content = std::make_unique<WidgetImage>(path);
		} else if (type == "text") {
			auto text = contentJson["text"].string_value();
			auto styleId = contentJson["style"].string_value();
			auto textContent = std::make_unique<WidgetText>();
			textContent->SetStyleId(styleId);
			textContent->SetText(text);
			content = std::move(textContent);
		} else {
			throw TempleException("Unknown widget content type: '{}'", type);
		}

		// Generic properties
		if (contentJson["x"].is_number()) {
			content->SetX((int) contentJson["x"].number_value());
		}
		if (contentJson["y"].is_number()) {
			content->SetY((int)contentJson["y"].number_value());
		}
		if (contentJson["width"].is_number()) {
			content->SetFixedWidth((int)contentJson["width"].number_value());
		}
		if (contentJson["height"].is_number()) {
			content->SetFixedHeight((int)contentJson["height"].number_value());
		}

		widget.AddContent(std::move(content));
	}
}

void LoadWidgetBase(const json11::Json &jsonObj, WidgetBase &widget, WidgetLoadingContext &context) {

	for (auto &styleSheetName : jsonObj["__styleFiles"].array_items()) {
		widgetTextStyles->LoadStylesFile(styleSheetName.string_value());
	}
	widgetTextStyles->LoadStyles(jsonObj["__styles"]);


	for (auto &style : jsonObj["__buttonStyleFiles"].array_items()) {
		widgetButtonStyles->LoadStyles(style);
	}
	widgetButtonStyles->LoadStyles(jsonObj["__buttonStyles"]);

	int x = (int) jsonObj["x"].number_value();
	int y = (int) jsonObj["y"].number_value();
	widget.SetPos(x, y);

	auto size = widget.GetSize();
	if (jsonObj["width"].is_number()) {
		size.width = (int)jsonObj["width"].number_value();
	}
	if (jsonObj["height"].is_number()) {
		size.height = (int)jsonObj["height"].number_value();
	}
	widget.SetSize(size);

	if (jsonObj["centerHorizontally"].is_bool()) {
		widget.SetCenterHorizontally(jsonObj["centerHorizontally"].bool_value());
	}
	if (jsonObj["centerVertically"].is_bool()) {
		widget.SetCenterVertically(jsonObj["centerVertically"].bool_value());
	}
	if (jsonObj["sizeToParent"].is_bool()) {
		widget.SetSizeToParent(jsonObj["sizeToParent"].bool_value());
	}

}

static std::unique_ptr<WidgetBase> LoadWidgetContainer(const json11::Json &jsonObj, WidgetLoadingContext &context) {

	auto width = (int)jsonObj["width"].number_value();
	auto height = (int) jsonObj["height"].number_value();

	auto result = std::make_unique<WidgetContainer>(width, height);

	LoadWidgetBase(jsonObj, *result, context);

	LoadContent(jsonObj["content"], *result);

	for (auto &childJson : jsonObj["children"].array_items()) {
		auto childWidget = LoadWidgetTree(childJson, context);
		childWidget->SetParent(result.get());
		result->Add(std::move(childWidget));
	}

	return result;
}


static std::unique_ptr<WidgetBase> LoadWidgetButton(const json11::Json &jsonObj, WidgetLoadingContext &context) {

	auto result = std::make_unique<WidgetButton>();

	LoadWidgetBase(jsonObj, *result, context);
	
	result->SetText(jsonObj["text"].string_value());

	WidgetButtonStyle buttonStyle;
	eastl::string styleId(jsonObj["style"].string_value().c_str());
	if (!styleId.empty()) {
		buttonStyle = widgetButtonStyles->GetStyle(styleId);
	}

	// Allow local overrides
	auto obj = jsonObj.object_items();
	if (obj.find("disabledImage") != obj.end()) {
		buttonStyle.disabledImagePath = obj["disabledImage"].string_value();
	}

	if (obj.find("normalImage") != obj.end()) {
		buttonStyle.normalImagePath = obj["normalImage"].string_value();
	}

	if (obj.find("hoverImage") != obj.end()) {
		buttonStyle.hoverImagePath = obj["hoverImage"].string_value();
	}

	if (obj.find("pressedImage") != obj.end()) {
		buttonStyle.pressedImagePath = obj["pressedImage"].string_value();
	}

	if (obj.find("textStyle") != obj.end()) {
		buttonStyle.textStyleId = obj["textStyle"].string_value();
	}

	if (obj.find("hoverTextStyle") != obj.end()) {
		buttonStyle.hoverTextStyleId = obj["hoverTextStyle"].string_value();
	}

	if (obj.find("pressedTextStyle") != obj.end()) {
		buttonStyle.pressedTextStyleId = obj["pressedTextStyle"].string_value();
	}

	if (obj.find("disabledTextStyle") != obj.end()) {
		buttonStyle.disabledTextStyleId = obj["disabledTextStyle"].string_value();
	}

	result->SetStyle(buttonStyle);

	return result;

}

static std::unique_ptr<WidgetBase> LoadWidgetScrollBar(const json11::Json &jsonObj, WidgetLoadingContext &context) {

	auto result = std::make_unique<WidgetScrollBar>();

	LoadWidgetBase(jsonObj, *result, context);
	
	return result;

}

static std::map<std::string, std::function<WidgetPtr(const json11::Json&, WidgetLoadingContext &context)>> sWidgetFactories {
	{ "container", LoadWidgetContainer },
	{ "button", LoadWidgetButton },
	{ "scrollBar", LoadWidgetScrollBar },
};

static std::unique_ptr<WidgetBase> LoadWidgetTree(const json11::Json &jsonObj, WidgetLoadingContext &context) {

	auto type = jsonObj["type"].string_value();

	// Is there a factory for the type?
	auto facIt = sWidgetFactories.find(type);
	if (facIt == sWidgetFactories.end()) {
		throw TempleException("Cannot process unknown widget type: '{}'", type);
	}

	auto widget = facIt->second(jsonObj, context);
	widget->SetSourceURI(context.sourceUri);

	auto &widgetRegistry = context.widgetRegistry;

	// If the widget had an ID, put it into the registry
	auto id = jsonObj["id"].string_value();
	if (!id.empty()) {
		if (widgetRegistry.find(id) != widgetRegistry.end()) {
			throw TempleException("Duplicate widget id: {}", id);
		}
		widgetRegistry[id] = widget.get();
		widget->SetId(id);
	}

	return widget;

}

WidgetDoc::WidgetDoc(const std::string &path, std::unique_ptr<WidgetBase> root, const std::map<std::string, WidgetBase*> &registry)
	: mPath(path), mRootWidget(std::move(root)), mWidgetsById(registry)
{
}

WidgetDoc WidgetDoc::Load(const std::string & path)
{
	std::string err;
	auto root = json11::Json::parse(vfs->ReadAsString(path), err);

	if (!root.is_object()) {
		throw TempleException("Unable to load widget doc '{}': {}", path, err);
	}

	WidgetRegistry registry;
	WidgetLoadingContext context{ path, registry };
	try {
		auto rootWidget = LoadWidgetTree(root, context);

		return WidgetDoc{ path, std::move(rootWidget), registry };
	} catch (const std::exception &e) {
		throw TempleException("Unable to load widget doc '{}': {}", path, e.what());
	}
}

std::unique_ptr<WidgetBase> WidgetDoc::TakeRootWidget()
{
	Expects(!!mRootWidget);
	return std::move(mRootWidget);
}

std::unique_ptr<WidgetContainer> WidgetDoc::TakeRootContainer()
{
	Expects(!!mRootWidget);
	if (!mRootWidget->IsContainer()) {
		throw TempleException("Expected root widget in '{}' to be a container.");
	}

	return std::unique_ptr<WidgetContainer>((WidgetContainer*) mRootWidget.release());
}

WidgetBase *WidgetDoc::GetWidget(const std::string & id) const
{
	auto it = mWidgetsById.find(id);
	if (it == mWidgetsById.end()) {
		throw TempleException("Couldn't find required widget id '{}' in widget doc '{}'", id, mPath);
	}
	return it->second;
}

WidgetContainer * WidgetDoc::GetWindow(const std::string & id) const
{
	auto widget = GetWidget(id);
	if (!widget->IsContainer()) {
		throw TempleException("Expected widget with id '{}' in doc '{}' to be a container!", id, mPath);
	}
	return (WidgetContainer*)widget;	
}

WidgetButtonBase * WidgetDoc::GetButton(const std::string & id) const
{
	auto widget = GetWidget(id);
	if (!widget->IsButton()) {
		throw TempleException("Expected widget with id '{}' in doc '{}' to be a button!", id, mPath);
	}
	return (WidgetButton*)widget;
}
