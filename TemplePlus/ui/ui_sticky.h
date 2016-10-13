#pragma once


class UiSticky
{
	int Show(const char *title, const char *bodyText, const char *confirmBtnTxt, const char *cancelBtnText, const char *checboxTxt, int *state, int(__cdecl*callback)(int));
};

extern UiSticky uiSticky;