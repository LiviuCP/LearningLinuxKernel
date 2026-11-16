#pragma once

#include <expected>
#include <string>

namespace StopWatch::Parser
{
struct Result
{
    size_t seconds;
    bool reverse{false};
    bool autoReset{false};
};

std::expected<Result, std::string> parse(int argc, char** argv);
} // namespace StopWatch::Parser
