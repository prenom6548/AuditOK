/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __BUILD_SOURCES_ARCHIVE_H__
#define __BUILD_SOURCES_ARCHIVE_H__

#include "KMBuilderSource.h"

class KMBuilderSourceArchive : public KMBuilderSource
{
    DECLARE_DYNCREATE(KMBuilderSourceArchive);

public:
    KMBuilderSourceArchive();
    ~KMBuilderSourceArchive();

    std::string getChecksum();
    std::string getUrl();
    bool ensureSource() { return true; }
    bool unzipSource() override;
    bool processNode(std::string path);

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

    // 源码下载，用于替换方法processNode的，后续有时间再彻底优化
    bool downloadSource(const std::string &destDir) override;

private:

    class Private;
    std::shared_ptr<Private> m_ptr;
    std::string m_unzipDestination;
    std::string m_unzipSource;
};

#endif
