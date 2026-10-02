/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMPS_H__
#define __KMPS_H__

#include <optional>

#include "common/KMSubCommand.h"
#include "common/KMBuildinOptions.h"
#include "common/KMConfigData.h"

/**
 * @brief ``kaiming ps`` 命令可以查看运行中的应用。
 */
class KMPS : public KMSubCommand
{
    DECLARE_DYNCREATE(KMPS);

public:
    KMPS();
    ~KMPS() override;

    /**
     * @brief 开明子命令入口函数
     * @param argc 参数个数
     * @param argv 参数列表
     * @return 0 成功 非0 失败
     */
    int dispose(int argc, char **argv) override;

    /**
     * @brief 查询开明进程信息
     */
    bool ps();

private:
    /**
     * @brief 打印信息
     * @param psInfos 需要打印的信息
     */
    void printAppInfo(const KMPSInfoList &psInfos) const;

private:
    class Options;
    class Private;
    std::shared_ptr<Private> d_ptr;
};

#endif // __KMPS_H__
