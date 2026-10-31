#pragma once

#include <optional>
#include <string_view>

namespace GCD::Loader
{
std::optional<std::string> loadKernelModule(const std::string_view moduleName);
} // namespace GCD::Loader
