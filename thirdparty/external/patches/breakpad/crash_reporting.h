
#pragma once

namespace google_breakpad {
	class ExceptionHandler;
}

#include <memory>
#include <string>
#include <functional>

#ifdef BREAKPAD_EXPORTS
#define BREAKPAD_API __declspec(dllexport)
#else
#define BREAKPAD_API __declspec(dllimport)
#endif

/*
	Abstracts the interface to Google breakpad
*/
class InProcessCrashReporting {
public:
    BREAKPAD_API InProcessCrashReporting(const std::wstring &minidump_folder,
                            std::function<void(const std::wstring&)> crash_callback);
    BREAKPAD_API ~InProcessCrashReporting();

    // Helper functions to cause crashes for testing
    BREAKPAD_API void DerefZeroCrash();
    BREAKPAD_API void InvalidParamCrash();
    BREAKPAD_API void PureCallCrash();

private:
    struct Detail;
    friend struct Detail;

    void ReportCrash(const std::wstring &minidump_file);

	std::unique_ptr<google_breakpad::ExceptionHandler> handler_;
    std::function<void(const std::wstring&)> crash_callback_;
};
