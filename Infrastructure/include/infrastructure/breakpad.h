#pragma once

namespace google_breakpad {
	class ExceptionHandler;
}

#include <memory>
#include <string>

class InProcessCrashReporting;

/*
	Abstracts the interface to Google breakpad
*/
class Breakpad {
public:
	Breakpad(const std::wstring &crashDumpFolder, bool fullDump = false);
	~Breakpad();

	void setExtraMessage(const std::wstring &extraMessage) {
		mExtraMessage = extraMessage;
	}
	const std::wstring &extraMessage() const {
		return mExtraMessage;
	}

private:
	std::unique_ptr<InProcessCrashReporting> mHandler;

	std::wstring mExtraMessage;
};
