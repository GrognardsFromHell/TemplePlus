#include "stdafx.h"
#include <temple/dll.h>
#include <util/fixes.h>
#include <tig/tig_msg.h>
#include <infrastructure/keyboard.h>

static struct TigConsoleAddresses : temple::AddressTable {
	BOOL * consoleDisabled; 
	BOOL* consoleActive;
	char * consoleString; //  [256]
	int * consolePos;
	void(__cdecl*ConsoleKeyUpHandler)();
	void(__cdecl*ConsoleKeyDownHandler)();
	void(__cdecl*ConsoleKeyDeleteHandler)();


	TigConsoleAddresses() {
		rebase(consoleDisabled, 0x10D26DB4);
		rebase(consoleActive, 0x10D26DBC);
		rebase(consoleString, 0x10D26DC0);
		rebase(consolePos, 0x10D26EC4);
		rebase(ConsoleKeyUpHandler, 0x101DF880);
		rebase(ConsoleKeyDownHandler, 0x101DF900);
		rebase(ConsoleKeyDeleteHandler, 0x101DF980);
		
	}
} addresses;

class TigConsoleReplacement : TempleFix
{
public: 
	const char* name() override 
	{
		return "TigConsole" "Function Replacements";
	} 

	static int(__cdecl *orgConsoleMsgHandler)(TigMsg* msg);
	static int ConsoleMsgHandler(TigMsg * msg);
	void apply() override 
	{
		orgConsoleMsgHandler = replaceFunction(0x101E0070, ConsoleMsgHandler);
	}
} tigConsoleReplacement;

int TigConsoleReplacement::ConsoleMsgHandler(TigMsg* msg)
{
	if (*addresses.consoleActive == false)
		return 0;
	if (msg->type == TigMsgType::MOUSE)
		return 1;

	// Handle pasting into the console (dont know why it's 22, really)
	if (msg->type == TigMsgType::CHAR && msg->arg1 == 22) {
		
		// Retrieve the clipboard content as a string
		std::string clipboardText;
		if (OpenClipboard(nullptr)) {
			auto clipboardData = GetClipboardData(CF_TEXT);
			if (clipboardData) {
				clipboardText = (char*)clipboardData;
			}
			CloseClipboard();
		}

		if (!clipboardText.empty() && *addresses.consolePos < 255) {
			auto curPos = *addresses.consolePos;
			auto remaining = 255 - curPos;
			auto count = std::min<size_t>(remaining, clipboardText.size());

			strncpy(addresses.consoleString + curPos, clipboardText.c_str(), count);
			*addresses.consolePos += count;
			return 1;
		}

	}

	if (msg->type != TigMsgType::KEYSTATECHANGE)
		return orgConsoleMsgHandler(msg);
	if (msg->arg2 != 1)
		return orgConsoleMsgHandler(msg);
	
	infrastructure::gKeyboard.Update();
	if (infrastructure::gKeyboard.IsKeyPressed(VK_UP))
	{
		addresses.ConsoleKeyUpHandler();
		return 1;
	}
	if (infrastructure::gKeyboard.IsKeyPressed(VK_LEFT))
	{
		if ( (*addresses.consolePos) > 0)
			(*addresses.consolePos)--;
		return 1;
	}

	if (infrastructure::gKeyboard.IsKeyPressed(VK_RIGHT))
	{
		int strLength = strlen(addresses.consoleString);
		if ((*addresses.consolePos) < strLength)
			(*addresses.consolePos)++;
		return 1;
	}

	if (infrastructure::gKeyboard.IsKeyPressed(VK_HOME))
	{
		(*addresses.consolePos) = 0;
		return 1;
	}

	if (infrastructure::gKeyboard.IsKeyPressed(VK_END))
	{
		int strLength = strlen(addresses.consoleString);
		if (strLength > 0)
			(*addresses.consolePos) = max(0,min(strLength , 256));
		return 1;
	}

	if (infrastructure::gKeyboard.IsKeyPressed(VK_DELETE))
	{
		addresses.ConsoleKeyDeleteHandler();
		return 1;
	}

	if (infrastructure::gKeyboard.IsKeyPressed(VK_DOWN))
	{
		addresses.ConsoleKeyDownHandler();
		return 1;
	}


	return 1;

}

int(__cdecl* TigConsoleReplacement::orgConsoleMsgHandler)(TigMsg* msg);