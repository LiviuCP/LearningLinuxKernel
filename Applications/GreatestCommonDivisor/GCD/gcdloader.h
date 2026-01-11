#pragma once

#include <string_view>

namespace GCD::Loader
{
void loadKernelModuleDivision();

constexpr std::string_view getDivisionModuleName()
{
    constexpr std::string_view divisionModuleName{"division"};
    return divisionModuleName;
}
} // namespace GCD::Loader
