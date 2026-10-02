/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __BUILDER_NEWPROJECT_H__
#define __BUILDER_NEWPROJECT_H__

#include<filesystem>

#include"KMSubCommand.h"
#include"KMBuilderConfig.h"

namespace fs = std::filesystem;

class KMBuilderNewProject : public KMSubCommand
{
DECLARE_DYNCREATE(KMBuilderNewProject);
public:
    KMBuilderNewProject();
    ~KMBuilderNewProject();
    

public:
    void init(int argc , char **argv);
    int dispose(int argc , char **agrv) override;
    int run();
    /**
     * @brief: 创建目录
     * @return: 无
     */
    fs::path createDir();
    /**
     * @brief: 将yaml文件放入到特定目录下
     * @return: 无
     */

    void createYaml();

    /**
     * @brief: 显示new_project的使用方法
     * @return: 无
     */

    void showUsage();
    /**
     * @brief: 检查参数
     * @return: true:参数正确，flase:参数错误
     */
    bool checkOptions(int argc,char **argv);

private:
    class Options;
    class Private;
    std::unique_ptr<Private>d;
    
    //kaiming-builder安装时将yaml文件部署的路径
    std::string m_yamlOriPath = YAML_TEMPLATE_PATH;   
   

};


#endif 