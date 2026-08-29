#pragma once

#include <string_view>

#if defined(IOCTL_GCD)
inline constexpr std::string_view divisionModuleName{"ioctl_division"};
#elif defined(PROCFS_GCD)
inline constexpr std::string_view divisionModuleName{"procfs_division"};
#elif defined(SYSFS_GCD)
inline constexpr std::string_view divisionModuleName{"sysfs_division"};
#else
static_assert(false && "Invalid implementation!");
#endif
