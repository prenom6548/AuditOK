/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "gestureguidance.h"
#include "ui_gestureguidance.h"
#include "itemwidget.h"
#include "flowlayout.h"

#include <QSettings>

GestureGuidance::GestureGuidance(QWidget* parent) : QWidget(parent),
                                                    ui(new Ui::GestureGuidance)
{
    setAttribute(Qt::WA_StyledBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setProperty("useStyleWindowManager", false);

    ui->setupUi(this);
    ui->label->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");

    gifFlowLayout = new TouchScreenSetting::FlowLayout(ui->frame, 0, 20, 20);
    gifFlowLayout->setContentsMargins(0, 0, 0, 0);
    gifFlowLayout->setSpacing(100);
    ui->frame->setLayout(gifFlowLayout);

    QString path = ":/intelGif/resources/intelRes/gesture.ini";
    gesturesetting = new QSettings(path, QSettings::IniFormat);

    addItem();
}

void GestureGuidance::addItem()
{
    gesturesetting->beginGroup("gesture");
    QStringList giflist = gesturesetting->allKeys();
    QMap<QString, int> list;
    for (int i = 0; i < giflist.count(); i++) {
        list.insert(giflist.at(i), gesturesetting->value(giflist.at(i)).toInt());
    }
    gesturesetting->sync();
    gesturesetting->endGroup();
    for(int i = 0; i < giflist.count(); i++) {
        ItemWidget* item = new ItemWidget(list.key(i), ui->frame);
        connect(item, &ItemWidget::picClicked, this, [=](){
            if (m_currentItem == item)
                return;
            if (m_currentItem != nullptr) {
                m_currentItem->stopMovieSlot();
            }
            m_currentItem = item;
            item->startMovieSlot();
        });
        gifFlowLayout->addWidget(item);
    }
}
