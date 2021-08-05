#pragma once

#include <exception>
#include <string_view>

#include <fmt/format.h>

class TempleException : public std::exception {
public:
	explicit TempleException(const std::string_view msg) : mMsg(msg) {}
	
    template<typename... Args>
    explicit TempleException(fmt::format_string<Args...> fmt, Args &&...args) : mMsg(fmt::format(fmt, std::forward<Args>(args)...))
    {
    }

	const char* what() const override {
		return mMsg.c_str();
	}
private:
	std::string mMsg;
};
