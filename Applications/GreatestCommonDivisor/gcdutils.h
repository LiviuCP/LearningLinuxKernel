#pragma once

#include <optional>

using ParsedArguments = std::optional<std::pair<int, int>>;

namespace GcdUtils
{
ParsedArguments parseArguments(int argc, char** argv);

int retrieveQuotient(int divided, int divider);
int retrieveRemainder(int divided, int divider);
int retrieveGreatestCommonDivisor(int first, int second);
} // namespace GcdUtils
