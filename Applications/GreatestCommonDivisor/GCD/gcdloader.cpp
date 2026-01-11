#include <optional>
#include <stdexcept>

#include "gcdloader.h"
#include "utils.h"

namespace GCD::Loader
{
namespace
{
std::optional<std::filesystem::path> getDivisionModulePath()
{
    const std::filesystem::path applicationPath{Utilities::getApplicationPath()};

    std::filesystem::path divisionModulePath{applicationPath.parent_path().parent_path().parent_path()};
    divisionModulePath /= Utilities::getModulesDirRelativePath();
    divisionModulePath /= getDivisionModuleName();
    divisionModulePath += Utilities::getModuleFileExtension();

    return std::filesystem::is_regular_file(divisionModulePath) ? std::optional{divisionModulePath} : std::nullopt;
}
} // namespace
} // namespace GCD::Loader

void GCD::Loader::loadKernelModuleDivision()
{
    const std::optional<std::filesystem::path> divisionModulePath{getDivisionModulePath()};

    if (divisionModulePath.has_value())
    {
        Utilities::loadKernelModule(*divisionModulePath);
    }

    if (!Utilities::isKernelModuleLoaded(std::string{getDivisionModuleName()}))
    {
        throw std::runtime_error{"Could not load kernel module Division!\nPlease check that the module file exists in "
                                 "consolidated output and try again by running the app with sudo."};
    }
}
