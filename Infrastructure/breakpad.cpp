
#include "crash_reporting.h"

#include "infrastructure/breakpad.h"
#include "infrastructure/format.h"
#include "platform/windows.h"

Breakpad::Breakpad(const std::wstring &crashDumpFolder)
{

	mHandler = std::make_unique<InProcessCrashReporting>(crashDumpFolder, [this](const std::wstring &minidump_path) {

		auto msg = fmt::format(L"Sorry! TemplePlus seems to have crashed. A crash report was written to {}.\n\n"
			L"If you want to report this issue, please contact us on our forums at RPGCodex or send an email to templeplushelp@gmail.com.",
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
