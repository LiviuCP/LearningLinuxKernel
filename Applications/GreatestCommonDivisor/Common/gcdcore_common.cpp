#include "gcdcore_common.h"
#include "gcdcore.h"

std::expected<int, std::string> GCD::Core::retrieveGreatestCommonDivisor(int first, int second)
{
    std::expected<int, std::string> result;

    do
    {
        if (first == 0 && second == 0)
        {
            result = std::unexpected{"Cannot retrieve greatest common divisor when both numbers are 0!"};
            break;
        }

        if (second == 0)
        {
            std::swap(first, second);
        }

        const std::expected<int, std::string> remainderResult{retrieveRemainder(first, second)};

        if (!remainderResult)
        {
            result = std::unexpected{remainderResult.error()};
            break;
        }

        result = remainderResult == 0 ? second : retrieveGreatestCommonDivisor(second, *remainderResult);
    } while (false);

    return result;
}
