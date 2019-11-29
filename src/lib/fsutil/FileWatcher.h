//
// Utility class for monitoring changes to a particular file.
//
// This is really meant to be a simple solution to monitor a single file in a 
// program, and creates a new thread for each file being monitored.
//
// If the time comes where something more sophisticated or capable is required, 
// do reexamine this implementation.
//

#pragma once

#include <string>
#include <functional>
#include <thread>
#include <memory>

namespace fsutil
{
    class FileWatcher
    {
    public:
        enum FileWatcherEvent
        {
            FileWatcherUninitialized = 0,
            FileWatcherModified,
            FileWatcherDeleted,
            FileWatcherMax
        };

        using FnChangeCallback =
            std::function<void(const std::string&, FileWatcherEvent)>;

        FileWatcher(
            const std::string& pathToWatch,
            FnChangeCallback fileChangeCallback
            );

        virtual ~FileWatcher();

    private:
        std::string      m_fileToMonitor;
        FnChangeCallback m_fileChangeCallback;

        int              m_inotifyFd;
        int              m_inotifyWd;

        bool             m_threadRunning;
        std::unique_ptr<std::thread>  m_spFileStatusPollingThread;
    };
}
