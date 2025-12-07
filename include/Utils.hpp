#pragma once

#include <string>

namespace Utils {

/**
 * @brief Retrieves the current user's home directory.
 * * First attempts to read the $HOME environment variable.
 * If that fails, it queries the password database (pwd.h) using a thread-safe method.
 * * @return std::string The absolute path to the home directory.
 * @throws std::runtime_error If the home directory cannot be determined.
 */
std::string getHomePath();

/**
 * @brief Expands environment variables in a path string.
 * * Currently only supports expanding "$HOME" at the beginning of the string.
 * Example: "$HOME/Downloads" -> "/home/user/Downloads"
 * * @param path The path string to expand.
 * @return std::string The path with "$HOME" replaced by the actual home directory.
 */
std::string expandPath(const std::string& path);

}  // namespace Utils