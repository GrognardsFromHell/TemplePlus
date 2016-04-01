#pragma once

namespace google_breakpad {
	class ExceptionHandler;
}

#include <memory>
#include <string>

/*
	Abstracts the interface to Google breakpad
*/
class Breakpad {
public:
	Breakpad(const std::wstring &crashDumpFolder);
	~Breakpad();
private:
	std::unique_ptr<google_breakpad::ExceptionHandler> mHandler;
};
