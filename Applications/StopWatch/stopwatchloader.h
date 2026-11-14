#pragma once

#include <optional>
#include <string>

namespace StopWatch::Loader
{
std::optional<std::string> loadRequiredKernelModules();
void unloadRequiredKernelModules();
} // namespace StopWatch::Loader
