/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Zero Liu    liuzenghui1@kylinos.cn
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

#ifndef MOUSEFIREKEY_H
#define MOUSEFIREKEY_H

#include "joysticksinglebasekey.h"

class MouseFireKey : public JoystickSingleBaseKey
{
    Q_OBJECT
public:
    MouseFireKey(KmreWindow* window);

    void enableEdit(bool enable) Q_DECL_OVERRIDE;
    void updateSize(bool update_panel = true) Q_DECL_OVERRIDE;

protected:
    void getTranslateCenterPos(QPoint pos) Q_DECL_OVERRIDE;

signals:

};

#endif // MOUSEFIREKEY_H
