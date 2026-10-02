/*
 * Copyright (c) KylinSoft  Co., Ltd. 2026. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#ifndef KM_DEFERRED_CLEANUP_STORE_H
#define KM_DEFERRED_CLEANUP_STORE_H

#include <filesystem>
#include <string>
#include <vector>

/**
 * @brief 延迟卸载旧 ref 的持久化状态
 */
class KMDeferredCleanupStore
{
public:
    /** @param infoDirectory 与 KMFolder::getInfoPath() 一致，一般为 basePath / "info" */
    explicit KMDeferredCleanupStore(const std::filesystem::path &infoDirectory);
    KMDeferredCleanupStore(std::string deferredRefsPath, std::string lastBootIdPath);

    std::vector<std::string> loadDeferredRefs() const;
    void saveDeferredRefs(const std::vector<std::string> &refs) const;

    static std::string readKernelBootId();

    std::string readLastCleanupBootId() const;
    void writeLastCleanupBootId(const std::string &bootId) const;

    const std::string &deferredRefsPath() const
    {
        return m_deferredRefsPath;
    }
    const std::string &lastBootIdPath() const
    {
        return m_lastBootIdPath;
    }

private:
    static std::string trimCopy(std::string s);

    std::string m_deferredRefsPath;
    std::string m_lastBootIdPath;
};

#endif
