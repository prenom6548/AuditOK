/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMOABREPACKAGE_H__
#define __KMOABREPACKAGE_H__

#include <string>
#include <memory>

#include "common/KMSubCommand.h"

class KMOABRepackage : public KMSubCommand
{
    DECLARE_DYNCREATE(KMOABRepackage);

public:
    explicit KMOABRepackage();
    virtual ~KMOABRepackage();

    int dispose(int argc, char **argv) override;

private:
    void init(int argc, char **argv);
    int run();

private:
    class Options;
    class Private;
    std::unique_ptr<Private> d;
};

#endif // !__KMOABREPACKAGE_H__
