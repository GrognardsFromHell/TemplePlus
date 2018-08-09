#pragma once

#include <exception>
#include <string>

#include <fmt/format.h>

class TempleException : public std::exception {
public:
	explicit TempleException(const std::string &msg) : mMsg(msg) {}
	
	template<typename... T>
	explicit TempleException(const char *format, const T &... args) 
		: mMsg(fmt::format(format, args...)) {}

	const char* what() const override {
		return mMsg.c_str();
	}
private:
	std::string mMsg;
};
