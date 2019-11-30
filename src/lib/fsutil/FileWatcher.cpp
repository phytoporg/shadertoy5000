#include "FileWatcher.h"

#include <experimental/filesystem>
#include <iostream>
#include <stdexcept>

#include <cstring>
#include <unistd.h>

#include <sys/types.h>
#include <sys/inotify.h>
#include <poll.h>

namespace fsutil
{
    FileWatcher::FileWatcher(
        const std::string& pathToWatch,
        FnChangeCallback fileChangeCallback
        ) : m_threadRunning(false), 
            m_fileChangeCallback(fileChangeCallback)
    {
        namespace fs = std::experimental::filesystem;
        if (!fs::is_regular_file(pathToWatch))
        {
            throw 
                std::runtime_error(
                    "FileWatcher: file does not exist -- " + 
                    pathToWatch);
        }

        m_threadRunning = true;
        m_spFileStatusPollingThread.reset(
            new std::thread([this, pathToWatch]() {

                m_inotifyFd = inotify_init1(IN_NONBLOCK);
                if (m_inotifyFd < 0)
                {
                    throw std::runtime_error(
                            "FileWatcher: inotify_init() failed. " +
                            std::string(strerror(errno))
                        );
                }

                m_inotifyWd =
                    inotify_add_watch(
                        m_inotifyFd,
                        pathToWatch.c_str(),
                        IN_MODIFY | IN_DELETE
                        );

               const size_t BufferLength = sizeof(struct inotify_event); 
               char buffer[BufferLength + 1];

               struct pollfd fds[] = { { m_inotifyFd, POLLIN, 0 } };
               const int TimeoutMs = 500;
               while(m_threadRunning)
               {
                   int res = poll(fds, 1, TimeoutMs);
                   if (res < 0)
                   {
                       throw std::runtime_error(
                           "File polling thread: select() failed. " +
                           std::string(strerror(errno)));
                   }
                   else if (res == 0)
                   {
                       //
                       // Timeout. Nothing to do.
                       //
                       continue;
                   }

                   while(true)
                   {
                       int length = read(m_inotifyFd, buffer, BufferLength); 
                       if (length <= 0)
                       {
                           break;
                       }
                       else
                       {
                       }

                       auto pEvent =
                           reinterpret_cast<struct inotify_event*>(buffer);
                       FileWatcherEvent event = FileWatcherUninitialized;
                       {
                           if (pEvent->mask & IN_MODIFY)
                           {
                               event = FileWatcherModified;
                           }
                           else if (pEvent->mask & IN_DELETE)
                           {
                               event = FileWatcherDeleted;
                           }

                           m_fileChangeCallback(m_fileToMonitor, event);
                       }
                   }
               }

            }));
    }

    FileWatcher::~FileWatcher()
    {
        m_threadRunning = false;

        try
        {
            m_spFileStatusPollingThread->join();
        }
        catch(std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
        inotify_rm_watch(m_inotifyFd, m_inotifyWd);
    }
}
