#include "stdafx.h"
#include "ui_chargen.h"

void PagianatedChargenSystem::SetPage(int page){
	if (page >= 0 && page <= mPageCount - 1){
		mWndPage = page;
	}
	
	auto prevBtn = (WidgetButton*)uiManager->GetAdvancedWidget(mPrevBtnId);
	auto nextBtn = (WidgetButton*)uiManager->GetAdvancedWidget(mNextBtnId);
	if (prevBtn){	
		prevBtn->SetDisabled(page <= 0);
	}
	if (nextBtn){
		nextBtn->SetDisabled(page >= mPageCount - 1);
	}
	
	mPageUpdateHandler();
}

void PagianatedChargenSystem::AddPageButtonsToWnd(unique_ptr<WidgetContainer>& wnd)
{
	auto nextBtn = make_unique<ChargenBigButton>();
	nextBtn->SetPos(310, 200);
	nextBtn->SetText("NEXT");
	//auto &pbtn = *nextBtn;
	nextBtn->SetClickHandler([&]() {
		SetPage(GetPage() + 1);
	});
	mNextBtnId = nextBtn->GetWidgetId();
	nextBtn->SetActivationState(ChargenBigButton::ChargenButtonActivationState::Active);

	auto prevBtn = make_unique<ChargenBigButton>();
	prevBtn->SetPos(10, 200);
	prevBtn->SetText("PREV");
	
	prevBtn->SetClickHandler([&]() {
		SetPage(GetPage() - 1);
	});
	mPrevBtnId = prevBtn->GetWidgetId();
	prevBtn->SetActivationState(ChargenBigButton::ChargenButtonActivationState::Active);

	wnd->Add(std::move(prevBtn));
	wnd->Add(std::move(nextBtn));
}

void PagianatedChargenSystem::SetPageUpdateHandler(std::function<void()> updateHandler){
	mPageUpdateHandler = updateHandler;
}
