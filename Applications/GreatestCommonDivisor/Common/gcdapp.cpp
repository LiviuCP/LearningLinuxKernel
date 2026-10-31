#include <cmath>
#include <iostream>

#include "gcdapp.h"
#include "gcdcore.h"
#include "gcdcore_common.h"
#include "gcdloader.h"
#include "gcdparser.h"
#include "utils.h"

#define SUCCESS 0
#define ERROR 1

/* How to use:
    - build the whole LearningLinuxKernel project
    - run the application by choosing either of the executables:
        sudo ./[Ioctl/Procfs/Sysfs]GreatestCommonDivisor [integer1] [integer2] # g.c.d. is retrieved
     (e.g. sudo ./IoctlGreatestCommonDivisor 6 10 # g.c.d. is 2)

    Notes:
    - the application requests loading of the required kernel module(s) so no action is required from user side other
   than running the app with "sudo" and providing the required arguments (divided and divider)
    - the kernel module(s) will be left in the same state that when the application got opened (loaded/unloaded) unless
   an expected error occured when loading/unloading one of the modules
 */

int GCD::App::run(int argc, char** argv)
{
    int retVal{ERROR};
    const ParsedArguments parsedArguments{GCD::Parser::parseArguments(argc, argv)};

    do
    {
        if (!parsedArguments.has_value())
        {
            std::cerr << "Invalid argument(s), should be integer! Please try again.\n";
            break;
        }

        Utilities::clearScreen();

        const std::optional<std::string> modulesLoadingErrorMessage{GCD::Loader::loadRequiredKernelModules()};

        if (modulesLoadingErrorMessage.has_value())
        {
            std::cerr << *modulesLoadingErrorMessage << "\n";
        }

        const std::expected<int, std::string> gcd{
            GCD::Core::retrieveGreatestCommonDivisor(parsedArguments->first, parsedArguments->second)};

        if (!gcd.has_value())
        {
            std::cerr << gcd.error() << "\n";
            break;
        }

        const std::expected<int, std::string> firstUncommonDivisor{
            GCD::Core::retrieveQuotient(parsedArguments->first, *gcd)};

        if (!firstUncommonDivisor.has_value())
        {
            std::cerr << firstUncommonDivisor.error() << "\n";
            break;
        }

        const std::expected<int, std::string> secondUncommonDivisor{
            GCD::Core::retrieveQuotient(parsedArguments->second, *gcd)};

        if (!secondUncommonDivisor.has_value())
        {
            std::cerr << secondUncommonDivisor.error() << "\n";
            break;
        }

        retVal = SUCCESS;

        std::cout << "Greatest common divisor of " << parsedArguments->first << " and " << parsedArguments->second
                  << " is: " << *gcd << "\n";

        std::cout << "Uncommon divisor for first number is: " << *firstUncommonDivisor << "\n";
        std::cout << "Uncommon divisor for second number is: " << *secondUncommonDivisor << "\n";

        if (std::abs(*gcd) == 1)
        {
            std::cout << "Numbers " << parsedArguments->first << " and " << parsedArguments->second
                      << " are prime with each other!\n";
        }
    } while (false);

    GCD::Loader::unloadRequiredKernelModules();

    return retVal;
}
