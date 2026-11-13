#include <cassert>
#include <iostream>

#include "stopwatchcore.h"
#include "stopwatchloader.h"
#include "utils.h"

#define SUCCESS 0
#define ERROR 1

static bool shouldStop{false};

static constexpr size_t minutesInHour{60};
static constexpr size_t secondsInMinute{60};
static constexpr size_t millisecondsInSecond{1000};
static constexpr size_t millisecondsSubScaleFactor{10};

static void printTimeCallback(size_t milliSecondsCount)
{
    const size_t seconds{milliSecondsCount / millisecondsInSecond};
    const size_t minutesToDisplay{seconds / secondsInMinute};
    const size_t secondsToDisplay{seconds % secondsInMinute};
    const size_t subDivisionsToDisplay{(milliSecondsCount % millisecondsInSecond) / millisecondsSubScaleFactor};

    Utilities::clearScreen();

    std::cout << "Elapsed time:\n\n";

    if (minutesToDisplay < 10)
    {
        std::cout << "0";
    }

    std::cout << minutesToDisplay << ":";

    if (secondsToDisplay < 10)
    {
        std::cout << "0";
    }

    std::cout << secondsToDisplay << ".";

    if (subDivisionsToDisplay < 10)
    {
        std::cout << "0";
    }

    std::cout << subDivisionsToDisplay << "\n\n" << std::flush;
}

static void stopCallback()
{
    shouldStop = true;
}

int main()
{
    int retVal{ERROR};
    do
    {
        const std::optional<std::string> modulesLoadingErrorMessage{StopWatch::Loader::loadRequiredKernelModules()};

        if (modulesLoadingErrorMessage.has_value())
        {
            std::cerr << *modulesLoadingErrorMessage << "\n";
            break;
        }

        const std::optional<std::string> stopWatchStartErrorMessage{
            StopWatch::Core::start(printTimeCallback, stopCallback)};

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

        const std::expected<bool, std::string> status{StopWatch::Core::getStatus()};

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

    StopWatch::Loader::unloadRequiredKernelModules();

    return retVal;
}
