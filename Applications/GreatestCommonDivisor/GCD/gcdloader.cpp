#include <optional>
#include <stdexcept>
#include <string_view>

#include "gcdloader.h"
#include "gcdutils.h"

static constexpr std::string_view divisionModuleRelativePath{"KernelModules/ConsolidatedOutput/division.ko"};

namespace GCD::Loader
{
namespace
{
std::optional<std::filesystem::path> getDivisionModulePath()
{
    const std::filesystem::path applicationPath{GCD::Utils::getApplicationPath()};

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
        // no need to include sudo in the command string -> the user needs to run the app with sudo anyway and if so the
        // command will be executed in sudo mode
        const std::string loadCommand{"insmod " + divisionModulePath->string() + " 2> /dev/null"};
        system(loadCommand.c_str());
    }

    if (!isKernelModuleDivisionLoaded())
    {
        throw std::runtime_error{
            "Could not load kernel module Division!\nPlease try again by running the app with sudo."};
    }
}

void GCD::Loader::unloadKernelModuleDivision()
{
    const std::optional<std::filesystem::path> divisionModulePath{getDivisionModulePath()};

    if (divisionModulePath.has_value())
    {
        // no need to include sudo in the command string -> the user needs to run the app with sudo anyway and if so the
        // command will be executed in sudo mode
        const std::string unloadCommand{"rmmod " + divisionModulePath->filename().stem().string()};
        system(unloadCommand.c_str());
    }
}

bool GCD::Loader::isKernelModuleDivisionLoaded()
{
    const std::string commandOutput{GCD::Utils::executeCommand("lsmod | grep -w division", READ_MODE)};
    return commandOutput.starts_with("division");
}
