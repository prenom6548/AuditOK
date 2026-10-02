/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMAPPLICATION_H__
#define __KMAPPLICATION_H__

#include <memory>

#include "common/KMSingleton.h"

/**
 * @brief 	 app框架类
 */
class KMApplication : public KMSingleton<KMApplication>
{
public:
    KMApplication(token, int argc, char **argv);
    ~KMApplication();

    void init();
    int exec();

    /**
      * @brief 处理应用程序调用情况
      * @param fileName 应用程序文件名
      */
    void specialHandle(const std::string &fileName);

private:
    class Options;
    class Private;
    std::unique_ptr<Private> d;
};

#endif // !__KMAPPLICATION_H__
