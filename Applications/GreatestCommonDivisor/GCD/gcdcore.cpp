#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>

#include "gcdcore.h"
#include "utils.h"

static constexpr std::string_view dividedFilePath{"/sys/kernel/division/divided"};
static constexpr std::string_view dividerFilePath{"/sys/kernel/division/divider"};
static constexpr std::string_view quotientFilePath{"/sys/kernel/division/quotient"};
static constexpr std::string_view remainderFilePath{"/sys/kernel/division/remainder"};
static constexpr std::string_view commandFilePath{"/sys/kernel/division/command"};
static constexpr std::string_view statusFilePath{"/sys/kernel/division/status"};

static constexpr std::string_view divideCommandStr{"divide"};
static constexpr std::string_view syncedStatusStr{"synced"};

static constexpr size_t maxStatusStrSize{16};

namespace GCD::Core
{
namespace
{
bool areDivisionSysfsFilesValid()
{
    return std::filesystem::exists(dividedFilePath) && std::filesystem::is_regular_file(dividedFilePath) &&
           std::filesystem::exists(dividerFilePath) && std::filesystem::is_regular_file(dividerFilePath) &&
           std::filesystem::exists(quotientFilePath) && std::filesystem::is_regular_file(quotientFilePath) &&
           std::filesystem::exists(remainderFilePath) && std::filesystem::is_regular_file(remainderFilePath) &&
           std::filesystem::exists(commandFilePath) && std::filesystem::is_regular_file(commandFilePath) &&
           std::filesystem::exists(statusFilePath) && std::filesystem::is_regular_file(statusFilePath);
}

void passDivisionOperandsToKernelModule(int divided, int divider)
{
    if (!areDivisionSysfsFilesValid())
    {
        throw std::runtime_error("At least one sysfs file from kernel module \"division\" does not exist or is "
                                 "invalid!\nPlease check that the module is loaded and generates correct files.");
    }

    if (const bool success{
            Utilities::writeStringToFile(std::to_string(divided), dividedFilePath, std::to_string(divided).size())};
        !success)
    {
        throw std::runtime_error("File " + std::string{dividedFilePath} +
                                 " could not be opened for writing!\nPlease try again by running the app with sudo.");
    }

    if (const bool success{
            Utilities::writeStringToFile(std::to_string(divider), dividerFilePath, std::to_string(divider).size())};
        !success)
    {
        throw std::runtime_error("File " + std::string{dividerFilePath} + " could not be opened for writing!");
    }

    if (const bool success{
            Utilities::writeStringToFile(std::string{divideCommandStr}, commandFilePath, divideCommandStr.size())};
        !success)
    {
        throw std::runtime_error("File " + std::string{commandFilePath} + " could not be opened for writing!");
    }
}

int retrieveResultFromKernelModule(const std::string_view resultFilePath)
{
    const std::optional<std::string> status{Utilities::readStringFromFile(statusFilePath, maxStatusStrSize)};

    if (!status.has_value())
    {
        throw std::runtime_error("File " + std::string{resultFilePath} +
                                 " could not be opened for reading!\nPlease try again by running the app with sudo.");
    }

    if (status != syncedStatusStr)
    {
        throw std::runtime_error(
            "Cannot read a valid result!\nEither the data is not synced or another error occurred.");
    }

    const std::optional<int> result{Utilities::readIntValueFromFile(resultFilePath)};

    if (!result.has_value())
    {
        throw std::runtime_error(
            "Reading the file " + std::string{resultFilePath} +
            " failed!\nIt could not be opened for reading or contains an invalid (non-integer) value.");
    }

    return *result;
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
    return retrieveResultFromKernelModule(quotientFilePath);
}

int GCD::Core::retrieveRemainder(int divided, int divider)
{
    if (divider == 0)
    {
        throw std::runtime_error{"Division by 0!"};
    }

    passDivisionOperandsToKernelModule(divided, divider);
    return retrieveResultFromKernelModule(remainderFilePath);
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
