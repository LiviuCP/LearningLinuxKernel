#include "sysfsgcdloader.h"
#include "gcdloaderimpl.h"
#include "utils.h"

void GCD::Loader::loadKernelModuleSysfsDivision()
{
    loadKernelModule(getModulePath(getDivisionModuleName()));
}

void GCD::Loader::loadKernelModuleUtilities()
{
    loadKernelModule(getModulePath(getUtilitiesModuleName()));
}

bool GCD::Loader::isKernelModuleSysfsDivisionLoaded()
{
    return Utilities::isKernelModuleLoaded(getDivisionModuleName());
}

bool GCD::Loader::isKernelModuleUtilitiesLoaded()
{
    return Utilities::isKernelModuleLoaded(getUtilitiesModuleName());
}
