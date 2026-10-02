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

#include "settingsframe.h"
#include "utils.h"
#include "xatom-helper.h"

#include <QApplication>
#include <QtSingleApplication>
#include <QObject>
#include <QDir>
#include <QTextCodec>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>

#include <unistd.h>

#include "global.h"

inline bool root_check()
{
    if (geteuid() == 0) {
        return true;
    }

    return false;
}

static QString translationPath()
{
    QString path;
    if (QDir("/usr/share/kylin-kmre-settings/translations").exists()) {
        path = "/usr/share/kylin-kmre-settings/translations";
        return path;
    }
    else {
        return qApp->applicationDirPath() + "/translations";
    }
}

int main(int argc, char *argv[])
{

    if (root_check()) {
        qWarning() << "Don't use root to run it";
        return -1;
    }

    if (Global::isKylinOSPro() || Global::isOpenKylin()) {
        qputenv("QT_QPA_PLATFORMTHEME", "ukui");
    }

#ifdef UKUI_WAYLAND
    qputenv("QT_QPA_PLATFORM", "wayland");
#else
    //Attention: 以下配置不能放在wayland环境下配置，否则导致配置界面显示异常，原因未知
    // 适配4K屏
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

#if QT_VERSION >= 0x040400
    // Enable icons in menus
    QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, false);
#endif
#endif

    QString id = QString("kylin-kmre-settings" + QLatin1String(getenv("DISPLAY")));
    QtSingleApplication a(id, argc, argv);

    if (a.isRunning()) {
        a.sendMessage(QApplication::arguments().length() > 1 ? QApplication::arguments().at(1) : a.applicationFilePath());
        qDebug() << QObject::tr("kylin-kmre-settings is already running!");
        return EXIT_SUCCESS;
    }
    else {
        QCoreApplication::setApplicationName(QObject::tr("kylin-kmre-settings"));
        QCoreApplication::setApplicationVersion("2.0");

#if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
        QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
        QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
        QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
        QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

        QString locale = QLocale::system().name();
        QString translationFile = "kylin-kmre-settings_" + locale + ".qm";
        QTranslator translator;
        if (!translator.load(translationFile, translationPath())) {
            qDebug() << "Load translation file："<< translationFile << " failed!";
        }
        else {
            a.installTranslator(&translator);
        }

        //加载Qt对话框默认的国际化
        QTranslator qtTranslator;
        qtTranslator.load("qt_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));// /usr/share/qt5/translations
        a.installTranslator(&qtTranslator);

        SettingsFrame w;
        w.showWindow();


        a.setActivationWindow(&w);
        QObject::connect(&a, SIGNAL(messageReceived(const QString&)), &w, SLOT(slotMessageReceived(const QString&)));
        w.show();

        return a.exec();
    }
}
