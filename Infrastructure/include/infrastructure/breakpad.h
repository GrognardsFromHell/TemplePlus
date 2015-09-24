#pragma once

namespace google_breakpad {
	class ExceptionHandler;
}

#include <memory>

/*
	Abstracts the interface to Google breakpad
*/
class Breakpad {
public:
	Breakpad();
	~Breakpad();
private:
	std::unique_ptr<google_breakpad::ExceptionHandler> mHandler;
};
