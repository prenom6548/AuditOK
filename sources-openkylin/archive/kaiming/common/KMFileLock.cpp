#include "KMFileLock.h"
#include "KMLogger.h"

#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <iostream>

KMFileLock::KMFileLock(const std::string &filePath, int maxRetries, int waitTimeMs)
    : m_filePath(filePath), m_fd(-1), m_maxRetries(maxRetries), m_waitTimeMs(waitTimeMs)
{
}

KMFileLock::~KMFileLock()
{
    releaseLock();
}

bool KMFileLock::tryLock(LockType type)
{
    return acquire(type, false);
}

bool KMFileLock::lock(LockType type)
{
    return acquire(type, true);
}

void KMFileLock::releaseLock()
{
    if (m_fd != -1)
    {
        flock(m_fd, LOCK_UN);
        close(m_fd);
        m_fd = -1;
        kmlogger.info("Lock released: %s", m_filePath.c_str());
    }
}

bool KMFileLock::acquire(LockType type, bool shouldRetry)
{
    if (m_fd == -1)
    {
        m_fd = open(m_filePath.c_str(), O_CREAT | O_RDWR, 0666);
        if (m_fd == -1)
        {
            kmlogger.error("Failed to open lock file: %s", m_filePath.c_str());
            return false;
        }
    }

    int lockMode = (type == LockType::READ) ? LOCK_SH : LOCK_EX;
    lockMode |= LOCK_NB;

    int retries = 0;
    while (flock(m_fd, lockMode) == -1)
    {
        if (!shouldRetry || retries >= m_maxRetries)
        {
            kmlogger.error("Failed to acquire %s lock after %d attempts.", type == LockType::READ ? "read" : "write", retries);
            close(m_fd);
            m_fd = -1;
            return false;
        }
        retries++;
        KMInfo("Lock file busy, retrying...");
        std::this_thread::sleep_for(std::chrono::milliseconds(m_waitTimeMs));
    }

    kmlogger.info("Successfully acquired %s lock: %s", type == LockType::READ ? "read" : "write", m_filePath.c_str());
    return true;
}
