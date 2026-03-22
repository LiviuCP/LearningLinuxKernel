#pragma once

#include <filesystem>
#include <optional>
#include <string_view>

namespace GCD::Loader
{
void loadKernelModule(const std::optional<std::filesystem::path>& modulePath);
std::optional<std::filesystem::path> getModulePath(const std::string_view moduleName);
} // namespace GCD::Loader
