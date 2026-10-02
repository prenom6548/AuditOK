/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn/kobe24_lixiang@126.com
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

#include "dockeripwidget.h"
#include "settingsgroup.h"
#include "ipitem.h"
#include "netmaskitem.h"
#include "global.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QFile>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QDBusInterface>
#include <QtDBus>
#include <QMessageBox>

using namespace Global;
//TODO: write /etc/docker/daemon.json with root
QPair<QString, bool> setIpAndNetmask(const QString &str)
{
    bool ret = false;

    QFile file(QDir::homePath() + "/1.json");
    if (file.open(QIODevice::ReadWrite)) {
        QJsonObject jsonObject;
        jsonObject.insert("bip", str);
        QJsonDocument jsonDoc;
        jsonDoc.setObject(jsonObject);
        file.write(jsonDoc.toJson());
        file.close();
        ret = true;
    }

    return QPair<QString, bool>(str, ret);
}

DockerIpWidget::DockerIpWidget(QWidget * parent)
    : SettingScrollContent(parent)
    , m_group(new SettingsGroup)
    , m_ipItem(new IpItem)
    , m_netmaskItem(new NetMaskItem)
    , m_okBtn(new QPushButton(this))
    , m_cancleBtn(new QPushButton(this))
    , m_rebootBtn(new QPushButton(this))
{
    //setTitleTip(tr("Default IP address of docker(requires restart)"));
    //setTitleTip(tr("When the default IP address of docker is set in the same network segment as the local IP, kmre in the container will not be able to connect to the Internet. The docker IP needs to be modified, and the system needs to be restarted after modification"));
    m_okBtn->setText(tr("OK"));
    m_okBtn->setFixedSize(120, 36);
    m_okBtn->setFocusPolicy(Qt::NoFocus);

    m_cancleBtn->setText(tr("Cancle"));
    m_cancleBtn->setFixedSize(120,36);
    m_cancleBtn->setFocusPolicy(Qt::NoFocus);

    m_rebootBtn->setText(tr("Restart system"));
    m_rebootBtn->setFixedSize(120, 36);
    m_rebootBtn->setFocusPolicy(Qt::NoFocus);
    m_rebootBtn->setEnabled(false);

    m_ipItem->setTitle(tr("IP"));
    m_netmaskItem->setTitle(tr("subnet mask"));
    m_group->setStyleSheet("QFrame{background:Pallete(Base);}");
    m_group->setAttribute(Qt::WA_TranslucentBackground, true);
    m_group->appendItem(m_ipItem);
    m_group->appendItem(m_netmaskItem);
    //m_group->setStyleSheet("QWidget{border-radius:6px;background:palette(Window);}");

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setMargin(0);
    hlayout->setSpacing(30);
    hlayout->addStretch();
    hlayout->addWidget(m_okBtn, 0, Qt::AlignVCenter);
    hlayout->addWidget(m_cancleBtn, 0, Qt::AlignVCenter);
    hlayout->addStretch();

    QFrame* frame = new QFrame;
    QVBoxLayout *layout =new QVBoxLayout();
    QLabel *m_title = new QLabel;
    m_title->setText(tr("Default IP address of docker morn"));
    m_title->setStyleSheet("QLabel{font-size:18pt;font-style: normal;}");
    QLabel *restart = new QLabel;
    restart->setText(tr("After the IP address is changed, you need to restart the system for the change to take effect"));
    QPushButton *line = new QPushButton;
    line->setFocusPolicy(Qt::NoFocus);
    line->setFixedSize(520,1);
    layout->setContentsMargins(30,2,39,40);
    layout->addWidget(m_title);
    layout->setSpacing(10);
    //layout->setMargin(10);
    layout->addWidget(m_group);
    layout->addSpacing(10);
    layout->addLayout(hlayout);
    layout->addSpacing(25);
    layout->addWidget(line);
    layout->addSpacing(25);
    layout->addWidget(restart);
    layout->addWidget(m_rebootBtn,0, Qt::AlignLeft);


    frame->setLayout(layout);
    this->setContent(frame);

//    connect(m_okBtn, &QPushButton::clicked, this, [=] () {
//    });
    connect(m_cancleBtn, &QPushButton::clicked, m_ipItem, &IpItem::clear);
    connect(m_cancleBtn, &QPushButton::clicked, m_netmaskItem, &NetMaskItem::clear);
    connect(m_rebootBtn, &QPushButton::clicked, this, [=]() {
    });
}

DockerIpWidget::~DockerIpWidget()
{
    m_ipItem->deleteLater();
    m_netmaskItem->deleteLater();
}
