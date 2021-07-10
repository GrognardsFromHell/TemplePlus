#pragma once

#include "ui_system.h"
#include "widgets/widgets.h"

struct UiSystemConf;

class UiPython : public UiSystem {
public:
	static constexpr auto Name = "python_ui";
	UiPython(const UiSystemConf& config);
	~UiPython();
	const std::string& GetName() const override;

	WidgetContainer* GetRootWidget(const std::string& id);
	WidgetBase* GetWidget(const std::string& id);
	WidgetContainer* AddRootWidget(const std::string& id);
	void AddWidget(WidgetBase* wid);

protected:

	std::map<std::string, WidgetBase* > pyWidgets;
	std::map<std::string, std::unique_ptr<WidgetContainer>> mRootWidgets;
};