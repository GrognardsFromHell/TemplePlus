#pragma once

#include <exception>
#include <string>

class TempleException : std::exception {
public:
	TempleException(const std::string &msg) : mMsg(msg) {}

	const char* what() const override {
		return mMsg.c_str();
	}
private:
	std::string mMsg;
};
