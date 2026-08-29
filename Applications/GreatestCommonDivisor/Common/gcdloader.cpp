#include <filesystem>
#include <optional>
#include <stdexcept>

#include "gcdloader.h"
#include "utils.h"

namespace GCD::Loader
{
namespace
{
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

void GCD::Loader::loadKernelModule(const std::string_view moduleName)
{
    const std::optional<std::filesystem::path> modulePath{getModulePath(moduleName)};

    if (modulePath.has_value())
    {
        Utilities::loadKernelModule(*modulePath);
    }

    if (!Utilities::isKernelModuleLoaded(moduleName))
    {
        throw std::runtime_error{"Could not load kernel module " + std::string{moduleName} +
                                 "!\nPlease check that the module file exists in "
                                 "consolidated output and try again by running the app with sudo."};
    }
}
