#include <algorithm>
#include <cassert>
#include <cctype>
#include <climits>
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

void Utilities::clearScreen()
{
    executeCommand("clear", WRITE_MODE);
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
