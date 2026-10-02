/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include <QCoreApplication>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMessage>
#include <QTranslator>
#include <QCommandLineParser>
#include <QDebug>
#include <QGuiApplication>
#include "utils.h"
#include <ukui-log4qt.h>
#include <QThread>

#define WORKING_DIRECTORY "/usr/share/ukui-screensaver"

int main(int argc, char **argv)
{
    initUkuiLog4qt("ukui-screensaver-command");
    QCoreApplication a(argc, argv);

    QString locale = QLocale::system().name();
    QTranslator translator;
    QString qmFile = QString(WORKING_DIRECTORY "/i18n_qm/%1.qm").arg(locale);
    translator.load(qmFile);
    a.installTranslator(&translator);

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "Start command for the ukui ScreenSaver."));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption lockOption(
        { "l", QStringLiteral("lock") }, QCoreApplication::translate("main", "lock the screen immediately"));

    QCommandLineOption queryOption(
        { "q", QStringLiteral("query") }, QCoreApplication::translate("main", "query the status of the screen saver"));

    QCommandLineOption unlockOption(
        { "u", QStringLiteral("unlock") }, QCoreApplication::translate("main", "unlock the screen saver"));
    QCommandLineOption screensaverOption(
        { "s", QStringLiteral("screensaver") }, QCoreApplication::translate("main", "show the screensaver"));
    QCommandLineOption blankOption(
        { "b", QStringLiteral("blank") },
        QCoreApplication::translate("main", "show blank and delay to lock,param:idle/lid/lowpower"),
        "lid");
    QCommandLineOption sessiontoolsOption(
        { "t", QStringLiteral("session-tools") }, QCoreApplication::translate("main", "show the session tools"));
    QCommandLineOption switchuserOption(
        QStringLiteral("switchuser"), QCoreApplication::translate("main", "show the switchuser window"));
    QCommandLineOption appBlockOption(
        { "a", QStringLiteral("app-block") },
        QCoreApplication::translate("main", "show the app block window"),
        QGuiApplication::translate("action", "which block type,param:Suspend/Hibernate/Restart/Shutdown/Logout"),
        "type");
    QCommandLineOption multiUsersOption(
        { "m", QStringLiteral("multiusers-block") },
        QCoreApplication::translate("main", "show the multiUsers block window"),
        QGuiApplication::translate("action", "which block type,param:Shutdown/Restart"),
        "type");

    parser.addOption(lockOption);
    parser.addOption(queryOption);
    parser.addOption(unlockOption);
    parser.addOption(screensaverOption);
    parser.addOption(blankOption);
    parser.addOption(sessiontoolsOption);
    parser.addOption(switchuserOption);
    parser.addOption(appBlockOption);
    parser.addOption(multiUsersOption);
    parser.process(a);

    if (!parser.isSet(lockOption) && !parser.isSet(queryOption) && !parser.isSet(unlockOption)
        && !parser.isSet(screensaverOption) && !parser.isSet(blankOption) && !parser.isSet(sessiontoolsOption)
        && !parser.isSet(appBlockOption) && !parser.isSet(multiUsersOption) && !parser.isSet(switchuserOption))
        return -1;

    QString displayNum = QString(qgetenv("DISPLAY")).replace(":", "").replace(".", "_");
    QString sessionDbus = QString("%1%2").arg(QString(SS_DBUS_SERVICE)).arg(displayNum);
    QDBusInterface *interface = new QDBusInterface(sessionDbus, SS_DBUS_PATH, SS_DBUS_INTERFACE);
    if (!interface->isValid()) {
        delete interface;
        interface = new QDBusInterface(SS_DBUS_SERVICE, SS_DBUS_PATH, SS_DBUS_INTERFACE);
    }
    QDBusReply<bool> stateReply = interface->call("GetLockState");
    if (!stateReply.isValid()) {
        qWarning() << "Get state error:" << stateReply.error();
        return 0;
    }
    qInfo() << "Command:" << parser.optionNames();
    if (parser.isSet(queryOption)) {
        if (stateReply)
            qDebug() << qPrintable(QObject::tr("The screensaver is active."));
        else
            qDebug() << qPrintable(QObject::tr("The screensaver is inactive."));
    } else if (parser.isSet(lockOption) && !stateReply) {
        QDBusMessage msg = interface->call("Lock");
        if (msg.type() == QDBusMessage::ErrorMessage)
            qDebug() << msg.errorMessage();
    } else if (parser.isSet(unlockOption)) {
        QDBusMessage msg = interface->call("UnLock");
        if (msg.type() == QDBusMessage::ErrorMessage)
            qDebug() << msg.errorMessage();
    } else if (parser.isSet(screensaverOption)) {
        QDBusMessage msg = interface->call("ShowScreensaver");
        if (msg.type() == QDBusMessage::ErrorMessage)
            qDebug() << msg.errorMessage();
    } else if (parser.isSet(blankOption) && !stateReply) {
        int nLockType = 0;
        qDebug() << "Param:" << parser.optionNames();
        QString opValue = parser.value(blankOption);
        if (opValue == "idle") {
            nLockType = 0;
        } else if (opValue == "lid") {
            nLockType = 1;
        } else if (opValue == "lowpower") {
            nLockType = 2;
        } else {
            return 0;
        }
        QDBusReply<bool> bLockSuccess = interface->call("LockByBlank", nLockType);
        if (bLockSuccess.isValid() && bLockSuccess.value()) {
            bool isLocked = false;
            for (int n = 0; n < 20; n++) {
                QDBusReply<bool> reply = interface->call("GetLockState");
                if (reply.isValid()) {
                    isLocked = reply.value();
                }
                if (isLocked) {
                    qDebug() << "LockState:" << n;
                    break;
                }
                QThread::msleep(200);
            }
        } else {
            qDebug() << "LockByBlank:" << bLockSuccess.error().message() << bLockSuccess.value();
        }
    } else if (parser.isSet(sessiontoolsOption) && !stateReply) {
        QDBusMessage msg = interface->call("SessionTools");
        if (msg.type() == QDBusMessage::ErrorMessage)
            qDebug() << msg.errorMessage();
    } else if (parser.isSet(switchuserOption) && !stateReply) {
        QDBusMessage msg = interface->call("SwitchUser");
        if (msg.type() == QDBusMessage::ErrorMessage)
            qDebug() << msg.errorMessage();
    } else if (parser.isSet(appBlockOption) && !stateReply) {
        QString opValue = parser.value(appBlockOption);
        QDBusMessage msg = interface->call("AppBlockWindow", opValue);
        if (msg.type() == QDBusMessage::ErrorMessage)
            qDebug() << msg.errorMessage();
    } else if (parser.isSet(multiUsersOption) && !stateReply) {
        QString opValue = parser.value(multiUsersOption);
        qDebug() << opValue;
        QDBusMessage msg = interface->call("MultiUserBlockWindow", opValue);
        if (msg.type() == QDBusMessage::ErrorMessage)
            qDebug() << msg.errorMessage();
    }
    return 0;
}
