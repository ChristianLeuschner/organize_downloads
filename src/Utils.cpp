#include "../include/Utils.hpp"

#include <pwd.h>        // For getpwuid_r (Linux specific)
#include <sys/types.h>  // For uid_t
#include <unistd.h>     // For sysconf, getuid

#include <cstdlib>    // For getenv
#include <cstring>    // For strerror
#include <stdexcept>  // For std::runtime_error
#include <vector>     // For std::vector buffer

namespace Utils {

std::string getHomePath() {
    // 1. Try to get the environment variable (fastest method)
    const char* homeDir = std::getenv("HOME");
    if (homeDir) {
        return std::string(homeDir);
    }

    // 2. POSIX fallback: query user database (thread-safe)

    // Determine necessary buffer size for getpwuid_r
    long bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1) {
        bufsize = 16384;  // Fallback size if sysconf fails
    }

    std::vector<char> buffer(bufsize);
    struct passwd pwd;
    struct passwd* result = nullptr;

    // getuid() gets the real user ID of the calling process
    int s = getpwuid_r(getuid(), &pwd, buffer.data(), bufsize, &result);

    if (result == nullptr) {
        if (s == 0) {
            throw std::runtime_error("Unable to determine home directory: User ID not found.");
        } else {
            throw std::runtime_error("Unable to determine home directory: " +
                                     std::string(std::strerror(s)));
        }
    }

    return std::string(pwd.pw_dir);
}

std::string expandPath(const std::string& path) {
    if (path.empty()) {
        return path;
    }

    const std::string homeVar = "$HOME";

    // Check if the path starts with "$HOME"
    if (path.rfind(homeVar, 0) == 0) {
        std::string homeDir = getHomePath();

        // Create a copy and replace the variable
        std::string expanded = path;
        expanded.replace(0, homeVar.length(), homeDir);
        return expanded;
    }

    // Return original path if no substitution was needed
    return path;
}


}  // namespace Utils