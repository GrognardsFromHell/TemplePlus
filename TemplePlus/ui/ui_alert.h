#pragma once

struct UiAlertSpec
{
	int type;
	int helpId; // hash of help.tab string (TAG_XXX )
	char *text;
};

class UiAlert
{
	/*
		shows a simple alert box, with content taken from the help.tab file
	*/
	int Show(int helpId, int(__cdecl*callback)(int), const char *btnText);
};


extern UiAlert uiAlert;