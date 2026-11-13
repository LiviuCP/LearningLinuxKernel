#pragma once

#include <expected>
#include <optional>
#include <string>

namespace StopWatch::Core
{
std::optional<std::string> start(void (*printCallback)(size_t), void (*stopCallback)());
std::expected<bool, std::string> getStatus();
} // namespace StopWatch::Core
