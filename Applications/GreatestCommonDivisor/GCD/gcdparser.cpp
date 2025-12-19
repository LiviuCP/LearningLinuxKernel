#include <algorithm>
#include <cassert>
#include <cctype>
#include <string_view>
#include <string>

#include "gcdparser.h"

namespace GCD::Parser
{
namespace
{
static constexpr std::string_view divisionModuleRelativePath{"KernelModules/ConsolidatedOutput/division.ko"};

bool isValidIntArg(const char* arg)
{
    bool isValidArg{false};

    assert(arg);
    const std::string argStr{arg ? arg : ""};

    if (!argStr.empty())
    {
        const bool isFirstCharDigit{static_cast<bool>(std::isdigit(argStr[0]))};
        const bool isFirstCharMinus{argStr[0] == '-'};
        const bool areRemainingCharsDigits{
            std::all_of(argStr.cbegin() + 1, argStr.cend(), [](char ch) { return std::isdigit(ch); })};

        isValidArg =
            argStr.size() > 1 ? (isFirstCharDigit || isFirstCharMinus) && areRemainingCharsDigits : isFirstCharDigit;
    }

    return isValidArg;
}
} // namespace
} // namespace GCD::Parser

ParsedArguments GCD::Parser::parseArguments(int argc, char** argv)
{
    bool areValidArguments{argc >= 1 && argv && argv[0]};
    assert(areValidArguments);

    ParsedArguments result;

    // default values in case not entered by user
    int first{0}, second{1};

    if (areValidArguments && argc > 1)
    {
        areValidArguments = isValidIntArg(argv[1]);

        if (areValidArguments)
        {
            first = atoi(argv[1]);
        }
    }

    if (areValidArguments && argc > 2)
    {
        areValidArguments = isValidIntArg(argv[2]);

        if (areValidArguments)
        {
            second = atoi(argv[2]);
        }
    }

    if (areValidArguments)
    {
        result = {first, second};
    }

    return result;
}
