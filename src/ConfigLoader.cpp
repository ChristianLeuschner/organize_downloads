#include "ConfigLoader.hpp"

#include <fstream>
#include <iostream>

#include "Utils.hpp"
#include "nlohmann/json.hpp"

// read JSON
namespace {
using json = nlohmann::json;

std::optional<SortRule> parseRule(const json& j) {
    // check if required fields exist and are of correct type
    if (!j.contains("extension") || !j["extension"].is_string() || !j.contains("destination") ||
        !j["destination"].is_string()) {
        return std::nullopt;
    }
    SortRule rule;
    rule.extension = j["extension"].get<std::string>();
    rule.destination = Utils::expandPath(j["destination"].get<std::string>());
    // extension should start with a dot
    if (rule.extension.empty() || rule.extension.front() != '.') {
        return std::nullopt;
    }
    return rule;
}

}  // namespace

// constructor
ConfigLoader::ConfigLoader(const fs::path& configPath) : m_configPath(configPath) {}

// singleton instance
ConfigLoader& ConfigLoader::getInstance(const fs::path& configPath) {
    static ConfigLoader instance(configPath);
    return instance;
}

// methods
std::optional<ConfigData> ConfigLoader::loadConfig() {
    std::ifstream configFile(m_configPath);

    if (!configFile.is_open()) {
        std::cerr << "Error: Could not open config file at " << m_configPath << std::endl;
        return std::nullopt;
    }

    try {
        json j = json::parse(configFile);

        if (!j.contains("watch_folder") || !j["watch_folder"].is_string() || !j.contains("rules") ||
            !j["rules"].is_array()) {
            std::cerr << "Error: Invalid config file format." << std::endl;
            return std::nullopt;
        }

        ConfigData configData;
        configData.main_folder = Utils::expandPath(j["watch_folder"].get<std::string>());
        for (const auto& json_rule : j["rules"]) {
            if (auto rule = parseRule(json_rule)) {
                configData.rules.push_back(std::move(*rule));
            } else {
                std::cerr << "Warning: Skipping invalid rule in config file." << std::endl;
            }
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        // store loaded data
        m_configData = configData;
        m_configData.is_valid = true;

        return m_configData;

    } catch (const json::parse_error& e) {
        std::cerr << "Error: JSON parse error in config file - " << e.what() << std::endl;
        return std::nullopt;
    } catch (const std::exception& e) {
        std::cerr << "Error: Exception while reading config file - " << e.what() << std::endl;
        return std::nullopt;
    }
}

ConfigData ConfigLoader::getConfigData() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_configData;
}