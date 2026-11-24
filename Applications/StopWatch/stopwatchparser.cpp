#include <algorithm>
#include <cassert>
#include <optional>
#include <vector>

#include "stopwatchparser.h"

namespace StopWatch::Parser
{
namespace
{
enum class ErrorCode
{
    INVALID,
    DUPLICATE,
    OUT_OF_BOUNDS
};

// excluding the application name
constexpr size_t maxArgumentsCount{3};

std::optional<ErrorCode> errorCode;

std::vector<std::string> collectArguments(int argc, char** argv)
{
    std::vector<std::string> arguments;
    assert(argv && argc >= 1);

    if (argv && argc > 1)
    {
        const size_t argumentsToParseCount{std::min<size_t>(static_cast<size_t>(argc - 1), maxArgumentsCount)};
        arguments.reserve(argumentsToParseCount);

        for (size_t index{0}; index < argumentsToParseCount; ++index)
        {
            const char* const currentArg{argv[index + 1]};
            assert(currentArg);

            if (currentArg)
            {
                arguments.emplace_back(currentArg);
            }
        }
    }

    return arguments;
}

bool checkIfValidOption(const std::string& argument, const std::string& optionToCheck, bool& optionFlag)
{
    bool optionFound{false};

    do
    {
        if (argument != optionToCheck)
        {
            break;
        }

        if (optionFlag)
        {
            errorCode = ErrorCode::DUPLICATE;
            break;
        }

        optionFlag = true;
        optionFound = true;
    } while (false);

    return optionFound;
}

void checkIfValidBoundedNumericValue(const std::string& argument, size_t lowerBound, size_t upperBound,
                                     std::optional<size_t>& intervalValue)
{
    do
    {
        try
        {
            if (lowerBound > upperBound)
            {
                assert(false);
                break;
            }

            if (std::ranges::any_of(argument, [](char c) { return !isdigit(c); }))
            {
                errorCode = ErrorCode::INVALID;
                break;
            }

            const size_t convertedValue{std::stoul(argument)};

            if (intervalValue)
            {
                errorCode = ErrorCode::DUPLICATE;
                break;
            }

            if (convertedValue < lowerBound || convertedValue > upperBound)
            {
                errorCode = ErrorCode::OUT_OF_BOUNDS;
                break;
            }

            intervalValue = convertedValue;
        }
        catch (...)
        {
            errorCode = ErrorCode::INVALID;
            break;
        }
    } while (false);
}

std::string getErrorMessage(ErrorCode errorCode)
{
    std::string errorMessage;

    switch (errorCode)
    {
    case ErrorCode::INVALID:
        errorMessage = "At least one argument is invalid!";
        break;
    case ErrorCode::DUPLICATE:
        errorMessage = "At least one argument is duplicate!";
        break;
    case ErrorCode::OUT_OF_BOUNDS:
        errorMessage = "The provided time interval is out of bounds!\nShould be within range: " +
                       std::to_string(Settings::minIntervalValueSec) + " - " +
                       std::to_string(Settings::maxIntervalValueSec) + " seconds";
        break;
    default:
        assert(false);
        break;
    }

    return errorMessage;
}

} // namespace
} // namespace StopWatch::Parser

std::expected<StopWatch::Settings::Parameters, std::string> StopWatch::Parser::parse(int argc, char** argv)
{
    std::expected<Settings::Parameters, std::string> result;
    std::optional<size_t> userEnteredIntervalValueSec;

    do
    {
        const std::vector<std::string> argumentsToParse{collectArguments(argc, argv)};

        if (argumentsToParse.empty())
        {
            result = {.interval = Settings::defaultIntervalValueSec};
            break;
        }

        Settings::Parameters parameters;

        for (const auto& argument : argumentsToParse)
        {
            bool optionFound{checkIfValidOption(argument, "--reverse", parameters.reverse)};

            if (errorCode)
            {
                break;
            }

            if (optionFound)
            {
                continue;
            }

            optionFound = checkIfValidOption(argument, "--autoreset", parameters.autoReset);

            if (errorCode)
            {
                break;
            }

            if (optionFound)
            {
                continue;
            }

            checkIfValidBoundedNumericValue(argument, Settings::minIntervalValueSec, Settings::maxIntervalValueSec,
                                            userEnteredIntervalValueSec);

            if (errorCode)
            {
                break;
            }

            if (userEnteredIntervalValueSec)
            {
                parameters.interval = *userEnteredIntervalValueSec;
            }
        }

        result = !errorCode ? std::expected<Settings::Parameters, std::string>{parameters}
                            : std::unexpected{getErrorMessage(*errorCode)};
    } while (false);

    return result;
}
