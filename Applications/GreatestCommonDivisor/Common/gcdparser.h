#pragma once

#include <optional>

using ParsedArguments = std::optional<std::pair<int, int>>;

namespace GCD::Parser
{
ParsedArguments parseArguments(int argc, char** argv);
} // namespace GCD::Parser
