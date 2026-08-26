#pragma once

#include <string_view>

constexpr std::string_view divisionModuleName{"procfs_division"};
constexpr std::string_view utilitiesModuleName{"kernel_utilities"};

namespace GCD::Loader
{
void loadKernelModuleProcfsDivision();
void loadKernelModuleUtilities();

bool isKernelModuleProcfsDivisionLoaded();
bool isKernelModuleUtilitiesLoaded();

constexpr std::string_view getDivisionModuleName()
{
    return divisionModuleName;
}

constexpr std::string_view getUtilitiesModuleName()
{
    return utilitiesModuleName;
}
} // namespace GCD::Loader
