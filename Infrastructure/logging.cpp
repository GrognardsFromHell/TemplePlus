#include "logging.h"
#include "format.h"
#include "spdlog/spdlog.h"
#include "include/spdlog/sinks/null_sink.h"

std::shared_ptr<spdlog::logger> logger;

struct OutputDebugStringSink : spdlog::sinks::base_sink<std::mutex>
{
protected:
	void _sink_it(const spdlog::details::log_msg& msg) override
	{
		OutputDebugStringA(msg.formatted.c_str());
	}
};

/*
	Initializes the shared pointer above to at least a null sink so logging calls
	before the logging system has been initialized do not crash.
*/
static class LoggingPreInitializer
{
public:
	LoggingPreInitializer()
	{
		auto nullSink = std::make_shared<spdlog::sinks::null_sink_mt>();
		logger = spdlog::create("core", { nullSink });
	}
} loggingPreInitializer;


void InitLogging()
{
	spdlog::set_level(spdlog::level::debug);

	try
	{
		// Always log to a file
		DeleteFile(L"TemplePlus.log");
		auto fileSink = std::make_shared<spdlog::sinks::simple_file_sink_mt>("TemplePlus.log", true);
		auto debugSink = std::make_shared<OutputDebugStringSink>();
		logger = spdlog::create("core", {fileSink, debugSink});
	}
	catch (const spdlog::spdlog_ex& e)
	{
		auto msg = fmt::format("Unable to initialize the logging subsystem:\n{}", e.what());
		MessageBoxA(nullptr, msg.c_str(), "Logging Error", MB_OK | MB_ICONERROR);
	}
}
