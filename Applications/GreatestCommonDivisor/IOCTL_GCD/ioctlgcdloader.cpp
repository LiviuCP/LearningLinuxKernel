#include "ioctlgcdloader.h"
#include "gcdloaderimpl.h"
#include "utils.h"

void GCD::Loader::loadKernelModuleIoctlDivision()
{
    loadKernelModule(getModulePath(getDivisionModuleName()));
}

bool GCD::Loader::isKernelModuleIoctlDivisionLoaded()
{
    return Utilities::isKernelModuleLoaded(getDivisionModuleName());
}
