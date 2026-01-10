#include <stdexcept>

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
