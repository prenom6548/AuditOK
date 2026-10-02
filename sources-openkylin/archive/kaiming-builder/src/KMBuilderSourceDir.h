/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __BUILD_SOURCE_DIR_H__
#define __BUILD_SOURCE_DIR_H__

#include "KMBuilderSource.h"

class KMBuilderSourceDir : public KMBuilderSource
{
    DECLARE_DYNCREATE(KMBuilderSourceDir);

public:
    KMBuilderSourceDir();
    ~KMBuilderSourceDir();

    bool ensureSource() final;
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
    class Private;
    std::shared_ptr<Private> m_ptr;
};

#endif
