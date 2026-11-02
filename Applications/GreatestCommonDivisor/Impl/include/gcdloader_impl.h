#pragma once

#include <functional>
#include <optional>
#include <string>

namespace GCD::Loader
{
std::optional<std::string> loadRequiredKernelModulesImpl(
    const std::function<std::optional<std::string>(const std::string_view)> loadKernelModuleCallback);
void unloadRequiredKernelModulesImpl();
} // namespace GCD::Loader
