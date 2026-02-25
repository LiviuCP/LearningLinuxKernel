#include <algorithm>
#include <cassert>
#include <cctype>
#include <climits>
#include <cstring>
#include <fcntl.h>
#include <sstream>
#include <stdexcept>
#include <unistd.h>

#include "utils.h"

namespace Utilities
{
namespace
{
std::string executeCommand(const std::string& command, bool isWriteMode)
{
    std::string commandOutput;
    char buffer[128];
    const char* mode{isWriteMode ? "w" : "r"};

    FILE* pipe{popen(command.c_str(), mode)};

    if (!pipe)
    {
        throw std::runtime_error{"Failed to open pipe!"};
    }

    try
    {
        while (fgets(buffer, sizeof buffer, pipe) != NULL)
        {
            commandOutput += buffer;
        }
    }
    catch (...)
    {
        pclose(pipe);
        throw std::runtime_error{"Error in reading from pipe!"};
    }

    pclose(pipe);

    return commandOutput;
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

void Utilities::loadKernelModule(const std::filesystem::path& kernelModulePath)
{
    if (std::filesystem::exists(kernelModulePath) && std::filesystem::is_regular_file(kernelModulePath))
    {
        // no need to include sudo in the command string -> the user needs to run the app with sudo anyway and if so the
        // command will be executed in sudo mode
        const std::string loadCommand{"insmod " + kernelModulePath.string() + " 2> /dev/null"};
        executeCommand(loadCommand, READ_MODE);
    }
}

void Utilities::unloadKernelModule(const std::string_view kernelModuleName)
{
    if (!kernelModuleName.empty())
    {
        // no need to include sudo in the command string -> the user needs to run the app with sudo anyway and if so the
        // command will be executed in sudo mode
        const std::string unloadCommand{"rmmod " + std::string{kernelModuleName}};
        executeCommand(unloadCommand, READ_MODE);
    }
}

bool Utilities::isKernelModuleLoaded(const std::string_view kernelModuleName)
{
    bool isLoaded{false};

    if (!kernelModuleName.empty())
    {
        const std::string command{"lsmod | grep -w " + std::string{kernelModuleName}};
        const std::string commandOutput{executeCommand(command, READ_MODE)};

        isLoaded = commandOutput.starts_with(kernelModuleName);
    }

    return isLoaded;
}

bool Utilities::createCharacterDeviceFile(const std::filesystem::path& deviceFilePath, int majorNumber, int minorNumber)
{
    bool success{false};

    if (!std::filesystem::exists(deviceFilePath) && majorNumber > 0 && minorNumber >= 0)
    {
        const std::string command{"mknod " + deviceFilePath.string() + " c " + std::to_string(majorNumber) + " " +
                                  std::to_string(minorNumber)};
        (void)executeCommand(command, WRITE_MODE);

        success = std::filesystem::exists(deviceFilePath);
    }

    return success;
}

int Utilities::getMajorDriverNumber(const std::string_view kernelModuleName)
{
    int result = -1;

    if (!kernelModuleName.empty())
    {
        const std::string command{"cat /proc/devices | grep -w " + std::string{kernelModuleName}};
        const std::string commandOutput{executeCommand(command, READ_MODE)};

        int majorNumber = -1;
        std::istringstream istr{commandOutput};

        istr >> majorNumber;

        if (!istr.fail())
        {
            result = majorNumber;
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
    const std::optional<std::string> str{readStringFromFile(filePath, maxCharsCountToRead)};

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

std::optional<std::string> Utilities::readStringFromFile(const std::filesystem::path& filePath, size_t charsCount)
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
            result = trimAndConvertToStdString(buffer);
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
    try
    {
        executeCommand("clear", WRITE_MODE);
    }
    catch (...)
    {
    }
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
