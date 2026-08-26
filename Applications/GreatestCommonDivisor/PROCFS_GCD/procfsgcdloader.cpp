#include "procfsgcdloader.h"
#include "gcdloaderimpl.h"
#include "utils.h"

void GCD::Loader::loadKernelModuleProcfsDivision()
{
    loadKernelModule(getModulePath(getDivisionModuleName()));
}

void GCD::Loader::loadKernelModuleUtilities()
{
    loadKernelModule(getModulePath(getUtilitiesModuleName()));
}

bool GCD::Loader::isKernelModuleProcfsDivisionLoaded()
{
    return Utilities::isKernelModuleLoaded(getDivisionModuleName());
}

bool GCD::Loader::isKernelModuleUtilitiesLoaded()
{
    return Utilities::isKernelModuleLoaded(getUtilitiesModuleName());
}
