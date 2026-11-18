#pragma once

#include <expected>
#include <string>

#include "stopwatchsettings.h"

namespace StopWatch::Parser
{
std::expected<StopWatch::Settings::Parameters, std::string> parse(int argc, char** argv);
} // namespace StopWatch::Parser
