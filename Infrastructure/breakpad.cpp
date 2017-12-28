
#include "crash_reporting.h"

#include "infrastructure/breakpad.h"
#include "infrastructure/format.h"
#include "platform/windows.h"

Breakpad::Breakpad(const std::wstring &crashDumpFolder)
{

	mHandler = std::make_unique<InProcessCrashReporting>(crashDumpFolder, [this](const std::wstring &minidump_path) {

		auto msg = fmt::format(L"Sorry! TemplePlus seems to have crashed. A crash report was written to {}.\n\n"
			L"If you want to report this issue, please make sure to attach this file.\nPlease also attach your savegame file from your ToEE folder under modules\\ToEE\\Save. Be sure to include the .tfaf, .tfai and .gsi files.",
			minidump_path);
		if (!extraMessage().empty()) {
			msg.append(extraMessage());
		}

		// Now starts the tedious work of reporting on this crash, heh.
		MessageBox(NULL, msg.c_str(), L"TemplePlus Crashed - Oops!", MB_OK);

	});
	
}

Breakpad::~Breakpad()
{
}
