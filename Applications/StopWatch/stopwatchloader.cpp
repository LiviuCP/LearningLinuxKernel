#include <cassert>
#include <filesystem>
#include <functional>
#include <string_view>

#include "stopwatchloader.h"
#include "utils.h"

namespace StopWatch::Loader
{
namespace
{
constexpr std::string_view timePeriodSignalerModuleName{"time_period_signaler"};

std::expected<bool, std::string> isTimePeriodSignalerModuleInitiallyLoaded;

std::filesystem::path getModulePath(const std::string_view moduleName)
{
    const std::filesystem::path applicationPath{Utilities::getApplicationPath()};

    std::filesystem::path modulePath{applicationPath.parent_path().parent_path().parent_path()};
    modulePath /= Utilities::getModulesDirRelativePath();
    modulePath /= moduleName;
    modulePath += Utilities::getModuleFileExtension();

    return modulePath;
}

std::optional<std::string> loadKernelModule(const std::string_view moduleName)
{
    return Utilities::loadKernelModule(getModulePath(moduleName));
}

std::optional<std::string> loadRequiredKernelModulesImpl(
    std::function<std::optional<std::string>(const std::string_view)> loadKernelModuleCallback)
{
    std::optional<std::string> errorMessage;

    do
    {
        if (!loadKernelModuleCallback)
        {
            assert(false);
            errorMessage =
                "No loading function provided for kernel module " + std::string{timePeriodSignalerModuleName} + "!";
            break;
        }

        isTimePeriodSignalerModuleInitiallyLoaded = Utilities::isKernelModuleLoaded(timePeriodSignalerModuleName);

        if (!isTimePeriodSignalerModuleInitiallyLoaded.has_value())
        {
            errorMessage = isTimePeriodSignalerModuleInitiallyLoaded.error();
            break;
        }

        if (!(*isTimePeriodSignalerModuleInitiallyLoaded))
        {
            const std::optional<std::string> loadErrorMessage{loadKernelModuleCallback(timePeriodSignalerModuleName)};

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
static void unloadRequiredKernelModulesImpl()
{
    if (isTimePeriodSignalerModuleInitiallyLoaded.has_value() && !(*isTimePeriodSignalerModuleInitiallyLoaded))
    {
        (void)Utilities::unloadKernelModule(timePeriodSignalerModuleName);
    }
}
} // namespace
} // namespace StopWatch::Loader

std::optional<std::string> StopWatch::Loader::loadRequiredKernelModules()
{
    return loadRequiredKernelModulesImpl(loadKernelModule);
}

void StopWatch::Loader::unloadRequiredKernelModules()
{
    unloadRequiredKernelModulesImpl();
}
