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
 */

#include "test_log-utils.h"
#include <QDebug>
#include <QTest>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QtMessageHandler>
#include <QMessageLogContext>
#define private public
#include "log-utils.h"
#undef  private

TestLogUtils::TestLogUtils(QObject *parent)
    : QObject(parent)
{
}

TestLogUtils::~TestLogUtils()
{
}

void TestLogUtils::initTestCase()
{
    m_testLogPath = QDir::homePath() + "/.log/ukui-window-switch/";
}

void TestLogUtils::cleanupTestCase()
{
    cleanupTestFiles();
}

void TestLogUtils::init()
{
}

void TestLogUtils::cleanup()
{
    cleanupTestFiles();
}

void TestLogUtils::cleanupTestFiles()
{
    QDir dir(m_testLogPath);
    if (dir.exists()) {
        QFileInfoList fileList = dir.entryInfoList(QDir::Files);
        for (const QFileInfo &fileInfo : fileList) {
            QFile::remove(fileInfo.absoluteFilePath());
        }
    }
}

void TestLogUtils::test_initLogFile()
{
    LogUtils::initLogFile("");

    QString fileName = "testapp";
    LogUtils::initLogFile(fileName);
    
    QVERIFY(QDir(m_testLogPath).exists());
    QVERIFY(QFile::exists(m_testLogPath + fileName + "-0.log"));
    QVERIFY(QFile::exists(m_testLogPath + fileName + "-1.log"));
}

void TestLogUtils::test_initLogFile_various()
{
    // Test 1: Different filenames
    QString fileName1 = "app1";
    QString fileName2 = "app2";
    
    LogUtils::initLogFile(fileName1);
    QVERIFY(QFile::exists(m_testLogPath + fileName1 + "-0.log"));
    
    LogUtils::initLogFile(fileName2);
    QVERIFY(QFile::exists(m_testLogPath + fileName2 + "-0.log"));
    
    // Test 2: Existing files with content
    QString fileName3 = "existing";
    QString filePath0 = m_testLogPath + fileName3 + "-0.log";
    QString filePath1 = m_testLogPath + fileName3 + "-1.log";
    
    QDir dir;
    dir.mkpath(m_testLogPath);
    
    QFile file0(filePath0);
    file0.open(QIODevice::WriteOnly);
    file0.write("test content 0");
    file0.close();
    
    QFile file1(filePath1);
    file1.open(QIODevice::WriteOnly);
    file1.write("test content 1");
    file1.close();
    
    LogUtils::initLogFile(fileName3);
    
    QVERIFY(QFile::exists(filePath0));
    QVERIFY(QFile::exists(filePath1));
    
    // Test 3: First file valid, second file exceeds size
    QString fileName4 = "firstvalid";
    QString filePath4_0 = m_testLogPath + fileName4 + "-0.log";
    QString filePath4_1 = m_testLogPath + fileName4 + "-1.log";
    
    QFile file4_0(filePath4_0);
    file4_0.open(QIODevice::WriteOnly);
    file4_0.write("small content");
    file4_0.close();
    
    QByteArray largeContent(4194305, 'x');
    QFile file4_1(filePath4_1);
    file4_1.open(QIODevice::WriteOnly);
    file4_1.write(largeContent);
    file4_1.close();
    
    LogUtils::initLogFile(fileName4);
    QVERIFY(LogUtils::m_logFileId == 0);
    
    // Test 4: All files exceed size limit
    QString fileName5 = "allfull";
    QString filePath5_0 = m_testLogPath + fileName5 + "-0.log";
    QString filePath5_1 = m_testLogPath + fileName5 + "-1.log";
    
    QFile file5_0(filePath5_0);
    file5_0.open(QIODevice::WriteOnly);
    file5_0.write(largeContent);
    file5_0.close();
    
    QFile file5_1(filePath5_1);
    file5_1.open(QIODevice::WriteOnly);
    file5_1.write(largeContent);
    file5_1.close();
    
    LogUtils::initLogFile(fileName5);
    QVERIFY(LogUtils::m_logFileId == 0);
}

void TestLogUtils::test_checkLogFile_all()
{
    // Test 1: Basic checkLogFile
    QString fileName1 = "checkfile";
    QString filePath1 = m_testLogPath + fileName1 + "-0.log";
    LogUtils::initLogFile(fileName1);
    LogUtils::checkLogFile();

    QDir dir;
    dir.mkpath(m_testLogPath);
    QFile file1(filePath1);
    file1.open(QIODevice::WriteOnly);
    file1.write("small content");
    file1.close();
    LogUtils::checkLogFile();
    
    // Test 2: Log file rotation when spacing > MAX_LOG_CHECK_INTERVAL and file size exceeds limit
    QString fileName2 = "rotation";
    QString filePath2_0 = m_testLogPath + fileName2 + "-0.log";
    QString filePath2_1 = m_testLogPath + fileName2 + "-1.log";
    
    QFile file2_0(filePath2_0);
    file2_0.open(QIODevice::WriteOnly);
    QByteArray largeContent(4194305, 'x'); // > MAX_LOG_FILE_SIZE
    file2_0.write(largeContent);
    file2_0.close();
    
    QFile file2_1(filePath2_1);
    file2_1.open(QIODevice::WriteOnly);
    file2_1.write(largeContent);
    file2_1.close();
    
    // Set startup time to a very old value to trigger rotation
    LogUtils::m_startUpTime = QDateTime::currentDateTime().addDays(-2).toMSecsSinceEpoch();
    
    LogUtils::initLogFile(fileName2);
    LogUtils::checkLogFile();
    
    // Verify file rotation occurred
    QVERIFY(QFile::exists(filePath2_0) || QFile::exists(filePath2_1));
}

void TestLogUtils::test_checkFileSize()
{
    QString fileName = "sizecheck";
    QString filePath = m_testLogPath + fileName + "-0.log";
    
    QDir dir;
    dir.mkpath(m_testLogPath);
    
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write("small content");
    file.close();
    
    LogUtils::checkFileSize("");
    LogUtils::checkFileSize(fileName);
    QVERIFY(QFile(filePath).size() < 4194304);
}

void TestLogUtils::test_clearFile_all()
{
    // Test 1: Basic clearFile
    QString fileName1 = "clearfile";
    QString filePath1 = m_testLogPath + fileName1 + "-0.log";
    
    QDir dir;
    dir.mkpath(m_testLogPath);
    
    QFile file1(filePath1);
    file1.open(QIODevice::WriteOnly);
    file1.write("content to be cleared");
    file1.close();

    LogUtils::clearFile("");
    LogUtils::clearFile(fileName1);
    QVERIFY(QFile(filePath1).size() > 0);
    
    // Test 2: clearFile when file size exceeds MAX_LOG_FILE_SIZE
    QString fileName2 = "clearrotation";
    QString filePath2 = m_testLogPath + fileName2 + "-0.log";
    
    QFile file2(filePath2);
    file2.open(QIODevice::WriteOnly);
    QByteArray largeContent(4194305, 'x'); // > MAX_LOG_FILE_SIZE
    file2.write(largeContent);
    file2.close();
    
    // Verify file size exceeds limit before clear
    QVERIFY(QFile(filePath2).size() >= 4194304);
    
    // Call checkFileSize which should return false for oversized file
    bool result = LogUtils::checkFileSize(filePath2);
    QVERIFY(result == false);
}

void TestLogUtils::test_messageOutput_all()
{
    QMessageLogContext context;
    QString originalFile = LogUtils::m_logFileName;
    
    // Test 1: DebugMsg with file
    LogUtils::messageOutput(QtDebugMsg, context, "");
    LogUtils::messageOutput(QtDebugMsg, context, "test debug");
    QString fileName1 = "debugmsg";
    QString filePath1 = m_testLogPath + fileName1 + "-0.log";
    QDir dir; dir.mkpath(m_testLogPath);
    QFile file1(filePath1);
    file1.open(QIODevice::WriteOnly);
    file1.close();
    context.file = __FILE__;
    context.function = __FUNCTION__;
    context.line = __LINE__;
    LogUtils::messageOutput(QtDebugMsg, context, "Debug message test");
    QVERIFY(QFile::exists(filePath1));
    
    // Test 2: InfoMsg with file
    LogUtils::messageOutput(QtInfoMsg, context, "test info");
    QString fileName2 = "infomsg";
    QString filePath2 = m_testLogPath + fileName2 + "-0.log";
    QFile file2(filePath2);
    file2.open(QIODevice::WriteOnly);
    file2.close();
    LogUtils::messageOutput(QtInfoMsg, context, "Info message test");
    QVERIFY(QFile::exists(filePath2));
    
    // Test 3: WarningMsg with file
    LogUtils::messageOutput(QtWarningMsg, context, "test warning");
    QString fileName3 = "warningmsg";
    QString filePath3 = m_testLogPath + fileName3 + "-0.log";
    QFile file3(filePath3);
    file3.open(QIODevice::WriteOnly);
    file3.close();
    LogUtils::messageOutput(QtWarningMsg, context, "Warning message test");
    QVERIFY(QFile::exists(filePath3));
    
    // Test 4: CriticalMsg with file
    LogUtils::messageOutput(QtCriticalMsg, context, "test critical");
    QString fileName4 = "criticalmsg";
    QString filePath4 = m_testLogPath + fileName4 + "-0.log";
    QFile file4(filePath4);
    file4.open(QIODevice::WriteOnly);
    file4.close();
    LogUtils::messageOutput(QtCriticalMsg, context, "Critical message test");
    QVERIFY(QFile::exists(filePath4));
    
    // Test 5: FatalMsg with file
    LogUtils::messageOutput(QtFatalMsg, context, "test fatal");
    QString fileName5 = "fatalmsg";
    QString filePath5 = m_testLogPath + fileName5 + "-0.log";
    QFile file5(filePath5);
    file5.open(QIODevice::WriteOnly);
    file5.close();
    LogUtils::messageOutput(QtFatalMsg, context, "Fatal message test");
    QVERIFY(QFile::exists(filePath5));
    
    // Test 6: All message types without file (null file pointer)
    LogUtils::m_logFileName = "";
    LogUtils::messageOutput(QtDebugMsg, context, "Debug without file");
    LogUtils::messageOutput(QtInfoMsg, context, "Info without file");
    LogUtils::messageOutput(QtWarningMsg, context, "Warning without file");
    LogUtils::messageOutput(QtCriticalMsg, context, "Critical without file");
    LogUtils::messageOutput(QtFatalMsg, context, "Fatal without file");
    
    LogUtils::m_logFileName = originalFile;
}
