#include <Objbase.h>

#include "platform/windows.h"
#include "infrastructure/exception.h"

std::string GetLastWin32Error() {
	auto error = GetLastError();

	// Curtesy of Stackoverflow:
	// http://stackoverflow.com/questions/455434/how-should-i-use-formatmessage-properly-in-c
	LPSTR errorText = nullptr;

	FormatMessageA(
		// use system message tables to retrieve error text
		FORMAT_MESSAGE_FROM_SYSTEM
		                          // allocate buffer on local heap for error text
		                          | FORMAT_MESSAGE_ALLOCATE_BUFFER
		                          // Important! will fail otherwise, since we're not 
		                          // (and CANNOT) pass insertion parameters
		                          | FORMAT_MESSAGE_IGNORE_INSERTS,
		                          nullptr, // unused with FORMAT_MESSAGE_FROM_SYSTEM
		                          error,
		                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		                          (LPSTR)&errorText, // output 
		                          0, // minimum size for output buffer
		                          nullptr); // arguments - see note 

	if (errorText) {
		std::string result(errorText);

		// release memory allocated by FormatMessage()
		LocalFree(errorText);
		return result;
	}

	return{};

}

ComInitializer::ComInitializer() {
	auto status = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (status != S_OK && status != S_FALSE) {
		throw TempleException("Unable to initialize COM systems");
	}

	mAlreadyInitialized = (status == S_FALSE);
}

ComInitializer::~ComInitializer() {
	if (!mAlreadyInitialized) {
		CoUninitialize();
	}
}
