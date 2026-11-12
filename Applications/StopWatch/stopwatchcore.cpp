#include <cassert>
#include <csignal>
#include <expected>
#include <string_view>

#include <fcntl.h>
#include <sys/ioctl.h>

#include "stopwatchcore.h"

// 9999 is an arbitrarily chosen "magic number" (in a "real" (production) system an official assignment would be
// required; might be the major driver number)
#define IOCTL_REQUEST_SIGNAL _IOW(9999, 'a', void*)
#define IOCTL_RESET _IOW(9999, 'b', void*)

#define SIGTIMEOUT 44
#define RUNNING true
#define STOPPED false

namespace StopWatch::Core
{
namespace
{

constexpr std::string_view devicePath{"/dev/timeperiodsignaler"};
size_t milliSecondsCount{0};
constexpr size_t timePeriodMilliseconds{10};
bool timeoutRequestSuccess{false};
bool interruptOccurred{false};

std::expected<bool, std::string> status{STOPPED};

void (*printFn)(size_t){nullptr};
void (*stopFn)(){nullptr};

// By measuring I noticed that 60 "real seconds" are equivalent to 40 "counted seconds" (kernel measured time)
// this deviation could differ from device to device so it should be measured and adjusted accordingly
constexpr size_t correctedTimePeriodMilliseconds{(timePeriodMilliseconds * 2) / 3};

bool ioctlRequestSignal()
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

bool ioctlReset()
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

void updateElapsedTime()
{
    if (status == RUNNING)
    {
        milliSecondsCount += timePeriodMilliseconds;
        printFn(milliSecondsCount);
    }
}

void requestNewTimeout()
{
    if (status == RUNNING)
    {
        if (!ioctlRequestSignal())
        {
            status = std::unexpected{"Requesting new timeout failed!"};
            stopFn();
        }
    }
}

void signalHandler(int signal)
{
    switch (signal)
    {
    case SIGINT: {
        // SIGINT should only trigger while the stopwatch is running
        assert(status == RUNNING);
        status = ioctlReset() ? std::expected<bool, std::string>{STOPPED} : std::unexpected{"Warning: reset failed."};
        stopFn();
        break;
    }
    case SIGTIMEOUT: {
        updateElapsedTime();
        requestNewTimeout();

        // the timeout signal handling might get interrupted by SIGINT (e.g. while the above printFn() is running)
        // SIGINT would then change the status to a non-running one
        // in that case a clean termination displaying the last recorded time should be ensured
        if (!status.has_value() || status == STOPPED)
        {
            printFn(milliSecondsCount);
            stopFn();
        }

        break;
    }
    default: {
        assert(false && "Unregistered signal!");
        break;
    }
    }
}
} // namespace
} // namespace StopWatch::Core

std::optional<std::string> StopWatch::Core::start(void (*printCallback)(size_t), void (*stopCallback)())
{
    std::optional<std::string> result;

    do
    {
        if (!printCallback)
        {
            result = "Missing print callback!";
            assert(false);
            break;
        }

        printFn = printCallback;

        if (!stopCallback)
        {
            result = "Missing stop callback";
            assert(false);
            break;
        }

        stopFn = stopCallback;

        std::signal(SIGINT, signalHandler);
        std::signal(SIGTIMEOUT, signalHandler);

        timeoutRequestSuccess = ioctlRequestSignal();

        if (!timeoutRequestSuccess)
        {
            result = "Requesting initial timeout failed!";
            break;
        }

        status = RUNNING;
    } while (false);

    return result;
}

std::expected<bool, std::string> StopWatch::Core::getStatus()
{
    return status;
}

size_t StopWatch::Core::getMilliSecondsCount()
{
    return milliSecondsCount;
}
