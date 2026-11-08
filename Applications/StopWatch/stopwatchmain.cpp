#include <cassert>
#include <csignal>
#include <iostream>
#include <string_view>

#include <fcntl.h>
#include <sys/ioctl.h>

#include "loader.h"
#include "utils.h"

// 9999 is an arbitrarily chosen "magic number" (in a "real" (production) system an official assignment would be
// required; might be the major driver number)
#define IOCTL_REQUEST_SIGNAL _IOW(9999, 'a', void*)
#define IOCTL_RESET _IOW(9999, 'b', void*)

#define SIGTIMEOUT 44

#define SUCCESS 0
#define ERROR 1

static constexpr std::string_view devicePath{"/dev/timeperiodsignaler"};
static size_t milliSecondsCount{0};
static constexpr size_t timePeriodMilliseconds{10};
static bool timeoutRequestSuccess{false};
static bool interruptOccurred{false};

// By measuring I noticed that 60 "real seconds" are equivalent to 40 "counted seconds" (kernel measured time)
// this deviation could differ from device to device so it should be measured and adjusted accordingly
static constexpr size_t correctedTimePeriodMilliseconds{(timePeriodMilliseconds * 2) / 3};

static bool ioctlRequestSignal()
{
    bool result{false};
    const int fd{open(devicePath.data(), O_WRONLY)};

    if (fd > 0)
    {
        const long retVal{ioctl(fd, IOCTL_REQUEST_SIGNAL, &correctedTimePeriodMilliseconds)};
        close(fd);

        result = retVal == 0;
    }

    return result;
}

static bool ioctlReset()
{
    bool result{false};
    const int fd{open(devicePath.data(), O_WRONLY)};

    if (fd > 0)
    {
        const long retVal{ioctl(fd, IOCTL_RESET, nullptr)};
        close(fd);

        result = retVal == 0;
    }

    return result;
}

static void printTime()
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

static void signalHandler(int signal)
{
    switch (signal)
    {
    case SIGINT: {
        if (!ioctlReset())
        {
            std::cerr << "Reset failed!\n";
        }
        interruptOccurred = true;
        break;
    }
    case SIGTIMEOUT: {
        milliSecondsCount += timePeriodMilliseconds;
        printTime();
        timeoutRequestSuccess = ioctlRequestSignal();
        break;
    }
    default: {
        assert(false && "Unregistered signal!");
        break;
    }
    }
}

int main()
{
    int retVal{ERROR};
    do
    {
        const std::optional<std::string> modulesLoadingErrorMessage{loadRequiredKernelModules()};

        if (modulesLoadingErrorMessage.has_value())
        {
            std::cerr << *modulesLoadingErrorMessage << "\n";
            break;
        }

        std::signal(SIGINT, signalHandler);
        std::signal(SIGTIMEOUT, signalHandler);

        timeoutRequestSuccess = ioctlRequestSignal();

        if (!timeoutRequestSuccess)
        {
            std::cerr << "Requesting initial timeout failed!\n";
            break;
        }

        printTime();

        for (;;)
        {
            if (!timeoutRequestSuccess)
            {
                std::cerr << "Requesting new timeout failed!\n";
                break;
            }

            if (interruptOccurred)
            {
                // backspaces are needed for getting rid of the printed ^C when hitting CTRL+C
                std::cout << "\b\bGoodbye!\n\a" << std::flush;
                break;
            }
        }

        retVal = timeoutRequestSuccess ? SUCCESS : ERROR;
    } while (false);

    unloadRequiredKernelModules();

    return retVal;
}
