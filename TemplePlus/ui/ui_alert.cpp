#include "stdafx.h"
#include "ui_alert.h"
#include <temple\dll.h>

UiAlert uiAlert;

int UiAlert::Show(int helpId, int(*callback)(int), const char * btnText)
{
	return temple::GetRef<int(__cdecl)(int, int(*)(int), const char*)>(0x100E6F10)(helpId, callback, btnText);
	return 0;
}

int UiAlert::ShowEx(int showType, int helpId, int(*callback)(int), const char* btnText, char* text)
{
	UiAlertSpec promptMini = { showType, helpId, text };
	// UiAlertShow_Impl
	return temple::GetRef<int(__cdecl )(UiAlertSpec*, int(*)(int), const char*)>(0x1009ABA0)(&promptMini, callback, btnText);
}
