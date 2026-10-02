/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMApplication.h"

int main(int argc, char **argv)
{
    KMApplication &a = KMApplication::instance(argc, argv);
    a.init();

    return a.exec();
}
