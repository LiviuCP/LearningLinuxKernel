#pragma once

#include <filesystem>
#include <optional>
#include <string_view>

namespace Utilities::Test
{
std::optional<std::filesystem::path> getModulePath(const std::string_view moduleName);
}
