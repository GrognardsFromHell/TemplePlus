
#include "infrastructure/breakpad.h"
#include "breakpad\exception_handler.h"
#include "infrastructure/format.h"

// Delegate back to breakpad
static bool HandleCrashCallbackDelegate(const wchar_t* dump_path,
	const wchar_t* minidump_id,
	void* context,
	EXCEPTION_POINTERS* exinfo,
	MDRawAssertionInfo* assertion,
	bool succeeded) {
	auto breakpad = (Breakpad*)context;
	
	auto msg = fmt::format(L"Sorry! TemplePlus seems to have crashed. A crash report was written to {}\\{}.dmp.\n\n"
		L"If you want to report this issue, please make sure to attach this file.\nPlease also attach your savegame file from your ToEE folder under modules\\ToEE\\Save. Be sure to include the .tfaf, .tfai and .gsi files.",
		dump_path,
		minidump_id);
	
	// Now starts the tedious work of reporting on this crash, heh.
	MessageBox(NULL, msg.c_str(), L"TemplePlus Crashed - Oops!", MB_OK);

	return false;
}

Breakpad::Breakpad(const std::wstring &crashDumpFolder)
{
	using google_breakpad::ExceptionHandler;

	CreateDirectory(crashDumpFolder.c_str(), nullptr);

	mHandler.reset(new ExceptionHandler(
		crashDumpFolder.c_str(),
		nullptr,
		HandleCrashCallbackDelegate,
		this,
		ExceptionHandler::HANDLER_ALL
	));

}

Breakpad::~Breakpad()
{
}
