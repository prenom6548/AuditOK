/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMEXECCOMMAND_H__
#define __KMEXECCOMMAND_H__

#include "KMSubCommand.h"

class KMExecCommand : public KMSubCommand
{
    DECLARE_DYNCREATE(KMExecCommand);

public:
    KMExecCommand();
    virtual ~KMExecCommand();

int dispose(int argc, char **argv) override;

private:
    void init(int argc, char **argv);

    int run();

private:
    class Options;
    class Private;
    std::unique_ptr<Private> d;
};

#endif // !__KMEXECCOMMAND_H__
