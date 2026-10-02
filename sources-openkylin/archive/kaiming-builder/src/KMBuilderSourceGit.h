/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __BUILD_SOURCE_GIT_H__
#define __BUILD_SOURCE_GIT_H__

#include "KMBuilderSource.h"

class KMBuilderSourceGit : public KMBuilderSource
{
    DECLARE_DYNCREATE(KMBuilderSourceGit);

public:
    KMBuilderSourceGit();
    ~KMBuilderSourceGit();

    bool updateSource() { return true; }
    bool ensureSource() { return true; }
    bool unzipSource() override;
    virtual bool processNode(std::string path);

    /**
     * @brief : 需要子类实现自身的反序列化
     * @param : [in]，sourceNode，清单中的源节点
     */ 
    virtual void deserializeEx(YAML::Node& sourceNode);
    
    /**
     * @brief : 需要子类实现将数据写入yaml节点
     * @param : [in], sourceNode, 待写入节点
     */
    virtual void toYaml(YAML::Node& sourceNode);

private:
    void generateGitCommand(const std::string &path);

    class Private;
    std::shared_ptr<Private> m_ptr;
    std::string m_gitDownload;
    std::string m_gitDownloadRepo;
    std::string m_gitUnzip;
    std::string m_gitUnzipPath;
};

#endif
