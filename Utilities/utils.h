#pragma once

#include <filesystem>
#include <string>

#define READ_MODE false
#define WRITE_MODE true

namespace Utilities
{
std::string executeCommand(const std::string& command, bool isWriteMode);
std::filesystem::path getApplicationPath();

void clearScreen();

void loadKernelModule(const std::filesystem::path& kernelModulePath);
void unloadKernelModule(const std::string& kernelModuleName);
bool isKernelModuleLoaded(const std::string& kernelModuleName);

bool isValidInteger(const char* str);
} // namespace Utilities
