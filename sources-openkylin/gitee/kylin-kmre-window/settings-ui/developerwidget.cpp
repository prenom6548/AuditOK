/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
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

#include "developerwidget.h"
#include "utils.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QDBusInterface>
#include <QDBusReply>
#include <QTimer>
#include <unistd.h>
#include <QJsonDocument>
#include <QJsonObject>
DeveloperWidget::DeveloperWidget(QWidget * parent)
    : SettingScrollContent(parent)
{
    QDBusInterface interface("cn.kylinos.Kmre.Manager", "/cn/kylinos/Kmre/Manager", "cn.kylinos.Kmre.Manager", QDBusConnection::sessionBus());
    QFrame *centralWidget = new QFrame;
    m_tipFrame = new QFrame;
    m_mainFrame = new QFrame;
    m_stackedLayout = new QStackedLayout;
    m_stackedLayout->setSpacing(0);
    m_stackedLayout->setMargin(0);
    this->m_title->setText(tr("Developer Mode"));
    m_tip = new QLabel;
    m_check = new QCheckBox;
    m_check->setText(tr("open developer mode"));
    m_ipadr = getContainerIp();
    m_tip->setWordWrap(true);
    m_tip->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_tip->setText(tr("Please connect to the KMRE environment via adb connect %1 or open the help documentation if in doubt").arg(m_ipadr));
    QDBusReply<QString> reply = interface.call("getSystemProp", 1, "debug_mode");
    if (reply == "1") {
        m_check->setChecked(true);
    }
    m_tip->setVisible(m_check->isChecked());
//    m_armtip = new QLabel;
//    m_armtip->setWordWrap(true);
//    m_armtip->setTextInteractionFlags(Qt::TextSelectableByMouse);
//    m_armtip->setText(tr("Please enter the following command on the terminal to install dependency:%1").arg("1)sudo dpkg --add-architecture amd64;2)sudo apt update;3)sudo apt-get install -f libstdc++6:amd64;"));
//    m_armtip->setVisible(false);
//#if defined(__aarch64__)
//    m_armtip->setVisible(m_check->isChecked());
//#endif
    m_tipLabel = new QLabel();
    m_tipLabel->setStyleSheet("QLabel {color: #ff5a5a;}");
    m_tipLabel->setAlignment(Qt::AlignCenter);
    m_tipLabel->setText(tr("KMRE is not running!"));

    QVBoxLayout *m_layout = new QVBoxLayout(m_mainFrame);
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    m_layout->addSpacing(8);
    m_layout->addWidget(m_check);
    m_layout->addSpacing(16);
//#if defined(__aarch64__)
//    m_layout->addWidget(m_armtip);
//#endif
    m_layout->addWidget(m_tip);
    m_layout->addStretch();

    QVBoxLayout *m_tiplayout = new QVBoxLayout(m_tipFrame);
    m_tiplayout->setSpacing(0);
    m_tiplayout->setMargin(0);
    m_tiplayout->addSpacing(8);
    m_tiplayout->addWidget(m_tipLabel, 0, Qt::AlignHCenter);
    m_tiplayout->addStretch();

    connect(m_check, &QCheckBox::toggled, this, &DeveloperWidget::modecheck);
    connect(m_check, &QCheckBox::clicked, this, &DeveloperWidget::setClicktime);

    m_stackedLayout->addWidget(m_tipFrame);
    m_stackedLayout->addWidget(m_mainFrame);
    m_stackedLayout->setCurrentWidget(m_mainFrame);
    centralWidget->setLayout(m_stackedLayout);
    this->setContent(centralWidget);
}

void DeveloperWidget::initDatas()
{
    //判断kmre环境是否已经运行
    QDBusInterface interface("cn.kylinos.Kmre", "/cn/kylinos/Kmre", "cn.kylinos.Kmre", QDBusConnection::systemBus());
    QDBusPendingReply<QString> reply = interface.asyncCall("GetPropOfContainer", kmre::utils::getUserName(), kmre::utils::getUid().toInt(), "sys.kmre.boot_completed");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=] (QDBusPendingCallWatcher *w) {
        QString value;
        if (!w->isError()) {
            QDBusPendingReply<QString> reply = *w;// 无返回值时<>中无需填写类型,如: QDBusPendingReply<> r = *w;
            if (reply.isValid()) {
                value = reply.value();
            }
        }
        w->deleteLater();

        if (value == "1") {
            m_stackedLayout->setCurrentWidget(m_mainFrame);
        }
        else {
            m_stackedLayout->setCurrentWidget(m_tipFrame);
        }
    });
}

void DeveloperWidget::modecheck(bool checked)
{
    int type = 1;
    QDBusInterface interface("cn.kylinos.Kmre.Manager", "/cn/kylinos/Kmre/Manager", "cn.kylinos.Kmre.Manager", QDBusConnection::sessionBus());
    if (checked) {
        interface.call("setSystemProp", type, "debug_mode", "1");
        m_tip->setVisible(true);
    }
    else {
        interface.call("setSystemProp", type, "debug_mode", "0");
        m_tip->setVisible(false);
    }
}

void DeveloperWidget::setClicktime()
{
    m_check->setEnabled(false);
    QTimer *timer = new QTimer;
    timer->start(1000);
    connect(timer, &QTimer::timeout, this, [=]() {
        timer->stop();
        m_check->setEnabled(true);
    });
}

QString DeveloperWidget::getContainerIp()
{
    QDBusInterface interface("cn.kylinos.Kmre", "/cn/kylinos/Kmre", "cn.kylinos.Kmre", QDBusConnection::systemBus());
    QDBusReply<QString> reply = interface.call("GetContainerNetworkInformation", kmre::utils::getUserName(),(int32_t)getuid());
    QString replystr = reply.value();
    const QJsonDocument jsonDocument = QJsonDocument::fromJson(replystr.toUtf8());
    QJsonObject obj = jsonDocument.object();
    if (obj.contains("IPAddress")) {
        QString ipstr = obj.value("IPAddress").toString();
        return ipstr;
    }
    return "128.128.0.2";
}
