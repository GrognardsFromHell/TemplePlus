
#pragma once

#include <string>
#include <infrastructure/mesparser.h>

#include "ui_system.h"

class UiOptions : public UiSystem {
public:
	static constexpr auto Name = "Options-UI";
	UiOptions(int width, int height);
	~UiOptions();
	void ResizeViewport(const UiResizeArgs &resizeArgs) override;
	const std::string &GetName() const override;

private:
	MesFile::Content mText;
};
