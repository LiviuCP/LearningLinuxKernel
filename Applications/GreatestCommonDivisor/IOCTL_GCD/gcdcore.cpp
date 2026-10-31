#include <cassert>
#include <filesystem>
#include <optional>
#include <string>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "divisionmodulename.h"
#include "gcdcore.h"

#define IOCTL_STORE_DIVIDED_VALUE _IOW(9999, 'a', int*)
#define IOCTL_STORE_DIVIDER_VALUE _IOW(9999, 'c', int*)
#define IOCTL_SHOW_QUOTIENT_VALUE _IOR(9999, 'e', int*)
#define IOCTL_SHOW_REMAINDER_VALUE _IOR(9999, 'f', int*)
#define IOCTL_DIVIDE _IOW(9999, 'g', void*)
#define IOCTL_GET_SYNCED_STATUS _IOR(9999, 'i', bool*)

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

std::optional<std::string> passDivisionOperandsToKernelModule(int divided, int divider)
{
    std::optional<std::string> errorMessage; // std::nullopt for success
    int fd{-1};

    do
    {
        if (!isDeviceFileValid())
        {
            errorMessage = "The device file from kernel module " + std::string{divisionModuleName} +
                           " does not exist or is invalid!\n"
                           "Please check that the module is loaded and generates correct files.";
            break;
        }

        fd = open(getDeviceFilePath().c_str(), O_WRONLY);

        if (fd < 0)
        {
            errorMessage = "The device file from kernel module " + std::string{divisionModuleName} +
                           " cannot be opened for writing!\n"
                           "Please try again by running the app with sudo.";
            break;
        }

        long retVal{ioctl(fd, IOCTL_STORE_DIVIDED_VALUE, &divided)};

        if (retVal != 0)
        {
            errorMessage = "Error in writing divided value to module!";
            break;
        }

        retVal = ioctl(fd, IOCTL_STORE_DIVIDER_VALUE, &divider);

        if (retVal != 0)
        {
            errorMessage = "Error in writing divider value to module!";
            break;
        }

        retVal = ioctl(fd, IOCTL_DIVIDE);

        if (retVal != 0)
        {
            errorMessage = "Error in performing division!";
            break;
        }
    } while (false);

    if (fd >= 0)
    {
        close(fd);
    }

    return errorMessage;
}

std::expected<int, std::string> retrieveResultFromKernelModule(const unsigned int command)
{
    std::expected<int, std::string> result;
    int fd{-1};

    do
    {
        int commandResult{0};
        fd = open(getDeviceFilePath().c_str(), O_RDONLY);

        if (fd < 0)
        {
            result = std::unexpected{"The device file from kernel module " + std::string{divisionModuleName} +
                                     " cannot be opened for reading!\n"
                                     "Please try again by running the app with sudo."};
            break;
        }

        bool isSynced;
        long retVal{ioctl(fd, IOCTL_GET_SYNCED_STATUS, &isSynced)};

        if (retVal != 0)
        {
            result = std::unexpected{"Error in reading sync status from module!"};
            break;
        }

        if (command != IOCTL_SHOW_QUOTIENT_VALUE && command != IOCTL_SHOW_REMAINDER_VALUE)
        {
            result = std::unexpected{"Invalid IOCTL command!"};
            break;
        }

        retVal = ioctl(fd, command, &commandResult);

        if (retVal != 0)
        {
            result = std::unexpected{"Error in reading quotient/remainder from module!"};
            break;
        }

        result = commandResult;
    } while (false);

    if (fd >= 0)
    {
        close(fd);
    }

    return result;
}
} // namespace
} // namespace GCD::Core

std::expected<int, std::string> GCD::Core::retrieveQuotient(int divided, int divider)
{
    const std::optional<std::string> errorMessage{divider != 0 ? passDivisionOperandsToKernelModule(divided, divider)
                                                               : "Division by 0!"};

    return !errorMessage ? retrieveResultFromKernelModule(IOCTL_SHOW_QUOTIENT_VALUE) : std::unexpected{*errorMessage};
}

std::expected<int, std::string> GCD::Core::retrieveRemainder(int divided, int divider)
{
    const std::optional<std::string> errorMessage{divider != 0 ? passDivisionOperandsToKernelModule(divided, divider)
                                                               : "Division by 0!"};

    return !errorMessage ? retrieveResultFromKernelModule(IOCTL_SHOW_REMAINDER_VALUE) : std::unexpected{*errorMessage};
}
