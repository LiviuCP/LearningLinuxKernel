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
constexpr size_t timePeriodMilliseconds{10};

size_t millisecondsCount{0};
size_t resetThresholdMilliseconds{0};

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

void checkAndHandleRunningInterrupted()
{
    // a clean termination displaying the last recorded time should be ensured
    if (status != RUNNING)
    {
        printFn(millisecondsCount);
        (void)ioctlReset(); // no need to record the reset result, just a safety precaution
        stopFn();
    }
}

void updateElapsedTime()
{
    if (status == RUNNING)
    {
        millisecondsCount += timePeriodMilliseconds;

        // auto-reset
        if (millisecondsCount == resetThresholdMilliseconds)
        {
            millisecondsCount = 0;
        }

        printFn(millisecondsCount);
        checkAndHandleRunningInterrupted();
    }
}

void requestNewTimeout()
{
    if (status == RUNNING)
    {
        if (!ioctlRequestSignal())
        {
            status = std::unexpected{"Requesting new timeout failed!"};
        }

        checkAndHandleRunningInterrupted();
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

std::optional<std::string> StopWatch::Core::start(void (*printCallback)(size_t), void (*stopCallback)(),
                                                  size_t resetThresholdMs)
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

        if (resetThresholdMs == 0)
        {
            result = "Invalid reset threshold";
            assert(false);
            break;
        }

        resetThresholdMilliseconds = resetThresholdMs;

        std::signal(SIGINT, signalHandler);
        std::signal(SIGTIMEOUT, signalHandler);

        if (!ioctlRequestSignal())
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
