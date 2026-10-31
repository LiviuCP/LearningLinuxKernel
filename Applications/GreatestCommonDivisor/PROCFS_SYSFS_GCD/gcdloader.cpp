#include <expected>

#include "divisionmodulename.h"
#include "gcdloader.h"
#include "gcdloader_common.h"
#include "utils.h"

namespace GCD::Loader
{
namespace
{
constexpr std::string_view utilitiesModuleName{"kernel_utilities"};

std::expected<bool, std::string> isUtilitiesModuleInitiallyLoaded;
std::expected<bool, std::string> isDivisionModuleInitiallyLoaded;
} // namespace
} // namespace GCD::Loader

std::optional<std::string> GCD::Loader::loadRequiredKernelModules()
{
    std::optional<std::string> errorMessage;

    do
    {
        isUtilitiesModuleInitiallyLoaded = Utilities::isKernelModuleLoaded(utilitiesModuleName);

        if (!isUtilitiesModuleInitiallyLoaded.has_value())
        {
            errorMessage = isUtilitiesModuleInitiallyLoaded.error();
            break;
        }

        if (!(*isUtilitiesModuleInitiallyLoaded))
        {
            const std::optional<std::string> loadErrorMessage{GCD::Loader::loadKernelModule(utilitiesModuleName)};

            if (loadErrorMessage.has_value())
            {
                errorMessage = *loadErrorMessage;
                break;
            }
        }

        isDivisionModuleInitiallyLoaded = Utilities::isKernelModuleLoaded(divisionModuleName);

        if (!isDivisionModuleInitiallyLoaded.has_value())
        {
            errorMessage = isDivisionModuleInitiallyLoaded.error();
            break;
        }

        if (!(*isDivisionModuleInitiallyLoaded))
        {
            const std::optional<std::string> loadErrorMessage{GCD::Loader::loadKernelModule(divisionModuleName)};

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
void GCD::Loader::unloadRequiredKernelModules()
{
    if (isDivisionModuleInitiallyLoaded.has_value() && !(*isDivisionModuleInitiallyLoaded))
    {
        (void)Utilities::unloadKernelModule(divisionModuleName);
    }

    if (isUtilitiesModuleInitiallyLoaded.has_value() && !(*isUtilitiesModuleInitiallyLoaded))
    {
        (void)Utilities::unloadKernelModule(utilitiesModuleName);
    }
}
