/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EMUGL_DISPLAY_WORK_MANAGER_H
#define EMUGL_DISPLAY_WORK_MANAGER_H

#include <QObject>

#include "displayfrontend/displayworkmanager.h"
#include "emugl_display_helper.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/syslog.h>

#include <string>
#include <pwd.h>
class EmuGLDisplayWorkManager : public DisplayWorkManager
{
public:
    explicit EmuGLDisplayWorkManager(DisplayWidget* widget, QObject *parent = nullptr);
    ~EmuGLDisplayWorkManager();

    virtual void initialize() override;
    virtual void enableDisplayUpdate(bool enable) override;

protected slots:

private:
    int emugl_connect_fd;

    int connect_emugl_init();
};

#endif // EMUGL_DISPLAY_WORK_MANAGER_H
