#pragma once

struct UiAlertSpec
{
	int type; // 0 - get help topic from helpId; 1 - get help topic from helpId put into history; 2 - simply show .->text;
	int helpId; // hash of help.tab string (TAG_XXX )
	char *text;
};

class UiAlert
{
public:
	/*
		shows a simple alert box, with content taken from the help.tab file
	*/
	int Show(int helpId, int(__cdecl*callback)(int), const char *btnText);
	static int ShowEx(int showType, int helpId, int(*callback)(int), const char* btnText, char* text);
};


extern UiAlert uiAlert;