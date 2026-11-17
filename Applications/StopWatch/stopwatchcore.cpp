#include <cassert>
#include <csignal>
#include <expected>
#include <string_view>

#include <fcntl.h>
#include <sys/ioctl.h>

#include "stopwatchcore.h"
#include "stopwatchparser.h"

// 9999 is an arbitrarily chosen "magic number" (in a "real" (production) system an official assignment would be
// required; might be the major driver number)
#define IOCTL_REQUEST_TIMEOUT_SIGNAL _IOW(9999, 'a', size_t*)
#define IOCTL_CANCEL_TIMEOUT_SIGNAL_REQUEST _IOW(9999, 'b', void*)

#define SIGTIMEOUT 44
#define RUNNING true
#define STOPPED false

namespace StopWatch::Core
{
namespace
{

constexpr std::string_view devicePath{"/dev/timeoutsignaler"};
constexpr size_t timeoutPeriodMilliseconds{10};

size_t millisecondsCount{0};
size_t resetThresholdMilliseconds{0};

std::expected<bool, std::string> status{STOPPED};

void (*printFn)(size_t){nullptr};
void (*stopFn)(){nullptr};

// By measuring I noticed that 60 "real seconds" are equivalent to 40 "counted seconds" (kernel measured time)
// this deviation could differ from device to device so it should be measured and adjusted accordingly
constexpr size_t correctedTimeoutPeriodMilliseconds{(timeoutPeriodMilliseconds * 2) / 3};

bool ioctlRequestTimeoutSignal()
{
    bool result{false};
    const int fd{open(devicePath.data(), O_WRONLY)};

    if (fd > 0)
    {
        const long retVal{ioctl(fd, IOCTL_REQUEST_TIMEOUT_SIGNAL, &correctedTimeoutPeriodMilliseconds)};
        close(fd);

        result = retVal == 0;
    }

    return result;
}

bool ioctlCancelTimeoutSignalRequest()
{
    bool result{false};
    const int fd{open(devicePath.data(), O_WRONLY)};

    if (fd > 0)
    {
        const long retVal{ioctl(fd, IOCTL_CANCEL_TIMEOUT_SIGNAL_REQUEST, nullptr)};
        close(fd);

        result = retVal == 0;
    }

    return result;
}

void checkAndHandleRunningInterrupted(bool shouldPrintElapsedTime = true)
{
    // a clean termination displaying the last recorded time should be ensured
    if (status != RUNNING)
    {
        printFn(millisecondsCount);
        (void)ioctlCancelTimeoutSignalRequest(); // no need to record the reset result, just a safety precaution
        stopFn();
    }
}

void updateElapsedTime()
{
    if (status == RUNNING)
    {
        millisecondsCount += timeoutPeriodMilliseconds;

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
        if (!ioctlRequestTimeoutSignal())
        {
            status = std::unexpected{"Requesting new timeout failed!"};
        }

        constexpr bool shouldNotPrintElapsedTime{false};
        checkAndHandleRunningInterrupted(shouldNotPrintElapsedTime);
    }
}

void signalHandler(int signal)
{
    switch (signal)
    {
    case SIGINT: {
        // SIGINT should only trigger while the stopwatch is running
        assert(status == RUNNING);
        status = ioctlCancelTimeoutSignalRequest() ? std::expected<bool, std::string>{STOPPED}
                                                   : std::unexpected{"Warning: cancelling timeout request failed."};
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
                                                  const StopWatch::Parser::Result& params)
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
            result = "Missing stop callback!";
            assert(false);
            break;
        }

        stopFn = stopCallback;

        if (params.seconds == 0)
        {
            result = "Invalid reset threshold!";
            assert(false);
            break;
        }

        resetThresholdMilliseconds = params.seconds * 1000;

        std::signal(SIGINT, signalHandler);
        std::signal(SIGTIMEOUT, signalHandler);

        if (!ioctlRequestTimeoutSignal())
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
