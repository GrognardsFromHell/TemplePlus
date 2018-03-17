#include "stdafx.h"
#include "ui_chargen.h"

#pragma region ChargenBigButton
void ChargenBigButton::SetActivationState(ChargenButtonActivationState st) {
	mActivationState = st;
	UpdateStyleByActivationState();
}

void ChargenBigButton::UpdateStyleByActivationState()
{
	switch (mActivationState) {

	case ChargenButtonActivationState::Active:
		SetStyle("chargen-active-button");
		break;
	case ChargenButtonActivationState::Disabled:
		SetStyle("chargen-disabled-button");
		break;
	case ChargenButtonActivationState::Done:
		SetStyle("chargen-done-button");
		break;
	default:
		SetStyle("chargen-button");
	}
}

void ChargenBigButton::Render() {
	WidgetButton::Render(); // do normal rendering
}

ChargenBigButton::ChargenBigButton() : WidgetButton() {
	SetCenterVertically(true);
}

#pragma endregion