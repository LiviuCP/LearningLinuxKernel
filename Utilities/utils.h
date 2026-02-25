#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#define READ_MODE false
#define WRITE_MODE true

namespace Utilities
{
void loadKernelModule(const std::filesystem::path& kernelModulePath);
void unloadKernelModule(const std::string_view kernelModuleName);
bool isKernelModuleLoaded(const std::string_view kernelModuleName);

bool createCharacterDeviceFile(const std::filesystem::path& deviceFilePath, int majorNumber, int minorNumber);
int getMajorDriverNumber(const std::string_view kernelModuleName);

std::optional<int> readIntValueFromFile(
    const std::filesystem::path& filePath); // reads a 32 bit integer from the beginning of a file
std::optional<std::string> readStringFromFile(const std::filesystem::path& filePath, size_t charsCount);
bool writeStringToFile(const std::string& str, const std::filesystem::path& filePath, size_t charsCount);
void clearFileContent(const std::filesystem::path& filePath);

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
