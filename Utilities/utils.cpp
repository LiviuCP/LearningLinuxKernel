#include <algorithm>
#include <cassert>
#include <cctype>
#include <climits>
#include <fstream>
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

std::optional<int> Utilities::readIntValueFromFile(const std::filesystem::path& filePath)
{
    std::optional<int> result;
    std::ifstream in{filePath};

    if (in.is_open())
    {
        int elementValue;
        in >> elementValue;

        if (!in.fail())
        {
            result = elementValue;
        }
    }

    return result;
}

std::optional<std::string> Utilities::readStringFromFile(const std::filesystem::path& filePath)
{
    std::optional<std::string> result;
    std::ifstream in{std::string{filePath}};

    if (in.is_open())
    {
        std::string str;
        in >> str;
        result = std::move(str);
    }

    return result;
}

bool Utilities::writeStringToFile(const std::string& str, const std::filesystem::path& filePath)
{
    bool success{false};
    std::ofstream out{std::string{filePath}};

    if (out.is_open())
    {
        out << str;
        success = true;
    }

    return success;
}

void Utilities::clearFileContent(const std::filesystem::path& filePath)
{
    if (!std::filesystem::exists(filePath) || !std::filesystem::is_regular_file(filePath))
    {
        throw std::runtime_error("File " + std::string{filePath} + " does not exist or is invalid!");
    }

    // this hack is needed to ensure the module attributes are updated even when the string is empty
    const std::string command{"echo | tee " + filePath.string() + " > /dev/null 2> /dev/null"};
    executeCommand(command, WRITE_MODE);
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
