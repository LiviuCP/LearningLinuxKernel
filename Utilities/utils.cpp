#include <algorithm>
#include <cassert>
#include <cctype>
#include <climits>
#include <stdexcept>
#include <unistd.h>

#include "utils.h"

std::string Utilities::executeCommand(const std::string& command, bool isWriteMode)
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

std::filesystem::path Utilities::getApplicationPath()
{
    char applicationPath[PATH_MAX + 1];
    ssize_t pathSize = readlink("/proc/self/exe", applicationPath, PATH_MAX);
    applicationPath[pathSize] = '\0';

    return {applicationPath};
}

void Utilities::clearScreen()
{
    Utilities::executeCommand("clear", WRITE_MODE);
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
