#include <cmath>
#include <iostream>

#include "gcdutils.h"

#define ERROR 1

/* How to use:
   - compile the kernel module "division" by running "make" from within its directory (Modules/Division)
   - load the kernel module by running "sudo insmod division.ko"
   - check that the module is loaded by running "lsmod | grep -i division"
   - run this application: sudo ./GreatestCommonDivisor [integer1] [integer2] # g.c.d. is retrieved
     (e.g. sudo ./GreatestCommonDivisor 6 10 # g.c.d. is 2)
 */

int main(int argc, char** argv)
{
    int retVal{0};
    const ParsedArguments parsedArguments{GcdUtils::parseArguments(argc, argv)};

    system("clear");

    if (parsedArguments.has_value())
    {
        try
        {
            const int gcd{GcdUtils::retrieveGreatestCommonDivisor(parsedArguments->first, parsedArguments->second)};
            std::cout << "Greatest common divisor of " << parsedArguments->first << " and " << parsedArguments->second
                      << " is: " << gcd << "\n";

            try
            {
                const int firstUncommonDivisor{GcdUtils::retrieveQuotient(parsedArguments->first, gcd)};
                const int secondUncommonDivisor{GcdUtils::retrieveQuotient(parsedArguments->second, gcd)};

                std::cout << "Uncommon divisor for first number is: " << firstUncommonDivisor << "\n";
                std::cout << "Uncommon divisor for second number is: " << secondUncommonDivisor << "\n";
            }
            catch (...)
            {
                // there should be no exception here as the gcd should be different from 0 (just for safety purposes as
                // the retrieveQuotient() is a throwing function
            }

            if (std::abs(gcd) == 1)
            {
                std::cout << "Numbers " << parsedArguments->first << " and " << parsedArguments->second
                          << " are prime with each other!\n";
            }
        }
        catch (const std::runtime_error& err)
        {
            retVal = ERROR;
            std::cerr << err.what() << "\n";
        }
    }
    else
    {
        std::cerr << "Invalid argument(s), should be integer! Please try again.\n";
    }

    return retVal;
}
