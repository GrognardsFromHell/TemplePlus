#pragma once

#include "ui_system.h"

class UiTurnBased : public UiSystem {
public:
	static constexpr auto Name = "TurnBased";
	UiTurnBased(int width, int height);
	~UiTurnBased();
	void Reset() override;
	void ResizeViewport(const UiResizeArgs &resizeArgs) override;
	const std::string &GetName() const override;

private:
	LgcyWidgetId &mWidgetId = temple::GetRef<LgcyWidgetId>(0x10C04108);
	objHndl &mObjHndl = temple::GetRef<objHndl>(0x10C04120);
	
};
