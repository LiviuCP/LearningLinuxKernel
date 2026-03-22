#pragma once

#include <string_view>

constexpr std::string_view ioctlDivisionModuleName{"ioctl_division"};

namespace GCD::Loader
{
void loadKernelModuleIoctlDivision();
bool isKernelModuleIoctlDivisionLoaded();

constexpr std::string_view getDivisionModuleName()
{
    return ioctlDivisionModuleName;
}

} // namespace GCD::Loader
