#pragma once

#include <cstdlib>

namespace StopWatch::Settings
{

inline constexpr size_t tickValueMs{100};
inline constexpr size_t minIntervalValueSec{10};
inline constexpr size_t maxIntervalValueSec{86400};
inline constexpr size_t defaultIntervalValueSec{3600};

struct Parameters
{
    // running interval (in seconds) before stopwatch gets stopped or autoreset
    size_t interval{defaultIntervalValueSec};

    bool reverse{false};
    bool autoReset{false};
};
} // namespace StopWatch::Settings
