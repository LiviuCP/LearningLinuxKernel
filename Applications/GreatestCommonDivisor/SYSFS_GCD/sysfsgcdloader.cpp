#include "sysfsgcdloader.h"
#include "gcdloaderimpl.h"
#include "utils.h"

void GCD::Loader::loadKernelModuleDivision()
{
    loadKernelModule(getModulePath(getDivisionModuleName()));
}

void GCD::Loader::loadKernelModuleUtilities()
{
    loadKernelModule(getModulePath(getUtilitiesModuleName()));
}

bool GCD::Loader::isKernelModuleDivisionLoaded()
{
    return Utilities::isKernelModuleLoaded(getDivisionModuleName());
}

bool GCD::Loader::isKernelModuleUtilitiesLoaded()
{
    return Utilities::isKernelModuleLoaded(getUtilitiesModuleName());
}
