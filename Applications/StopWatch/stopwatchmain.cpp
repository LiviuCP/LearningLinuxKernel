#include <cassert>
#include <iostream>

#include "stopwatchcore.h"
#include "stopwatchloader.h"
#include "utils.h"

#define SUCCESS 0
#define ERROR 1

static bool shouldStop{false};

static void printTimeCallback(size_t milliSecondsCount)
{
    const size_t seconds{milliSecondsCount / 1000};
    const size_t minutes{seconds / 60};
    const size_t remainingSeconds{seconds % 60};
    const size_t residual{(milliSecondsCount % 1000) / 10};

    Utilities::clearScreen();

    std::cout << "Elapsed time:\n\n";

    if (minutes < 10)
    {
        std::cout << "0";
    }

    std::cout << minutes << ":";

    if (remainingSeconds < 10)
    {
        std::cout << "0";
    }

    std::cout << remainingSeconds << ".";

    if (residual < 10)
    {
        std::cout << "0";
    }

    std::cout << residual << "\n\n" << std::flush;
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

        while (!shouldStop)
            ;

        const std::expected<bool, std::string> status{StopWatch::Core::getStatus()};

        if (!status.has_value())
        {
            std::cerr << status.error() << "\n";
            break;
        }

        const bool isStopped{!(*status)};

        if (isStopped)
        {
            // backspaces are needed for getting rid of the printed ^C when hitting CTRL+C
            std::cout << "\b\bGoodbye!\n\a" << std::flush;
            retVal = SUCCESS;
            break;
        }

        assert(false && "Invalid status received!");
    } while (false);

    StopWatch::Loader::unloadRequiredKernelModules();

    return retVal;
}
