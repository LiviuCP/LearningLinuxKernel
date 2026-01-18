#include <filesystem>
#include <optional>
#include <stdexcept>

#include "gcdloader.h"
#include "utils.h"

namespace GCD::Loader
{
namespace
{
void loadKernelModule(const std::optional<std::filesystem::path>& modulePath)
{
    std::string moduleName;

    if (modulePath.has_value())
    {
        moduleName = std::string{modulePath->filename().stem()};
        Utilities::loadKernelModule(*modulePath);
    }

    if (!Utilities::isKernelModuleLoaded(moduleName))
    {
        throw std::runtime_error{"Could not load kernel module " + moduleName +
                                 "!\nPlease check that the module file exists in "
                                 "consolidated output and try again by running the app with sudo."};
    }
}

std::optional<std::filesystem::path> getModulePath(const std::string_view moduleName)
{
    const std::filesystem::path applicationPath{Utilities::getApplicationPath()};

    std::filesystem::path modulePath{applicationPath.parent_path().parent_path().parent_path()};
    modulePath /= Utilities::getModulesDirRelativePath();
    modulePath /= moduleName;
    modulePath += Utilities::getModuleFileExtension();

    return std::filesystem::is_regular_file(modulePath) ? std::optional{modulePath} : std::nullopt;
}

} // namespace
} // namespace GCD::Loader

void GCD::Loader::loadKernelModuleDivision()
{
    loadKernelModule(getModulePath(GCD::Loader::getDivisionModuleName()));
}

void GCD::Loader::loadKernelModuleUtilities()
{
    loadKernelModule(getModulePath(GCD::Loader::getUtilitiesModuleName()));
}

bool GCD::Loader::isKernelModuleDivisionLoaded()
{
    return Utilities::isKernelModuleLoaded(GCD::Loader::getDivisionModuleName());
}

bool GCD::Loader::isKernelModuleUtilitiesLoaded()
{
    return Utilities::isKernelModuleLoaded(GCD::Loader::getUtilitiesModuleName());
}
