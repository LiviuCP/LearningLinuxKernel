#include <cassert>
#include <iostream>

#include "stopwatchapp.h"
#include "stopwatchcore.h"
#include "stopwatchloader.h"
#include "stopwatchparser.h"
#include "stopwatchsettings.h"

#include "utils.h"

#define SUCCESS 0
#define ERROR 1

namespace StopWatch::App
{
namespace
{
bool shouldStop{false};

constexpr size_t minutesInHour{60};
constexpr size_t secondsInMinute{60};
constexpr size_t millisecondsInSecond{1000};
constexpr size_t millisecondsSubScaleFactor{100};

void printTimeUnit(size_t value, const std::string& terminatingStr, bool shouldUsePrecedingZero = true)
{
    if (shouldUsePrecedingZero && value < 10)
    {
        std::cout << "0";
    }

    std::cout << value << terminatingStr;
}

void printTimeCallback(size_t millisecondsCount)
{
    const size_t seconds{millisecondsCount / millisecondsInSecond};
    const size_t minutes{seconds / secondsInMinute};
    const size_t hours{minutes / minutesInHour};
    const size_t minutesToDisplay{minutes % minutesInHour};
    const size_t secondsToDisplay{seconds % secondsInMinute};
    const size_t secondSubDivisionsToDisplay{(millisecondsCount % millisecondsInSecond) / millisecondsSubScaleFactor};

    Utilities::clearScreen();

    std::cout << "Elapsed time:\n\n";

    printTimeUnit(hours, ":");
    printTimeUnit(minutesToDisplay, ":");
    printTimeUnit(secondsToDisplay, ".");
    printTimeUnit(secondSubDivisionsToDisplay, "\n\n", false);

    std::cout << std::flush;
}

void stopCallback()
{
    shouldStop = true;
}
} // namespace
} // namespace StopWatch::App

int StopWatch::App::run(int argc, char** argv)
{
    int retVal{ERROR};
    do
    {
        const Parser::ParsingResult parsingResult{Parser::parse(argc, argv)};

        if (!parsingResult)
        {
            std::cerr << parsingResult.error() << "\n";
            break;
        }

        const std::optional<std::string> modulesLoadingErrorMessage{Loader::loadRequiredKernelModules()};

        if (modulesLoadingErrorMessage.has_value())
        {
            std::cerr << *modulesLoadingErrorMessage << "\n";
            break;
        }

        const std::optional<std::string> stopWatchStartErrorMessage{
            Core::start(printTimeCallback, stopCallback, *parsingResult)};

        if (stopWatchStartErrorMessage.has_value())
        {
            std::cerr << *stopWatchStartErrorMessage << "\n";
            break;
        }

        // this loop runs continuously until stopped by the stop callback
        while (!shouldStop)
            ;

        // backspaces are needed for erasing the printed ^C when hitting CTRL+C
        std::cout << "\b\bGoodbye!\n\a" << std::flush;

        const std::expected<bool, std::string> status{Core::getStatus()};

        if (!status.has_value())
        {
            std::cerr << status.error() << "\n";
            break;
        }

        const bool isStopped{!(*status)};

        if (isStopped)
        {
            retVal = SUCCESS;
            break;
        }

        // an invalid running status might trigger this
        assert(false && "Invalid status received!");
    } while (false);

    Loader::unloadRequiredKernelModules();

    return retVal;
}