
#pragma once

#define NO_COPY_OR_MOVE(TYPE) TYPE(const TYPE&) = delete;\
	TYPE(const TYPE&&) = delete;\
	TYPE& operator=(const TYPE&) = delete;\
	TYPE& operator=(const TYPE&&) = delete;

#define NO_COPY(TYPE) TYPE(const TYPE&&) = delete;\
	TYPE& operator=(const TYPE&&) = delete;
