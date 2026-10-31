#include <cassert>
#include <filesystem>
#include <optional>
#include <string>

#include "divisionmodulename.h"
#include "gcdcore.h"
#include "utils.h"

#if defined(PROCFS_GCD)
static constexpr std::string_view dividedFilePath{"/proc/division/divided"};
static constexpr std::string_view dividerFilePath{"/proc/division/divider"};
static constexpr std::string_view quotientFilePath{"/proc/division/quotient"};
static constexpr std::string_view remainderFilePath{"/proc/division/remainder"};
static constexpr std::string_view commandFilePath{"/proc/division/command"};
static constexpr std::string_view statusFilePath{"/proc/division/status"};
#elif defined(SYSFS_GCD)
static constexpr std::string_view dividedFilePath{"/sys/kernel/division/divided"};
static constexpr std::string_view dividerFilePath{"/sys/kernel/division/divider"};
static constexpr std::string_view quotientFilePath{"/sys/kernel/division/quotient"};
static constexpr std::string_view remainderFilePath{"/sys/kernel/division/remainder"};
static constexpr std::string_view commandFilePath{"/sys/kernel/division/command"};
static constexpr std::string_view statusFilePath{"/sys/kernel/division/status"};
#else
static_assert(false && "Invalid implementation!");
#endif

static constexpr std::string_view divideCommandStr{"divide"};
static constexpr std::string_view syncedStatusStr{"synced"};

static constexpr size_t maxStatusStrSize{16};

namespace GCD::Core
{
namespace
{
bool areDivisionProcfsFilesValid()
{
    return std::filesystem::exists(dividedFilePath) && std::filesystem::is_regular_file(dividedFilePath) &&
           std::filesystem::exists(dividerFilePath) && std::filesystem::is_regular_file(dividerFilePath) &&
           std::filesystem::exists(quotientFilePath) && std::filesystem::is_regular_file(quotientFilePath) &&
           std::filesystem::exists(remainderFilePath) && std::filesystem::is_regular_file(remainderFilePath) &&
           std::filesystem::exists(commandFilePath) && std::filesystem::is_regular_file(commandFilePath) &&
           std::filesystem::exists(statusFilePath) && std::filesystem::is_regular_file(statusFilePath);
}

std::optional<std::string> passDivisionOperandsToKernelModule(int divided, int divider)
{
    std::optional<std::string> errorMessage; // std::nullopt for success

    do
    {
        if (!areDivisionProcfsFilesValid())
        {
            errorMessage = "At least one procfs file from kernel module " + std::string{divisionModuleName} +
                           " does not exist or is invalid!\n"
                           "Please check that the module is loaded and generates correct files.";
            break;
        }

        if (const bool success{
                Utilities::writeStringToFile(std::to_string(divided), dividedFilePath, std::to_string(divided).size())};
            !success)
        {
            errorMessage = "File " + std::string{dividedFilePath} +
                           " could not be opened for writing!\nPlease try again by running the app with sudo.";
            break;
        }

        if (const bool success{
                Utilities::writeStringToFile(std::to_string(divider), dividerFilePath, std::to_string(divider).size())};
            !success)
        {
            errorMessage = "File " + std::string{dividerFilePath} + " could not be opened for writing!";
            break;
        }

        if (const bool success{
                Utilities::writeStringToFile(std::string{divideCommandStr}, commandFilePath, divideCommandStr.size())};
            !success)
        {
            errorMessage = "File " + std::string{commandFilePath} + " could not be opened for writing!";
        }
    } while (false);

    return errorMessage;
}

std::expected<int, std::string> retrieveResultFromKernelModule(const std::string_view resultFilePath)
{
    std::expected<int, std::string> result;

    do
    {
        const std::optional<std::string> status{
            Utilities::readStringFromFile(statusFilePath, maxStatusStrSize, TRIM_MODE)};

        if (!status.has_value())
        {
            result =
                std::unexpected{"File " + std::string{resultFilePath} +
                                " could not be opened for reading!\nPlease try again by running the app with sudo."};
            break;
        }

        if (status != syncedStatusStr)
        {
            result = std::unexpected{
                "Cannot read a valid result!\nEither the data is not synced or another error occurred."};
            break;
        }

        const std::optional<int> readResult{Utilities::readIntValueFromFile(resultFilePath)};

        if (!readResult.has_value())
        {
            result = std::unexpected{
                "Reading the file " + std::string{resultFilePath} +
                " failed!\nIt could not be opened for reading or contains an invalid (non-integer) value."};
            break;
        }

        result = *readResult;
    } while (false);

    return result;
}
} // namespace
} // namespace GCD::Core

std::expected<int, std::string> GCD::Core::retrieveQuotient(int divided, int divider)
{
    const std::optional<std::string> errorMessage{divider != 0 ? passDivisionOperandsToKernelModule(divided, divider)
                                                               : "Division by 0!"};

    return !errorMessage ? retrieveResultFromKernelModule(quotientFilePath) : std::unexpected{*errorMessage};
}

std::expected<int, std::string> GCD::Core::retrieveRemainder(int divided, int divider)
{
    const std::optional<std::string> errorMessage{divider != 0 ? passDivisionOperandsToKernelModule(divided, divider)
                                                               : "Division by 0!"};

    return !errorMessage ? retrieveResultFromKernelModule(remainderFilePath) : std::unexpected{*errorMessage};
}
