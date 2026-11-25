#pragma once

#include <expected>
#include <string>

#include "stopwatchsettings.h"

namespace StopWatch::Parser
{
using ParsingResult = std::expected<StopWatch::Settings::Parameters, std::string>;
using ParsingError = std::unexpected<std::string>;

ParsingResult parse(int argc, char** argv);
} // namespace StopWatch::Parser
