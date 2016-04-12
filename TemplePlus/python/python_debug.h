
#pragma once

void PyDebug_Init();

using DebugFunctionWithArgs = std::function<void(const std::vector<std::string>&)>;
using DebugFunction = std::function<void()>;

/**
 * Registers a debug function that can be called via debug.<name>(...).
 * Any arguments that are passed are converted to string and passed in the vector.
 */
void RegisterDebugFunctionWithArgs(const char *name, std::function<void(const std::vector<std::string>&)> function);

/**
 * Registers a debug function that can be called via debug.<name>(...).
 * This variant of the function does not accept any parameters.
 */
void RegisterDebugFunction(const char *name, std::function<void()> function);
