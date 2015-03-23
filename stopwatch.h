
#pragma once

class StopwatchReporter {
public:
	StopwatchReporter(const string &reportFormat) : mFormat(reportFormat) {
		mStart = chrono::high_resolution_clock::now();
	}
	~StopwatchReporter() {
		auto duration = chrono::duration_cast<chrono::milliseconds>(
			chrono::high_resolution_clock::now() - mStart
			);
		LOG(info) << format(mFormat) % (format("%d ms") % duration.count());
	}
private:
	string mFormat;
	chrono::time_point<chrono::high_resolution_clock> mStart;
};
