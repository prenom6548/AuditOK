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

#include "warningnotice.h"
#include "kmrewindow.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDBusReply>
#include <QtDBus>
#include <QDBusInterface>
#include <sys/syslog.h>

WarningNotice::WarningNotice(const QString& warnMsg, KmreWindow* parent)
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint)
{
    setWindowTitle(tr("note"));

    QLabel *iconLbl = new QLabel(this);
    iconLbl->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(36, 36));

    QLabel *messageLbl = new QLabel(warnMsg, this);
    messageLbl->setWordWrap(true);

    QHBoxLayout *labelLayout = new QHBoxLayout();
    labelLayout->addWidget(iconLbl);
    labelLayout->addWidget(messageLbl);

    QPushButton *ignoreBtn = new QPushButton(tr("Don't remind anymore"), this);
    QString pkgName = parent->getPackageName();
    connect(ignoreBtn, &QPushButton::clicked, this, [this, pkgName] {
        QDBusInterface interface("cn.kylinos.Kmre.Pref", "/cn/kylinos/Kmre/Pref", "cn.kylinos.Kmre.Pref", QDBusConnection::systemBus());
        QDBusReply<bool> reply = interface.call("removeAppFromWarningList", pkgName);
        if (!(reply.isValid() && reply.value())) {
            syslog(LOG_ERR, "remove app from warning list failed!");
        }
        this->close();
    });

    QPushButton *closeBtn = new QPushButton(tr("close"), this);
    connect(closeBtn, &QPushButton::clicked, this, [this] {
        //this->close();
        this->hide();
    });

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(ignoreBtn);
    btnLayout->addWidget(closeBtn);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(4, 10, 4, 10);
    layout->addLayout(labelLayout);
    layout->addLayout(btnLayout);

    setLayout(layout);
}

WarningNotice::~WarningNotice()
{
    syslog(LOG_DEBUG, "Delete warning notice dialog");
}
