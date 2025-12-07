#pragma once

#include <pwd.h>
#include <sys/inotify.h>  // Linux-System-Header für inotify
#include <unistd.h>

#include <atomic>
#include <mutex>

#include "ConfigLoader.hpp"

// inotify buffer size
#define EVENT_BUF_LEN (1024 * (sizeof(struct inotify_event) + 16))

class FileWatcher {
   public:
    FileWatcher(ConfigLoader& configLoader);

    ~FileWatcher();

    void start();
    void stop();

   private:
    ConfigLoader& m_configLoader;

    // system descriptors for inotify
    int m_inotify_fd = -1;
    int m_watch_fd = -1;         // main folder
    int m_config_watch_fd = -1;  // config file

    // manage thread
    std::atomic<bool> m_running{false};
    std::thread m_worker_thread;

    // main event loop
    void runEventLoop();

    // helper
    bool setupWatch();
    std::string getHomePath();
};