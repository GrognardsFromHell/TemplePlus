
#pragma once

#include <string>
#include <map>

#include "widgets.h"

using WidgetRegistry = std::map<std::string, WidgetBase*>;

/**
 * Contains a definition for a grabbag of widgets. 
 */
class WidgetDoc {
public:
	WidgetDoc(const std::string &path,
			  std::unique_ptr<WidgetBase> root,
			  const WidgetRegistry &registry);

	static WidgetDoc Load(const std::string &path);

	/** 
	 * Returns the root widget defined in the widget doc. The caller takes ownership of the widget.
	 * This function can only be called once per widget doc instance!
	 */
	std::unique_ptr<WidgetBase> TakeRootWidget();

	/**
 	 * Returns the root widget defined in the widget doc, assuming it is a container widget. 
	 * If the root widget is NOT a container, this method will throw an exception.
	 * The caller takes ownership of the widget.
	 * This function can only be called once per widget doc instance!
	 */
	std::unique_ptr<WidgetContainer> TakeRootContainer();

	WidgetBase *GetWidget(const std::string &id) const;
	WidgetContainer *GetWindow(const std::string &id) const;
	WidgetButtonBase *GetButton(const std::string &id) const;

private:
	std::string mPath;
	std::unique_ptr<WidgetBase> mRootWidget;
	WidgetRegistry mWidgetsById;
};
