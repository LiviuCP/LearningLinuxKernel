#include "testutils.h"
#include "utils.h"

std::optional<std::filesystem::path> Utilities::Test::getModulePath(const std::string_view moduleName)
{
    std::filesystem::path modulePath{Utilities::getApplicationPath()};
    modulePath = modulePath.parent_path().parent_path();
    modulePath /= Utilities::getModulesDirRelativePath();
    modulePath /= moduleName;
    modulePath += Utilities::getModuleFileExtension();

    return std::filesystem::is_regular_file(modulePath) ? std::optional{modulePath} : std::nullopt;
}
