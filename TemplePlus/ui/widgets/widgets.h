
#pragma once

#include <EASTL/vector.h>
#include <memory>
#include "../ui.h"
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

	TigRect GetContentArea() const;

protected:
	LgcyWidget *mWidget = nullptr;
	WidgetContainer *mParent = nullptr;
	std::string mSourceURI;
	std::string mId;
	bool mCenterHorizontally = false;
	bool mCenterVertically = false;
	bool mSizeToParent = false;

	eastl::vector<std::unique_ptr<WidgetContent>> mContent;
};

class WidgetContainer : public WidgetBase {
public:
	WidgetContainer(int width, int height);

	void Add(std::unique_ptr<WidgetBase> childWidget);

	WidgetBase *PickWidget(int x, int y) override;

	bool IsContainer() const override {
		return true;
	}

	const eastl::vector<std::unique_ptr<WidgetBase>> &GetChildren() const {
		return mChildren;
	}

private:
	LgcyWindow *mWindow;
	eastl::vector<std::unique_ptr<WidgetBase>> mChildren;
};

class WidgetButtonBase : public WidgetBase {
public:
	WidgetButtonBase();

	bool HandleMessage(const TigMsg &msg);

	void SetDisabled(bool disabled) {
		mDisabled = true;
	}
	bool IsDisabled() const {
		return mDisabled;
	}
	void SetClickHandler(std::function<void()> handler) {
		mClickHandler = handler;
	}

	bool IsButton() const override {
		return true;
	}

protected:
	LgcyButton *mButton;
	bool mDisabled = false;

	std::function<void()> mClickHandler;
};

struct WidgetButtonStyle {
	std::string normalImagePath;
	std::string hoverImagePath;
	std::string pressedImagePath;
	std::string disabledImagePath;
	std::string textStyleId;
	std::string hoverTextStyleId;
	std::string pressedTextStyleId;
	std::string disabledTextStyleId;
};

class WidgetButton : public WidgetButtonBase {
public:

	WidgetButton();

	void SetStyle(const WidgetButtonStyle &style) 
	{
		mStyle = style;
		UpdateContent();
	}
	const WidgetButtonStyle &GetStyle() 
	{
		return mStyle;
	}

	void Render() override;

	void SetText(const std::string &text);

private:
	WidgetButtonStyle mStyle;

	void UpdateContent();
	void UpdateAutoSize();

	std::unique_ptr<WidgetImage> mNormalImage;
	std::unique_ptr<WidgetImage> mHoverImage;
	std::unique_ptr<WidgetImage> mPressedImage;
	std::unique_ptr<WidgetImage> mDisabledImage;
	WidgetText mLabel;
	
};
