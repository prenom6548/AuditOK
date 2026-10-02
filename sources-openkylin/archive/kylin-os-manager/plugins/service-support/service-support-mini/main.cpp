#include <QApplication>
#include <QTranslator>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QScreen>
#include "log.hpp"
#include "voluntarilywidget.h"
#include "passivitywidget.h"
#include "blacklist.h"

void singleton(const QString &exec) {
    QProcess process;
    process.setProgram("bash");
    process.setArguments({"-c", "ps -eo pid,args | grep service-support-mini | grep -v grep"});
    process.start();
    process.waitForFinished();

    const auto output = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    const auto lines = output.split('\n');
    for (const auto &line: lines) {
        auto elements = line.split(' ');
        elements.removeAll("");

        if (elements.size() != 4) {
            continue;
        }
        if (elements.at(1) != "/opt/kylin-os-manager/tools/service-support-mini" ||
            !elements.at(2).contains("-mode") ||
            !elements.at(3).contains("-exe"))
        {
            continue;
        }

        auto current_pid = elements.at(0).trimmed();
        auto current_mode = elements.at(2);
        current_mode = current_mode.remove("-mode=");
        auto current_exec = elements.at(3);
        current_exec = current_exec.remove("-exe=");

        if (current_pid.toInt() == QCoreApplication::applicationPid()) {
            // 过滤掉自己
            continue;
        }

        if (current_mode == "1" && current_exec == exec) {
            // 主动反馈弹窗已运行, 置顶弹窗
            QProcess p;
            p.setProgram("bash");
            auto top_up_command = QString("wmctrl -i -a $(wmctrl -p -G -l | grep %1 | head -1 | awk '{print $1}')").arg(current_pid);
            p.setArguments({"-c", top_up_command});
            p.startDetached();

            qInfo() << "Global feedback [" << exec << "] already running, current process exit.";

            exit(0);
        }
    }
}

int main(int argc, char *argv[]) {
#ifndef DEBUG_MODE
    qInstallMessageHandler(kdk::kabase::Log::logOutput);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QApplication app(argc, argv);

    // 加载应用翻译文件
    QString kylinFileCrush = "/usr/share/kylin-os-manager/translations";
    QTranslator trans_global;
    if (!trans_global.load(QLocale(), "kom-service-support-mini", "_", kylinFileCrush)) {
        qWarning() << "main Load global translations file" << QLocale() << "failed!";
    } else {
        app.installTranslator(&trans_global);
    }
    // 加载service-support-backend翻译文件
    QTranslator trans_global_2;
    if (!trans_global_2.load(QLocale(), "kom-service-support", "_", kylinFileCrush)) {
        qWarning() << "main Load global2 translations file" << QLocale() << "failed!";
    } else {
        app.installTranslator(&trans_global_2);
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

    //参数解析
    QStringList args =  app.arguments();
    args.removeFirst();
#ifdef DEBUG_MODE
    if (args.isEmpty()) {
        // /opt/kylin-os-manager/tools/service-support-mini -mode=2 -exe=kylin-printer -path=/home/yyf/sensors -restart=2f7573722f62696e2f6b796c696e2d7072696e746572
        args = QStringList()<< "-mode=3" << "-path=/home/yyf/sensors" << "-exe=kylin-printer" << "-restart=2f7573722f62696e2f6b796c696e2d7072696e746572";
    }
#endif
    QString mode;
    QString path;
    QString exe;
    QString restart;
    QString code;
    for (QString arg: args) {
        if (!arg.contains("=")) {
            continue;
        }
        if (arg.startsWith("-mode")) {
            mode = arg.mid(arg.indexOf("=") +1);
        } else if (arg.startsWith("-path")) {
            path = arg.mid(arg.indexOf("=") +1);
        } else if (arg.startsWith("-exe")) {
            exe = arg.mid(arg.indexOf("=") +1);
        } else if (arg.startsWith("-restart")) {
            restart = QByteArray::fromHex(arg.mid(arg.indexOf("=") +1).toLocal8Bit());
        }else if (arg.startsWith("-code")) {
            code = arg.mid(arg.indexOf("=") +1);
        }
    }
    if (exe.isEmpty()){
        qDebug()<<"exe为空";
        exit(-1);
    }
    QFileInfo fi(path);
    if ((mode == "2" || mode == "3") && (!fi.exists())){
        qDebug()<<"path不存在 "<<path;
        exit(-1);
    }

    // 单例运行
    singleton(exe);

    VoluntarilyWidget voluntarily;
    voluntarily.setArgs(path, exe, restart, mode, code);
    PassivityWidget passivity;
    passivity.setArgs(path, exe, restart, mode, code);
    if (mode == "1") {
        voluntarily.run();
        const auto primary_screen = QGuiApplication::screenAt(QCursor::pos())->geometry();
        const auto x = primary_screen.width() / 2 - voluntarily.width() / 2;
        const auto y = primary_screen.height() / 2 - voluntarily.height() / 2;
        voluntarily.move(x, y);
        voluntarily.show();
    } else if (mode == "2") {
        // 判断可执行文件是否在 7 天不在弹出的黑名单里
        if (Blacklist::Contain(exe)) {
            exit(-1);
        }
        passivity.run();
    } else if (mode == "3") {
        passivity.run();
    } else {
        qDebug()<<"mode值不合规 "<<mode;
        exit(-1);
    }
    qDebug()<<"解析参数完成";

    return app.exec();
}
