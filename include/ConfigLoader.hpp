#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct SortRule {
    std::string extension;
    std::string destination;
};

struct ConfigData {
    std::string main_folder;
    std::vector<SortRule> rules;  // TODO: change to map for faster lookup?
    bool is_valid = false;
};

class ConfigLoader {
   public:
    static ConfigLoader& getInstance(const fs::path& configPath);

    std::optional<ConfigData> loadConfig();

    ConfigData getConfigData() const;

   private:
    ConfigLoader(const fs::path& configPath);

    const fs::path m_configPath;

    ConfigData m_configData;

    // Prevent copying and assignment (standard for Singleton)

    ConfigLoader(const ConfigLoader&) = delete;
    ConfigLoader& operator=(const ConfigLoader&) = delete;
};