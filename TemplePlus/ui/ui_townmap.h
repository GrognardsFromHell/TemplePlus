#pragma once

#include "ui/ui.h"
#include "ui_system.h"

struct UiSystemConf;
class WidgetContainer;
using POSARRAY = XMFLOAT3[200];
class UiTownmap : public UiSystem, public SaveGameAwareUiSystem {
public:
	static constexpr auto Name = "Townmap-UI";
	UiTownmap(const UiSystemConf &config);
	~UiTownmap();
	void Reset() override;
	bool SaveGame(TioFile *file) override;
	bool LoadGame(const UiSaveFile &saveGame) override;
	void ResizeViewport(const UiResizeArgs &resizeArgs) override;
	const std::string &GetName() const override;

	// Was @ 10128B60 (ui_townmap_is_visible)
	bool IsVisible() const {
		return mVisible != 0;
	}

	bool IsAvailable();
	
	void Show();
	void Hide();

	void PositionsInit();
	void CenterOnParty(); // centers display on party leader

private:
	
	int& mVisible = temple::GetRef<int>(0x10BE1F28);
	int& mAvailable = temple::GetRef<int>(0x10BE1F74);
	int& mPositionsInited = temple::GetRef<int>(0x10BE1F38);
	int& mCurrentlyDisplayedMapIdx = temple::GetRef<int>(0x10BE1F48);
	float& mX = temple::GetRef<float>(0x10BE1F4C);
	float& mY = temple::GetRef<float>(0x10BE1F50);
	int& mEdgeX = temple::GetRef<int>(0x10BE1F54);
	int& mEdgeY = temple::GetRef<int>(0x10BE1F58);
	POSARRAY& mPositions = temple::GetRef<POSARRAY>(0x10BE0D70);
	int (&mMapsVisited)[200] = temple::GetRef<int[200]>(0x10BE1AD8);
	//std::unique_ptr<WidgetContainer> mMainWidget;
	std::map<int, WidgetContainer*> mPageWidgets;
	// The widget that contains all pages
	WidgetContainer *mPagesWidget;

	void RepositionWidgets(int width, int height);
};
