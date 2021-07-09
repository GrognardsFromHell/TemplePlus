
#pragma once

#include <EASTL/vector.h>
#include <memory>
#include "../ui.h"
#include "tig/tig_msg.h"
#include "widget_content.h"

class WidgetContent;
class WidgetContainer;

class WidgetBase {
public:
	virtual ~WidgetBase();
	
	virtual void Render();

	virtual bool HandleMessage(const TigMsg &msg);

	virtual bool IsContainer() const {
		return false;
	}
	virtual bool IsButton() const {
		return false;
	}
	virtual bool IsScrollView() const {
		return false;
	}

	/**
	 * Picks the widget a the x,y coordinate local to this widget. 
	 * Null if the coordinates are outside of this widget. If no 
	 * other widget inside is at the given coordinate, will just return this.
	 */
	virtual WidgetBase *PickWidget(int x, int y);

	void AddContent(std::unique_ptr<WidgetContent> content);

	void Show();
	void Hide();
	void SetVisible(bool visible);
	bool IsVisible() const;

	void BringToFront();
		
	void SetParent(WidgetContainer *parent) {
		Expects(!mParent || mParent == parent);
		mParent = parent;
	}
	WidgetContainer *GetParent() const {
		return mParent;
	}

	void SetPos(int x, int y);
	DirectX::XMINT2 GetPos() const;
	int GetX() const {
		return GetPos().x;
	}
	int GetY() const {
		return GetPos().y;
	}
	void SetX(int x) {
		SetPos(x, GetY());
	}
	void SetY(int y) {
		SetPos(GetX(), y);
	}

	int GetWidth() const {
		return GetSize().width;
	}
	int GetHeight() const {
		return GetSize().height;
	}

	LgcyWidgetId GetWidgetId() const {
		return mWidget->widgetId;
	}

	void SetWidth(int width) {
		SetSize({ width, GetHeight() });
	}
	void SetHeight(int height) {
		SetSize({ GetWidth(), height });
	}

	void SetSize(gfx::Size size);
	gfx::Size GetSize() const;

	/**
	 * A unique id for this widget within the source URI (see below).
	 */
	const std::string &GetId() const {
		return mId;
	}
	void SetId(const std::string &id) {
		mId = id;
	}

	/**
	 * If this widget was loaded from a file, indicates the URI to that file to more easily identify it.
	 */
	const std::string &GetSourceURI() const {
		return mSourceURI;
	}
	void SetSourceURI(const std::string &sourceUri) {
		mSourceURI = sourceUri;
	}

	void SetCenterHorizontally(bool enable) {
		mCenterHorizontally = enable;
	}
	void SetCenterVertically(bool enable) {
		mCenterVertically = enable;
	}
	void SetSizeToParent(bool enable) {
		mSizeToParent = enable;
	}

	/*
	 Returns the {x,y,w,h} rect, but regards modification from parent and subtracts the margins.
	 Content area controls:
	 - Mouse handling active area
	 - Rendering area
	 */
	TigRect GetContentArea(bool includingMargins = false) const; 
	TigRect GetVisibleArea() const;

	void SetMouseMsgHandler(std::function<bool(const TigMouseMsg &msg)> handler) {
		mMouseMsgHandler = handler;
	}

	void SetWidgetMsgHandler(std::function<bool(const TigMsgWidget &msg)> handler) {
		mWidgetMsgHandler = handler;
	}

	void SetKeyStateChangeHandler(std::function<bool(const TigKeyStateChangeMsg &msg)> handler) {
		mKeyStateChangeHandler = handler;
	}

	void SetCharHandler(std::function<bool(const TigCharMsg &msg)> handler) {
		mCharHandler = handler;
	}

	virtual bool HandleMouseMessage(const TigMouseMsg &msg);

	virtual void OnUpdateTime(uint32_t timeMs);

	void SetAutoSizeWidth(bool enable) {
		mAutoSizeWidth = enable;
	}
	void SetAutoSizeHeight(bool enable) {
		mAutoSizeHeight = enable;
	}

protected:
	LgcyWidget *mWidget = nullptr;
	WidgetContainer *mParent = nullptr;
	std::string mSourceURI;
	std::string mId;
	bool mCenterHorizontally = false;
	bool mCenterVertically = false;
	bool mSizeToParent = false;
	bool mAutoSizeWidth = true;
	bool mAutoSizeHeight = true;
	RECT mMargins = {0,0,0,0};
	std::function<bool(const TigMouseMsg &msg)> mMouseMsgHandler;
	std::function<bool(const TigMsgWidget &msg)> mWidgetMsgHandler;
	std::function<bool(const TigKeyStateChangeMsg &msg)> mKeyStateChangeHandler;
	std::function<bool(const TigCharMsg &msg)> mCharHandler;

	eastl::vector<std::unique_ptr<WidgetContent>> mContent;
};

class WidgetContainer : public WidgetBase {
public:
	WidgetContainer(const WidgetContainer&) = delete; // delete copy constructor, for pybind

	WidgetContainer(int width, int height);

	virtual void Add(std::unique_ptr<WidgetBase> childWidget);
	virtual void Clear();

	WidgetBase *PickWidget(int x, int y) override;

	bool IsContainer() const override {
		return true;
	}

	const eastl::vector<std::unique_ptr<WidgetBase>> &GetChildren() const {
		return mChildren;
	}

	void Render() override;

	bool HandleMouseMessage(const TigMouseMsg &msg) override;

	void SetScrollOffsetY(int scrollY);
	int GetScrollOffsetY() const {
		return mScrollOffsetY;
	}

private:
	LgcyWindow *mWindow;
	eastl::vector<std::unique_ptr<WidgetBase>> mChildren;
	
	int mScrollOffsetY = 0;
};

class WidgetButtonBase : public WidgetBase {
public:
	WidgetButtonBase();

	bool HandleMessage(const TigMsg &msg);

	void SetDisabled(bool disabled) {
		mDisabled = disabled;
	}
	bool IsDisabled() const {
		return mDisabled;
	}

	void SetActive(bool isActive){
		mActive = isActive;
	}
	bool IsActive() const{
		return mActive;
	}

	void SetClickHandler(std::function<void()> handler) {
		mClickHandler = [=](int, int) { handler(); };
	}
	void SetClickHandler(std::function<void(int x, int y)> handler) {
		mClickHandler = handler;
	}

	bool IsButton() const override {
		return true;
	}

	bool IsRepeat() const {
		return mRepeat;
	}
	void SetRepeat(bool enable) {
		mRepeat = enable;
	}
	int GetRepeatInterval() const {
		return mRepeatInterval;
	}
	void SetRepeatInterval(int interval) {
		mRepeatInterval = interval;
	}

	void OnUpdateTime(uint32_t timeMs) override;
	
protected:
	LgcyButton *mButton;
	bool mDisabled = false;
	bool mActive = false; // is the state associated with the button active? Note: this is separate from mDisabled, which determines if the button itself is disabled or not
	bool mRepeat = false;
	uint32_t mRepeatInterval = 200;
	uint32_t mLastClickTriggered = 0;

	std::function<void(int x, int y)> mClickHandler;
};

struct WidgetButtonStyle {
	std::string normalImagePath;
	std::string activatedImagePath;
	std::string hoverImagePath;
	std::string pressedImagePath;
	std::string disabledImagePath;
	std::string frameImagePath;

	std::string textStyleId;
	std::string hoverTextStyleId;
	std::string pressedTextStyleId;
	std::string disabledTextStyleId;
	int soundEnter = -1;
	int soundLeave = -1;
	int soundDown = -1;
	int soundClick = -1;
};

class WidgetButton : public WidgetButtonBase {
public:
	WidgetButton(const WidgetButton&) = delete; // delete copy constructor, for pybind

	WidgetButton();

	/*
	 central style definitions:
	 templeplus/button_styles.json
	 */
	void SetStyle(const WidgetButtonStyle &style);
	/*
	 directly fetch style from widgetButtonStyles
	 */
	void SetStyle(const eastl::string& styleName);
	const WidgetButtonStyle &GetStyle() 
	{
		return mStyle;
	}

	void Render() override;

	void SetText(const std::string &text);

private:
	WidgetButtonStyle mStyle;

	/*
	  1. updates the WidgetImage pointers below, using WidgetButtonStyle file paths
	  2. Updates mLabel
	 */
	void UpdateContent();
	void UpdateAutoSize();

	std::unique_ptr<WidgetImage> mNormalImage;
	std::unique_ptr<WidgetImage> mActivatedImage;
	std::unique_ptr<WidgetImage> mHoverImage;
	std::unique_ptr<WidgetImage> mPressedImage;
	std::unique_ptr<WidgetImage> mDisabledImage;
	std::unique_ptr<WidgetImage> mFrameImage;
	WidgetText mLabel;
	
};


class WidgetScrollBarHandle;

class WidgetScrollBar : public WidgetContainer {
friend class WidgetScrollBarHandle;
public:

	WidgetScrollBar();

	int GetMin() const {
		return mMin;
	}
	void SetMin(int value) {
		mMin = value;
		if (mMin > mMax) {
			mMin = mMax;
		}
		if (mValue < mMin) {
			SetValue(mMin);
		}
	}
	int GetMax() const {
		return mMax;
	}
	void SetMax(int value) {
		mMax = value;
		if (mMax < mMin) {
			mMax = mMin;
		}
		if (mValue > mMax) {
			SetValue(mMax);
		}
	}
	int GetValue() const {
		return mValue;
	}
	void SetValue(int value) {
		if (value < mMin) {
			value = mMin;
		}
		if (value > mMax) {
			value = mMax;
		}
		mValue = value;
		if (mValueChanged) {
			mValueChanged(mValue);
		}
	}

	void Render() override;

	void SetValueChangeHandler(std::function<void(int)> handler) {
		mValueChanged = handler;
	}

private:
	LgcyScrollBar *mScrollBar;

	std::function<void(int)> mValueChanged;

	int mValue = 0;
	int mMin = 0;
	int mMax = 150;

	WidgetButton *mUpButton;
	WidgetButton *mDownButton;
	WidgetButton *mTrack;
	WidgetScrollBarHandle *mHandleButton;

	int GetHandleHeight() const; // gets height of handle button (scaled according to Min/Max values)
	int GetScrollRange() const; // gets range of possible values for Handle Button position
	int GetTrackHeight() const; // gets height of track area

};

class WidgetScrollView : public WidgetContainer {
public:
	WidgetScrollView(int width, int height);

	void Add(std::unique_ptr<WidgetBase> childWidget) override;
	void Clear() override;

	int GetInnerWidth() const;
	int GetInnerHeight() const;
	
	bool IsScrollView() const override {
		return true;
	}

	void SetPadding(int padding);
	int GetPadding() const;

	bool HandleMouseMessage(const TigMouseMsg &msg) override;

private:
	WidgetContainer *mContainer;
	WidgetScrollBar *mScrollBar;
	int mPadding = 5;

	void UpdateInnerHeight();
	void UpdateLayout();
};
