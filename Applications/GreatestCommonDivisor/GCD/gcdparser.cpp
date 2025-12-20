#include <cassert>
#include <cstdlib>

#include "gcdparser.h"
#include "gcdutils.h"

ParsedArguments GCD::Parser::parseArguments(int argc, char** argv)
{
    bool areValidArguments{argc >= 1 && argv && argv[0]};
    assert(areValidArguments);

    ParsedArguments result;

    // default values in case not entered by user
    int first{0}, second{1};

    if (areValidArguments && argc > 1)
    {
        areValidArguments = GCD::Utils::isValidInteger(argv[1]);

        if (areValidArguments)
        {
            first = atoi(argv[1]);
        }
    }

    if (areValidArguments && argc > 2)
    {
        areValidArguments = GCD::Utils::isValidInteger(argv[2]);

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
