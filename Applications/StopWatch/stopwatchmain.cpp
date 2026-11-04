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

static bool ioctlRequestSignal()
{
    bool result{false};
    const int fd{open(devicePath.data(), O_WRONLY)};

    if (fd > 0)
    {
        const long retVal{ioctl(fd, IOCTL_REQUEST_SIGNAL, nullptr)};
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
        std::cout << "\nTimeout signal " << signal << " arrived! ";
        std::cout << "Press CTRL + C to exit.\n\n" << std::flush;
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
            std::cerr << "Requesting signal failed!\n";
        }

        std::cout << "Waiting for timeout...\n" << std::flush;

        for (;;)
            ;
    } while (false);

    return 0;
}
