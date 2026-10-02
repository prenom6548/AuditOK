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
#include "autobootui.h"
#include <QVBoxLayout>
#include <kysdk/applications/accessinfohelper.h>

using namespace kdk;

AutoBootUi::AutoBootUi(QWidget *parent) : QWidget(parent)
{
    initUi();
}

void AutoBootUi::initUi()
{
    QVBoxLayout *vLyt = new QVBoxLayout(this);
    vLyt->setSpacing(8);
    vLyt->setContentsMargins(0, 0, 0, 0);

    KLabel *mTitleLabel = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mTitleLabel, "Autoboot", "", "");
    mTitleLabel->setContentsMargins(16, 0, 0, 0);
    //~ contents_path /Autostart/Autostart Settings
    mTitleLabel->setText(tr("Autostart Settings"));

    mAutobootWidget = new SettingGroup(this);

    vLyt->addWidget(mTitleLabel);
    vLyt->addWidget(mAutobootWidget);
    vLyt->addStretch();
}

void AutoBootUi::ensureAddButton()
{
    // 须在 initItem 之后调用：早建会导致「添加」排在焦点链最前
    if (mAddBtn || !mAutobootWidget) {
        return;
    }

    mAddWidget = new UkccFrame(mAutobootWidget);
    mAddWidget->setLineWidth(0);
    QHBoxLayout *lyt = new QHBoxLayout(mAddWidget);
    lyt->setContentsMargins(0, 0, 0, 0);
    mAddBtn = new AddButton(mAddWidget);
    KDK_EXTEND_ALL_INFO_FORMAT(mAddBtn, "Autoboot", "", "");
    //~ contents_path /Autostart/Add
    mAddBtn->setAccessibleName(tr("Add"));
    lyt->addWidget(mAddBtn);
}

void AutoBootUi::resetUi()
{
    if (this->layout() != NULL) {
        QLayoutItem *item;
        while ((item = this->layout()->takeAt(0)) != NULL)
        {
            if(item->widget()) {
               item->widget()->setParent(NULL);
            }
            delete item;
            item = nullptr;
        }
    }
    delete this->layout();
    // 旧控件已脱离父子关系，清空指针以便 ensureAddButton 重新创建
    mAutobootWidget = nullptr;
    mAddWidget = nullptr;
    mAddBtn = nullptr;
    initUi();
}
