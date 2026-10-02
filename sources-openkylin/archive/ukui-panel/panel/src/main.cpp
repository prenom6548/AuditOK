/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *          iaom <zhangpengfei@kylinos.cn>
 *
 */

#include <QGuiApplication>
#include <QApplication>

#include <QLocale>
#include <QTranslator>
#include <QDebug>

#include "qtsingleapplication.h"
#include "shell.h"
#include "log-utils.h"
#include "message-processor.h"

void loadTranslation() {
    const QString dir = PANEL_TRANSLATION_DIR;
    const QString prefix = "panel_";
    const QString locale = QLocale::system().name();
    const QString fileName = dir + prefix + locale;

    qDebug() << "loadTranslation:" << dir << locale << fileName;

    auto translator = new QTranslator(QCoreApplication::instance());
    if (translator->load(fileName)) {
        QCoreApplication::installTranslator(translator);

    } else {
        translator->deleteLater();
    }
}

int main(int argc, char *argv[])
{
#ifndef PANEL_DISABLE_LOG_FILE
    LogUtils::initLogFile("ukui-panel");
    qInstallMessageHandler(LogUtils::messageOutput);
#endif

    QCoreApplication::setApplicationName("ukui-panel");
    QCoreApplication::setOrganizationName("ukui");
    QCoreApplication::setOrganizationDomain("ukui.org");
    QCoreApplication::setApplicationVersion("4.1.0.0");

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QString sessionType(qgetenv("XDG_SESSION_TYPE"));
    QString displayEnv = (sessionType == "wayland") ? QLatin1String("WAYLAND_DISPLAY") : QLatin1String("DISPLAY");
    QString display(qgetenv(displayEnv.toUtf8().data()));
    QString appid = QString("ukui-panel-%1").arg(display);
    qDebug() << "ukui-panel start on display:" << display << "displayEnv:" << displayEnv << "sessionType:" << sessionType;
    if(sessionType == "wayland") {
        qputenv("QT_WAYLAND_DISABLE_FIXED_POSITIONS", "true");
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "ukui-shell");
    }

    QtSingleApplication app(appid, argc, argv);
    if (app.isRunning()) {
        MessageProcessor processor;
        if(processor.preprocessMessage(QtSingleApplication::arguments())) {
            app.sendMessage(QtSingleApplication::arguments().join(" ").toUtf8());
        }
        return QtSingleApplication::exec();
    }
    //no message need process yet.
    QObject::connect(&app, &QtSingleApplication::messageReceived, &app, [&](const QString &message) {
        qDebug() << "ukui-panel receive message:" << message;
    });

    // --_-- //
    QCoreApplication::instance()->setProperty("display", display);
    QCoreApplication::instance()->setProperty("displayEnv", displayEnv);

    loadTranslation();
    UkuiPanel::Shell::self()->start();

    return QtSingleApplication::exec();
}
