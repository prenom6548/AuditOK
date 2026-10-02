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
#include <QProcess>
#include <QTimer>
#include <libkylog.h>
#include <singleapplication.h>
#include "kyplugin.h"
#include "kom_application_interface.h"
#include "defines.h"
#include "mainwindow.h"
#include "kom-utils.h"

// 全局变量
BaseData g_base;
CommandLineArgument g_commandLineArgument;    // 记录命令行参数

static bool ParseCommandLine(QCommandLineParser &parser, CommandLineArgument &argument, QString &errorMessage) {
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    const QCommandLineOption jump_tab_option("jumpTab", "Jump to the specified tab page", "jumpTabName");
    parser.addOption(jump_tab_option);
    const QCommandLineOption jump_secondary_option("pluginMessage", "The data passed to the jump tab", "jumpTabAdditionalData");
    parser.addOption(jump_secondary_option);
    const QCommandLineOption repair_option("repair", "Use kylin os manager's fault detection function to fix errors");
    parser.addOption(repair_option);
    const QCommandLineOption error_code_option("errorCode", "Error codes that need to be fixed", "errorCodeNeedFixed");
    parser.addOption(error_code_option);
    const QCommandLineOption error_data_option("errorData", "The data carried by the error code", "errorCodeCarryData");
    parser.addOption(error_data_option);
    const QCommandLineOption module_option("module", "Module name that needs to be repaired", "moduleName");
    parser.addOption(module_option);
    const QCommandLineOption path_option("path", "Report log path", "logPath");
    parser.addOption(path_option);
    const QCommandLineOption diagnosis_option("diagnosis", "Use kylin os manager fault diagnosis function to detect specified issues");
    parser.addOption(diagnosis_option);
    const QCommandLineOption mode_option("mode", "Types to be tested", "typeNeedDiagnosis");
    parser.addOption(mode_option);
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

    // 插件跳转
    if (parser.isSet(jump_tab_option)) {
        argument.jumpTabName = parser.value(jump_tab_option);
    }
    if (parser.isSet(jump_secondary_option)) {
        argument.jumpSecondaryName = parser.value(jump_secondary_option);
    }

    // 故障检测
    if (parser.isSet(repair_option)) {
        argument.repair = true;
    }
    if (parser.isSet(error_code_option)) {
        argument.error_code = parser.value(error_code_option);
    }
    if (parser.isSet(error_data_option)) {
        argument.error_data = parser.value(error_data_option);
    }
    if (parser.isSet(module_option)) {
        argument.module = parser.value(module_option);
    }
    if (parser.isSet(path_option)) {
        argument.path = parser.value(path_option);
    }
    if (parser.isSet(diagnosis_option)) {
        argument.diagnosis = true;
    }
    if (parser.isSet(mode_option)) {
        argument.diagnosis_type = parser.value(mode_option);
    }

    return true;
}

static bool VerifyCommandLineArgs() {
    if (g_commandLineArgument.repair) {
        if (g_commandLineArgument.error_code.isEmpty() ||
            g_commandLineArgument.module.isEmpty())
        {
            qCritical() << "Error code and module cannot be empty in repair mode";
            return false;
        }
    }
    if (g_commandLineArgument.repair && g_commandLineArgument.diagnosis) {
        qCritical() << "Parameter conflict, --repair and --diagnosis can only set one";
        return false;
    }
    if (g_commandLineArgument.diagnosis) {
        if (g_commandLineArgument.diagnosis_type.isEmpty()) {
            qCritical() << "Type cannot be empty in diagnosis mode";
            return false;
        }
    }

    return true;
}

// 通信协议
// 标识字符串\n十六进制编码字符串1\n十六进制编码字符串2...
void SendMessage(kdk::QtSingleApplication &app) {
    // 修复模式
    if (g_commandLineArgument.repair) {
        QByteArray data;
        data.append("repair");
        data.append('\n');
        data.append(g_commandLineArgument.module.toLocal8Bit().toHex());    // 组件名
        data.append('\n');
        data.append(g_commandLineArgument.path.toLocal8Bit().toHex());    // 日志路径
        data.append("\n");
        data.append(g_commandLineArgument.error_code.toLocal8Bit().toHex());    // 错误码
        if (!g_commandLineArgument.error_data.isEmpty()) {
            data.append('\n');
            data.append(g_commandLineArgument.error_data.toLocal8Bit().toHex());    // 错误数据
        }
        app.sendMessage(data);
        return;
    }

    // 指定诊断模式
    if (g_commandLineArgument.diagnosis) {
        QByteArray data;
        data.append("diagnosis");
        data.append('\n');
        data.append(g_commandLineArgument.diagnosis_type.toLocal8Bit().toHex());
        app.sendMessage(data);
        return;
    }

    // 页面跳转
    if (g_commandLineArgument.jumpTabName == "ServiceSupport" &&
        g_commandLineArgument.jumpSecondaryName == "")
    {
        app.sendMessage("DisplayServiceSupport");
        return;
    } else if (g_commandLineArgument.jumpTabName == "ServiceSupport" &&
               g_commandLineArgument.jumpSecondaryName == "ProblemFeedback")
    {
        app.sendMessage("DisplayProblemFeedback");
        return;
    } else if (g_commandLineArgument.jumpTabName == "DeviceManager" &&
               g_commandLineArgument.jumpSecondaryName == "")
    {
        app.sendMessage("DisplayDeviceManager");
        return;
    } else if (g_commandLineArgument.jumpTabName == "DeviceManager" &&
               g_commandLineArgument.jumpSecondaryName == "HardwareInfo")
    {
        app.sendMessage("DisplayHardwareInfo");
        return;
    } else if (g_commandLineArgument.jumpTabName == "DeviceManager" &&
              g_commandLineArgument.jumpSecondaryName == "DriverManager")
    {
        app.sendMessage("DisplayDriverManager");
        return;
    } else if (g_commandLineArgument.jumpTabName == "JunkClean" &&
               g_commandLineArgument.jumpSecondaryName == "MaxDiskCapacity")
    {
        app.sendMessage("MaxDiskCapacity");
        return;
    }

    app.sendMessage("running");
}

int main(int argc, char *argv[]) {
    // 加载插件
    std::vector<std::string> loadPath;
#ifndef DEBUG_MODE
    loadPath.push_back("/usr/lib/kylin-os-manager/plugins/");
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
    for (const auto &path: loadPath) {
        pluginManager.loadFromFolder(path, true);
    }

    std::vector<KomApplicationProvider *> tabProviders;
    pluginManager.getProviders(tabProviders);

#ifndef DEBUG_MODE
    qInstallMessageHandler(kom::KomUtils::LogOutput);
#endif

    // 设置高分辨率
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    kdk::QtSingleApplication app(argc, argv);

    // 处理命令行参数
    if (argc > 1) {
        g_base.start_with_arg = true;
    }

    QCommandLineParser parser;
    QString errorMessage;
    if (!ParseCommandLine(parser, g_commandLineArgument, errorMessage)) {
        qCritical() << "parser command line argument error: " << errorMessage;
        return ERROR_COMMAND_LINE_PARSING_FAILED;
    }
    if (!VerifyCommandLineArgs()) {
        qCritical() << "verify command line args fail.";
        return ERROR_COMMAND_LINE_VERIFY_FAILED;
    }

    // 判断麒麟管家是否已经在运行
    if (app.isRunning()) {
        qInfo() << "kylin os manager is already running !";
        SendMessage(app);
        return 0;
    }

    // 创建用户配置目录
    QDir dir;
    if (!dir.mkpath(QString("%1/.kylin-os-manager").arg(getenv("HOME")))) {
        qCritical() << "create user config dir fail !";
    }

    // 加载应用翻译文件
    QString kylinFileCrush = "/usr/share/kylin-os-manager/translations";
    QTranslator trans_global;
    if (!trans_global.load(QLocale(), "kylin-os-manager", "_", kylinFileCrush)) {
        qWarning() << "main Load global translations file" << QLocale() << "failed!";
    } else {
        app.installTranslator(&trans_global);
    }
    // 加载 Qt 翻译文件
    QTranslator trans_qt;
    QString qtTransPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    if (!trans_qt.load(QLocale(), "qt", "_", qtTransPath)) {
        qWarning() << "main Load qt translations file" << QLocale() << "failed!";
    } else {
        app.installTranslator(&trans_qt);
    }
    // 加载 sdk 的翻译
    QString locale = QLocale::system().name();
    QTranslator sdk_trans;
    if (!sdk_trans.load(":/translations/gui_" + locale + ".qm")) {
        qDebug() << "Load sdk translation file failed!";
    } else {
        app.installTranslator(&sdk_trans);
    }

    QApplication::setApplicationName(QApplication::tr("KylinOsManager"));

    // 设置 wayland 环境下的最小化拉起
    kom::KomUtils::SetWaylandActivationWindow(app);

    MainWindow w(tabProviders);
    w.show();

    // 先尽快让界面显示出来，界面显示出来后在处理界面上的元素
    QTimer::singleShot(5, [&app, &w]() {
        // 初始化界面
        w.Init();

        // 设置最小化拉起
        app.setActivationWindow(&w);

        // 添加安全中心网络控制白名单
#ifndef V11_ENVIRONMENT
        kom::KomUtils::setNetworkWhiteListFromPackage(kom::KomUtils::Application);
#endif

        QObject::connect(&app, &kdk::QtSingleApplication::messageReceived, [&w](QString message) {
            auto data = message.toLocal8Bit();
            auto index = data.indexOf('\n');
            if (index != -1) {
                // 判断标记
                QString header = data.mid(0, index);
                if (header == "repair") {
                    // 修复模式
                    w.ControlPlugin(MainWindow::PluginOperate::kRepairSpecificError, data.mid(index + 1, -1));
                } else if (header == "diagnosis") {
                    // 指定诊断模式
                    w.ControlPlugin(MainWindow::PluginOperate::kDiagnosisSpecificType, data.mid(index + 1, -1));
                }
            } else if (data == "DisplayServiceSupport") {
                w.ControlPlugin(MainWindow::PluginOperate::kDisplayServiceSupport);
            } else if (data == "DisplayProblemFeedback") {
                w.ControlPlugin(MainWindow::PluginOperate::kDisplayProblemFeedback);
            } else if (data == "DisplayDeviceManager") {
                w.ControlPlugin(MainWindow::PluginOperate::kDisplayDeviceManager);
            } else if (data == "DisplayHardwareInfo") {
                w.ControlPlugin(MainWindow::PluginOperate::kDisplayHardwareInfo);
            } else if (data == "DisplayDriverManager") {
                w.ControlPlugin(MainWindow::PluginOperate::kDisplayDriverManager);
            } else if (data == "MaxDiskCapacity") {
                w.ControlPlugin(MainWindow::PluginOperate::kRunJunkClean, "MaxDiskCapacity");
            }
        });
    });

    return app.exec();
}
