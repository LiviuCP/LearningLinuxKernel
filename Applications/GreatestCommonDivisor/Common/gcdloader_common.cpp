#include <filesystem>
#include <optional>

#include "gcdloader_common.h"
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
} // namespace
} // namespace GCD::Loader

std::optional<std::string> GCD::Loader::loadKernelModule(const std::string_view moduleName)
{
    return Utilities::loadKernelModule(getModulePath(moduleName));
}
