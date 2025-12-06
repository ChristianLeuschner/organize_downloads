#include "FileSorter.hpp"

#include <iostream>

// singleton instance
FileSorter& FileSorter::getInstance() {
    static FileSorter instance;
    return instance;
}

fs::path FileSorter::getUniqueDestination(const fs::path& targetPath) {
    fs::path currentPath = targetPath;

    // return immediately if no conflict
    if (!fs::exists(currentPath)) {
        return currentPath;
    }

    // Decompose path: directory, basename, extension
    const fs::path parentDir = targetPath.parent_path();
    const std::string stem = targetPath.stem().string();  // Filename without extension
    const std::string extension = targetPath.extension().string();

    int counter = 1;
    while (true) {
        // Construct new filename: basename + _N + .extension
        std::string newStem = stem + "_" + std::to_string(counter);
        fs::path newFilename = newStem + extension;

        currentPath = parentDir / newFilename;

        if (!fs::exists(currentPath)) {
            // Unique path found
            return currentPath;
        }

        counter++;
        if (counter > 100) {
            // Safety stop to avoid errors or infinite loops
            throw std::runtime_error("Too many name conflicts (" + targetPath.string() + ").");
        }
    }
}

bool FileSorter::sort(const fs::path& sourcePath, const ConfigData& config) {
    if (!config.is_valid || !fs::exists(sourcePath)) {
        std::cerr << "ERROR: Invalid config data or source file does not exist: "
                  << sourcePath.string() << std::endl;
        return false;
    }

    // get extension
    const std::string fileExtension = sourcePath.extension().string();

    // get rule
    for (const auto& rule : config.rules) {
        if (rule.extension == fileExtension) {
            // create target directory
            try {
                // Creates all missing directories recursively
                fs::create_directories(rule.destination);
            } catch (const fs::filesystem_error& e) {
                std::cerr << "ERROR: target directory creation failed (" << rule.destination
                          << "): " << e.what() << std::endl;
                return false;
            }

            // determine final target path considering conflicts
            fs::path initialTargetPath = rule.destination / sourcePath.filename();
            fs::path finalTargetPath;

            try {
                finalTargetPath = getUniqueDestination(initialTargetPath);
            } catch (const std::runtime_error& e) {
                std::cerr << "ERROR: Could not resolve name conflict: " << e.what() << std::endl;
                return false;
            }

            // move file
            try {
                fs::rename(sourcePath, finalTargetPath);
                std::cout << "SUCCESS: File moved: " << sourcePath.filename().string() << " -> "
                          << finalTargetPath.parent_path().filename().string() << std::endl;
                return true;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "FATAL: Error while moving file (" << sourcePath.string()
                          << "): " << e.what() << std::endl;
                return false;
            }
        }
    }

    // Default
    std::cout << "WARNING: No rule found for extension '" << fileExtension << "'. File ignored."
              << std::endl;
    return true;  // Operation successful (file was intentionally ignored)
}