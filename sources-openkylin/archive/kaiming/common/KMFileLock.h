/*
 * @Author: caoxiansheng caoxiansheng@kylinos.cn
 * @Date: 2025-08-20 11:15:20
 * @LastEditors: caoxiansheng caoxiansheng@kylinos.cn
 * @LastEditTime: 2025-08-20 11:15:20
 * @FilePath: /kaiming/common/KMFileLock.h
 * @Description: 
 * 
 * Copyright (c) by KylinSoft  Co., Ltd. 2025. All Rights Reserved.
 */

#ifndef __KM_FILE_LOCK_H__
#define __KM_FILE_LOCK_H__

#include <string>

class KMFileLock {
public:
    enum class LockType { READ, WRITE };

    KMFileLock(const std::string &filePath = "/opt/kaiming/info/kaiming.lock", int maxRetries = 30, int waitTimeMs = 1000);
    ~KMFileLock();

    KMFileLock(const KMFileLock &) = delete;
    KMFileLock &operator=(const KMFileLock &) = delete;

    bool tryLock(LockType type);
    bool lock(LockType type);
    void releaseLock();

    bool isLocked() const { return m_fd != -1; }

private:
    bool acquire(LockType type, bool shouldRetry);

private:
    std::string m_filePath;
    int m_fd;
    int m_maxRetries;
    int m_waitTimeMs;
};

#endif // __KM_FILE_LOCK_H__
