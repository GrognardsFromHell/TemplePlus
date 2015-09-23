
#pragma once

// For some reason spdlog depends on format.h but doesn't include it itself
#include "format.h"
#include "spdlog/logger.h"

// Global TemplePlus logger
extern std::shared_ptr<spdlog::logger> logger;

void InitLogging();
