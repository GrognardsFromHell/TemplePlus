#include "stdafx.h"
#include "ui_alert.h"
#include <temple\dll.h>

UiAlert uiAlert;

int UiAlert::Show(int helpId, int(*callback)(int), const char * btnText)
{
	return temple::GetRef<int(__cdecl)(int, int(*)(int), const char*)>(0x100E6F10)(helpId, callback, btnText);
	return 0;
}
