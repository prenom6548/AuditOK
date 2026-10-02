/*
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
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>

#include "qtsingleapplication.h"
#include "ukui-menu-application.h"

#define LOG_FILE_COUNT         2
#define MAX_LOG_FILE_SIZE      4194304
#define MAX_LOG_CHECK_INTERVAL 43200000

static int logFileId = -1;
static QString logFile     = "ukui-menu.log";
static QString logFilePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.log/ukui-menu/";
static QString logFileName = logFilePath + "ukui-menu-%1.log";
static quint64 startupTime;

bool checkFileSize(const QString &fileName) {
    return QFile(fileName).size() < MAX_LOG_FILE_SIZE;
}

void clearFile(const QString &fileName) {
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    file.write("");
    file.flush();
    file.close();
}

void initLogFile() {
    QDir dir;
    if (!dir.exists(logFilePath)) {
        if (!dir.mkpath(logFilePath)) {
            qWarning() << "Unable to create" << logFilePath;
            return;
        }
    }

    for (int i = 0; i < LOG_FILE_COUNT; ++i) {
        logFile = logFileName.arg(i);
        if (QFile::exists(logFile)) {
            if (checkFileSize(logFile)) {
                logFileId = i;
                break;
            }
        } else {
            QFile file(logFile);
            file.open(QIODevice::WriteOnly);
            file.close();
        }
    }

    if (logFileId < 0) {
        logFileId = 0;
        logFile = logFileName.arg(logFileId);
        clearFile(logFile);
    }

    qInfo() << "init log file:" << logFile;
}

void checkLogFile() {
    quint64 logTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    quint64 spacing = std::max(logTime, startupTime) - std::min(logTime, startupTime);

    if (spacing <= MAX_LOG_CHECK_INTERVAL || checkFileSize(logFile)) {
        return;
    }

    logFileId = ((logFileId + 1) % LOG_FILE_COUNT);
    logFile = logFileName.arg(logFileId);
    if (!checkFileSize(logFile)) {
        clearFile(logFile);
    }
}

void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    checkLogFile();

    QByteArray localMsg = msg.toLocal8Bit();
    QByteArray currentTime = QTime::currentTime().toString().toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";

    FILE *log_file = fopen(logFile.toLocal8Bit().constData(), "a+");

    switch (type) {
        case QtDebugMsg:
            if (!log_file) {
                break;
            }
            fprintf(log_file, "Debug: %s: %s (%s:%u, %s)\n", currentTime.constData(), localMsg.constData(), file, context.line, function);
            break;
        case QtInfoMsg:
            fprintf(log_file? log_file: stdout, "Info: %s: %s (%s:%u, %s)\n", currentTime.constData(), localMsg.constData(), file, context.line, function);
            break;
        case QtWarningMsg:
            fprintf(log_file? log_file: stderr, "Warning: %s: %s (%s:%u, %s)\n", currentTime.constData(), localMsg.constData(), file, context.line, function);
            break;
        case QtCriticalMsg:
            fprintf(log_file? log_file: stderr, "Critical: %s: %s (%s:%u, %s)\n", currentTime.constData(), localMsg.constData(), file, context.line, function);
            break;
        case QtFatalMsg:
            fprintf(log_file? log_file: stderr, "Fatal: %s: %s (%s:%u, %s)\n", currentTime.constData(), localMsg.constData(), file, context.line, function);
            break;
    }

    if (log_file) {
        fclose(log_file);
    }
}

int main(int argc, char *argv[])
{
    startupTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
#ifndef UKUI_MENU_LOG_FILE_DISABLE
    initLogFile();
    qInstallMessageHandler(messageOutput);
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#else

#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QCoreApplication::setApplicationName("ukui-menu");
    QCoreApplication::setOrganizationName("ukui");
    QCoreApplication::setOrganizationDomain("ukui.org");
    QCoreApplication::setApplicationVersion("4.23.0.0");

    QString display;
    QString sessionType(qgetenv("XDG_SESSION_TYPE"));
    if (sessionType == "wayland") {
        display = QLatin1String(getenv("WAYLAND_DISPLAY"));
    } else {
        display = QLatin1String(getenv("DISPLAY"));
    }

    QString appid = QString("ukui-menu-%1").arg(display);
    qDebug() << "ukui-menu launch with:" << display << "appid:" << appid;
    QtSingleApplication app(appid, argc, argv);
    QGuiApplication::instance()->setProperty("display", display);

    QTranslator translator;
    QString translationFile{(QString(UKUI_MENU_TRANSLATION_DIR) + "/ukui-menu_" + QLocale::system().name() + ".qm")};
    // translation files
    if (QFile::exists(translationFile) && translator.load(translationFile)) {
        QCoreApplication::installTranslator(&translator);
    } else {
        qWarning() << "Unable to load translation file:" << translationFile;
    }

    UkuiMenu::MenuMessageProcessor messageProcessor;

    if (app.isRunning()) {
        if (UkuiMenu::MenuMessageProcessor::preprocessMessage(QtSingleApplication::arguments())) {
            app.sendMessage(QtSingleApplication::arguments().join(" ").toUtf8());
        }
        return 0;
    }

    UkuiMenu::UkuiMenuApplication menuApplication(&messageProcessor);
    messageProcessor.processMessage(QtSingleApplication::arguments().join(" ").toUtf8());
    QObject::connect(&app, &QtSingleApplication::messageReceived,
                     &messageProcessor, &UkuiMenu::MenuMessageProcessor::processMessage);

    qInfo() << "ukui-menu startup time:" << (QDateTime::currentDateTime().toMSecsSinceEpoch() - startupTime)
             << ",date:" << QDateTime::currentDateTime().toString();
    return QtSingleApplication::exec();
}
