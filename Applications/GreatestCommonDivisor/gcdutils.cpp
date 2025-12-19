#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <unistd.h>

#include "gcdutils.h"

namespace GcdUtils
{
namespace
{
static constexpr std::string_view dividedFilePath{"/sys/kernel/division/divided"};
static constexpr std::string_view dividerFilePath{"/sys/kernel/division/divider"};
static constexpr std::string_view quotientFilePath{"/sys/kernel/division/quotient"};
static constexpr std::string_view remainderFilePath{"/sys/kernel/division/remainder"};
static constexpr std::string_view commandFilePath{"/sys/kernel/division/command"};
static constexpr std::string_view statusFilePath{"/sys/kernel/division/status"};

static constexpr std::string_view divideCommandStr{"divide"};
static constexpr std::string_view syncedStatusStr{"synced"};
static constexpr std::string_view divisionModuleRelativePath{"KernelModules/ConsolidatedOutput/division.ko"};

bool isValidIntArg(const char* arg)
{
    bool isValidArg{false};

    assert(arg);
    const std::string argStr{arg ? arg : ""};

    if (!argStr.empty())
    {
        const bool isFirstCharDigit{static_cast<bool>(std::isdigit(argStr[0]))};
        const bool isFirstCharMinus{argStr[0] == '-'};
        const bool areRemainingCharsDigits{
            std::all_of(argStr.cbegin() + 1, argStr.cend(), [](char ch) { return std::isdigit(ch); })};

        isValidArg =
            argStr.size() > 1 ? (isFirstCharDigit || isFirstCharMinus) && areRemainingCharsDigits : isFirstCharDigit;
    }

    return isValidArg;
}

bool areDivisionSysfsFilesValid()
{
    return std::filesystem::exists(dividedFilePath) || std::filesystem::is_regular_file(dividedFilePath) ||
           std::filesystem::exists(dividerFilePath) || std::filesystem::is_regular_file(dividerFilePath) ||
           std::filesystem::exists(quotientFilePath) || std::filesystem::is_regular_file(quotientFilePath) ||
           std::filesystem::exists(remainderFilePath) || std::filesystem::is_regular_file(remainderFilePath) ||
           std::filesystem::exists(commandFilePath) || std::filesystem::is_regular_file(commandFilePath) ||
           std::filesystem::exists(statusFilePath) || std::filesystem::is_regular_file(statusFilePath);
}

std::filesystem::path getApplicationPath()
{
    char applicationPath[PATH_MAX + 1];
    ssize_t pathSize = readlink("/proc/self/exe", applicationPath, PATH_MAX);
    applicationPath[pathSize] = '\0';

    return {applicationPath};
}

std::optional<std::filesystem::path> getDivisionModulePath()
{
    const std::filesystem::path applicationPath{getApplicationPath()};

    std::filesystem::path divisionModulePath{applicationPath.parent_path().parent_path().parent_path()};
    divisionModulePath /= divisionModuleRelativePath;

    return std::filesystem::is_regular_file(divisionModulePath) ? std::optional{divisionModulePath} : std::nullopt;
}

std::string executeCommand(const std::string& command)
{
    std::string commandOutput;
    char buffer[128];

    FILE* pipe{popen(command.c_str(), "r")};

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

void passDivisionOperandsToKernelModule(int divided, int divider)
{
    if (!areDivisionSysfsFilesValid())
    {
        throw std::runtime_error("At least one sysfs file from kernel module \"division\" does not exist or is "
                                 "invalid!\nPlease check that the module is loaded and generates correct files.");
    }

    std::ofstream toDividedFile{std::string{dividedFilePath}};

    if (!toDividedFile.is_open())
    {
        throw std::runtime_error("File " + std::string{dividedFilePath} +
                                 " could not be opened for writing!\nPlease try again by running the app with sudo.");
    }

    std::ofstream toDividerFile{std::string{dividerFilePath}};

    if (!toDividerFile.is_open())
    {
        throw std::runtime_error("File " + std::string{dividerFilePath} + " could not be opened for writing!");
    }

    std::ofstream toCommandFile{std::string{commandFilePath}};

    if (!toCommandFile.is_open())
    {
        throw std::runtime_error("File " + std::string{commandFilePath} + " could not be opened for writing!");
    }

    toDividedFile << divided;
    toDividedFile.close();
    toDividerFile << divider;
    toDividerFile.close();
    toCommandFile << divideCommandStr;
    toCommandFile.close();
}

int retrieveResultFromKernelModule(const std::string_view resultFilePath)
{
    std::ifstream fromStatusFile{std::string{statusFilePath}};

    if (!fromStatusFile.is_open())
    {
        throw std::runtime_error("File " + std::string{resultFilePath} +
                                 " could not be opened for reading!\nPlease try again by running the app with sudo.");
    }

    std::string status;
    fromStatusFile >> status;

    if (status != syncedStatusStr)
    {
        throw std::runtime_error(
            "Cannot read a valid result!\nEither the data is not synced or another error occurred.");
    }

    std::ifstream fromResultFile{std::string{resultFilePath}};

    if (!fromResultFile.is_open())
    {
        throw std::runtime_error("File " + std::string{resultFilePath} + " could not be opened for reading!");
    }

    int result;
    fromResultFile >> result;

    if (fromResultFile.fail())
    {
        throw std::runtime_error("Reading the file " + std::string{resultFilePath} +
                                 " failed!\nPossibly it contains an invalid (non-integer) value.");
    }

    return result;
}
} // namespace
} // namespace GcdUtils

ParsedArguments GcdUtils::parseArguments(int argc, char** argv)
{
    bool areValidArguments{argc >= 1 && argv && argv[0]};
    assert(areValidArguments);

    ParsedArguments result;

    // default values in case not entered by user
    int first{0}, second{1};

    if (areValidArguments && argc > 1)
    {
        areValidArguments = isValidIntArg(argv[1]);

        if (areValidArguments)
        {
            first = atoi(argv[1]);
        }
    }

    if (areValidArguments && argc > 2)
    {
        areValidArguments = isValidIntArg(argv[2]);

        if (areValidArguments)
        {
            second = atoi(argv[2]);
        }
    }

    if (areValidArguments)
    {
        result = {first, second};
    }

    return result;
}

void GcdUtils::loadKernelModuleDivision()
{
    const std::optional<std::filesystem::path> divisionModulePath{getDivisionModulePath()};

    if (divisionModulePath.has_value())
    {
        // no need to include sudo in the command string -> the user needs to run the app with sudo anyway and if so the
        // command will be executed in sudo mode
        const std::string loadCommand{"insmod " + divisionModulePath->string() + " 2> /dev/null"};
        system(loadCommand.c_str());
    }

    if (!isKernelModuleDivisionLoaded())
    {
        throw std::runtime_error{
            "Could not load kernel module Division!\nPlease try again by running the app with sudo."};
    }
}

void GcdUtils::unloadKernelModuleDivision()
{
    const std::optional<std::filesystem::path> divisionModulePath{getDivisionModulePath()};

    if (divisionModulePath.has_value())
    {
        // no need to include sudo in the command string -> the user needs to run the app with sudo anyway and if so the
        // command will be executed in sudo mode
        const std::string unloadCommand{"rmmod " + divisionModulePath->filename().stem().string()};
        system(unloadCommand.c_str());
    }
}

bool GcdUtils::isKernelModuleDivisionLoaded()
{
    const std::string commandOutput{executeCommand("lsmod | grep -w division")};
    return commandOutput.starts_with("division");
}

int GcdUtils::retrieveQuotient(int divided, int divider)
{
    if (divider == 0)
    {
        throw std::runtime_error{"Division by 0!"};
    }

    passDivisionOperandsToKernelModule(divided, divider);
    return retrieveResultFromKernelModule(quotientFilePath);
}

int GcdUtils::retrieveRemainder(int divided, int divider)
{
    if (divider == 0)
    {
        throw std::runtime_error{"Division by 0!"};
    }

    passDivisionOperandsToKernelModule(divided, divider);
    return retrieveResultFromKernelModule(remainderFilePath);
}

int GcdUtils::retrieveGreatestCommonDivisor(int first, int second)
{
    if (first == 0 && second == 0)
    {
        throw std::runtime_error{"Cannot retrieve greatest common divisor when both numbers are 0!"};
    }

    if (second == 0)
    {
        std::swap(first, second);
    }

    const int remainder{retrieveRemainder(first, second)};
    return remainder == 0 ? second : retrieveGreatestCommonDivisor(second, remainder);
}
