/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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
 */

#include <QtTest>
#include <QTemporaryDir>

#define private public
#include "../../src/log-utils.h"
#undef private

class logUtils_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_checkFileSizeAndClearFile();
    void test_messageOutput_writesAllTypes();
    void test_messageOutput_handlesNullContextAndOpenFailure();
    void test_checkLogFile_keepsSmallRecentFile();
    void test_checkLogFile_rotatesOnLargeFile();
    void test_checkLogFile_rotatesToSmallFileWithoutClearing();
    void test_initLogFile_createsHomeLogFiles();
};

void logUtils_test::init()
{
    LogUtils::m_startUpTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    LogUtils::m_logFileId = 0;
    LogUtils::m_logFileName.clear();
}

void logUtils_test::test_checkFileSizeAndClearFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString fileName = dir.path() + "/test.log";
    QFile file(fileName);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("abc");
    file.close();

    QVERIFY(LogUtils::checkFileSize(fileName));
    LogUtils::clearFile(fileName);
    QCOMPARE(QFile(fileName).size(), 0);
}

void logUtils_test::test_messageOutput_writesAllTypes()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    LogUtils::m_logFileName = dir.path() + "/message-%0.log";
    LogUtils::m_logFileId = 0;
    LogUtils::m_startUpTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    QMessageLogContext context("file.cpp", 12, "function", "category");
    LogUtils::messageOutput(QtDebugMsg, context, "debug-msg");
    LogUtils::messageOutput(QtInfoMsg, context, "info-msg");
    LogUtils::messageOutput(QtWarningMsg, context, "warning-msg");
    LogUtils::messageOutput(QtCriticalMsg, context, "critical-msg");
    LogUtils::messageOutput(QtFatalMsg, context, "fatal-msg");

    QFile file(LogUtils::m_logFileName.arg(0));
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QByteArray data = file.readAll();
    QVERIFY(data.contains("Debug:"));
    QVERIFY(data.contains("Info:"));
    QVERIFY(data.contains("Warning:"));
    QVERIFY(data.contains("Critical:"));
    QVERIFY(data.contains("Fatal:"));
}

void logUtils_test::test_messageOutput_handlesNullContextAndOpenFailure()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    LogUtils::m_logFileName = dir.path() + "/blocked-%0.log";
    LogUtils::m_logFileId = 0;
    LogUtils::m_startUpTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    QVERIFY(QDir().mkdir(LogUtils::m_logFileName.arg(0)));

    QMessageLogContext context(nullptr, 0, nullptr, nullptr);
    LogUtils::messageOutput(QtDebugMsg, context, "debug-without-file");
    LogUtils::messageOutput(QtInfoMsg, context, "info-without-file");
    LogUtils::messageOutput(QtWarningMsg, context, "warning-without-file");
    LogUtils::messageOutput(QtCriticalMsg, context, "critical-without-file");
    LogUtils::messageOutput(QtFatalMsg, context, "fatal-without-file");
    LogUtils::messageOutput(static_cast<QtMsgType>(999), context, "unknown-type");

    QVERIFY(QFileInfo(LogUtils::m_logFileName.arg(0)).isDir());
}

void logUtils_test::test_checkLogFile_keepsSmallRecentFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    LogUtils::m_logFileName = dir.path() + "/rotate-%0.log";
    LogUtils::m_logFileId = 0;
    LogUtils::m_startUpTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    QFile(LogUtils::m_logFileName.arg(0)).open(QIODevice::WriteOnly);

    LogUtils::checkLogFile();

    QCOMPARE(LogUtils::m_logFileId.loadRelaxed(), 0);
}

void logUtils_test::test_checkLogFile_rotatesOnLargeFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    LogUtils::m_logFileName = dir.path() + "/rotate-%0.log";
    LogUtils::m_logFileId = 0;
    LogUtils::m_startUpTime = 0;

    QFile current(LogUtils::m_logFileName.arg(0));
    QVERIFY(current.open(QIODevice::WriteOnly));
    current.write("old");
    current.close();
    QFile next(LogUtils::m_logFileName.arg(1));
    QVERIFY(next.open(QIODevice::WriteOnly));
    next.resize(4194304);
    next.close();

    LogUtils::checkLogFile();

    QCOMPARE(LogUtils::m_logFileId.loadRelaxed(), 1);
    QCOMPARE(QFile(LogUtils::m_logFileName.arg(1)).size(), 0);
}

void logUtils_test::test_checkLogFile_rotatesToSmallFileWithoutClearing()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    LogUtils::m_logFileName = dir.path() + "/rotate-%0.log";
    LogUtils::m_logFileId = 0;
    LogUtils::m_startUpTime = 0;

    QFile next(LogUtils::m_logFileName.arg(1));
    QVERIFY(next.open(QIODevice::WriteOnly));
    next.write("keep");
    next.close();

    LogUtils::checkLogFile();

    QCOMPARE(LogUtils::m_logFileId.loadRelaxed(), 1);
    QFile result(LogUtils::m_logFileName.arg(1));
    QVERIFY(result.open(QIODevice::ReadOnly));
    QCOMPARE(result.readAll(), QByteArray("keep"));
}

void logUtils_test::test_initLogFile_createsHomeLogFiles()
{
    LogUtils::m_logFileId = -1;

    LogUtils::initLogFile("ukui-sidebar-autotest");

    QVERIFY(LogUtils::m_logFileName.contains("ukui-sidebar-autotest-%0.log"));
    QVERIFY(LogUtils::m_logFileId.loadRelaxed() >= 0);
    QVERIFY(LogUtils::m_logFileName.startsWith(QDir::homePath() + "/.log/ukui-sidebar/"));
}

QTEST_MAIN(logUtils_test)

#include "log-utils-test.moc"
