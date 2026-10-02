/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef AUTOBOOTUI_H
#define AUTOBOOTUI_H

#include <QObject>
#include <QWidget>

#include "hoverwidget.h"
#include "addbutton.h"
#include "switchwidget.h"
#include "settinggroup.h"
#include "ukccframe.h"
#include <klabel.h>

using namespace kdk;

class AutoBootUi : public QWidget
{
    Q_OBJECT
public:
    explicit AutoBootUi(QWidget *parent = nullptr);
    void initUi();
    void resetUi();
    // 在列表项创建之后再构建「添加」，使 Qt 默认 Tab 序与视觉顺序一致
    void ensureAddButton();

    SettingGroup *getAutobootWidget()
    {
        return mAutobootWidget;
    }

    UkccFrame *getAddWidget()
    {
        return mAddWidget;
    }

    AddButton *getAddBtn()
    {
        return mAddBtn;
    }

private:
    SettingGroup *mAutobootWidget = nullptr;
    UkccFrame *mAddWidget = nullptr;
    AddButton *mAddBtn = nullptr;

};

#endif // AUTOBOOTUI_H
