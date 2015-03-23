
#pragma once

class TempleException : std::exception {
public:
	TempleException(const string &msg) : mMsg(msg) {}

	const char* what() const override {
		return mMsg.c_str();
	}
private:
	string mMsg;
};
