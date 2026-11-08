#pragma once

#include <expected>
#include <optional>
#include <string>

namespace StopWatch::Core
{
std::optional<std::string> start();
std::expected<bool, std::string> running();
size_t getMilliSecondsCount();
} // namespace StopWatch::Core
