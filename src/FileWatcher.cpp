#include "FileWatcher.hpp"

#include <algorithm>  // Für std::min/max
#include <cstring>    // Für strerror() oder perror()
#include <iostream>

// (de)constructor
FileWatcher::FileWatcher(ConfigLoader& configLoader) : m_configLoader(configLoader) {}
FileWatcher::~FileWatcher() { stop(); }

// methods
void FileWatcher::start() {
    if (m_running.load()) {
        std::cerr << "FileWatcher is already running!" << std::endl;
        return;
    }

    if (!setupWatch()) {
        std::cerr << "Failed to set up file watcher!" << std::endl;
        return;
    }
    m_running.store(true);
    m_worker_thread = std::thread(&FileWatcher::runEventLoop, this);
    std::cout << "FileWatcher started." << std::endl;
}

void FileWatcher::stop() {
    if (!m_running.load()) {
        return;
    }
    m_running.store(false);

    // close inotify descriptor and remove watches
    if (m_inotify_fd != -1) {
        if (m_watch_fd != -1) {
            inotify_rm_watch(m_inotify_fd, m_watch_fd);
        }
        if (m_config_watch_fd != -1) {
            inotify_rm_watch(m_inotify_fd, m_config_watch_fd);
        }
        close(m_inotify_fd);  // force end of blocking read
        m_inotify_fd = -1;
    }
    if (m_worker_thread.joinable()) {
        m_worker_thread.join();
    }
    std::cout << "FileWatcher stopped." << std::endl;
}

bool FileWatcher::setupWatch() {
    const auto config = m_configLoader.getConfigData();
    if (!config.is_valid || config.main_folder.empty()) {
        std::cerr << "Invalid configuration data or missing watch folder!" << std::endl;
        return false;
    }

    // initialize inotify
    m_inotify_fd = inotify_init();
    if (m_inotify_fd < 0) {
        perror("inotify_init");
        return false;
    }

    // add watch for the main folder
    m_watch_fd =
        inotify_add_watch(m_inotify_fd, config.main_folder.c_str(), IN_CLOSE_WRITE | IN_MOVED_TO);
    if (m_watch_fd < 0) {
        perror("watch_main_folder");
        return false;
    }
    std::cout << "Watching folder: " << config.main_folder << std::endl;

    // add watch for config file
    std::string configPath = getHomePath() + "/.config/filesorter/rules.json";
    m_config_watch_fd = inotify_add_watch(m_inotify_fd, configPath.c_str(), IN_CLOSE_WRITE);
    if (m_config_watch_fd < 0) {
        perror("watch_config_file");
        return false;
    }
    std::cout << "Watching folder: " << config.main_folder << std::endl;
    return true;
}

void FileWatcher::runEventLoop() {
    // static buffer on stack for inotitfy events
    char buffer[EVENT_BUF_LEN];
    __attribute__((aligned(8)));
    std::cout << "FileWatcher event loop started." << std::endl;

    // main loop
    while (m_running.load()) {
        // wait for events (blocking)
        ssize_t length = read(m_inotify_fd, buffer, EVENT_BUF_LEN);

        // exception handling (e.g. interrupted by stop())
        if (length < 0) {
            if (!m_running.load()) {
                break;
            }
            std::cerr << "Error reading inotify events: " << strerror(errno) << std::endl;
            continue;
        }

        // process all events
        size_t i = 0;
        while (i < length) {
            // cast buffer to event
            struct inotify_event* event = (struct inotify_event*)&buffer[i];

            // check which watch triggered the event
            if (event->wd == m_watch_fd) {
                if (event->mask & (IN_CLOSE_WRITE | IN_MOVED_TO)) {
                    fs::path filePath = m_configLoader.getConfigData().main_folder;
                    filePath /= event->name;  // append filename to path
                    triggerSorter(filePath, m_configLoader);
                }
            }
            if (event->wd == m_config_watch_fd) {
                if (event->mask & IN_CLOSE_WRITE) {
                    std::cout << "  Config file changed, reloading..." << std::endl;
                    m_configLoader.loadConfig();
                }
            }

            // next event
            i += sizeof(struct inotify_event) + event->len;
        }
    }
}

std::string FileWatcher::getHomePath() {
    const char* homeDir = getenv("HOME");
    if (homeDir) {
        return std::string(homeDir);
    } else {
        struct passwd* pwd = getpwuid(getuid());
        if (pwd) {
            return std::string(pwd->pw_dir);
        } else {
            throw std::runtime_error("Unable to determine home directory.");
        }
    }
}

// Fiktive Funktion, die später den FileSorter auslösen wird
void triggerSorter(const fs::path& filePath, ConfigLoader& configLoader) {
    // Hier würde später der FileSorter::sort() Aufruf stehen
    std::cout << "DEBUG: Sortierungs-Event ausgelöst für: " << filePath.filename().string()
              << std::endl;
}