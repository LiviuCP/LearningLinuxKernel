#include <cmath>
#include <iostream>

#include "divisionmodulename.h"
#include "gcdapp.h"
#include "gcdcore.h"
#include "gcdloader.h"
#include "gcdparser.h"
#include "utils.h"

#define SUCCESS 0
#define ERROR 1

/* How to use:
    - build the whole LearningLinuxKernel project
    - run this application: sudo ./ProcfsGreatestCommonDivisor [integer1] [integer2] # g.c.d. is retrieved
     (e.g. sudo ./ProcfsGreatestCommonDivisor 6 10 # g.c.d. is 2)

    Notes:
    - the application requests loading of the ProcfsDivision and KernelUtilities kernel modules so no action is required
   from user side other that running the app with "sudo" and providing the required arguments (divided and divider)
    - the kernel modules will be left in the same state that they had when the application got opened: if a module is
   open it will be left open, same for the closed state
    - the KernelUtilities module is used by ProcfsDivision so it should be loaded beforehand; the two modules should be
   unloaded in reverse order
    - same usage and notes for the sysfs implementation, only the app executable and division kernel module are
   different: SysfsGreatestCommonDivisor and SysfsDivision
 */

static constexpr std::string_view utilitiesModuleName{"kernel_utilities"};

int GCD::App::run(int argc, char** argv)
{
    int retVal{SUCCESS};
    const ParsedArguments parsedArguments{GCD::Parser::parseArguments(argc, argv)};

    if (parsedArguments.has_value())
    {
        try
        {
            Utilities::clearScreen();

            const bool isUtilitiesModuleInitiallyLoaded{Utilities::isKernelModuleLoaded(utilitiesModuleName)};

            if (!isUtilitiesModuleInitiallyLoaded)
            {
                GCD::Loader::loadKernelModule(utilitiesModuleName);
            }

            const bool isDivisionModuleInitiallyLoaded{Utilities::isKernelModuleLoaded(divisionModuleName)};

            if (!isDivisionModuleInitiallyLoaded)
            {
                GCD::Loader::loadKernelModule(divisionModuleName);
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
                Utilities::unloadKernelModule(divisionModuleName);
            }

            if (!isUtilitiesModuleInitiallyLoaded)
            {
                Utilities::unloadKernelModule(utilitiesModuleName);
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
