#include <cassert>

#include "divisionmodulename.h"
#include "gcdloader_impl.h"
#include "utils.h"

namespace GCD::Loader
{
namespace
{
std::expected<bool, std::string> isDivisionModuleInitiallyLoaded;
} // namespace
} // namespace GCD::Loader

std::optional<std::string> GCD::Loader::loadRequiredKernelModulesImpl(
    std::function<std::optional<std::string>(const std::string_view)> loadKernelModuleCallback)
{
    std::optional<std::string> errorMessage;

    do
    {
        if (!loadKernelModuleCallback)
        {
            assert(false);
            errorMessage = "No loading function provided for kernel module " + std::string{divisionModuleName} + "!";
            break;
        }

        isDivisionModuleInitiallyLoaded = Utilities::isKernelModuleLoaded(divisionModuleName);

        if (!isDivisionModuleInitiallyLoaded.has_value())
        {
            errorMessage = isDivisionModuleInitiallyLoaded.error();
            break;
        }

        if (!(*isDivisionModuleInitiallyLoaded))
        {
            const std::optional<std::string> loadErrorMessage{loadKernelModuleCallback(divisionModuleName)};

            if (loadErrorMessage.has_value())
            {
                errorMessage = *loadErrorMessage;
                break;
            }
        }
    } while (false);

    return errorMessage;
}

// for simplicity error logging will be skipped when unloading modules
void GCD::Loader::unloadRequiredKernelModulesImpl()
{
    if (isDivisionModuleInitiallyLoaded.has_value() && !(*isDivisionModuleInitiallyLoaded))
    {
        (void)Utilities::unloadKernelModule(divisionModuleName);
    }
}
