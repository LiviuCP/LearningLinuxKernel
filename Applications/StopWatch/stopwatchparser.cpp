#include <algorithm>
#include <cassert>
#include <optional>
#include <vector>

#include "stopwatchparser.h"

namespace StopWatch::Parser
{
namespace
{
// excluding the application name
constexpr size_t maxArgumentsCount{3};
constexpr size_t minSecondsCount{10};
constexpr size_t maxSecondsCount{3600};

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
} // namespace
} // namespace StopWatch::Parser

std::expected<StopWatch::Settings::Parameters, std::string> StopWatch::Parser::parse(int argc, char** argv)
{
    std::expected<StopWatch::Settings::Parameters, std::string> expectedValue;
    std::optional<size_t> seconds;

    do
    {
        const std::vector<std::string> argumentsToParse{collectArguments(argc, argv)};

        if (argumentsToParse.empty())
        {
            expectedValue = {.interval = maxSecondsCount};
            break;
        }

        StopWatch::Settings::Parameters result;

        bool hasInvalidArgument{false};
        bool hasDuplicateArgument{false};
        bool hasOutOfBoundsArgument{false};

        for (const auto& argument : argumentsToParse)
        {
            if (argument == "--reverse")
            {
                if (result.reverse)
                {
                    hasDuplicateArgument = true;
                    break;
                }

                result.reverse = true;
                continue;
            }

            if (argument == "--autoreset")
            {
                if (result.autoReset)
                {
                    hasDuplicateArgument = true;
                    break;
                }

                result.autoReset = true;
                continue;
            }

            try
            {
                if (std::ranges::any_of(argument, [](char c) { return !isdigit(c); }))
                {
                    hasInvalidArgument = true;
                    break;
                }

                const size_t secondsCount{std::stoul(argument)};

                if (seconds)
                {
                    hasDuplicateArgument = true;
                    break;
                }

                if (secondsCount < minSecondsCount || secondsCount > maxSecondsCount)
                {
                    hasOutOfBoundsArgument = true;
                    break;
                }

                seconds = secondsCount;
            }
            catch (...)
            {
                hasInvalidArgument = true;
                break;
            }
        }

        if (hasInvalidArgument)
        {
            expectedValue = std::unexpected<std::string>{"At least one argument is invalid!"};
            break;
        }

        if (hasDuplicateArgument)
        {
            expectedValue = std::unexpected<std::string>{"At least one argument is duplicate!"};
            break;
        }

        if (hasOutOfBoundsArgument)
        {
            expectedValue = std::unexpected<std::string>{
                "The provided time interval is out of bounds!\nShould be within range: " +
                std::to_string(minSecondsCount) + " - " + std::to_string(maxSecondsCount) + " seconds"};
            break;
        }

        result.interval = seconds ? *seconds : maxSecondsCount;
        expectedValue = result;
    } while (false);

    return expectedValue;
}
