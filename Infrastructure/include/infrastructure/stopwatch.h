
#pragma once

#include <string>
#include <chrono>
#include "logging.h"

class Stopwatch {
public:
	using Clock = std::chrono::high_resolution_clock;

	Stopwatch() {
		mStart = Clock::now();
	}

	int GetElapsedMs() const {
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
			Clock::now() - mStart
		);
		return (int) duration.count();
	}

private:
	std::chrono::time_point<Clock> mStart;
};

class StopwatchReporter {
public:
	StopwatchReporter(const std::string &reportFormat) : mFormat(reportFormat) {
	}
	~StopwatchReporter() {
		logger->info(mFormat.c_str(), fmt::format("{} ms", sw.GetElapsedMs()));
	}
private:
	std::string mFormat;
	Stopwatch sw;	
};
