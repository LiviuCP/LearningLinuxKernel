#include "gcdloader.h"
#include "gcdloader_impl.h"
#include "utils.h"

namespace GCD::Loader
{
namespace
{
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

} // namespace
} // namespace GCD::Loader

std::optional<std::string> GCD::Loader::loadRequiredKernelModules()
{
    return loadRequiredKernelModulesImpl(loadKernelModule);
}

void GCD::Loader::unloadRequiredKernelModules()
{
    unloadRequiredKernelModulesImpl();
}
