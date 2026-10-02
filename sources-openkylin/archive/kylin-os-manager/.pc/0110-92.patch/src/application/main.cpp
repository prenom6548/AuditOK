/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 */

#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>
#include <QStandardPaths>
#include <QLocale>
#include <QCommandLineParser>
#include "singleapplication.h"
#include "log.hpp"
#include "mainwindow.h"
#include "defines.h"

#include "kyplugin.h"
#include "kom_application_interface.h"

// 全局变量，记录命令行参数
CommandLineArgument g_commandLineArgument;

bool parseCommandLine(QCommandLineParser &parser, CommandLineArgument &argument, QString &errorMessage) {
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    const QCommandLineOption jumpTabOption("jumpTab", "jump tab", "jumpTabName");
    parser.addOption(jumpTabOption);
    const QCommandLineOption help = parser.addHelpOption();
    const QCommandLineOption version = parser.addVersionOption();

    if (!parser.parse(QCoreApplication::arguments())) {
        errorMessage = parser.errorText();
        return false;
    }

    if (parser.isSet(version)) {
        argument.type = CommandType::VERSION;
        return true;
    }

    if (parser.isSet(help)) {
        argument.type = CommandType::HELP;
        return true;
    }

    if (parser.isSet(jumpTabOption)) {
        argument.jumpTabName = parser.value(jumpTabOption);
    }

    return true;
}

int main(int argc, char *argv[])
{
    std::vector<std::string> loadPath;
#ifndef DEBUG_MODE
    //loadPath.push_back("/usr/lib/kylin-os-manager/plugins/");
    //loadPath.push_back("/usr/local/lib/kylin-os-manager/plugins/");
    loadPath.push_back("/opt/kylin-os-manager/plugins/");
    //std::string loadHomePath {getenv("HOME")};
    //loadHomePath.append("/");
    //loadHomePath.append(".kylin-os-manager/plugins/");
    //loadPath.push_back(loadHomePath);
#else
    std::string localPath;
    char path[4096];
    getcwd(path, 4096);
    localPath.append(path);
    localPath.append("/../../plugins/");
    loadPath.push_back(localPath);
#endif
    kyplugin::KyPlugin pluginManager;
    pluginManager.acceptProviderType<KomApplicationProvider>();
    for (const auto &path: loadPath)
        pluginManager.loadFromFolder(path, true);

    std::vector<KomApplicationProvider *> tabProviders;
    pluginManager.getProviders(tabProviders);

#ifndef DEBUG_MODE
    qInstallMessageHandler(kdk::kabase::Log::logOutput);
#endif
// 修改分辨率的显示
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    qDebug() << "setAttribute success";
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    qDebug() << "setHighDpiScaleFactorRoundingPolicy success";
#endif

    kdk::QtSingleApplication app(argc, argv);
    if (app.isRunning()) {
        qInfo() << "kylin os manager is already running !";
        app.sendMessage("running", 4000);
        return 0;
    }

    QCommandLineParser parser;
    QString errorMessage;
    if (!parseCommandLine(parser, g_commandLineArgument, errorMessage)) {
        qCritical() << "parser command line argument error . " << errorMessage;
        return -1;
    }

    /* 创建用户配置目录 */
    QDir dir;
    if (!dir.mkpath(QString("%1/.kylin-os-manager").arg(getenv("HOME")))) {
        qCritical() << "create user config dir fail !";
    }

    //检测单例
    QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    int singleInstanceFd = open(QString(homePath.at(0) + "/.config/kylin-os-manager%1.lock").arg(getenv("DISPLAY")).toUtf8().data(),
                                O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (singleInstanceFd < 0) {
        exit(1);
    }
    if (lockf(singleInstanceFd, F_TLOCK, 0)) {
        syslog(LOG_ERR, "Can't lock single file, kylin-os-manager is already running!");
        exit(0);
    }

    /* 加载应用翻译文件 */
    QString kylinFileCrush = "/usr/share/kylin-os-manager/translations";
    QTranslator trans_global;
    if (!trans_global.load(QLocale(), "kylin-os-manager", "_", kylinFileCrush)) {
        qWarning() << "main Load global translations file" << QLocale() << "failed!";
    } else {
        app.installTranslator(&trans_global);
    }
    /* 加载Qt翻译文件 */
    QTranslator trans_qt;
    QString qtTransPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    if (!trans_qt.load(QLocale(), "qt", "_", qtTransPath)) {
        qWarning() << "main Load qt translations file" << QLocale() << "failed!";
    } else {
        app.installTranslator(&trans_qt);
    }
    //加载SDK的翻译
    QString locale = QLocale::system().name();
    QTranslator sdk_trans;
    if (!sdk_trans.load(":/translations/gui_" + locale + ".qm")) {
        qDebug() << "Load sdk translation file failed!";
    } else {
        app.installTranslator(&sdk_trans);
    }

    app.setApplicationName(MainWindow::tr("kylin-os-manager"));

    MainWindow w(tabProviders);
    app.setActivationWindow(&w);
    w.show();

    return app.exec();
}
