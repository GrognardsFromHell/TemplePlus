
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>

#include "MinHook.h"

#include <string>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace boost::filesystem;
using namespace boost;

#define LOG(T) BOOST_LOG_TRIVIAL(T)