/*
 * KWin Style UKUI
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#ifndef FILEDIALOGINTERFACE_H
#define FILEDIALOGINTERFACE_H
#include "qpa/qplatformdialoghelper.h"

//定义接口
class FileDialogInterface
{
public:
    virtual ~FileDialogInterface() {}
    virtual QPlatformDialogHelper* create() = 0;
};

//一定是唯一的标识符
#define FileDialogInterface_iid "UKUIPlatformTheme.Plugin.FileDialogInterface"

Q_DECLARE_INTERFACE(FileDialogInterface, FileDialogInterface_iid)

#endif // FILEDIALOGINTERFACE_H
