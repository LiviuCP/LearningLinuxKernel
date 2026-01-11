#pragma once

#include <filesystem>
#include <string_view>

#define READ_MODE false
#define WRITE_MODE true

namespace Utilities
{
void loadKernelModule(const std::filesystem::path& kernelModulePath);
void unloadKernelModule(const std::string_view kernelModuleName);
bool isKernelModuleLoaded(const std::string_view kernelModuleName);

void clearScreen();
std::filesystem::path getApplicationPath();

constexpr std::string_view getModulesDirRelativePath()
{
    constexpr std::string_view modulesDirRelativePath{"KernelModules/ConsolidatedOutput"};
    return modulesDirRelativePath;
}

constexpr std::string_view getModuleFileExtension()
{
    constexpr std::string_view moduleFileExtension{".ko"};
    return moduleFileExtension;
}

bool isValidInteger(const char* str);
} // namespace Utilities
