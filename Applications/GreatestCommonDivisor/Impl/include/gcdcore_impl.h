#pragma once

#include <expected>
#include <string>

namespace GCD::Core
{
std::expected<int, std::string> retrieveQuotientImpl(int divided, int divider);
std::expected<int, std::string> retrieveRemainderImpl(int divided, int divider);
} // namespace GCD::Core
