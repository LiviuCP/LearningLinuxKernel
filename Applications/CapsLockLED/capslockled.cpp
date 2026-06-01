#include <fcntl.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

#include "capslockled.h"

#define ON true
#define OFF false

#define CAPS_LOCK_LED_ON 0x04
#define ALL_LEDS_OFF 0x00

static constexpr std::string_view consoleFile{"/dev/console"};

static constexpr std::chrono::milliseconds oneSecond{1000};
static constexpr std::chrono::milliseconds dotTurnedOnDuration{oneSecond / 6};
static constexpr std::chrono::milliseconds dotTurnedOffDuration{oneSecond / 9};
static constexpr std::chrono::milliseconds lineTurnedOnDuration{oneSecond / 2};
static constexpr std::chrono::milliseconds lineTurnedOffDuration{oneSecond / 3};
static constexpr std::chrono::milliseconds idleDuration{oneSecond};

namespace CapsLockLED
{
namespace
{
bool ioctlIsCapsLockLEDOn()
{
    bool isOn{false};

    const int fd{open(consoleFile.data(), O_RDONLY)};

    if (fd > 0)
    {
        unsigned char onFlags;
        const long retVal{ioctl(fd, KDGETLED, &onFlags)};

        if (retVal == 0)
        {
            isOn = onFlags & CAPS_LOCK_LED_ON;
        }
        else
        {
            std::cerr << "Cannot get the CAPS-LOCK LED value via ioctl!\n";
        }

        close(fd);
    }
    else
    {
        std::cerr << "Cannot open the " << consoleFile << " file for reading!\n";
    }

    return isOn;
}

void ioctlSetCapsLockLEDValue(bool on)
{
    const int fd{open(consoleFile.data(), O_WRONLY)};

    if (fd > 0)
    {
        const long retVal{ioctl(fd, KDSETLED, on ? CAPS_LOCK_LED_ON : ALL_LEDS_OFF)};

        if (retVal != 0)
        {
            std::cerr << "Cannot set the CAPS-LOCK LED value via ioctl!\n";
        }

        close(fd);
    }
    else
    {
        std::cerr << "Cannot open the " << consoleFile << " file for writing!\n";
    }
}

void signalDot()
{
    ioctlSetCapsLockLEDValue(ON);
    std::this_thread::sleep_for(dotTurnedOnDuration);
    ioctlSetCapsLockLEDValue(OFF);
    std::this_thread::sleep_for(dotTurnedOffDuration);
}

void signalLine()
{
    ioctlSetCapsLockLEDValue(ON);
    std::this_thread::sleep_for(lineTurnedOnDuration);
    ioctlSetCapsLockLEDValue(OFF);
    std::this_thread::sleep_for(lineTurnedOffDuration);
}

void signalLetterM()
{
    signalLine();
    signalLine();
    std::clog << "M.";
}

void signalLetterO()
{
    signalLine();
    signalLine();
    signalLine();
    std::clog << "O.";
}

void signalLetterS()
{
    signalDot();
    signalDot();
    signalDot();
    std::clog << "S.";
}

} // namespace
} // namespace CapsLockLED

void CapsLockLED::setLEDValue(bool on)
{
    const bool isOn{ioctlIsCapsLockLEDOn()};

    if (isOn != on)
    {
        std::cout << "CAPS-LOCK LED is: " << (isOn ? "ON" : "OFF") << "\n";
        ioctlSetCapsLockLEDValue(on);
        std::cout << "CAPS-LOCK LED set to: " << (on ? "ON" : "OFF") << "\n";
    }
    else
    {
        std::cout << "CAPS-LOCK LED is already " << (on ? "ON" : "OFF") << "!\n";
    }
}

void CapsLockLED::sendSOSSignal(size_t count)
{
    assert(count >= minSOSCount() && count <= maxSOSCount());

    if (count == maxSOSCount())
    {
        std::clog << "Number of SOS signals capped to maximum allowed value: " << maxSOSCount() << "\n";
    }
    else if (count == minSOSCount())
    {
        std::clog << "Minimum number of SOS signals to be sent: " << minSOSCount() << "\n";
    }

    ioctlSetCapsLockLEDValue(OFF);
    std::this_thread::sleep_for(idleDuration);

    const size_t countSent{count};

    while (count > 0)
    {
        signalLetterS();
        std::this_thread::sleep_for(
            lineTurnedOffDuration); // keep dotTurnedOffDuration + lineTurnedOffDuration break between dots and lines

        signalLetterO();
        std::this_thread::sleep_for(dotTurnedOffDuration); // keep same break between lines and dots

        signalLetterS();
        std::clog << "\n";

        std::this_thread::sleep_for(idleDuration);

        --count;
    }

    std::clog << countSent << " mayday signal(s) sent.\n";
}

void CapsLockLED::sendSMSSignal()
{
    ioctlSetCapsLockLEDValue(OFF);
    std::this_thread::sleep_for(idleDuration);

    signalLetterS();
    std::this_thread::sleep_for(
        lineTurnedOffDuration); // keep dotTurnedOffDuration + lineTurnedOffDuration break between dots and lines

    signalLetterM();
    std::this_thread::sleep_for(dotTurnedOffDuration); // keep same break between lines and dots

    signalLetterS();
    std::clog << "\n";

    std::this_thread::sleep_for(idleDuration);
}
