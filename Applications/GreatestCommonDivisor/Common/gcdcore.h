#pragma once

#include <expected>
#include <string>

namespace GCD::Core
{
std::expected<int, std::string> retrieveGreatestCommonDivisor(int first, int second);
std::expected<int, std::string> retrieveQuotient(int divided, int divider);
std::expected<int, std::string> retrieveRemainder(int divided, int divider);
} // namespace GCD::Core
