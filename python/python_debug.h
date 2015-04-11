
#pragma once

#include <vector>

// Instantiate to get your own python debug function callable via name()
class PythonDebugFunc {
public:
	typedef void (*CallbackFn)();

	explicit PythonDebugFunc(const char *name, CallbackFn callback) : mName(name), mCallback(callback) {
		registry().push_back(this);
	}

	const char *name() const {
		return mName;
	}

	CallbackFn callback() const {
		return mCallback;
	}

	static const vector<PythonDebugFunc*> &callbacks() {
		return registry();
	}

	PythonDebugFunc(PythonDebugFunc&) = delete;
	PythonDebugFunc &operator=(PythonDebugFunc&) = delete;

private:
	const char *mName;
	CallbackFn mCallback;

	static vector<PythonDebugFunc*> &registry();
};
