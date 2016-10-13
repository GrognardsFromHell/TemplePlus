#include "stdafx.h"
#include "ui_sticky.h"
#include <temple/dll.h>

UiSticky uiSticky;

int UiSticky::Show(const char * title, const char * bodyText, const char * confirmBtnTxt, const char * cancelBtnText, const char * checboxTxt, int * state, int(*callback)(int))
{
	return temple::GetRef<int(__cdecl)(const char * , const char * , const char * , const char * , const char * , int * , int(*)(int))>(0x1019A920)(title, bodyText, confirmBtnTxt, cancelBtnText, checboxTxt, state, callback);
}
