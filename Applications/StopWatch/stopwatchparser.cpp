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
constexpr size_t maxSecondsCount{3600};
} // namespace
} // namespace StopWatch::Parser

std::expected<StopWatch::Parser::Result, std::string> StopWatch::Parser::parse(int argc, char** argv)
{
    std::expected<StopWatch::Parser::Result, std::string> expectedValue;
    StopWatch::Parser::Result result;
    std::optional<size_t> seconds;

    do
    {
        assert(argv && argc >= 1);

        if (!argv || argc <= 1)
        {
            break;
        }

        const size_t argumentsToParseCount{std::min<size_t>(static_cast<size_t>(argc - 1), maxArgumentsCount)};

        std::vector<std::string> argumentsToParse;

        for (size_t index{1}; index <= argumentsToParseCount; ++index)
        {
            assert(argv[index]);

            if (argv[index])
            {
                argumentsToParse.emplace_back(std::string{argv[index]});
            }
        }

        bool duplicate{false};
        bool invalid{false};
        bool maxSecondsValueExceeded{false};

        for (const auto& argument : argumentsToParse)
        {
            if (argument == "--reverse")
            {
                if (result.reverse)
                {
                    duplicate = true;
                    break;
                }

                result.reverse = true;
                continue;
            }

            if (argument == "--autoreset")
            {
                if (result.autoReset)
                {
                    duplicate = true;
                    break;
                }

                result.autoReset = true;
                continue;
            }

            try
            {
                const size_t secondsCount{std::stoul(argument)};

                if (seconds)
                {
                    duplicate = true;
                    break;
                }

                if (secondsCount > maxSecondsCount)
                {
                    maxSecondsValueExceeded = true;
                    break;
                }

                seconds = secondsCount;
            }
            catch (...)
            {
                invalid = true;
                break;
            }
        }

        if (duplicate)
        {
            expectedValue = std::unexpected<std::string>{"Duplicate arguments!"};
            break;
        }

        if (invalid)
        {
            expectedValue = std::unexpected<std::string>{"At least one argument is invalid!"};
            break;
        }

        if (maxSecondsValueExceeded)
        {
            expectedValue = std::unexpected<std::string>{"The maximum allowed time interval has been exceeded!"};
            break;
        }

        result.seconds = seconds ? *seconds : maxSecondsCount;
        expectedValue = result;
    } while (false);

    return expectedValue;
}
