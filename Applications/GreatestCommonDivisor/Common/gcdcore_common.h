#pragma once

#include <expected>
#include <string>

namespace GCD::Core
{
std::expected<int, std::string> retrieveGreatestCommonDivisor(int first, int second);
}
