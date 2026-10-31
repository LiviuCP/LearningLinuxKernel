#pragma once

#include <optional>
#include <string>

namespace GCD::Loader
{
std::optional<std::string> loadRequiredKernelModules();
void unloadRequiredKernelModules();
} // namespace GCD::Loader
