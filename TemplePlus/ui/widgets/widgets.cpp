
#include "stdafx.h"
#include "widgets.h"
#include "widget_content.h"
#include "widget_styles.h"
#include "tig/tig_msg.h"
#include "tig/tig_startup.h"
#include "tig/tig_mouse.h"
#include "ui/ui_render.h"
#include "messages/messagequeue.h"
#include "gameview.h"

#include <graphics/device.h>

static void RenderWidget(LgcyWidgetId widgetId) {
	auto widget = uiManager->GetAdvancedWidget(widgetId);
	widget->Render();
}

static BOOL HandleMessage(LgcyWidgetId widgetId, TigMsg *msg) {
	auto widget = uiManager->GetAdvancedWidget(widgetId);
	return widget->HandleMessage(*msg) ? TRUE : FALSE;
}

WidgetBase * WidgetBase::PickWidget(int x, int y)
{
	if (!IsVisible()) {
		return nullptr;
	}

	if (x >= 0 && y >= 0 && x < (int) mWidget->width && y < (int) mWidget->height) {
		return this;
	}
	return nullptr;
}

void WidgetBase::AddContent(std::unique_ptr<WidgetContent> content)
{
	mContent.emplace_back(std::move(content));
}

void WidgetBase::Show()
{
	SetVisible(true);
}

void WidgetBase::Hide()
{
	SetVisible(false);
}

void WidgetBase::SetVisible(bool visible)
{
	uiManager->SetHidden(mWidget->widgetId, !visible);
}

bool WidgetBase::IsVisible() const
{
	return !mWidget->IsHidden();
}

void WidgetBase::BringToFront()
{
	uiManager->BringToFront(mWidget->widgetId);
}

void WidgetBase::SetPos(int x, int y)
{
	mWidget->x = x;
	mWidget->y = y;
}

DirectX::XMINT2 WidgetBase::GetPos() const
{
	return{ mWidget->x, mWidget->y };
}

WidgetBase::~WidgetBase()
{
	if (mWidget->parentId != -1) {
		uiManager->RemoveChildWidget(GetWidgetId());
	}
	if (mParent) {
		// TODO mParent->Remove(this);
	}
	uiManager->RemoveWidget(GetWidgetId());
}

void WidgetBase::Render()
{
	if (!IsVisible()) {
		return;
	}

	if (mSizeToParent) {
		int containerWidth = mParent ? mParent->GetWidth() : (int)tig->GetRenderingDevice().GetCamera().GetScreenWidth();
		int containerHeight = mParent ? mParent->GetHeight() : (int)tig->GetRenderingDevice().GetCamera().GetScreenHeight();
		SetSize({ containerWidth, containerHeight });
	}

	TigRect contentArea(GetContentArea());

	// Size to content
	if (contentArea.width == 0 && contentArea.height == 0) {
		for (auto &content : mContent) {
			auto preferred = content->GetPreferredSize();
			contentArea.width = std::max(contentArea.width, preferred.width);
			contentArea.height = std::max(contentArea.height, preferred.height);
		}
		if (contentArea.width != 0 && contentArea.height != 0) {
			mWidget->width = contentArea.width;
			mWidget->height = contentArea.height;
		}
	}
	
	if (mCenterHorizontally) {
		int containerWidth = mParent ? mParent->GetWidth() : (int)tig->GetRenderingDevice().GetCamera().GetScreenWidth();		
		int x = (containerWidth - GetWidth()) / 2;
		if (x != GetX()) {
			SetX(x);
			contentArea = GetContentArea();
		}
	}

	if (mCenterVertically) {
		int containerHeight = mParent ? mParent->GetHeight() : (int)tig->GetRenderingDevice().GetCamera().GetScreenHeight();
		int y = (containerHeight - GetHeight()) / 2;
		if (y != GetY()) {
			SetY(y);
			contentArea = GetContentArea();
		}
	}
	
	for (auto &content : mContent) {
		TigRect specificContentArea = contentArea;
		// Shift according to the content item positioning
		if (content->GetX() != 0) {
			specificContentArea.x += content->GetX();
			specificContentArea.width -= content->GetX();
		}
		if (content->GetY() != 0) {
			specificContentArea.y += content->GetY();
			specificContentArea.height -= content->GetY();
		}

		// Constraint width if necessary
		if (content->GetFixedWidth() != 0 && content->GetFixedWidth() < specificContentArea.width) {
			specificContentArea.width = content->GetFixedWidth();
		}
		if (content->GetFixedHeight() != 0 && content->GetFixedHeight() < specificContentArea.height) {
			specificContentArea.height = content->GetFixedHeight();
		}

		if (content->GetContentArea() != specificContentArea) {
			content->SetContentArea(specificContentArea);
		}
		
		content->Render();
	}

}

bool WidgetBase::HandleMessage(const TigMsg &msg)
{
	if (msg.type == TigMsgType::WIDGET && mWidgetMsgHandler) {
		return mWidgetMsgHandler((const TigMsgWidget&)msg);
	} else if (msg.type == TigMsgType::KEYSTATECHANGE && mKeyStateChangeHandler) {
		return mKeyStateChangeHandler((const TigKeyStateChangeMsg&)msg);
	} else if (msg.type == TigMsgType::CHAR && mCharHandler) {
		return mCharHandler((const TigCharMsg&)msg);
	}

	if (msg.type == TigMsgType::MOUSE) {
		TigMouseMsg mouseMsg = *(const TigMouseMsg*)&msg.arg1;
		return HandleMouseMessage(mouseMsg);
	}

	return false;
}

void WidgetBase::SetSize(gfx::Size size)
{
	mWidget->width = size.width;
	mWidget->height = size.height;
}

gfx::Size WidgetBase::GetSize() const
{
	return{ (int)mWidget->width, (int)mWidget->height };
}

static TigRect GetContentArea(LgcyWidgetId id) {
	
	auto widget = uiManager->GetWidget(id);
	TigRect bounds{ widget->x, widget->y, (int) widget->width, (int) widget->height };
	
	auto advWidget = uiManager->GetAdvancedWidget(id);

	// The content of an advanced widget container may be moved
	int scrollOffsetY = 0;
	if (advWidget->GetParent()) {
		auto container = advWidget->GetParent();
		scrollOffsetY = container->GetScrollOffsetY();
	}
	
	if (widget->parentId != -1) {
		TigRect parentBounds = GetContentArea(widget->parentId);
		bounds.x += parentBounds.x;
		bounds.y += parentBounds.y - scrollOffsetY;
		
		// Clamp width/height if necessary
		int parentRight = parentBounds.x + parentBounds.width;
		int parentBottom = parentBounds.y + parentBounds.height;
		if (bounds.x >= parentRight) {
			bounds.width = 0;
		}
		if (bounds.y >= parentBottom) {
			bounds.height = 0;
		}

		if (bounds.x + bounds.width > parentRight) {
			bounds.width = std::max(0, parentRight - bounds.x);
		}
		if (bounds.y + bounds.height > parentBottom) {
			bounds.height = std::max(0, parentBottom - bounds.y);
		}
	}

	return bounds;
}

TigRect WidgetBase::GetContentArea() const
{
	return ::GetContentArea(mWidget->widgetId);
}

TigRect WidgetBase::GetVisibleArea() const
{	
	if (mParent) {
		TigRect parentArea = mParent->GetVisibleArea();
		int parentLeft = parentArea.x;
		int parentTop = parentArea.y;
		int parentRight = parentLeft + parentArea.width;
		int parentBottom = parentTop + parentArea.height;

		int clientLeft = parentArea.x + mWidget->x;
		int clientTop = parentArea.y + mWidget->y - mParent->GetScrollOffsetY();
		int clientRight = clientLeft + mWidget->width;
		int clientBottom = clientTop + mWidget->height;

		clientLeft = std::max(parentLeft, clientLeft);
		clientTop = std::max(parentTop, clientTop);

		clientRight = std::min(parentRight, clientRight);
		clientBottom = std::min(parentBottom, clientBottom);

		if (clientRight <= clientLeft) {
			clientRight = clientLeft;
		}
		if (clientBottom <= clientTop) {
			clientBottom = clientTop;
		}

		return{
			clientLeft,
			clientTop,
			clientRight - clientLeft,
			clientBottom - clientTop
		};		
	} else {
		return{ mWidget->x, mWidget->y, (int) mWidget->width, (int) mWidget->height };
	}

}

bool WidgetBase::HandleMouseMessage(const TigMouseMsg & msg)
{
	if (mMouseMsgHandler) {
		return mMouseMsgHandler(msg);
	}
	return false;
}

void WidgetBase::OnUpdateTime(uint32_t timeMs)
{
}

WidgetContainer::WidgetContainer(int width, int height)
{
	LgcyWindow window(0, 0, width, height);

	window.render = RenderWidget;
	window.handleMessage = ::HandleMessage;

	auto widgetId = uiManager->AddWindow(window);
	uiManager->SetAdvancedWidget(widgetId, this);
	mWindow = uiManager->GetWindow(widgetId);
	mWidget = mWindow;

}

void WidgetContainer::Add(std::unique_ptr<WidgetBase> childWidget)
{
	childWidget->SetParent(this);
	// If the child widget was a top-level window before, remove it
	uiManager->RemoveWindow(childWidget->GetWidgetId());
	uiManager->AddChild(mWindow->widgetId, childWidget->GetWidgetId());
	mChildren.emplace_back(std::move(childWidget));
}

void WidgetContainer::Clear()
{
	mChildren.clear();
}

WidgetBase * WidgetContainer::PickWidget(int x, int y)
{
	for (auto it = mChildren.rbegin(); it != mChildren.rend(); it++) {
		auto child = it->get();

		if (!child->IsVisible()) {
			continue;
		}

		int localX = x - child->GetPos().x;
		int localY = y - child->GetPos().y + mScrollOffsetY;
		
		auto result = child->PickWidget(localX, localY);
		if (result) {
			return result;
		}
	}
	return WidgetBase::PickWidget(x, y);
}

void WidgetContainer::Render()
{
	if (!IsVisible()) {
		return;
	}

	WidgetBase::Render();

	auto visArea = GetVisibleArea();

	for (auto &child : mChildren) {
		if (child->IsVisible()) {			
			tig->GetRenderingDevice().SetScissorRect(visArea.x, visArea.y, visArea.width, visArea.height);
			child->Render();
		}
	}

	tig->GetRenderingDevice().ResetScissorRect();

}

bool WidgetContainer::HandleMouseMessage(const TigMouseMsg & msg)
{
	auto area = GetContentArea();

	// Iterate in reverse order since this list is ordered in ascending z-order
	for (auto it = mChildren.rbegin(); it != mChildren.rend(); it++) {
		auto child = it->get();

		int x = msg.x - area.x;
		int y = msg.y - area.y + GetScrollOffsetY();
		
		if (child->IsVisible() && x >= child->GetX() && y >= child->GetY() && x < child->GetX() + child->GetWidth() && y < child->GetY() + child->GetHeight()) {
			if (child->HandleMouseMessage(msg)) {
				return true;
			}
		}
	}

	return WidgetBase::HandleMouseMessage(msg);
}

void WidgetContainer::SetScrollOffsetY(int scrollY)
{
	mScrollOffsetY = scrollY;
	uiManager->RefreshMouseOverState();
}

WidgetButtonBase::WidgetButtonBase()
{
	LgcyButton button;

	// This is our special sauce...
	button.render = RenderWidget;
	button.handleMessage = ::HandleMessage;

	auto widgetId = uiManager->AddButton(button);
	uiManager->SetAdvancedWidget(widgetId, this);
	mButton = uiManager->GetButton(widgetId);
	mWidget = mButton;
}

bool WidgetButtonBase::HandleMessage(const TigMsg & msg)
{
	
	if (msg.type == TigMsgType::WIDGET) {
		TigMsgWidget &widgetMsg = (TigMsgWidget&)msg;
		if (widgetMsg.widgetEventType == TigMsgWidgetEvent::Clicked) {
			if (mClickHandler && !mDisabled) {
				auto contentArea = GetContentArea();
				int x = widgetMsg.x - contentArea.x;
				int y = widgetMsg.y - contentArea.y;
				mClickHandler(x, y);
				mLastClickTriggered = msg.createdMs;
			}
			return true;
		}
	}
	return WidgetBase::HandleMessage(msg);
}

void WidgetButtonBase::OnUpdateTime(uint32_t timeMs)
{
	if (mRepeat && mButton->buttonState == LgcyButtonState::Down) {
		auto pos = mouseFuncs.GetPos();
		if (mClickHandler && !mDisabled && mLastClickTriggered + mRepeatInterval < timeMs) {
			auto contentArea = GetContentArea();
			int x = pos.x - contentArea.x;
			int y = pos.y - contentArea.y;
			mClickHandler(x, y);
			mLastClickTriggered = timeMs;
		}
	}
}

WidgetButton::WidgetButton()
{
}

void WidgetButton::SetStyle(const WidgetButtonStyle & style)
{
	mStyle = style;
	mButton->sndHoverOn = style.soundEnter;
	mButton->sndHoverOff = style.soundLeave;
	mButton->sndDown = style.soundDown;
	mButton->sndClick = style.soundClick;
	UpdateContent();
}

void WidgetButton::SetStyle(const eastl::string & styleName){
	widgetButtonStyles->GetStyle(styleName);
}

void WidgetButton::Render()
{
	auto contentArea = GetContentArea();

	// Always fall back to the default
	auto image = mNormalImage.get();
	
	if (mDisabled) {
		if (mDisabledImage) {
			image = mDisabledImage.get();
		} else {
			image = mNormalImage.get();
		}

		if (!mStyle.disabledTextStyleId.empty()) {
			mLabel.SetStyleId(mStyle.disabledTextStyleId);
		} else {
			mLabel.SetStyleId(mStyle.textStyleId);
		}
	} else {
		if (mButton->buttonState == LgcyButtonState::Down) {
			if (mPressedImage) {
				image = mPressedImage.get();
			} else if (mHoverImage) {
				image = mHoverImage.get();
			} else {
				image = mNormalImage.get();
			}

			if (!mStyle.pressedTextStyleId.empty()) {
				mLabel.SetStyleId(mStyle.pressedTextStyleId);
			} else if (!mStyle.hoverTextStyleId.empty()) {
				mLabel.SetStyleId(mStyle.hoverTextStyleId);
			} else {
				mLabel.SetStyleId(mStyle.textStyleId);
			}
		} 
		else if (IsActive()){
			// Activated, else Pressed, else Hovered, (else Normal)
			if (mActivatedImage){
				image = mActivatedImage.get();
			}
			else if (mPressedImage){
				image = mPressedImage.get();
			}
			else if (mHoverImage){
				image = mHoverImage.get();
			}


			if (mButton->buttonState == LgcyButtonState::Hovered
				|| mButton->buttonState == LgcyButtonState::Released){
				if (!mStyle.hoverTextStyleId.empty()) {
					mLabel.SetStyleId(mStyle.hoverTextStyleId);
				}
				else {
					mLabel.SetStyleId(mStyle.textStyleId);
				}
			}
			else {
				mLabel.SetStyleId(mStyle.textStyleId);
			}
		}
		else if (mButton->buttonState == LgcyButtonState::Hovered 
			|| mButton->buttonState == LgcyButtonState::Released) {
			if (mHoverImage) {
				image = mHoverImage.get();
			} else {
				image = mNormalImage.get();
			}

			if (!mStyle.hoverTextStyleId.empty()) {
				mLabel.SetStyleId(mStyle.hoverTextStyleId);
			}
			else {
				mLabel.SetStyleId(mStyle.textStyleId);
			}
		} else {
			image = mNormalImage.get();
			mLabel.SetStyleId(mStyle.textStyleId);
		}
	}

	if (image) {
		image->SetContentArea(contentArea);
		image->Render();
	}
	
	mLabel.SetContentArea(contentArea);
	mLabel.Render();
}

void WidgetButton::SetText(const std::string & text)
{
	mLabel.SetText(text);
	UpdateAutoSize();
}

void WidgetButton::UpdateContent()
{

	if (!mStyle.normalImagePath.empty()) {
		mNormalImage = std::make_unique<WidgetImage>(mStyle.normalImagePath);
	} else {
		mNormalImage.reset();
	}

	if (!mStyle.activatedImagePath.empty()) {
		mActivatedImage = std::make_unique<WidgetImage>(mStyle.activatedImagePath);
	}
	else {
		mActivatedImage.reset();
	}

	if (!mStyle.hoverImagePath.empty()) {
		mHoverImage = std::make_unique<WidgetImage>(mStyle.hoverImagePath);
	} else {
		mHoverImage.reset();
	}

	if (!mStyle.pressedImagePath.empty()) {
		mPressedImage = std::make_unique<WidgetImage>(mStyle.pressedImagePath);
	} else {
		mPressedImage.reset();
	}

	if (!mStyle.disabledImagePath.empty()) {
		mDisabledImage = std::make_unique<WidgetImage>(mStyle.disabledImagePath);
	} else {
		mDisabledImage.reset();
	}

	mLabel.SetStyleId(mStyle.textStyleId);
	
	UpdateAutoSize();

}

void WidgetButton::UpdateAutoSize()
{
	// Try to auto-size
	if (mAutoSizeWidth || mAutoSizeHeight) {
		gfx::Size prefSize;
		if (mNormalImage) {
			prefSize = mNormalImage->GetPreferredSize();
		} else {
			prefSize = mLabel.GetPreferredSize();
		}

		if (mAutoSizeWidth && mAutoSizeHeight) {
			SetSize(prefSize);
		} else if (mAutoSizeWidth) {
			SetWidth(prefSize.width);
		} else if (mAutoSizeHeight) {
			SetHeight(prefSize.height);
		}
	}
}

class WidgetScrollBarHandle : public WidgetButtonBase {
public:
	WidgetScrollBarHandle(WidgetScrollBar &scrollBar);

	void Render() override;

	bool HandleMouseMessage(const TigMouseMsg &msg) override;

private:
	WidgetScrollBar &mScrollBar;

	WidgetImage mTop;
	WidgetImage mTopClicked;
	WidgetImage mHandle;
	WidgetImage mHandleClicked;
	WidgetImage mBottom;
	WidgetImage mBottomClicked;

	int mDragY = 0;
	int mDragGrabPoint = 0;
};

WidgetScrollBarHandle::WidgetScrollBarHandle(WidgetScrollBar &scrollBar) :
	mScrollBar(scrollBar),
	mTop("art/scrollbar/top.tga"),
	mTopClicked("art/scrollbar/top_click.tga"),
	mHandle("art/scrollbar/fill.tga"),
	mHandleClicked("art/scrollbar/fill_click.tga"),
	mBottom("art/scrollbar/bottom.tga"),
	mBottomClicked("art/scrollbar/bottom_click.tga") 
{
	SetWidth(mHandle.GetPreferredSize().width);
}

void WidgetScrollBarHandle::Render() {
	auto contentArea = GetContentArea();

	auto topArea = contentArea;
	topArea.width = mTop.GetPreferredSize().width;
	topArea.height = mTop.GetPreferredSize().height;
	mTop.SetContentArea(topArea);
	mTop.Render();

	auto bottomArea = contentArea;	
	bottomArea.width = mBottom.GetPreferredSize().width;
	bottomArea.height = mBottom.GetPreferredSize().height;
	bottomArea.y = contentArea.y + contentArea.height - bottomArea.height; // Align to bottom
	mBottom.SetContentArea(bottomArea);
	mBottom.Render();

	int inBetween = bottomArea.y - topArea.y - topArea.height;
	if (inBetween > 0) {
		auto centerArea = contentArea;
		centerArea.y = topArea.y + topArea.height;
		centerArea.height = inBetween;
		centerArea.width = mHandle.GetPreferredSize().width;
		mHandle.SetContentArea(centerArea);
		mHandle.Render();
	}
}

bool WidgetScrollBarHandle::HandleMouseMessage(const TigMouseMsg &msg) {
	if (uiManager->GetMouseCaptureWidgetId() == GetWidgetId()) {
		if (msg.flags & MSF_POS_CHANGE) {
			int curY = mDragY + msg.y - mDragGrabPoint;

			int scrollRange = mScrollBar.GetScrollRange();
			auto vPercent = (curY - mScrollBar.mUpButton->GetHeight()) / (float) scrollRange;
			if (vPercent < 0) {
				vPercent = 0;
			} else if (vPercent > 1) {
				vPercent = 1;
			}
			auto newVal = mScrollBar.mMin + (mScrollBar.mMax - mScrollBar.mMin) * vPercent;

			mScrollBar.SetValue((int)newVal);
		}
		if (msg.flags & MSF_LMB_RELEASED) {
			uiManager->UnsetMouseCaptureWidgetId(GetWidgetId());
		}
	} else {
		if (msg.flags & MSF_LMB_DOWN) {
			uiManager->SetMouseCaptureWidgetId(GetWidgetId());
			mDragGrabPoint = msg.y;
			mDragY = GetY();
		}		
	}
	return true;
}

WidgetScrollBar::WidgetScrollBar() : WidgetContainer(0, 0)
{
	auto upButton = std::make_unique<WidgetButton>();
	upButton->SetParent(this);
	upButton->SetStyle(widgetButtonStyles->GetStyle("scrollbar-up"));
	upButton->SetClickHandler([this]() {
		SetValue(GetValue() - 1);
	});
	upButton->SetRepeat(true);

	auto downButton = std::make_unique<WidgetButton>();
	downButton->SetParent(this);
	downButton->SetStyle(widgetButtonStyles->GetStyle("scrollbar-down"));
	downButton->SetClickHandler([this]() {
		SetValue(GetValue() + 1);
	});
	downButton->SetRepeat(true);

	auto track = std::make_unique<WidgetButton>();
	track->SetParent(this);
	track->SetStyle(widgetButtonStyles->GetStyle("scrollbar-track"));
	track->SetClickHandler([this](int x, int y) {
		// The y value is in relation to the track, we need to add it's own Y value,
		// and compare against the current position of the handle
		y += mTrack->GetY();
		if (y < mHandleButton->GetY()) {
			SetValue(GetValue() - 5);
		} else if (y >= mHandleButton->GetY() + mHandleButton->GetHeight()) {
			SetValue(GetValue() + 5);
		}
	});
	track->SetRepeat(true);

	auto handle = std::make_unique<WidgetScrollBarHandle>(*this);
	handle->SetParent(this);
	handle->SetHeight(100);
	
	SetWidth(std::max(upButton->GetWidth(), downButton->GetWidth()));

	mUpButton = upButton.get();
	mDownButton = downButton.get();
	mTrack = track.get();
	mHandleButton = handle.get();

	Add(std::move(track));
	Add(std::move(upButton));
	Add(std::move(downButton));
	Add(std::move(handle));
}

void WidgetScrollBar::Render()
{
	mDownButton->SetY(GetHeight() - mDownButton->GetHeight());

	// Update the track position
	mTrack->SetWidth(GetWidth());
	mTrack->SetY(mUpButton->GetHeight());
	mTrack->SetHeight(GetHeight() - mUpButton->GetHeight() - mDownButton->GetHeight());

	int handleOffset = (int)(((mValue - mMin) / (float)mMax) * GetScrollRange());
	mHandleButton->SetY(mUpButton->GetHeight() + handleOffset);
	mHandleButton->SetHeight(GetHandleHeight());

	WidgetContainer::Render();
}

int WidgetScrollBar::GetHandleHeight() const
{	
	return 5 * GetTrackHeight() / (5 + GetMax() - GetMin()) + 20;
}

int WidgetScrollBar::GetScrollRange() const
{
	return GetTrackHeight() - GetHandleHeight();
}

int WidgetScrollBar::GetTrackHeight() const
{
	return GetHeight() - mUpButton->GetHeight() - mDownButton->GetHeight();
}

WidgetScrollView::WidgetScrollView(int width, int height) : WidgetContainer(width, height)
{
	auto scrollBar = std::make_unique<WidgetScrollBar>();
	scrollBar->SetHeight(height);
	scrollBar->SetX(width - scrollBar->GetWidth());	
	scrollBar->SetValueChangeHandler([this](int newValue) {
		mContainer->SetScrollOffsetY(newValue);
	});
	mScrollBar = scrollBar.get();
	WidgetContainer::Add(std::move(scrollBar));
	
	auto scrollView = std::make_unique<WidgetContainer>(GetInnerWidth(), height);
	mContainer = scrollView.get();
	WidgetContainer::Add(std::move(scrollView));

	UpdateLayout();
}

void WidgetScrollView::Add(std::unique_ptr<WidgetBase> childWidget)
{
	mContainer->Add(std::move(childWidget));
}

void WidgetScrollView::Clear()
{
	mContainer->Clear();
}

int WidgetScrollView::GetInnerWidth() const
{
	return GetWidth() - mScrollBar->GetWidth() - 2 * mPadding;
}

int WidgetScrollView::GetInnerHeight() const
{
	return GetHeight() - 2 * mPadding;
}

void WidgetScrollView::SetPadding(int padding)
{
	mPadding = padding;

	UpdateLayout();	
}

int WidgetScrollView::GetPadding() const
{
	return mPadding;
}

void WidgetScrollView::UpdateInnerHeight()
{
	int innerHeight = 0;
	for (auto &child : mContainer->GetChildren()) {
		auto bottom = child->GetY() + child->GetHeight();
		if (bottom > innerHeight) {
			innerHeight = bottom;
		}
	}
	mScrollBar->SetMax(innerHeight);
}

void WidgetScrollView::UpdateLayout()
{
	mContainer->SetX(mPadding);
	mContainer->SetWidth(GetInnerWidth());

	mContainer->SetY(mPadding);
	mContainer->SetHeight(GetInnerHeight());
}
