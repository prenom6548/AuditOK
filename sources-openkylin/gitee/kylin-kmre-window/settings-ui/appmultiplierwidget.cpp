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

#include "appmultiplierwidget.h"
#include "settingsgroup.h"
#include "switchbutton.h"
#include "switchwidget.h"
#include "utils.h"

#include <QDebug>
#include <QBoxLayout>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QFile>
#include <QStackedLayout>
#include <QMovie>

#include <unistd.h>
#include <sys/syslog.h>

AppMultiplierWidget::AppMultiplierWidget(QWidget *parent) :
    SettingScrollContent(parent)
    , m_systemBusInterface(new QDBusInterface("cn.kylinos.Kmre", "/cn/kylinos/Kmre", "cn.kylinos.Kmre", QDBusConnection::systemBus(), this))
    , m_managerBusInterface(new QDBusInterface("cn.kylinos.Kmre.Manager", "/cn/kylinos/Kmre/Manager", "cn.kylinos.Kmre.Manager", QDBusConnection::sessionBus(), this))
    , m_windowBusInterface(new QDBusInterface("cn.kylinos.Kmre.Window", "/cn/kylinos/Kmre/Window", "cn.kylinos.Kmre.Window", QDBusConnection::sessionBus(), this))
    , m_group(new SettingsGroup)
{
    //setTitleTip(tr("App Multiplier"));

    QFrame *centralWidget = new QFrame;
    m_tipFrame = new QFrame;
    m_mainFrame = new QFrame;
    m_switchframe = new QFrame;
    m_stackedLayout = new QStackedLayout;
    m_stackedLayout->setSpacing(0);
    m_stackedLayout->setMargin(0);

    m_multiplier = new QLabel();
    QMovie *multiplier = new QMovie(":/res/multiplier.gif");
    QSize s(240,135);
    multiplier->setScaledSize(s);
//    multiplier->setBackgroundColor("#3790FA");

    m_multiplier->setMovie(multiplier);
    //m_loading->setScaledContents(true);
    multiplier->start();
    m_multiplier->setFixedSize(245,140);
    m_multiplier->setStyleSheet("QLabel{border-radius:6px;border: 1px solid #707070;background:#E6E5E5;}");
    m_multiplier->setFocusPolicy(Qt::NoFocus);
    m_multiplier->setAlignment(Qt::AlignCenter);

    m_list = new QListWidget;
    m_list->setFixedSize(500,370);
    m_list->setAlternatingRowColors(true);
    m_list->setSelectionMode(QAbstractItemView::NoSelection);
    m_list->setObjectName("listframe");
    m_list->setStyleSheet("QFrame#listframe{background:palette(Base);border: 1px solid #E9E9E9;}");

    m_tipLabel = new QLabel();
    m_tipLabel->setStyleSheet("QLabel {color: #ff5a5a;}");
    m_tipLabel->setAlignment(Qt::AlignCenter);
    m_tipLabel->setText(tr("KMRE is not running!"));
    m_tipLabel->setVisible(false);

    m_titleLabel = new QLabel();
    m_titleLabel->setWordWrap(true);
    m_titleLabel->adjustSize();
    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_titleLabel->setText(tr("App Multiplier displays apps in dual windows. Not all apps are supported."));

    m_noResultLabel = new QLabel();
    m_noResultLabel->setWordWrap(true);
    m_noResultLabel->adjustSize();
    m_noResultLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_noResultLabel->setText(tr("No App is installed which supporting App Multiplier."));
    m_noResultLabel->setVisible(false);

    m_mainLabel = new QLabel();
    m_mainLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_mainLabel->setText(tr("App Multiplier"));
    m_mainAppMultiplierBtn = new SwitchButton();
    m_mainAppMultiplierBtn->setEnabled(false);
    m_mainAppMultiplierBtn->setChecked(false);
    m_switchframe->setObjectName("switchframe");
    m_switchframe->setFixedWidth(500);
    m_switchframe->setStyleSheet("QFrame{border-radius:6px;background:palette(Base);} QFrame#switchframe{border: 1px solid #E9E9E9;}");
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setMargin(10);
    hlayout->setSpacing(30);
    hlayout->addWidget(m_mainLabel, 0, Qt::AlignVCenter);
    hlayout->addStretch();
    hlayout->addWidget(m_mainAppMultiplierBtn, 0, Qt::AlignVCenter);
    m_switchframe->setLayout(hlayout);

    QVBoxLayout *tiplayout = new QVBoxLayout(m_tipFrame);
    tiplayout->setMargin(0);
    tiplayout->setSpacing(30);
    tiplayout->addWidget(m_tipLabel, 0, Qt::AlignHCenter);
    tiplayout->addStretch();

    QVBoxLayout *layout =new QVBoxLayout(m_mainFrame);
    this->m_title->setText(tr("App Multiplier"));
    QLabel *m_title = new QLabel;
    m_title->setText(tr("App Multiplier"));
    m_title->setStyleSheet("QLabel{font-size:18pt;font-style: normal;}");
//    layout->setContentsMargins(30,2,39,40);
//    layout->addWidget(m_title);
    layout->addSpacing(8);
    layout->addWidget(m_titleLabel);
    layout->addWidget(m_multiplier, 0, Qt::AlignCenter);
    layout->addWidget(m_switchframe);
    layout->addWidget(m_list);
    layout->addWidget(m_noResultLabel);
    layout->setMargin(0);

    m_stackedLayout->addWidget(m_tipFrame);
    m_stackedLayout->addWidget(m_mainFrame);
    m_stackedLayout->setCurrentWidget(m_mainFrame);
    centralWidget->setLayout(m_stackedLayout);

    this->setContent(centralWidget);

    connect(m_mainAppMultiplierBtn, &SwitchButton::checkedChanged, this, [=](const bool checked) {
        if (checked) {
            m_systemBusInterface->call("SetPropOfContainer", kmre::utils::getUserName(), (int32_t)getuid(), "persist.kmre.multiwindow", "true");
            m_mainMultiWindowStatus = "true";
        }
        else {
            m_systemBusInterface->call("SetPropOfContainer", kmre::utils::getUserName(), (int32_t)getuid(), "persist.kmre.multiwindow", "false");
            m_mainMultiWindowStatus = "false";
        }

        QList<SettingsItem *> items = m_list->findChildren<SettingsItem*>();
        for (SettingsItem *item : items) {
            //item->setEnabled((m_mainMultiWindowStatus == "true") ? true : false);
            SwitchWidget *switchBtn = qobject_cast<SwitchWidget *>(item);
            if (switchBtn) {
                switchBtn->setDisabledFlag((m_mainMultiWindowStatus == "true") ? false : true);
            }
        }
    });

    QObject::connect(m_windowBusInterface, SIGNAL(postAppMultipliers(QString)), this, SLOT(onAppMultipliers(QString)));
    QObject::connect(m_windowBusInterface, SIGNAL(postResponseInfo(int,QString,QString,int,QString)), this, SLOT(onPostResponseInfo(int,QString,QString,int,QString)));

    if (m_managerBusInterface) {
        m_managerBusInterface->call("controlApp", 0, "test", 17);//向android请求更新平行界面
    }
}

AppMultiplierWidget::~AppMultiplierWidget()
{
    m_installedAppsMap.clear();
    this->clearAllItems();

    if (m_systemBusInterface) {
        delete m_systemBusInterface;
        m_systemBusInterface = nullptr;
    }

    if (m_managerBusInterface) {
        delete m_managerBusInterface;
        m_managerBusInterface = nullptr;
    }

    if (m_windowBusInterface) {
        delete m_windowBusInterface;
        m_windowBusInterface = nullptr;
    }
}

void AppMultiplierWidget::initAppMultiplier()
{
    //判断kmre环境是否已经运行
    /*QDBusMessage response = m_systemBusInterface->call("GetPropOfContainer", kmre::utils::getUserName(), kmre::utils::getUid().toInt(), "sys.kmre.boot_completed");
    if (response.type() == QDBusMessage::ReplyMessage) {
        QString value = response.arguments().takeFirst().toString();
        if (value == "1") {
            m_stackedLayout->setCurrentWidget(m_mainFrame);
            m_noResultLabel->setVisible(false);
            m_group->setVisible(false);

            this->getMainMultiwindowStatus();
        }
        else {
            m_tipLabel->setVisible(true);
            m_stackedLayout->setCurrentWidget(m_tipFrame);
        }
    }
    else {
        m_tipLabel->setVisible(true);
        m_stackedLayout->setCurrentWidget(m_tipFrame);
    }*/

    QDBusPendingReply<QString> reply = m_systemBusInterface->asyncCall("GetPropOfContainer", kmre::utils::getUserName(), kmre::utils::getUid().toInt(), "sys.kmre.boot_completed");
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
            m_noResultLabel->setVisible(false);
            m_list->setVisible(false);

            this->getMainMultiwindowStatus();
        }
        else {
            m_tipLabel->setVisible(true);
            m_stackedLayout->setCurrentWidget(m_tipFrame);
        }

        this->getSubsMultiwindowStatus();
    });
}

void AppMultiplierWidget::getMainMultiwindowStatus()
{
    m_mainAppMultiplierBtn->setEnabled(false);

    //QDBusInterface interface("cn.kylinos.Kmre", "/cn/kylinos/Kmre", "cn.kylinos.Kmre", QDBusConnection::systemBus());
    QDBusPendingReply<QString> reply = m_systemBusInterface->asyncCall("GetPropOfContainer", kmre::utils::getUserName(), (int32_t)getuid(), "persist.kmre.multiwindow");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=] (QDBusPendingCallWatcher *w) {
        m_mainAppMultiplierBtn->setEnabled(true);

        m_mainMultiWindowStatus.clear();
        if (!w->isError()) {
            QDBusPendingReply<QString> reply = *w;// 无返回值时<>中无需填写类型,如: QDBusPendingReply<> r = *w;
            if (reply.isValid()) {
                m_mainMultiWindowStatus = reply.value();
            }
        }
        w->deleteLater();

        m_mainAppMultiplierBtn->blockSignals(true);
        if (m_mainMultiWindowStatus == "true") {
            m_mainAppMultiplierBtn->setChecked(true);
        }
        else {
            m_mainAppMultiplierBtn->setChecked(false);
        }
        m_mainAppMultiplierBtn->blockSignals(false);

        this->getSubsMultiwindowStatus();
    });
}

void AppMultiplierWidget::getSubsMultiwindowStatus()
{
    //获取已安装的安卓软件列表
    this->checkInstalledApps();

    //获取支持平行界面的软件列表及其状态
    QDBusPendingReply<QString> reply = m_windowBusInterface->asyncCall("getAppMultipliers");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=] (QDBusPendingCallWatcher *w) {
        if (!w->isError()) {
            QDBusPendingReply<QString> reply = *w;// 无返回值时<>中无需填写类型,如: QDBusPendingReply<> r = *w;
            if (reply.isValid()) {
                QString apps = reply.value();
                this->parseAppMultipliers(apps);
            }
        }
        w->deleteLater();
    });
}

void AppMultiplierWidget::onAppMultipliers(const QString &apps)
{
    //获取已安装的安卓软件列表
    this->checkInstalledApps();

    this->parseAppMultipliers(apps);
}

void AppMultiplierWidget::parseAppMultipliers(const QString &apps)
{
    if (m_installedAppsMap.count() == 0) {
        return;
    }

    this->clearAllItems();

    int number = 0;

    if (!apps.isEmpty() && !apps.isNull()) {
        QJsonParseError err;
        const QJsonDocument jsonDocment = QJsonDocument::fromJson(apps.toUtf8(), &err);
        if (err.error != QJsonParseError::NoError) {
            return;
        }
        if (jsonDocment.isNull() || jsonDocment.isEmpty()) {
            return;
        }
        if (jsonDocment.isObject()) {
            QJsonObject jsonObj = jsonDocment.object();

            if (jsonObj.contains("AppMultiplier")) {
                QJsonValue arrayVaule = jsonObj.value("AppMultiplier");
                if (arrayVaule.isArray()) {
                    QJsonArray array = arrayVaule.toArray();
                    for (int i = 0;i<array.size();i++) {
                        QJsonValue value = array.at(i);
                        QJsonObject child = value.toObject();
                        if (child.contains("packageName") && child.contains("multiplier")) {
                            QString key = child.value("packageName").toString();

                            if (m_installedAppsMap.keys().contains(key)) {
                                QString appName = m_installedAppsMap.value(key);
                                if (m_switchWidgets[key] == nullptr) {
                                    SettingsItem *item = nullptr;
                                    item = this->createSwitchWidget(key, appName, child.value("multiplier").toInt());
                                    if (item != nullptr) {
                                        m_switchWidgets[key] = item;
                                        m_list->addItem(item->getItem());
                                        item->getItem()->setSizeHint(QSize(490,50));
                                        m_list->setItemWidget(item->getItem(),item);
//                                        m_group->appendItem(item);
                                        number++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (number > 0) {
        m_noResultLabel->setVisible(false);
        m_list->setVisible(true);
        m_switchframe->setVisible(true);

        QList<SettingsItem *> items = m_list->findChildren<SettingsItem*>();
        for (SettingsItem *item : items) {
            //item->setEnabled((m_mainMultiWindowStatus == "true") ? true : false);
            SwitchWidget *switchBtn = qobject_cast<SwitchWidget *>(item);
            if (switchBtn) {
                switchBtn->setDisabledFlag((m_mainMultiWindowStatus == "true") ? false : true);
            }
        }
    }
    else {
        m_noResultLabel->setVisible(true);
        m_list->setVisible(false);
        m_switchframe->setVisible(false);
    }
}

SettingsItem *AppMultiplierWidget::createSwitchWidget(const QString &packageName, const QString &appName, int multiplier)
{
    if (packageName.isEmpty() || packageName.isNull()) {
        return nullptr;
    }

    SwitchWidget *w = new SwitchWidget;
    w->setFixedHeight(50);
    QString iconPath = kmre::utils::getIconPath(packageName);
    if (!QFile::exists(iconPath)) {
        w->setIcon(QPixmap(":/res/kmre.svg"));
    }
    else {
        w->setIcon(QPixmap(iconPath));
    }
    w->setName(packageName);
    w->setTitle(appName);
    if (multiplier == 1) {
        w->setChecked(true);
    }
    else {
        w->setChecked(false);
    }
    connect(w, &SwitchWidget::checkedChanged, [=](const bool checked) {
        if (m_managerBusInterface) {
            if (checked) {
                m_managerBusInterface->call("controlApp", 0, packageName, 15);//开启平行界面
            }
            else {
                m_managerBusInterface->call("controlApp", 0, packageName, 16);//关闭平行界面
            }
        }
    });

    return w;
}

void AppMultiplierWidget::clearAllItems()
{
    for (auto *w : m_switchWidgets) {
        if (w) {
            w->deleteLater();
        }
    }
    m_switchWidgets.clear();
    m_list->clear();

//    QList<SettingsItem *> items = m_group->findChildren<SettingsItem*>();
//    for (SettingsItem *item : items) {
//        m_group->removeItem(item);
//        item->deleteLater();
//    }
}

//获取已安装的安卓软件列表
void AppMultiplierWidget::checkInstalledApps()
{
    m_installedAppsMap.clear();

    auto appList = kmre::utils::getInstalledAppList();
    for (const auto& app : appList) {
        m_installedAppsMap[app.first] = app.second;
    }
}

void AppMultiplierWidget::onPostResponseInfo(int id, const QString &pkgName, const QString &category, int ret, const QString &info)
{
    Q_UNUSED(id);
    Q_UNUSED(info);

    if (pkgName.isNull() || pkgName.isEmpty()) {
        return;
    }
    if (category.isNull() || category.isEmpty()) {
        return;
    }
    if (category == "app_multiplier") {
        QList<SettingsItem *> items = m_list->findChildren<SettingsItem*>();
        for (SettingsItem *item : items) {
            SwitchWidget *switchBtn = qobject_cast<SwitchWidget *>(item);
            if (switchBtn) {
                if (switchBtn->name() == pkgName) {
                    switchBtn->setChecked((ret == 1) ? true: false);
                    break;
                }
            }
        }
    }
}
