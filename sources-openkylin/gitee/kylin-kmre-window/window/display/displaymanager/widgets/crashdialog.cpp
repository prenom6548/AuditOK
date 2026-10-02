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

#include "crashdialog.h"
#include <QGridLayout>
#include <QLabel>
#include <QIcon>
#include <QPainter>
#include <syslog.h>

CrashDialog::CrashDialog(QWidget *parent)
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint)
    , mRestartBtn(new QPushButton(tr("restart"), this))
    , mCancelBtn(new QPushButton(tr("cancel"), this))
{
    setWindowTitle(tr("Error"));

    QIcon icon(":/res/error.png");
    QLabel *crashIcon = new QLabel(this);
    crashIcon->setFixedSize(42, 42);
    crashIcon->setPixmap(icon.pixmap(crashIcon->size()));
    QLabel *crashMessage = new QLabel(tr("App stopped unexpectly, restart it now ?"), this);
    crashMessage->setWordWrap(true);
    connect(mRestartBtn, &QPushButton::clicked, this, [&] {emit sigCrashRestart(true);});
    connect(mCancelBtn, &QPushButton::clicked, this, [&] {emit sigCrashRestart(false);});

    QGridLayout *layout = new QGridLayout();
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(crashIcon, 0, 0, 2, 1, Qt::AlignCenter);
    layout->addWidget(crashMessage, 0, 1, 2, 5, Qt::AlignLeft);
    layout->addWidget(mRestartBtn, 2, 0, 1, 3, Qt::AlignLeft);
    layout->addWidget(mCancelBtn, 2, 3, 1, 3, Qt::AlignRight);

    setLayout(layout);
    setFixedSize(240, 120);
}

CrashDialog::~CrashDialog()
{
}

void CrashDialog::closeEvent(QCloseEvent *event)
{
    emit sigCrashRestart(false);
    
    QWidget::closeEvent(event);
}