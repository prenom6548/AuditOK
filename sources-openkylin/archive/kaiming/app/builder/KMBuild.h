/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMBUILD_H__
#define __KMBUILD_H__

#include "common/KMSubCommand.h"

/**
 * @brief 	 组装沙箱运行环境及命令行参数信息，并在沙箱中运行命令
 * @note     供kaiming-builder构建流程中调用
 */
class KMBuild : public KMSubCommand
{
    DECLARE_DYNCREATE(KMBuild);

public:
    KMBuild();
    ~KMBuild() override;

    int dispose(int argc, char **argv) override;

private:
    void init(int argc, char **argv);
    int run();

    void setOptionEnvContext();

    void setupAppInfo();
    void setProcess();

    void cleanup() const;
    void parseOptionsToContext();

    static int getTerminalSize(int &rows, int &columns);

private:
    class Options;
    class Private;
    std::unique_ptr<Private> d;
};

#endif // !__KMBUILD_H__
