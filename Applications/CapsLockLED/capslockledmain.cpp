#include <algorithm>
#include <iostream>

#include "capslockled.h"

/* Small application that performs various tasks by using the CAPS-LOCK LED:
   - turn on the CAPS-LOCK LED
   - turn off the CAPS-LOCK LED
   - send one or multiple S.O.S. signals using Morse code
   - send one S.M.S. signal using Morse code
   Caveat! Ensure CAPS-LOCK is turned off before running the app.
 */

void displayUsageMessage()
{
    std::cerr << "Usage: \n\n";
    std::cerr << "sudo ./CapsLockLED -on: turn on the LED\n";
    std::cerr << "sudo ./CapsLockLED -off: turn off the LED\n";
    std::cerr << "sudo ./CapsLockLED -sos [count]: send one or more (maximum " << CapsLockLED::maxSOSCount()
              << ") S.O.S. signal(s)\n";
    std::cerr << "sudo ./CapsLockLED -sms: send one S.M.S. signal\n";
}

size_t computeSOSCount(const std::string& timesCountStr)
{
    const bool isValidCountString{std::ranges::all_of(timesCountStr, [](char c) { return isdigit(c); })};
    return isValidCountString ? std::clamp<size_t>(static_cast<size_t>(std::stoi(timesCountStr)),
                                                   CapsLockLED::minSOSCount(), CapsLockLED::maxSOSCount())
                              : CapsLockLED::minSOSCount();
}

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        const std::string operation{argv[1]};

        if (operation == "-on" || operation == "-off")
        {
            CapsLockLED::setLEDValue(operation == "-on");
        }
        else if (operation == "-sos")
        {
            CapsLockLED::sendSOSSignal(argc > 2 ? computeSOSCount({argv[2]}) : CapsLockLED::minSOSCount());
        }
        else if (operation == "-sms")
        {
            CapsLockLED::sendSMSSignal();
        }
        else
        {
            std::cerr << "Invalid operation! ";
            displayUsageMessage();
        }
    }
    else
    {
        std::cerr << "No arguments entered! ";
        displayUsageMessage();
    }

    return 0;
}
