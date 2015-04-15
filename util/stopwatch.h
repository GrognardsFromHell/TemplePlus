
#pragma once

class Stopwatch {
public:
	Stopwatch() {
		mStart = chrono::high_resolution_clock::now();
	}

	int GetElapsedMs() const {
		auto duration = chrono::duration_cast<chrono::milliseconds>(
			chrono::high_resolution_clock::now() - mStart
		);
		return (int) duration.count();
	}

private:
	chrono::time_point<chrono::high_resolution_clock> mStart;
};

class StopwatchReporter {
public:
	StopwatchReporter(const string &reportFormat) : mFormat(reportFormat) {
	}
	~StopwatchReporter() {
		logger->info(mFormat.c_str(), format("{} ms", sw.GetElapsedMs()));
	}
private:
	string mFormat;
	Stopwatch sw;	
};
