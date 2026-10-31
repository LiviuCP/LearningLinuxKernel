#include <algorithm>
#include <cassert>
#include <cctype>
#include <climits>
#include <cstring>
#include <expected>
#include <fcntl.h>
#include <sstream>
#include <unistd.h>

#include "utils.h"

namespace Utilities
{
namespace
{
std::expected<std::string, std::string> executeCommand(const std::string& command, bool isWriteMode)
{
    std::expected<std::string, std::string> result;

    do
    {
        const char* mode{isWriteMode ? "w" : "r"};
        FILE* pipe{popen(command.c_str(), mode)};

        if (!pipe)
        {
            result = std::unexpected{"Failed to open pipe!"};
            break;
        }

        try
        {
            std::string commandOutput;
            char buffer[128];

            while (fgets(buffer, sizeof buffer, pipe) != NULL)
            {
                commandOutput += buffer;
            }

            result = commandOutput;
        }
        catch (...)
        {
            result = std::unexpected{"Error in reading from pipe!"};
        }

        pclose(pipe);
    } while (false);

    return result;
}

std::string trimAndConvertToStdString(const char* str)
{
    std::string result;
    const size_t strLength{str ? strlen(str) : 0};

    if (strLength > 0)
    {
        // trim left/right
        size_t leftIndex = 0;

        while (leftIndex < strLength && std::isspace(str[leftIndex]))
        {
            ++leftIndex;
        }

        size_t rightIndex = strLength - 1;

        while (rightIndex > leftIndex && std::isspace(str[rightIndex]))
        {
            --rightIndex;
        }

        result.reserve(rightIndex - leftIndex + 1);
        std::copy(str + leftIndex, str + rightIndex + 1, std::back_inserter(result));
    }

    return result;
}

} // namespace
} // namespace Utilities

std::optional<std::string> Utilities::loadKernelModule(const std::filesystem::path& kernelModulePath)
{
    std::optional<std::string> errorMessage;

    do
    {
        if (!std::filesystem::exists(kernelModulePath) || !std::filesystem::is_regular_file(kernelModulePath))
        {
            errorMessage = "Module file " + kernelModulePath.string() +
                           " does not exist or is invalid! Cannot load the kernel module.";
            break;
        }

        const std::string fileNameWithoutExtension{kernelModulePath.stem().string()};
        const std::expected<bool, std::string> isModuleInitiallyLoaded{isKernelModuleLoaded(fileNameWithoutExtension)};

        if (!isModuleInitiallyLoaded.has_value())
        {
            errorMessage = isModuleInitiallyLoaded.error();
            break;
        }

        // module is already loaded, no action needed
        if (*isModuleInitiallyLoaded)
        {
            break;
        }

        // no need to include sudo in the command string -> the user needs to run the app with sudo anyway and if so the
        // command will be executed in sudo mode
        const std::string loadCommand{"insmod " + kernelModulePath.string()};
        const std::expected<std::string, std::string> commandResult{executeCommand(loadCommand, READ_MODE)};

        if (!commandResult.has_value())
        {
            errorMessage = commandResult.error();
            break;
        }

        const std::expected<bool, std::string> hasModuleSuccessfullyLoaded{
            isKernelModuleLoaded(fileNameWithoutExtension)};

        if (!hasModuleSuccessfullyLoaded.has_value())
        {
            errorMessage = hasModuleSuccessfullyLoaded.error();
            break;
        }

        if (!(*hasModuleSuccessfullyLoaded))
        {
            errorMessage = "Unable to load module: " + kernelModulePath.string() +
                           ".\nCheck its dependencies and ensure sudo is being used.";
            break;
        }

    } while (false);

    return errorMessage;
}

std::optional<std::string> Utilities::unloadKernelModule(const std::string_view kernelModuleName)
{
    std::optional<std::string> errorMessage;

    do
    {
        // kernel module name should not include the .ko extension from file (hence should contain only alphanumeric and
        // '_' characters)
        if (kernelModuleName.empty() ||
            std::ranges::any_of(kernelModuleName, [](char ch) { return !isalnum(ch) && ch != '_'; }))
        {
            errorMessage = "Invalid module name: " + std::string{kernelModuleName};
            break;
        }

        const std::expected<bool, std::string> isModuleInitiallyLoaded{isKernelModuleLoaded(kernelModuleName)};

        if (!isModuleInitiallyLoaded.has_value())
        {
            errorMessage = isModuleInitiallyLoaded.error();
            break;
        }

        // module is already unloaded, no action needed
        if (!(*isModuleInitiallyLoaded))
        {
            break;
        }

        // no need to include sudo in the command string -> the user needs to run the app with sudo anyway and if so the
        // command will be executed in sudo mode
        const std::string unloadCommand{"rmmod " + std::string{kernelModuleName}};
        const std::expected<std::string, std::string> commandResult{executeCommand(unloadCommand, READ_MODE)};

        if (!commandResult.has_value())
        {
            errorMessage = commandResult.error();
            break;
        }

        const std::expected<bool, std::string> isModuleLoaded{isKernelModuleLoaded(kernelModuleName)};

        if (!isModuleLoaded.has_value())
        {
            errorMessage = isModuleLoaded.error();
            break;
        }

        if ((*isModuleLoaded))
        {
            errorMessage = "Unable to unload module: " + std::string{kernelModuleName} +
                           ".\nCheck its dependencies and ensure sudo is being used.";
            break;
        }

    } while (false);

    return errorMessage;
}

std::expected<bool, std::string> Utilities::isKernelModuleLoaded(const std::string_view kernelModuleName)
{
    std::expected<bool, std::string> result;

    do
    {
        // kernel module name should not include the .ko extension from file (hence should contain only alphanumeric and
        // '_' characters)
        if (kernelModuleName.empty() ||
            std::ranges::any_of(kernelModuleName, [](char ch) { return !isalnum(ch) && ch != '_'; }))
        {
            result = std::unexpected{"Invalid module name!"};
            break;
        }

        const std::string kernelModuleNameStr{kernelModuleName};
        const std::string command{"lsmod | grep -w " + kernelModuleNameStr};
        const std::expected<std::string, std::string> commandResult{executeCommand(command, READ_MODE)};

        if (!commandResult)
        {
            result = std::unexpected{commandResult.error()};
            break;
        }

        // space is required to avoid corner cases where a module name is the prefix of another module name
        result = commandResult->starts_with(kernelModuleNameStr + ' ');
    } while (false);

    return result;
}

std::optional<std::string> Utilities::createCharacterDeviceFile(const std::filesystem::path& deviceFilePath,
                                                                int majorNumber, int minorNumber)
{
    std::optional<std::string> errorMessage;

    do
    {
        if (std::filesystem::exists(deviceFilePath))
        {
            errorMessage = "Path " + deviceFilePath.string() + " already exists.";
            break;
        }

        if (majorNumber <= 0)
        {
            errorMessage = "Invalid major number: " + std::to_string(majorNumber);
            break;
        }

        if (minorNumber < 0)
        {
            errorMessage = "Invalid minor number: " + std::to_string(minorNumber);
            break;
        }

        const std::string command{"mknod " + deviceFilePath.string() + " c " + std::to_string(majorNumber) + " " +
                                  std::to_string(minorNumber)};

        const std::expected<std::string, std::string> commandResult{executeCommand(command, WRITE_MODE)};

        if (!commandResult.has_value())
        {
            errorMessage = commandResult.error();
            break;
        }

        if (!std::filesystem::exists(deviceFilePath))
        {
            errorMessage = "An unknown error occurred.";
            break;
        }
    } while (false);

    return errorMessage;
}

int Utilities::getMajorDriverNumber(const std::string_view kernelModuleName)
{
    int result = -1;

    if (!kernelModuleName.empty())
    {
        const std::string command{"cat /proc/devices | grep -w " + std::string{kernelModuleName}};
        const std::expected<std::string, std::string> commandResult{executeCommand(command, READ_MODE)};

        if (commandResult)
        {
            int majorNumber = -1;
            std::istringstream istr{*commandResult};

            istr >> majorNumber;

            if (!istr.fail())
            {
                result = majorNumber;
            }
        }
    }

    return result;
}

std::optional<int> Utilities::readIntValueFromFile(const std::filesystem::path& filePath)
{
    std::optional<int> result;

    constexpr size_t maxAllowedDigitsCount{10}; // 32 bit integer considered
    constexpr size_t signCharsCount{1};         // '-'

    // in case there are some chars on the left/right side to be trimmed (and to reach a power of 2 bytes count)
    constexpr size_t paddingBytesCount{5};

    constexpr size_t maxCharsCountToRead{signCharsCount + maxAllowedDigitsCount + paddingBytesCount};
    const std::optional<std::string> str{readStringFromFile(filePath, maxCharsCountToRead, TRIM_MODE)};

    if (str.has_value() && isValidInteger(str->c_str()))
    {
        const bool isNegative{(*str)[0] == '-'};
        const size_t charsCount{str->size()};
        const bool isValidCharsCount{charsCount <= maxAllowedDigitsCount + static_cast<size_t>(isNegative)};

        if (isValidCharsCount)
        {
            const size_t firstDigitPos{static_cast<size_t>(isNegative)};
            const size_t lastDigitPos{charsCount - 1};

            int powerOfTenMultiplier{1};
            result = 0;

            for (size_t index{lastDigitPos}; index > firstDigitPos; --index)
            {
                const int currentDigit{static_cast<int>((*str)[index] - '0')};
                result = *result + currentDigit * powerOfTenMultiplier;
                powerOfTenMultiplier *= 10;
            }

            const int firstDigit = static_cast<int>((*str)[firstDigitPos] - '0');

            result = *result + firstDigit * powerOfTenMultiplier;
            result = *result * (isNegative ? -1 : 1);
        }
    }

    return result;
}

std::optional<std::string> Utilities::readStringFromFile(const std::filesystem::path& filePath, size_t charsCount,
                                                         bool shouldTrimInput)
{
    std::optional<std::string> result;
    const int fd{open(filePath.string().c_str(), O_RDONLY | O_NONBLOCK)};

    if (fd >= 0)
    {
        const size_t bufferSize{charsCount + 1};
        char buffer[bufferSize];
        memset(buffer, '\0', bufferSize);
        const ssize_t readCharsCount{read(fd, buffer, charsCount)};

        if (readCharsCount >= 0)
        {
            result = shouldTrimInput ? trimAndConvertToStdString(buffer) : std::string{buffer};
        }

        close(fd);
    }

    return result;
}

bool Utilities::writeStringToFile(const std::string& str, const std::filesystem::path& filePath, size_t charsCount)
{
    bool success{false};

    const int fd{open(filePath.string().c_str(), O_WRONLY | O_NONBLOCK)};

    if (fd >= 0)
    {
        const ssize_t writtenCharsCount{write(fd, str.c_str(), charsCount)};
        success = writtenCharsCount >= 0;
        close(fd);
    }

    return success;
}

void Utilities::clearFileContent(const std::filesystem::path& filePath)
{
    writeStringToFile("", filePath, 0);
}

void Utilities::clearScreen()
{
    (void)executeCommand("clear", WRITE_MODE);
}

std::filesystem::path Utilities::getApplicationPath()
{
    char applicationPath[PATH_MAX + 1];
    ssize_t pathSize = readlink("/proc/self/exe", applicationPath, PATH_MAX);
    applicationPath[pathSize] = '\0';

    return {applicationPath};
}

bool Utilities::isValidInteger(const char* str)
{
    bool isValidArg{false};

    assert(str);
    const std::string strObj{str ? str : ""};

    if (!strObj.empty())
    {
        const bool isFirstCharDigit{static_cast<bool>(std::isdigit(strObj[0]))};
        const bool isFirstCharMinus{strObj[0] == '-'};
        const bool areRemainingCharsDigits{
            std::all_of(strObj.cbegin() + 1, strObj.cend(), [](char ch) { return std::isdigit(ch); })};

        isValidArg =
            strObj.size() > 1 ? (isFirstCharDigit || isFirstCharMinus) && areRemainingCharsDigits : isFirstCharDigit;
    }

    return isValidArg;
}
