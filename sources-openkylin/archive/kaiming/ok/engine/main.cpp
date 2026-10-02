/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMOABApplication.h"

int main(int argc, char **argv)
{
    KMOABApplication &a = KMOABApplication::instance(argc, argv);
    a.init();

    return a.exec();
}
