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

#include "main.h"
#include "usd_base_class.h"
#include "save-screen.h"
#include "clib-syslog.h"
#include "lightdm-config-helper.h"
#include "calibrate-touch-device.h"


#define XSETTINGS_SCHEMA    "org.ukui.SettingsDaemon.plugins.xsettings"
#define MOUSE_SCHEMA        "org.ukui.peripherals-mouse"
#define SCALING_KEY         "scaling-factor"
#define CURSOR_SIZE         "cursor-size"
#define CURSOR_THEME        "cursor-theme"

bool g_isGet = false;
bool g_isSet = false;

QCommandLineParser *s_parser;
QCommandLineOption *s_userOption;
QCommandLineOption *s_setOption;

int QAPP(int argc, char *argv[])
{
    UsdBaseClass::printTimeWithMsg("save-param start..",__FUNCTION__, __LINE__);
    QApplication app(argc, argv);
    QApplication::setApplicationName("save-param");
    QApplication::setApplicationVersion("1.0.0");

    s_parser->process(app);

    SaveScreenParam saveParam;

    QTimer::singleShot(3*1000, [=](){
        USD_LOG(LOG_DEBUG,"can't get finished signal must exit..");//
        std::exit(0);
    });

    if (s_parser->isSet(*s_userOption)){
        saveParam.setUserConfigParam();
    } else {
        std::exit(0);
    }

    return app.exec();
}

//for upgrade 2203 to 2603.
int QCoreApp(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("save-param");
    QCoreApplication::setApplicationVersion("1.0.0");

    s_parser->process(app);

    QTimer::singleShot(3*1000, [=](){
        USD_LOG(LOG_DEBUG,"can't get finished signal must exit..");//
        std::exit(0);
    });

    LightDmConfigHelper configHelper;

    QObject::connect(&configHelper, &LightDmConfigHelper::applyFinish, [=](){
        USD_LOG(LOG_DEBUG,"get applyFinish signal.....");
        exit(0);
    });

    if (s_parser->isSet(*s_setOption)) {
        QString user = s_parser->value(*s_setOption);
        configHelper.setName(user);
        configHelper.syncSettingsToLightDmDir();
    }

    return app.exec();
}

int main(int argc, char *argv[])
{
    bool isSetQGSettings = false;
    for (int var = 0; var < argc; ++var) {
        if (!strcmp(argv[var],"-s")){
            isSetQGSettings = true;
        }
    }

    s_parser = new QCommandLineParser();
    s_parser->setApplicationDescription(QApplication::translate("main", "Qt"));
    s_parser->addHelpOption();  // 添加帮助选项 （"-h" 或 "--help"）
    s_parser->addVersionOption();  // 添加版本选项 ("-v" 或 "--version")
    s_parser->setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    s_setOption = new QCommandLineOption(QStringList() << "s" << "save",
                                         QApplication::translate("main", "Get the gsettings parameters of the current user and save them to the lightdm user directory"),
                                         QApplication::translate("main", "save"), "");

    s_userOption = new QCommandLineOption(QStringList() << "u" << "user",
                                          QApplication::translate("main", "Get the display parameters saved by the user in the lightdm personal folder and set them"),
                                          QApplication::translate("main", "user"), "");
    s_parser->addOption(*s_userOption);
    s_parser->addOption(*s_setOption);

    if (isSetQGSettings) {
        QCoreApp(argc, argv);
    } else {
        QAPP(argc, argv);
    }
}
