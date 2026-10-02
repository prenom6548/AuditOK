#include "application-info-log-utils-test.h"

#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#define private public
#include "log-utils.h"
#undef private

using namespace UkuiSearch;

void ApplicationInfoLogUtilsTest::logUtilsInitLogFileCreatesAndSelectsExpectedFiles()
{
    const QString logDir = QDir::homePath() + QStringLiteral("/.log/ukui-search");
    QVERIFY(QDir().mkpath(logDir));

    const QString baseName = QStringLiteral("codex-log-utils-init");
    const QString log0 = logDir + QLatin1Char('/') + baseName + QStringLiteral("-0.log");
    const QString log1 = logDir + QLatin1Char('/') + baseName + QStringLiteral("-1.log");

    {
        QFile file(log0);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        file.resize(4194304);
    }
    {
        QFile file(log1);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        QVERIFY(file.write("keep-me") > 0);
    }

    LogUtils::m_logFileId = -1;
    LogUtils::initLogFile(baseName);
    QCOMPARE(LogUtils::m_logFileId.loadRelaxed(), 1);
    QCOMPARE(LogUtils::m_logFileName, logDir + QLatin1Char('/') + baseName + QStringLiteral("-%0.log"));
    QCOMPARE(QFile(log1).size(), qint64(7));

    QFile::remove(log1);
    LogUtils::m_logFileId = -1;
    LogUtils::initLogFile(baseName);
    QCOMPARE(LogUtils::m_logFileId.loadRelaxed(), 0);
    QVERIFY(QFile::exists(log0));
    QVERIFY(QFile::exists(log1));
    QCOMPARE(QFile(log0).size(), qint64(0));

    QFile::remove(log0);
    QFile::remove(log1);
}

void ApplicationInfoLogUtilsTest::logUtilsWritesAllMessageTypesAndRotatesFiles()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    LogUtils::m_logFileId = -1;
    LogUtils::m_logFileName = tempDir.path() + QStringLiteral("/whitebox-%0.log");

    const QString log0 = tempDir.path() + QStringLiteral("/whitebox-0.log");
    const QString log1 = tempDir.path() + QStringLiteral("/whitebox-1.log");
    LogUtils::clearFile(log0);
    LogUtils::clearFile(log1);
    LogUtils::m_logFileId = 0;
    LogUtils::m_startUpTime = QDateTime::currentMSecsSinceEpoch();
    QVERIFY(QFile::exists(log0));
    QVERIFY(QFile::exists(log1));
    QVERIFY(LogUtils::checkFileSize(log0));

    QMessageLogContext context(__FILE__, __LINE__, Q_FUNC_INFO, "whitebox");
    LogUtils::messageOutput(QtDebugMsg, context, QStringLiteral("debug-message"));
    LogUtils::messageOutput(QtInfoMsg, context, QStringLiteral("info-message"));
    LogUtils::messageOutput(QtWarningMsg, context, QStringLiteral("warning-message"));
    LogUtils::messageOutput(QtCriticalMsg, context, QStringLiteral("critical-message"));
    LogUtils::messageOutput(QtFatalMsg, context, QStringLiteral("fatal-message"));

    QFile currentLog(LogUtils::m_logFileName.arg(LogUtils::m_logFileId.loadRelaxed()));
    QVERIFY(currentLog.open(QIODevice::ReadOnly));
    const QByteArray content = currentLog.readAll();
    QVERIFY(content.contains("debug-message"));
    QVERIFY(content.contains("info-message"));
    QVERIFY(content.contains("warning-message"));
    QVERIFY(content.contains("critical-message"));
    QVERIFY(content.contains("fatal-message"));

    QFile oversized(LogUtils::m_logFileName.arg(LogUtils::m_logFileId.loadRelaxed()));
    QVERIFY(oversized.open(QIODevice::WriteOnly | QIODevice::Truncate));
    oversized.resize(4194304);
    oversized.close();
    QVERIFY(!LogUtils::checkFileSize(oversized.fileName()));

    const int beforeRotate = LogUtils::m_logFileId.loadRelaxed();
    LogUtils::checkLogFile();
    QCOMPARE(LogUtils::m_logFileId.loadRelaxed(), (beforeRotate + 1) % 2);

    LogUtils::clearFile(oversized.fileName());
    QCOMPARE(QFile(oversized.fileName()).size(), qint64(0));
}

void ApplicationInfoLogUtilsTest::logUtilsHandlesMissingLogFileAndTimeBasedRotation()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString missingDir = tempDir.path() + QStringLiteral("/missing");
    LogUtils::m_logFileName = missingDir + QStringLiteral("/whitebox-%0.log");
    LogUtils::m_logFileId = 0;
    LogUtils::m_startUpTime = QDateTime::currentMSecsSinceEpoch();

    QMessageLogContext context(nullptr, 0, nullptr, "whitebox");
    LogUtils::messageOutput(QtDebugMsg, context, QStringLiteral("debug-without-file"));
    LogUtils::messageOutput(QtInfoMsg, context, QStringLiteral("info-without-file"));
    LogUtils::messageOutput(QtWarningMsg, context, QStringLiteral("warning-without-file"));
    LogUtils::messageOutput(QtCriticalMsg, context, QStringLiteral("critical-without-file"));
    LogUtils::messageOutput(QtFatalMsg, context, QStringLiteral("fatal-without-file"));

    const QString log0 = tempDir.path() + QStringLiteral("/rotate-0.log");
    const QString log1 = tempDir.path() + QStringLiteral("/rotate-1.log");
    LogUtils::m_logFileName = tempDir.path() + QStringLiteral("/rotate-%0.log");
    LogUtils::clearFile(log0);
    LogUtils::clearFile(log1);
    LogUtils::m_logFileId = 0;
    LogUtils::m_startUpTime = 1;

    LogUtils::checkLogFile();
    QCOMPARE(LogUtils::m_logFileId.loadRelaxed(), 1);
}

QTEST_MAIN(ApplicationInfoLogUtilsTest)
