#pragma once


struct UiSystemConf;

class UiDmWidgets
{
	
public:
	bool DmWidgetsInit(const UiSystemConf & conf);


	void DmBtnRender(int widId);
	void Activate();
	void SetButtonVis(bool newState);
	void SetDmPortraitRect(TigRect & rect);

protected:
	LgcyWidgetId dmBtnWndId  = 0;
	LgcyWidgetId dmBtnId = 0;

	int mTexId; // Raptor texture
	int mIconTexId; // Raptor texture
	TigRect mRaptorRect = TigRect(0,0,96,96); // Raptor Rect

};

extern UiDmWidgets uiDm;