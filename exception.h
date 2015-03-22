
#pragma once

class TempleException : std::exception {
public:
	TempleException(const string &msg) : mMsg(msg) {}
	TempleException(const format &format) : mMsg(format.str()) {}

	const char* what() const override {
		return mMsg.c_str();
	}
private:
	string mMsg;
};
