#pragma once

#include <filesystem>
#include <string>

#include "ConfigLoader.hpp"

namespace fs = std::filesystem;

class FileSorter {
   public:
    // Singleton Access method
    static FileSorter& getInstance();

    // Main method: Applies sorting rules and moves the file
    bool sort(const fs::path& sourcePath, const ConfigData& config);

   private:
    FileSorter() = default;
    ~FileSorter() = default;

    // handles name conflicts
    fs::path getUniqueDestination(const fs::path& targetPath);

    // Prevent copying and assignment (standard for Singleton)
    FileSorter(const FileSorter&) = delete;
    FileSorter& operator=(const FileSorter&) = delete;
};