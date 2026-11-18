#pragma once

#include <expected>
#include <optional>
#include <string>

namespace StopWatch::Settings
{
struct Parameters;
}

namespace StopWatch::Core
{
std::optional<std::string> start(void (*printCallback)(size_t), void (*stopCallback)(),
                                 const StopWatch::Settings::Parameters& params);
std::expected<bool, std::string> getStatus();
} // namespace StopWatch::Core
