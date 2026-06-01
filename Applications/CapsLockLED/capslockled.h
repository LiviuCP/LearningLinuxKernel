#pragma once

#include <cstddef>

namespace CapsLockLED
{
constexpr size_t minSOSCount()
{
    return 1;
}

constexpr size_t maxSOSCount()
{
    return 10;
}

void setLEDValue(bool on);
void sendSOSSignal(size_t count);
void sendSMSSignal();
} // namespace CapsLockLED
