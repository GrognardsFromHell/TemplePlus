
#include "stdafx.h"
#include "widgets.h"
#include "widget_content.h"
#include "tig/tig_msg.h"
#include "tig/tig_startup.h"

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
		if (content->GetContentArea() != contentArea) {
			content->SetContentArea(contentArea);
		}
		
		content->Render();
	}
}

bool WidgetBase::HandleMessage(const TigMsg &msg)
{
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

	if (widget->parentId != -1) {
		TigRect parentBounds = GetContentArea(widget->parentId);
		bounds.x += parentBounds.x;
		bounds.y += parentBounds.y;
		
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
			bounds.width = parentRight - bounds.x;
		}
		if (bounds.y + bounds.height > parentBottom) {
			bounds.height = parentBottom - bounds.y;
		}
	}

	return bounds;
}

TigRect WidgetBase::GetContentArea() const
{
	return ::GetContentArea(mWidget->widgetId);
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

WidgetBase * WidgetContainer::PickWidget(int x, int y)
{
	for (auto it = mChildren.rbegin(); it != mChildren.rend(); it++) {
		auto child = it->get();

		if (!child->IsVisible()) {
			continue;
		}

		int localX = x - child->GetPos().x;
		int localY = y - child->GetPos().y;
		
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

	for (auto &child : mChildren) {
		if (child->IsVisible()) {
			child->Render();
		}
	}
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
				mClickHandler();
			}
			return true;
		}
	}
	return false;
}

WidgetButton::WidgetButton()
{
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
		} else if (mButton->buttonState == LgcyButtonState::Hovered 
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
	mLabel.SetText(uiAssets->ApplyTranslation(text));
	UpdateAutoSize();
}

void WidgetButton::UpdateContent()
{

	if (!mStyle.normalImagePath.empty()) {
		mNormalImage = std::make_unique<WidgetImage>(mStyle.normalImagePath);
	} else {
		mNormalImage.reset();
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
	if (!mButton->width || !mButton->height) {
		gfx::Size prefSize;
		if (mNormalImage) {
			prefSize = mNormalImage->GetPreferredSize();
		} else {
			prefSize = mLabel.GetPreferredSize();
		}

		// If the label is set to "center", it might return a pref. width of 0
		// but a correct preferred height
		if (!prefSize.width) {
			prefSize.width = GetWidth();
		}
		if (!prefSize.height) {
			prefSize.height = GetHeight();
		}
		SetSize(prefSize);
	}
}
