#pragma once

#include <string_view>

constexpr std::string_view divisionModuleName{"division"};
constexpr std::string_view utilitiesModuleName{"kernelutilities"};

namespace GCD::Loader
{
void loadKernelModuleDivision();
void loadKernelModuleUtilities();

bool isKernelModuleDivisionLoaded();
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
