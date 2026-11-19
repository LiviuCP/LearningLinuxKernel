#pragma once

#include <cstdlib>

namespace StopWatch::Settings
{
struct Parameters
{
    // running interval (in seconds) before stopwatch gets stopped or autoreset
    size_t interval;

    bool reverse{false};
    bool autoReset{false};
};
} // namespace StopWatch::Settings
