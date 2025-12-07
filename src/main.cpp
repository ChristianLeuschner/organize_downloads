#include <unistd.h>  // For sleep()

#include <csignal>  // For signal handling
#include <iostream>
#include <stdexcept>
#include <thread>

#include "../include/ConfigLoader.hpp"
#include "../include/FileWatcher.hpp"

// Global variables: Necessary because signal handlers cannot receive arguments
static std::atomic<bool> g_running{true};
static std::unique_ptr<FileWatcher> g_watcher = nullptr;
static ConfigLoader* g_configLoader = nullptr;

/**
 * Signal Handler function: Handles system signals like SIGTERM (stop) and SIGHUP (reload).
 */
void handle_signal(int signal) {
    if (signal == SIGTERM || signal == SIGINT) {
        std::cout << "\nINFO: SIGTERM/SIGINT received. Starting shutdown..." << std::endl;
        g_running.store(false);  // Exits the main loop cleanly

    } else if (signal == SIGHUP) {
        std::cout << "INFO: SIGHUP received. Reloading configuration..." << std::endl;

        // Triggers config reload via the ConfigLoader
        if (g_configLoader) {
            g_configLoader->loadConfig();
        }
    }
}

int main(int argc, char* argv[]) {
    // 1. Argument Parsing (Path to rules.json)
    if (argc != 2) {
        std::cerr << "USAGE: " << argv[0] << " <Path/to/rules.json>" << std::endl;
        return 1;
    }
    const fs::path configPath = argv[1];

    // 2. Component Initialization
    try {
        // Initialize ConfigLoader Singleton with the config path argument
        g_configLoader = &ConfigLoader::getInstance(configPath);

        // Initial load check
        if (!g_configLoader->loadConfig().has_value()) {
            std::cerr << "FATAL: Could not load configuration. Exiting program." << std::endl;
            return 1;
        }

        // Initialize FileWatcher
        g_watcher = std::make_unique<FileWatcher>(*g_configLoader);

        // 3. Set up Signal Handling (OS level control)
        std::signal(SIGTERM, handle_signal);  // Systemd Stop
        std::signal(SIGINT, handle_signal);   // Ctrl+C
        std::signal(SIGHUP, handle_signal);   // Systemd Reload

        // 4. Start Daemon functionality
        std::cout << "INFO: Daemon starting..." << std::endl;
        g_watcher->start();

        // 5. Main Loop (Blocks the main thread resource-efficiently)
        // This loop prevents main() from exiting immediately.
        while (g_running.load()) {
            // Sleep is resource-friendly, letting the FileWatcher thread handle events.
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        if (g_watcher) {
            g_watcher->stop();  // Stops the blocking Watcher thread
        }

    } catch (const std::exception& e) {
        std::cerr << "FATAL: Unhandled error in main program: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "INFO: Daemon shut down." << std::endl;
    return 0;
}