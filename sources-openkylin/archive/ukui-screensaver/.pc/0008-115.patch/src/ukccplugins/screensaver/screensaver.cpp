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
#include "screensaver.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QTranslator>
#include <QLocale>
#include <QApplication>

#define PREENTRY_TIME "preentryTime"
#define SCREENSAVER_TYPE "screensaverType"
#define SHOWTIME_UKUI "showBreakTimeUkui"
#define SHOWTIME_CUSTOM "showBreakTimeCustom"
#define SCREENLOCK_ENABLED "screenLockEnabled"
#define CUSTOM_PATH "customPath"
#define SWITCH_RANDOW "switchRandom"
#define CYCLE_TIME "cycleTime"
#define CUSTOM_TEXT "customText"
#define CUSTOM_TEXT_CENTERED "customTextCentered"

Screensaver::Screensaver() : mFirstLoad(true)
{
    QTranslator *translator = new QTranslator(this);
    translator->load("/usr/share/ukui-screensaver/ukccplugins/i18n_qm/" + QLocale::system().name());
    QApplication::installTranslator(translator);

    pluginName = tr("Screensaver");
    pluginType = PERSONALIZED;
    QtConcurrent::run([=] { preInitialize(); });
}

Screensaver::~Screensaver()
{
    if (!mFirstLoad) {
        screensaverUi->deleteLater();
        screensaverUi = nullptr;
    }
}

QString Screensaver::plugini18nName()
{
    return pluginName;
}

int Screensaver::pluginTypes()
{
    return pluginType;
}

void Screensaver::preInitialize()
{
    screensaverInterface = new QDBusInterface(
        "org.ukui.ScreenSaver",
        "/Screensaver",
        "org.ukui.ScreenSaver.session.Screensaver",
        QDBusConnection::sessionBus(),
        this);
    if (!screensaverInterface->isValid()) {
        qCritical() << "org.ukui.ScreenSaver.session.ScreenSaver DBus error:" << screensaverInterface->lastError();
        return;
    }
    QDBusMessage message = screensaverInterface->call("ping"); // 判断dbus路径是否存在
    if (message.type() == QDBusMessage::ErrorMessage
        && message.errorMessage().contains("No such object path", Qt::CaseInsensitive)) {
        qCritical() << screensaverInterface << ":" << message.errorMessage();
        return;
    }

    QPluginLoader pluginLoader("/usr/lib/ukui-screensaver/libscreensaver-default.so");
    pluginLoader.load();
    QObject *plugin = pluginLoader.instance();
    screensaverPtr = nullptr;
    if (plugin) {
        screensaverPtr = std::unique_ptr<ScreensaverPlugin>(qobject_cast<ScreensaverPlugin *>(plugin));
    } else {
        qWarning() << "pluginLoader '/usr/lib/ukui-screensaver/libscreensaver-default.so' failed";
    }
}

QWidget *Screensaver::pluginUi()
{
    if (mFirstLoad) {
        mFirstLoad = false;
        screensaverUi = new ScreensaverUi;
        initOptions();
        initContent();
        connectUiSignals();
        connectToServer();
    }
    return screensaverUi;
}

const QString Screensaver::name() const
{
    return QStringLiteral("Screensaver");
}

void Screensaver::plugin_leave()
{
    return;
}

bool Screensaver::isShowOnHomePage() const
{
    return true;
}

QIcon Screensaver::icon() const
{
    return QIcon::fromTheme("ukui-screensaver-symbolic");
}

bool Screensaver::isEnable() const
{
    return true;
}

void Screensaver::dataChanged(const QString &key)
{
    if (key != toChangeKey) {
        if (key == PREENTRY_TIME) {
            initIdleTime();
        } else if (key == SCREENSAVER_TYPE) {
            initScreensaver();
        } else if (key == CUSTOM_PATH) {
            initCustomSource();
        } else if (key == SWITCH_RANDOW) {
            initSwicthMode();
        } else if (key == CYCLE_TIME) {
            initSwitchTime();
        } else if (key == CUSTOM_TEXT) {
            initCustomText();
        } else if (key == CUSTOM_TEXT_CENTERED) {
            initTextPosition();
        } else if (key == SHOWTIME_UKUI) {
            initShowBreakTimeUkui();
        } else if (key == SHOWTIME_CUSTOM) {
            initShowBreakTimeCustom();
        } else if (key == SCREENLOCK_ENABLED) {
            initLockscreen();
        }
    }
    toChangeKey = "";
}

void Screensaver::initOptions()
{
    QStringList textList = {};
    QList<int> dataList = {};
    QStringList dataList2 = {};

    textList.clear();
    textList << tr("5min") << tr("10min") << tr("15min") << tr("30min") << tr("1hour") << tr("Never");

    dataList.clear();
    dataList << 5 << 10 << 15 << 30 << 60 << -1;

#ifdef OPENKYLIN
    textList.insert(0, tr("1min"));
    dataList.insert(0, 1);
#endif
    screensaverUi->setIdleTimeOptions(textList, dataList);

    textList.clear();
    textList << tr("UKUI") << tr("Customize");
    dataList2.clear();
    dataList2 << "ukui"
              << "custom";
    screensaverUi->setScreensaverOptions(textList, dataList2);

    textList.clear();
    textList << tr("1min") << tr("5min") << tr("10min") << tr("30min");
    dataList.clear();
    dataList << 60 << 300 << 600 << 1800;
    screensaverUi->setSwitchTimeOptions(textList, dataList);
}

void Screensaver::initContent()
{
    initIdleTime();
    initScreensaver();
    initCustomSource();
    initSwicthMode();
    initSwitchTime();
    initCustomText();
    initTextPosition();
    initShowBreakTimeUkui();
    initShowBreakTimeCustom();
    initLockscreen();
}

void Screensaver::initPreview()
{
    if (screensaverPtr) {
        qInfo() << "screensaverPtr can be used";
        QWidget *widget = screensaverPtr->createWidget(false, screensaverUi);
        widget->show();
        screensaverUi->updatePreview(widget);
    } else {
        qInfo() << "screensaverPtr can not be used";
    }
}

void Screensaver::initIdleTime()
{
    int time = screensaverInterface->property(PREENTRY_TIME).toInt();
    screensaverUi->setIdleTime(time);
}

void Screensaver::initScreensaver()
{
    QString type = screensaverInterface->property(SCREENSAVER_TYPE).toString();
    screensaverUi->setScreensaver(type);
    initPreview();
}

void Screensaver::initCustomSource()
{
    QString path = screensaverInterface->property(CUSTOM_PATH).toString();
    screensaverUi->setSourcePath(path);
}

void Screensaver::initSwicthMode()
{
    bool random = screensaverInterface->property(SWITCH_RANDOW).toBool();
    screensaverUi->setSwitchRandom(random);
}

void Screensaver::initSwitchTime()
{
    int time = screensaverInterface->property(CYCLE_TIME).toInt();
    screensaverUi->setSwitchTime(time);
}

void Screensaver::initCustomText()
{
    QString text = screensaverInterface->property(CUSTOM_TEXT).toString();
    screensaverUi->setCustomText(text);
}

void Screensaver::initTextPosition()
{
    bool centered = screensaverInterface->property(CUSTOM_TEXT_CENTERED).toBool();
    screensaverUi->setTextCentered(centered);
}

void Screensaver::initShowBreakTimeUkui()
{
    bool b = screensaverInterface->property(SHOWTIME_UKUI).toBool();
    screensaverUi->setShowUkuiTime(b);
}

void Screensaver::initShowBreakTimeCustom()
{
    bool b = screensaverInterface->property(SHOWTIME_CUSTOM).toBool();
    screensaverUi->setShowCustomTime(b);
}

void Screensaver::initLockscreen()
{
    bool b = screensaverInterface->property(SCREENLOCK_ENABLED).toBool();
    screensaverUi->setLockscreen(b);
}

void Screensaver::connectUiSignals()
{
    QDBusConnection::sessionBus().connect(
        "org.ukui.ScreenSaver",
        "/Screensaver",
        "org.ukui.ScreenSaver.session.Screensaver",
        "changed",
        this,
        SLOT(dataChanged(QString)));
    connect(screensaverUi, &ScreensaverUi::idleTimeChanged, this, [=](int time) {
        toChangeKey = PREENTRY_TIME;
        screensaverInterface->call("setPreentryTime", time);
        UkccCommon::buriedSettings(name(), QString("Idle time"), QString("select"), QString::number(time));
    });
    connect(screensaverUi, &ScreensaverUi::screensaverChanged, this, [=](QString type) {
        toChangeKey = SCREENSAVER_TYPE;
        screensaverInterface->call("setScreensaverType", type);
        UkccCommon::buriedSettings(name(), QString("Screensaver program"), QString("select"), type);
    });
    connect(screensaverUi, &ScreensaverUi::customPathChanged, this, [=](QString path) {
        toChangeKey = CUSTOM_PATH;
        screensaverInterface->call("setCustomPath", path);
        UkccCommon::buriedSettings(name(), QString("Screensaver source Select"), QString("select"), path);
    });
    connect(screensaverUi, &ScreensaverUi::switchRandomChanged, this, [=](bool b) {
        toChangeKey = SWITCH_RANDOW;
        screensaverInterface->call("setSwitchRandom", b);
        UkccCommon::buriedSettings(name(), QString("Random switching"), QString("settings"), b ? "Random" : "Ordinal");
    });
    connect(screensaverUi, &ScreensaverUi::switchTimeChanged, this, [=](int time) {
        toChangeKey = CYCLE_TIME;
        screensaverInterface->call("setCycleTime", time);
        UkccCommon::buriedSettings(name(), QString("Switching time"), QString("select"), QString::number(time));
    });
    connect(screensaverUi, &ScreensaverUi::customTextChanged, this, [=](QString text) {
        toChangeKey = CUSTOM_TEXT;
        screensaverInterface->call("setCustomText", text);
        UkccCommon::buriedSettings(name(), QString("Text(up to 30 characters):"), QString("settings"), text);
    });
    connect(screensaverUi, &ScreensaverUi::textCenteredChanged, this, [=](bool b) {
        toChangeKey = CUSTOM_TEXT_CENTERED;
        screensaverInterface->call("setCustomTextCentered", b);
        UkccCommon::buriedSettings(
            name(), QString("Text position"), QString("settings"), b ? "Centered" : "Randow(Bubble text)");
    });
    connect(screensaverUi, &ScreensaverUi::showUkuiTimeChanged, this, [=](bool b) {
        toChangeKey = SHOWTIME_UKUI;
        screensaverInterface->call("setShowBreakTimeUkui", b);
        UkccCommon::buriedSettings(
            name(), QString("custom Show rest time"), QString("settings"), UkccCommon::boolToString(b));
    });
    connect(screensaverUi, &ScreensaverUi::showCustomChanged, this, [=](bool b) {
        toChangeKey = SHOWTIME_CUSTOM;
        screensaverInterface->call("setShowBreakTimeCustom", b);
        UkccCommon::buriedSettings(
            name(), QString("ukui Show rest time"), QString("settings"), UkccCommon::boolToString(b));
    });
    connect(screensaverUi, &ScreensaverUi::lockscreenChanged, this, [=](bool b) {
        toChangeKey = SCREENLOCK_ENABLED;
        screensaverInterface->call("setScreenLockEnabled", b);
        UkccCommon::buriedSettings(
            name(), QString("Lock screen when screensaver boot"), QString("settings"), UkccCommon::boolToString(b));
    });

    // 如果在dbus端监听gsetting的改变信号，再发送changed信号，此时再去initPreview可能会异常
    // 因为界面端程序此时去读取gsetting的值可能还是修改前的值，该问题应该是qgsetting的设计逻辑问题
    // 因此这里去监听gsetting的改变而不是dbus的changed
    QGSettings *mScreensaverGsettings = nullptr;
    if (!mScreensaverGsettings) {
        const QByteArray id("org.ukui.screensaver");
        if (QGSettings::isSchemaInstalled(id)) {
            mScreensaverGsettings = new QGSettings(id, QByteArray(), this);
        }
        connect(mScreensaverGsettings, &QGSettings::changed, this, [=](QString key) {
            if (key == "mode") {
                initPreview();
            }
        });
    }
}

void Screensaver::connectToServer()
{
    QThread *NetThread = new QThread;
    MThread *NetWorker = new MThread;
    NetWorker->moveToThread(NetThread);
    connect(NetThread, &QThread::started, NetWorker, &MThread::run);
    connect(NetWorker, &MThread::keychangedsignal, this, &Screensaver::keyChangedSlot);
    connect(NetThread, &QThread::finished, NetWorker, &MThread::deleteLater);
    NetThread->start();
}

void Screensaver::keyChangedSlot(const QString &key)
{
    if (key == "ukui-screensaver") {
        initContent();
    }
}
