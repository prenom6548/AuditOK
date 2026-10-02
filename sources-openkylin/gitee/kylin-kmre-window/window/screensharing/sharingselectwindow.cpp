/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Alan Xie    xiehuijun@kylinos.cn
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

#include "sharingselectwindow.h"
#include "screensharing.h"
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include "syslog.h"

SharingSelectWindow::SharingSelectWindow(int num, QWidget *parent)
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("sharing screen"));

    QVBoxLayout *layout = new QVBoxLayout();

    QLabel *messageLbl = new QLabel(this);
    messageLbl->setText(tr("Please select sharing screen:"));
    messageLbl->setWordWrap(true);

    layout->addWidget(messageLbl, 0, Qt::AlignCenter);

    for (int i = 0; i < num; ++i) {
        QPushButton *screenBtn = new QPushButton(this);
        screenBtn->setText(QString(tr("Screen")) + QString::number(i + 1));
        screenBtn->setFixedSize(280, 60);

        connect(screenBtn, &QPushButton::clicked, this, [this, i] {
            syslog(LOG_DEBUG, "Select sharing screen '%d'", i);
            ScreenSharing::getInstance()->setSharingScreenNum(i);
            ScreenSharing::getInstance()->setSharingStatus(SharingStatus::eReady);
            this->close();
        });

        layout->addWidget(screenBtn, 0, Qt::AlignCenter);
    }

    setLayout(layout);
    setFixedWidth(300);
}

SharingSelectWindow::~SharingSelectWindow()
{
    syslog(LOG_DEBUG, "Delete sharing select window");
}

void SharingSelectWindow::closeEvent(QCloseEvent *event)
{
    ScreenSharing* screenSharing = ScreenSharing::getInstance();
    if (!screenSharing->isSharingReady()) {
        screenSharing->setSharingScreenNum(0);// set default value
        screenSharing->setSharingStatus(SharingStatus::eReady);
    }
    
    QWidget::closeEvent(event);
}