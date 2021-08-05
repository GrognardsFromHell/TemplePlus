
#include "crash_reporting.h"
#include "client/windows/handler/exception_handler.h"

struct InProcessCrashReporting::Detail {

    // Delegate back to breakpad
    static bool HandleCrashCallbackDelegate(const wchar_t* dump_path,
        const wchar_t* minidump_id,
        void* context,
        EXCEPTION_POINTERS* exinfo,
        MDRawAssertionInfo* assertion,
        bool succeeded);

};

// Delegate back to breakpad
bool InProcessCrashReporting::Detail::HandleCrashCallbackDelegate(const wchar_t* dump_path,
    const wchar_t* minidump_id,
    void* context,
    EXCEPTION_POINTERS* exinfo,
    MDRawAssertionInfo* assertion,
    bool succeeded) {
    
    auto crash_reporting = static_cast<InProcessCrashReporting*>(context);

    // Build the path to the dump file
    std::wstring minidump_file(dump_path);
    minidump_file.append(L"\\");
    minidump_file.append(minidump_id);
    minidump_file.append(L".dmp");
    
    crash_reporting->ReportCrash(minidump_file);

    return false;
}

InProcessCrashReporting::InProcessCrashReporting(const std::wstring &minidump_folder,
                                                 std::function<void(const std::wstring&)> crash_callback)
                                                 : crash_callback_(crash_callback)
{
	using google_breakpad::ExceptionHandler;

	CreateDirectory(minidump_folder.c_str(), nullptr);

	handler_ = std::make_unique<ExceptionHandler>(
		minidump_folder.c_str(),
		nullptr,
		InProcessCrashReporting::Detail::HandleCrashCallbackDelegate,
		this,
		ExceptionHandler::HANDLER_ALL
	);

}

InProcessCrashReporting::~InProcessCrashReporting() = default;

void InProcessCrashReporting::DerefZeroCrash() {
  int* x = 0;
  *x = 1;
}

void InProcessCrashReporting::InvalidParamCrash() {
  printf(NULL);
}

class Derived;

class Base {
 public:
  Base(Derived* derived);
  virtual ~Base();
  virtual void DoSomething() = 0;

 private:
  Derived* derived_;
};

class Derived : public Base {
 public:
  Derived();
  virtual void DoSomething();
};

Base::Base(Derived* derived)
    : derived_(derived) {
}

Base::~Base() {
  derived_->DoSomething();
}

#pragma warning(push)
#pragma warning(disable:4355)
// Disable warning C4355: 'this' : used in base member initializer list.
Derived::Derived()
    : Base(this) {  // C4355
}
#pragma warning(pop)

void Derived::DoSomething() {
}

void InProcessCrashReporting::PureCallCrash() {
  Derived derived;
}

void InProcessCrashReporting::ReportCrash(const std::wstring &minidump_file) {
    if (crash_callback_) {
        crash_callback_(minidump_file);
    }
}
