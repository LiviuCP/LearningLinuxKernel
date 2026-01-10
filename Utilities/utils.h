#pragma once

#include <filesystem>
#include <string>

#define READ_MODE false
#define WRITE_MODE true

namespace Utilities
{
std::string executeCommand(const std::string& command, bool isWriteMode);
std::filesystem::path getApplicationPath();
} // namespace Utilities
