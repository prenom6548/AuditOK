/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMBUILDEXPORT_H__
#define __KMBUILDEXPORT_H__

#include "common/KMSubCommand.h"

/**
 * @brief 	 将构建结果导出成离线包
 * @note	 供kaiming-builder构建流程中调用
 */
class KMBuildExport : public KMSubCommand
{
    DECLARE_DYNCREATE(KMBuildExport);

public:
    KMBuildExport();
    ~KMBuildExport() override;

    int dispose(int argc, char **argv) override;
    
private:
    void init(int argc, char **argv);
    int run();

    void parseSubpackagesJson();

    void generateSubpackages();

    void generatePackagesByModules();

private:
    class Options;
    class Private;
    std::unique_ptr<Private> d;
};

#endif // !__KMBUILDEXPORT_H__
