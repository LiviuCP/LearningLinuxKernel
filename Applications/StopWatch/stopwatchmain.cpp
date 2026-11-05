#include <csignal>
#include <filesystem>
#include <iostream>
#include <string_view>

#include <fcntl.h>
#include <sys/ioctl.h>

// 9999 is an arbitrarily chosen "magic number" (in a "real" (production) system an official assignment would be
// required; might be the major driver number)
#define IOCTL_REQUEST_SIGNAL _IOW(9999, 'a', void*)
#define IOCTL_RESET _IOW(9999, 'b', void*)

#define SIGTIMEOUT 44

static constexpr std::string_view devicePath{"/dev/timeperiodsignaler"};
static size_t milliSecondsCount{0};
static constexpr size_t timePeriodMilliseconds{10};

static bool ioctlRequestSignal()
{
    bool result{false};
    const int fd{open(devicePath.data(), O_WRONLY)};

    if (fd > 0)
    {
        const long retVal{ioctl(fd, IOCTL_REQUEST_SIGNAL, &timePeriodMilliseconds)};
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

    system("clear");
    std::cout << "\t" << minutes << ":" << remainingSeconds << "." << residual << "\n\n\t" << std::flush;
}

static void ctrlCInterruptHandler(int signal)
{
    if (signal == SIGINT)
    {
        // backspaces are needed for getting rid of the printed ^C when hitting CTRL+C
        std::cout << "\b\bGoodbye!\n\a";

        if (!ioctlReset())
        {
            std::cerr << "Reset failed!\n";
        }

        exit(signal);
    }
}

static void timeoutSignalHandler(int signal)
{
    if (signal == SIGTIMEOUT)
    {
        milliSecondsCount += timePeriodMilliseconds;

        printTime();

        if (!ioctlRequestSignal())
        {
            std::cerr << "Requesting new timeout failed!\n";
        }
    }
}

int main()
{
    do
    {
        std::signal(SIGINT, ctrlCInterruptHandler);
        std::signal(SIGTIMEOUT, timeoutSignalHandler);

        const bool deviceFileExists{std::filesystem::exists(std::string{devicePath})};

        if (!deviceFileExists)
        {
            std::cerr << "Device file does not exist! Please check if the driver got loaded.\n";
            break;
        }

        if (!ioctlRequestSignal())
        {
            std::cerr << "Requesting new timeout failed!\n";
            break;
        }

        printTime();

        for (;;)
            ;
    } while (false);

    return 0;
}
