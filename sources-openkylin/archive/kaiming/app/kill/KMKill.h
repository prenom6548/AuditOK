/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMKILL_H__
#define __KMKILL_H__

#include <memory>

#include "common/KMSubCommand.h"
#include "common/KMBuildinOptions.h"
#include <nlohmann/json.hpp>
#include "common/KMConfigData.h"

/**
 * @brief kill command
 */
class KMKill : public KMSubCommand
{
    DECLARE_DYNCREATE(KMKill);

public:
    KMKill();
    ~KMKill() override;

    /**
     * @brief 开明子命令入口函数
     * @param argc 参数个数
     * @param argv 参数列表
     * @return 0 成功 非0 失败
     */
    int dispose(int argc, char **argv) override;

private:
    /**
     * @brief 调用dbus接口杀死进程
     * @param id 应用名
     */
    bool kill(std::string id);

private:
    class Options;
    class Private;
    std::shared_ptr<Private> m_dPtr;
};

#endif // __KMKILL_H__