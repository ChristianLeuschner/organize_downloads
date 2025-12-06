#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

namespace fs = std::filesystem;

struct SortRule {
    std::string extension;
    std::string destination;
};

struct ConfigData {
    std::string watch_folder;
    std::vector<SortRule> rules;
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
};