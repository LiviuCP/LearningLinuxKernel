#include <optional>
#include <stdexcept>
#include <string_view>

#include "gcdloader.h"
#include "utils.h"

static constexpr std::string_view divisionModuleRelativePath{"KernelModules/ConsolidatedOutput/division.ko"};

namespace GCD::Loader
{
namespace
{
std::optional<std::filesystem::path> getDivisionModulePath()
{
    const std::filesystem::path applicationPath{Utilities::getApplicationPath()};

    std::filesystem::path divisionModulePath{applicationPath.parent_path().parent_path().parent_path()};
    divisionModulePath /= divisionModuleRelativePath;

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

        if (!Utilities::isKernelModuleLoaded(divisionModulePath->filename().stem().string()))
        {
            throw std::runtime_error{
                "Could not load kernel module Division!\nPlease check that the module file exists in "
                "consolidated output and try again by running the app with sudo."};
        }
    }
    else
    {
        throw std::runtime_error{"Invalid kernel module file path!"};
    }
}

std::string GCD::Loader::getDivisionModuleName()
{
    const std::optional<std::filesystem::path> divisionModulePath{getDivisionModulePath()};
    return divisionModulePath.has_value() ? divisionModulePath->filename().stem().string() : "division.ko";
}
