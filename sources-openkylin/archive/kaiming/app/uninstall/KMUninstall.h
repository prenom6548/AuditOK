/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMUNINSTALLER_H__
#define __KMUNINSTALLER_H__

#include <memory>
#include <vector>

#include "common/KMSubCommand.h"
#include "common/KMBuildinOptions.h"
#include "common/KMPackageManager.h"


/**
 * @brief ``kaiming uninstall`` 命令可以用来卸载开明应用。
 */
class KMUninstall : public KMSubCommand
{
    DECLARE_DYNCREATE(KMUninstall);

public:
    KMUninstall();
    ~KMUninstall() override;

    int dispose(int argc, char **argv) override;
    int process(const std::vector<std::string> &vec_name);
    void init();

private:
    class Options;
    class Private;
    shared_ptr<Private> m_dPtr;

};

#endif // !__KMUNINSTALLER_H__