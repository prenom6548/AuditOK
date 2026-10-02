/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMOABAPPLICATION_H__
#define __KMOABAPPLICATION_H__

#include <memory>

#include "common/KMSingleton.h"

/**
 * @brief : OK框架类
 */
class KMOABApplication : public KMSingleton<KMOABApplication>
{
public:
    KMOABApplication(token, int argc, char **argv);
    ~KMOABApplication();

    void reset(int argc, char **argv);

    void init();
    int exec();

private:
    class Options;
    class Private;
    std::unique_ptr<Private> d;
};

#endif // !__KMOABAPPLICATION_H__
