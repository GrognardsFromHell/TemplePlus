#include "infrastructure/logging.h"
#include <fmt/format.h>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/msvc_sink.h"
#include "include/spdlog/sinks/null_sink.h"
#include "infrastructure/stringutil.h"

std::shared_ptr<spdlog::logger> logger;

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


void InitLogging(const std::wstring &logFile, spdlog::level::level_enum logLevel)
{
	if ((int)logLevel < 0) {
		logLevel = spdlog::level::trace;
	}
	else if ((int)logLevel > spdlog::level::off ) {
		logLevel = spdlog::level::off;
	}

	spdlog::set_level(logLevel);

	try {
		// Always log to a file
		DeleteFile(logFile.c_str());
		auto fileSink = std::make_shared<spdlog::sinks::simple_file_sink_mt>(ucs2_to_local(logFile), true);
		// This is slower but prevents logs from being truncated on crash
		fileSink->set_force_flush(true);
		auto debugSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
		spdlog::drop_all(); // Reset all previous loggers
		logger = spdlog::create("core", {fileSink, debugSink});
	}
	catch (const spdlog::spdlog_ex& e)
	{
		auto msg = fmt::format("Unable to initialize the logging subsystem:\n{}", e.what());
		MessageBoxA(nullptr, msg.c_str(), "Logging Error", MB_OK | MB_ICONERROR);
	}
}
