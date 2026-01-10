#pragma once

#include <filesystem>

#define READ_MODE false
#define WRITE_MODE true

namespace GCD::Utils
{
void clearScreen();
std::filesystem::path getApplicationPath();
bool isValidInteger(const char* str);
} // namespace GCD::Utils
