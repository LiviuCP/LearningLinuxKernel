#include <cmath>
#include <iostream>

#include "gcdcore.h"
#include "gcdloader.h"
#include "gcdparser.h"
#include "gcdutils.h"

#define SUCCESS 0
#define ERROR 1

/* How to use:
    - build the whole LearningLinuxKernel project
    - run this application: sudo ./GreatestCommonDivisor [integer1] [integer2] # g.c.d. is retrieved
     (e.g. sudo ./GreatestCommonDivisor 6 10 # g.c.d. is 2)

    Note: the application requests loading of the Division kernel module so no action is required from user side other
   that running the app with "sudo" and the required arguments (divided and divider).
 */

int main(int argc, char** argv)
{
    int retVal{SUCCESS};
    const ParsedArguments parsedArguments{GCD::Parser::parseArguments(argc, argv)};

    if (parsedArguments.has_value())
    {
        try
        {
            GCD::Utils::clearScreen();

            const bool isDivisionModuleInitiallyLoaded{GCD::Loader::isKernelModuleDivisionLoaded()};

            if (!isDivisionModuleInitiallyLoaded)
            {
                GCD::Loader::loadKernelModuleDivision();
            }

            const int gcd{GCD::Core::retrieveGreatestCommonDivisor(parsedArguments->first, parsedArguments->second)};

            std::cout << "Greatest common divisor of " << parsedArguments->first << " and " << parsedArguments->second
                      << " is: " << gcd << "\n";

            try
            {
                const int firstUncommonDivisor{GCD::Core::retrieveQuotient(parsedArguments->first, gcd)};
                const int secondUncommonDivisor{GCD::Core::retrieveQuotient(parsedArguments->second, gcd)};

                std::cout << "Uncommon divisor for first number is: " << firstUncommonDivisor << "\n";
                std::cout << "Uncommon divisor for second number is: " << secondUncommonDivisor << "\n";
            }
            catch (...)
            {
                // there should be no exception here as the gcd should be different from 0 (just for safety purposes as
                // the retrieveQuotient() is a throwing function
                throw std::runtime_error{"An unknown error occurred!"};
            }

            if (std::abs(gcd) == 1)
            {
                std::cout << "Numbers " << parsedArguments->first << " and " << parsedArguments->second
                          << " are prime with each other!\n";
            }

            if (!isDivisionModuleInitiallyLoaded)
            {
                GCD::Loader::unloadKernelModuleDivision();
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
