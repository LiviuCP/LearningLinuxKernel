#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "ioctlgcdcore.h"

#define IOCTL_STORE_DIVIDED_VALUE _IOW(9999, 'a', int*)
#define IOCTL_STORE_DIVIDER_VALUE _IOW(9999, 'c', int*)
#define IOCTL_SHOW_QUOTIENT_VALUE _IOR(9999, 'e', int*)
#define IOCTL_SHOW_REMAINDER_VALUE _IOR(9999, 'f', int*)
#define IOCTL_DIVIDE _IOW(9999, 'g', void*)
#define IOCTL_GET_SYNCED_STATUS _IOR(9999, 'i', uint8_t*)

static constexpr std::string_view deviceDirPath{"/dev"};
static constexpr std::string_view baseDeviceFileName{"ioctldivision"};

namespace GCD::Core
{
namespace
{
std::filesystem::path getDeviceFilePath()
{
    std::filesystem::path deviceFilePath{deviceDirPath};
    deviceFilePath /= std::string{baseDeviceFileName};

    return deviceFilePath;
}

bool isDeviceFileValid()
{
    return std::filesystem::is_character_file(getDeviceFilePath());
}

void passDivisionOperandsToKernelModule(int divided, int divider)
{
    if (!isDeviceFileValid())
    {
        throw std::runtime_error("The device file from kernel module \"ioctl_division\" does not exist or is "
                                 "invalid!\nPlease check that the module is loaded and generates correct files.");
    }

    const int fd{open(getDeviceFilePath().c_str(), O_WRONLY)};

    if (fd < 0)
    {
        throw std::runtime_error("The device file from kernel module \"ioctl_division\" cannot be opened for "
                                 "writing!\nPlease try again by running the app with sudo.");
    }

    long retVal{ioctl(fd, IOCTL_STORE_DIVIDED_VALUE, &divided)};

    if (retVal != 0)
    {
        close(fd);
        throw std::runtime_error("Error in writing divided value to module!");
    }

    retVal = ioctl(fd, IOCTL_STORE_DIVIDER_VALUE, &divider);

    if (retVal != 0)
    {
        close(fd);
        throw std::runtime_error("Error in writing divider value to module!");
    }

    retVal = ioctl(fd, IOCTL_DIVIDE);

    if (retVal != 0)
    {
        close(fd);
        throw std::runtime_error("Error in performing division!");
    }

    close(fd);
}

int retrieveResultFromKernelModule(const unsigned int command)
{
    int result{0};
    const int fd{open(getDeviceFilePath().c_str(), O_RDONLY)};

    if (fd < 0)
    {
        throw std::runtime_error("The device file from kernel module \"ioctl_division\" cannot be opened for "
                                 "reading!\nPlease try again by running the app with sudo.");
    }

    bool isSynced;
    long retVal{ioctl(fd, IOCTL_GET_SYNCED_STATUS, &isSynced)};

    if (retVal != 0)
    {
        close(fd);
        throw std::runtime_error("Error in reading sync status from module!");
    }

    if (command == IOCTL_SHOW_QUOTIENT_VALUE || command == IOCTL_SHOW_REMAINDER_VALUE)
    {
        retVal = ioctl(fd, command, &result);
    }
    else
    {
        close(fd);
        throw std::runtime_error("Invalid IOCTL command!");
    }

    if (retVal != 0)
    {
        close(fd);
        throw std::runtime_error("Error in reading quotient/remainder from module!");
    }

    close(fd);

    return result;
}
} // namespace
} // namespace GCD::Core

int GCD::Core::retrieveQuotient(int divided, int divider)
{
    if (divider == 0)
    {
        throw std::runtime_error{"Division by 0!"};
    }

    passDivisionOperandsToKernelModule(divided, divider);
    return retrieveResultFromKernelModule(IOCTL_SHOW_QUOTIENT_VALUE);
}

int GCD::Core::retrieveRemainder(int divided, int divider)
{
    if (divider == 0)
    {
        throw std::runtime_error{"Division by 0!"};
    }

    passDivisionOperandsToKernelModule(divided, divider);
    return retrieveResultFromKernelModule(IOCTL_SHOW_REMAINDER_VALUE);
}

int GCD::Core::retrieveGreatestCommonDivisor(int first, int second)
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
