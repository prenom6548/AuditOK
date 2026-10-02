/*
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include <gtest/gtest.h>
#include <QString>
#include "ukui-log4qt.h"
#include "ukui-logconfigurator.h"
#include "ukui-logmacros.h"
#include "ukui-logrolling.h"
#include "../../src/log4qt/log4qt/propertyconfigurator.h"
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QFileInfoList>

#include <QObject>
#include <QBuffer>
#include <QBitArray>
#include <QDebug>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QTextStream>
#include <QThread>
#include <QDataStream>
#include "../../src/log4qt/log4qt/helpers/properties.h"
#include "../../src/log4qt/log4qt/logger.h"
#include "../../src/log4qt/log4qt/varia/listappender.h"
#include "../../src/log4qt/log4qt/varia/debugappender.h"
#include "../../src/log4qt/log4qt/varia/nullappender.h"
#include "../../src/log4qt/log4qt/basicconfigurator.h"
#include "../../src/log4qt/log4qt/consoleappender.h"
#include "../../src/log4qt/log4qt/dailyrollingfileappender.h"
#include "../../src/log4qt/log4qt/fileappender.h"
#include "../../src/log4qt/log4qt/helpers/configuratorhelper.h"
#include "../../src/log4qt/log4qt/helpers/datetime.h"
#include "../../src/log4qt/log4qt/helpers/factory.h"
#include "../../src/log4qt/log4qt/helpers/initialisationhelper.h"
#include "../../src/log4qt/log4qt/helpers/optionconverter.h"
#include "../../src/log4qt/log4qt/helpers/patternformatter.h"
#include "../../src/log4qt/log4qt/helpers/properties.h"
#include "../../src/log4qt/log4qt/helpers/logerror.h"
#include "../../src/log4qt/log4qt/logmanager.h"
#include "../../src/log4qt/log4qt/loggerrepository.h"
#include "../../src/log4qt/log4qt/patternlayout.h"
#include "../../src/log4qt/log4qt/propertyconfigurator.h"
#include "../../src/log4qt/log4qt/rollingfileappender.h"
#include "../../src/log4qt/log4qt/simplelayout.h"
#include "../../src/log4qt/log4qt/ttcclayout.h"
#include "../../src/log4qt/log4qt/varia/denyallfilter.h"
#include "../../src/log4qt/log4qt/varia/levelmatchfilter.h"
#include "../../src/log4qt/log4qt/varia/levelrangefilter.h"
#include "../../src/log4qt/log4qt/varia/stringmatchfilter.h"

using namespace Log4Qt;

// 辅助函数，用于检查日志输出（在实际测试中可能需要更复杂的逻辑来捕获和验证日志输出）
void CheckLogOutput(const QString &expected)
{
    // 注意：这里只是一个占位符，实际实现需要捕获日志输出并进行比较
    // 例如，可以使用重定向输出流或使用日志框架提供的API来获取日志内容
    // 由于示例代码中没有提供这些功能，因此这里仅作为示意
    EXPECT_EQ(QString("Dummy log output for testing"), expected); // 需要替换为实际的日志捕获和比较逻辑
}

// 测试 initUkuiLog4qt 函数
TEST(UkuiLog4qtTest, TestInit)
{
    QString appName = "test-app";
    int result = initUkuiLog4qt(appName);
    EXPECT_NE(result, -1); // 假设非-1表示成功初始化

    // 可以通过检查日志配置或输出进一步验证初始化是否成功
    // 例如，检查日志文件是否被创建或日志输出是否符合预期
    // 这里使用CheckLogOutput作为示意，但实际实现需要调整
    CheckLogOutput("Expected log output after initialization");
}

// 测试日志宏
TEST(UkuiLog4qtTest, TestLogMacros)
{
    // 初始化日志系统（在实际测试中，这个初始化可能已经在测试框架的setup函数中完成）
    initUkuiLog4qt("test-app-for-macros");

    // 使用日志宏记录不同级别的日志
    KyDebug("This is a debug message");
    KyInfo("This is an info message");
    KyWarning("This is a warning message");
    KyCritical("This is a critical message");
    // KyFatal("This is a fatal message");
}

// Global variables for testing
QTemporaryDir *tempDir = nullptr;
// 当前属性列表
Log4Qt::Properties g_log4qtProperties;

// Test fixture for UKUI log system
class UkuiLogTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        // Initialize temporary directory for log files
        tempDir = new QTemporaryDir();
        ASSERT_TRUE(tempDir->isValid());

        // Initialize UKUI log system
        UkuiLog4qtConfig::instance()->init("testapp");
    }

    static void TearDownTestSuite()
    {
        // Cleanup
        delete tempDir;
        UkuiLog4qtConfig::instance()->shutdown();
    }
};

TEST_F(UkuiLogTest, InitializeLogSystem)
{
    // Verify that log system is initialized correctly
    ASSERT_TRUE(UkuiLog4qtConfig::instance()->m_isInited);
}

TEST_F(UkuiLogTest, ConfigureLogProperties)
{
    // Verify that properties are applied correctly
    Log4Qt::Properties properties;
    Log4Qt::PropertyConfigurator::configure(properties);
    EXPECT_EQ(properties.property("log4j.threshold"), "DEBUG");
    EXPECT_EQ(properties.property("log4j.rootLogger"), "DEBUG,daily");
    EXPECT_EQ(properties.property("ukui.log.dailyDatePattern"), ".yyyy-MM-dd");
}

TEST_F(UkuiLogTest, LogFileRolling)
{
    // Configure log rolling with a temporary directory
    QString logFilePath = tempDir->path() + "/test.log";
    UkuiLog4qtRolling roller(logFilePath, 3, 1024 * 1024, 60 * 1000); // 3 files, 1MB each, check every minute
    roller.start();

    // Simulate log file creation and growth
    QFile file(logFilePath);
    for (int i = 0; i < 4; ++i) {
        file.open(QIODevice::WriteOnly | QIODevice::Append);
        file.write(QByteArray("a").repeated(1024 * 1024)); // Write 1MB
        file.close();

        // Sleep for a short duration to simulate time passing (not necessary in this simplified test)
        QThread::sleep(1);
    }

    // Stop the roller and verify the number of rolled files
    roller.stop();
    roller.wait();

    QDir dir(tempDir->path());
    QStringList filters;
    filters << "test.log.*";
    dir.setNameFilters(filters);
    QList<QFileInfo> fileList = dir.entryInfoList(QDir::Files);

    // Expect 3 rolled files (test.log.0, test.log.1, test.log.2) and the current test.log
    EXPECT_EQ(fileList.size(), 4);
}

// 用于测试的辅助函数
QString createTempFile(const QString &dirPath, const QString &baseName = "testlog")
{
    QFile file(dirPath + "/" + baseName + ".log");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "This is a test log file." << Qt::endl;
        file.close();
    }
    return file.fileName();
}

// 测试类
class UkuiLog4qtRollingTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        tempDir.setAutoRemove(true);
        baseFilePath = tempDir.path() + "/testlog.log";
        createTempFile(tempDir.path()); // 创建一个初始日志文件

        rolling = new UkuiLog4qtRolling(baseFilePath, 3, 1, 1, nullptr); // 最大文件数3，最大文件大小1MB，检查间隔1秒
    }

    static void TearDownTestSuite()
    {
        rolling->stop();
        rolling->wait();
        delete rolling;
    }

    static QTemporaryDir tempDir;
    static QString baseFilePath;
    static UkuiLog4qtRolling *rolling;
};

QTemporaryDir UkuiLog4qtRollingTest::tempDir;
QString UkuiLog4qtRollingTest::baseFilePath = "";
UkuiLog4qtRolling *UkuiLog4qtRollingTest::rolling = nullptr;

TEST_F(UkuiLog4qtRollingTest, TestFileCountLimit)
{
    // 创建超过最大文件数的日志文件
    for (int i = 1; i <= 5; ++i) {
        createTempFile(tempDir.path(), QString("testlog_%1").arg(i));
    }

    // 启动滚动检查
    rolling->start();
    QThread::sleep(2); // 等待足够的时间让检查完成

    // 验证文件数量
    QDir dir(tempDir.path());
    dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    QFileInfoList fileList = dir.entryInfoList();
    EXPECT_EQ(fileList.count(), 4); // 应该只保留4个文件（包括初始的testlog.log和新创建的3个）
}

TEST_F(UkuiLog4qtRollingTest, TestFileSizeLimit)
{
    // 创建一个超过最大文件大小的日志文件
    QFile file(baseFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        for (int i = 0; i < 1024 * 1024 / 20; ++i) { // 大约1MB的内容
            out << "This is a line to exceed the size limit." << Qt::endl;
        }
        file.close();
    }

    // 启动滚动检查
    rolling->start();
    QThread::sleep(2); // 等待足够的时间让检查完成

    // 验证文件大小
    QFileInfo fileInfo(baseFilePath);
    EXPECT_LE(fileInfo.size(), quint64(1 * 1024 * 1024)); // 文件大小应该被截断到1MB以内
}

TEST_F(UkuiLog4qtRollingTest, TestConfigChange)
{
    // 修改配置并验证重启
    rolling->setFileCheckLimit(2, 2 * 1024 * 1024, 1); // 最大文件数2，最大文件大小2MB

    // 创建超过新限制的日志文件
    for (int i = 1; i <= 3; ++i) {
        createTempFile(tempDir.path(), QString("testlog_%1").arg(i));
    }

    // 启动滚动检查
    rolling->start();
    QThread::sleep(2); // 等待足够的时间让检查完成

    // 验证文件数量
    QDir dir(tempDir.path());
    dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    QFileInfoList fileList = dir.entryInfoList();
    EXPECT_EQ(fileList.count(), 2); // 应该只保留2个文件
}

LOG4QT_DECLARE_STATIC_LOGGER(test_logger, Test::TestLog4Qt)
// 测试类
class Log4QtTest : public ::testing::Test, public QObject
{
protected:
    static void SetUpTestSuite()
    {
        mSkipLongTests = false;
        mpLoggingEvents = 0;
        mProperties = &mDefaultProperties;

        // Logging
        LogManager::resetConfiguration();
        LogManager::instance()->loggers();
        LogManager::instance()->setThreshold(Level::DEBUG_INT);
        qDebug() << LogManager::instance()->threshold() << LogManager::instance()->version();
        LogManager::instance()->welcome();
        qDebug() << *LogManager::instance();

        // File system
        QString name = QDir::tempPath() + "/Log4QtTest_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        if (!mTemporaryDirectory.mkdir(name))
            FAIL();
        mTemporaryDirectory.setPath(name);
        qDebug() << "Using temporaray directory: " << mTemporaryDirectory.path();

        // Appender to track events generated by Log4Qt
        mpLoggingEvents = new Log4Qt::ListAppender();
        mpLoggingEvents->retain();
        mpLoggingEvents->setName("Log4QtTest");
        mpLoggingEvents->setConfiguratorList(true);
        resetLogging();
    }

    static void TearDownTestSuite()
    {
        LogManager::resetConfiguration();

        if (!deleteDirectoryTree(mTemporaryDirectory.path()))
            FAIL();
        mpLoggingEvents->release();
    }

    static void resetLogging()
    {
        Log4Qt::Logger *p_logger;

        // Log4Qt logger
        p_logger = LogManager::logLogger();
        p_logger->setAdditivity(false);
        p_logger->setLevel(Level::WARN_INT);
        p_logger->removeAllAppenders();

        // Log4QtTest appender
        p_logger->addAppender(mpLoggingEvents);
        mpLoggingEvents->clearList();
        mpLoggingEvents->clearFilters();
        mpLoggingEvents->setThreshold(Level::WARN_INT);

        // Test logger
        p_logger = test_logger();
        p_logger->setAdditivity(true);
        p_logger->setLevel(Level::NULL_INT);
    }

    static bool deleteDirectoryTree(const QString &rName)
    {
        QFileInfo file_info(rName);
        if (!file_info.exists())
            return true;
        if (file_info.isDir()) {
            QDir d(rName);
            QStringList members = d.entryList(
                QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Hidden,
                QDir::Name | QDir::DirsFirst);
            QString member;
            Q_FOREACH (member, members)
                if (!deleteDirectoryTree(rName + '/' + member))
                    return false;
            if (d.rmdir(rName))
                return true;
            qDebug() << "Unable to remove directory: " << rName;
            return false;
        } else {
            QFile f(rName);
            if (f.remove())
                return true;
            qDebug() << "Unable to remove file: " << rName << "(" << f.errorString() << ")";
            return false;
        }
    }

    bool compareStringLists(
        const QStringList &rActual,
        const QStringList &rExpected,
        const QString &rEntry,
        const QString &rEntries,
        QString &rResult)
    {
        QString tab("   ");
        QString eol("\n");

        // Generate content string
        QString content;
        int i;
        content += tab + "Actual: " + rEntries + ": " + QString::number(rActual.count()) + eol;
        for (i = 0; i < rActual.count(); i++)
            content += tab + tab + '\'' + rActual.at(i) + '\'' + eol;
        content += tab + "Expected: " + rEntries + ": " + QString::number(rExpected.count()) + eol;
        for (i = 0; i < rExpected.count(); i++)
            content += tab + tab + '\'' + rExpected.at(i) + '\'' + eol;

        // Check count
        if (rActual.count() != rExpected.count()) {
            rResult = tab + "Compared " + rEntry + " counts are not the same" + eol;
            rResult += content;
            return false;
        }

        // Check entries
        for (i = 0; i < rActual.count(); i++) {
            if (rActual.at(i) != rExpected.at(i)) {
                rResult = tab + rEntry + " " + QString::number(i + 1) + " is not the same" + eol;
                rResult += content;
                return false;
            }
        }

        rResult.clear();
        return true;
    }

    bool validateDirContents(const QString &rName, const QStringList &rExpected, QString &rResult)
    {
        QDir dir(rName);
        if (!dir.exists()) {
            rResult = QString("The dir '%1' does not exist").arg(rName);
            return false;
        }

        QStringList actual = dir.entryList(
            QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Hidden,
            QDir::Name | QDir::DirsFirst);
        if (!compareStringLists(actual, rExpected, "Entry", "Entries", rResult)) {
            QString error = "The directory contents validation failed.\n   '%1'\n%2";
            rResult = error.arg(rName, rResult);
            return false;
        }

        return true;
    }

    bool validateFileContents(const QString &rName, const QStringList &rExpected, QString &rResult)
    {
        QFile file(rName);
        if (!file.exists()) {
            rResult = QString("The expected file '%1' does not exist (%2)").arg(rName).arg(file.errorString());
            return false;
        }
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            rResult = QString("The expected file '%1' cannot be opened (%2)").arg(rName).arg(file.errorString());
            return false;
        }

        QStringList actual;
        QTextStream textstream(&file);
        QString line = textstream.readLine();
        while (!line.isNull()) {
            actual << line;
            line = textstream.readLine();
        }
        if (!compareStringLists(actual, rExpected, "Line", "Lines", rResult)) {
            QString error = "The file contents validation failed.\n   '%1'\n%2";
            rResult = error.arg(rName, rResult);
            return false;
        }

        return true;
    }

    static QString dailyRollingFileAppenderSuffix(const QDateTime &rDateTime)
    {
        QString result(".");
        result += QString::number(rDateTime.date().year()).rightJustified(4, '0');
        result += '-';
        result += QString::number(rDateTime.date().month()).rightJustified(2, '0');
        result += '-';
        result += QString::number(rDateTime.date().day()).rightJustified(2, '0');
        result += '-';
        result += QString::number(rDateTime.time().hour()).rightJustified(2, '0');
        result += '-';
        result += QString::number(rDateTime.time().minute()).rightJustified(2, '0');
        return QString(result);
    }

    QString enumValueToKey(QObject *pObject, const char *pEnumeration, int value)
    {
        Q_ASSERT(pObject);
        Q_ASSERT(!QString(pEnumeration).isEmpty());

        int i = pObject->metaObject()->indexOfEnumerator(pEnumeration);
        Q_ASSERT(i >= 0);
        QMetaEnum enumerator = pObject->metaObject()->enumerator(i);
        Q_ASSERT(enumerator.isValid());
        QString result = enumerator.valueToKey(value);
        Q_ASSERT(!result.isNull());
        return result;
    }

    static bool mSkipLongTests;
    static QDir mTemporaryDirectory;
    static Log4Qt::ListAppender *mpLoggingEvents;
    static Log4Qt::Properties mDefaultProperties;
    static Log4Qt::Properties mProperties;
};

bool Log4QtTest::mSkipLongTests = false;
QDir Log4QtTest::mTemporaryDirectory;
Log4Qt::ListAppender *Log4QtTest::mpLoggingEvents;
Log4Qt::Properties Log4QtTest::mDefaultProperties;
Log4Qt::Properties Log4QtTest::mProperties;

TEST_F(Log4QtTest, DateTime_compability)
{
    QDateTime reference = QDateTime(QDate(2001, 9, 7), QTime(15, 7, 5, 9));
    QDateTime q_date_time(reference);
    DateTime date_time(reference);

    EXPECT_EQ(date_time.toString("YYYY-MM-ddTHH:mm:ss"), q_date_time.toString("YYYY-MM-ddTHH:mm:ss"));

    EXPECT_EQ(date_time.toString("dd MMM yyyy HH:mm:ss.zzz"), q_date_time.toString("dd MMM yyyy HH:mm:ss.zzz"));
}

TEST_F(Log4QtTest, DateTime_week)
{
    qInfo() << "-------------------------0";
    EXPECT_EQ(static_cast<DateTime>(QDateTime(QDate(2001, 2, 9), QTime(15, 7, 5, 9))).toString("w"), "6");
    qInfo() << "-------------------------1";
    EXPECT_EQ(static_cast<DateTime>(QDateTime(QDate(2001, 2, 9), QTime(15, 7, 5, 9))).toString("ww"), "06");
    EXPECT_EQ(static_cast<DateTime>(QDateTime(QDate(2001, 9, 7), QTime(15, 7, 5, 9))).toString("w"), "36");
    qInfo() << "-------------------------2";
}

TEST_F(Log4QtTest, DateTime_milliseconds)
{
    qInfo() << "-------------------------3";
    EXPECT_EQ(
        DateTime(QDateTime(QDate(2001, 9, 7), QTime(15, 7, 5, 9), Qt::UTC)).toMilliSeconds(), Q_INT64_C(999875225009));
    qInfo() << "-------------------------4";
    EXPECT_EQ(
        DateTime::fromMilliSeconds(Q_INT64_C(999875225009)).toUTC(),
        QDateTime(QDate(2001, 9, 7), QTime(15, 7, 5, 9), Qt::UTC));
    qInfo() << "-------------------------5";
}

TEST_F(Log4QtTest, PatternFormatter)
{
    QString eol;
#ifdef Q_OS_WIN32
    eol = QLatin1String("\r\n");
#else
    eol = QLatin1String("\n");
#endif // Q_OS_WIN32
    qint64 absolute_timestamp = DateTime(QDateTime(QDate(2001, 9, 7), QTime(15, 7, 5, 9))).toMilliSeconds();
    QHash<QString, QString> properties;
    properties.insert("A", "a");
    properties.insert("B", "b");
    properties.insert("C", "c");
    Log4Qt::PatternFormatter pattern_formatter("%d{ISO8601} [%t] %p %c %x - %m%n:%X");
    EXPECT_EQ(
        pattern_formatter.format(LoggingEvent(
            test_logger(),
            Level(Level::DEBUG_INT),
            "This is the message",
            "NDC",
            properties,
            "main",
            absolute_timestamp)),
        "2001-09-07 15:07:05.009 [main] DEBUG Test::TestLog4Qt NDC - This is the message" + eol);
    pattern_formatter.parseIntegerOption("-1");
    pattern_formatter.parseIntegerOption("abc");
    qDebug() << pattern_formatter;

    EXPECT_EQ(mpLoggingEvents->list().count(), 0);
    resetLogging();
}

TEST_F(Log4QtTest, Properties_default)
{
    mDefaultProperties.clear();
    mDefaultProperties.setProperty("X", "x");

    mProperties.clear();
    mProperties.setProperty("A", "a");
    mProperties.setProperty("B", "");
    mProperties.setProperty("C", QString());

    EXPECT_EQ(mProperties.property("A"), "a");
    EXPECT_EQ(mProperties.property("B"), "");
    EXPECT_EQ(mProperties.property("C"), "");
    EXPECT_EQ(mProperties.property("X"), "x");
    EXPECT_EQ(mProperties.property("D"), QString());
}

TEST_F(Log4QtTest, Properties_names)
{
    mDefaultProperties.clear();
    mDefaultProperties.setProperty("X", "x");

    mProperties.clear();
    mProperties.setProperty("A", "a");
    mProperties.setProperty("B", "");

    QStringList property_names = mProperties.propertyNames();
    EXPECT_EQ(property_names.count(), 3);
    EXPECT_EQ(property_names.contains("A"), true);
    EXPECT_EQ(property_names.count("B"), true);
    EXPECT_EQ(property_names.count("X"), true);
}

TEST_F(Log4QtTest, Properties_load_device)
{
    mpLoggingEvents->clearList();

    QByteArray buffer;
    QBuffer deviceW(&buffer);
    QTextStream streamW(&deviceW);

    buffer.clear();
    deviceW.open(QIODevice::WriteOnly);
    streamW << "Truth = Beauty" << endl;
    deviceW.close();

    QBuffer device(&buffer);
    QTextStream stream(&device);
    device.open(QIODevice::ReadOnly);

    Properties properties;
    properties.load(&device);

    EXPECT_EQ(properties.count(), 1);
    EXPECT_EQ(properties.contains("Truth"), true);
    EXPECT_EQ(properties.value("Truth"), "Beauty");
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);
    resetLogging();
}

TEST_F(Log4QtTest, Properties_load_settings)
{
    QSettings settings(mTemporaryDirectory.path() + "/PropetiesLoadSettings.ini", QSettings::IniFormat);
    QBitArray bit_array(5, true);

    settings.setValue("A", "a");
    settings.setValue("Group/B", "b");
    settings.setValue("Group/C", true);
    settings.setValue("Group/D", bit_array);
    settings.setValue("Group/Subgroup/E", "e");

    settings.beginGroup("Group");
    Properties properties;
    properties.load(settings);

    EXPECT_EQ(properties.count(), 3);
    EXPECT_EQ(properties.contains("B"), true);
    EXPECT_EQ(properties.value("B"), QString("b"));
    EXPECT_EQ(properties.contains("C"), true);
    EXPECT_EQ(properties.value("C"), QString("true"));
    EXPECT_EQ(properties.contains("D"), true);
    EXPECT_EQ(properties.value("D"), QString());
}

/******************************************************************************
 * OptionConverter requires Properties                                        */

TEST_F(Log4QtTest, OptionConverter_boolean)
{
    mpLoggingEvents->clearList();
    EXPECT_EQ(OptionConverter::toBoolean("0", true), false);
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);

    EXPECT_EQ(OptionConverter::toBoolean("tRuE", false), true);
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);
    resetLogging();
}

TEST_F(Log4QtTest, OptionConverter_filesize)
{
    mpLoggingEvents->clearList();
    bool ok;
    EXPECT_EQ(OptionConverter::toFileSize("135", &ok), Q_INT64_C(135));
    EXPECT_EQ(ok, true);
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);

    EXPECT_EQ(OptionConverter::toFileSize("2kb", &ok), Q_INT64_C(2048));
    EXPECT_EQ(ok, true);
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);
    resetLogging();
}

TEST_F(Log4QtTest, OptionConverter_int)
{
    resetLogging();
    mpLoggingEvents->clearList();
    bool ok;
    EXPECT_EQ(OptionConverter::toFileSize("12", &ok), 12);
    EXPECT_EQ(ok, true);
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);

    EXPECT_EQ(OptionConverter::toFileSize("12x", &ok), 0);
    EXPECT_EQ(ok, false);
    EXPECT_EQ(mpLoggingEvents->list().count(), 1);
}

TEST_F(Log4QtTest, OptionConverter_level)
{
    resetLogging();
    mpLoggingEvents->clearList();
    EXPECT_EQ(
        OptionConverter::toLevel("WaRn", Log4Qt::Level(Log4Qt::Level::ERROR_INT)),
        Log4Qt::Level(Log4Qt::Level::WARN_INT));
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);
}

TEST_F(Log4QtTest, OptionConverter_substitution)
{
    mpLoggingEvents->clearList();
    mProperties.setProperty("D", QString());
    EXPECT_EQ(OptionConverter::findAndSubst(mProperties, "D"), "");
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);

    mProperties.setProperty("S1", "begin${A}end");
    EXPECT_EQ(OptionConverter::findAndSubst(mProperties, "S1"), "beginaend");
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);
}

TEST_F(Log4QtTest, OptionConverter_target)
{
    mpLoggingEvents->clearList();
    bool ok;
    EXPECT_EQ(OptionConverter::toTarget("STDOUT_TARGET", &ok), (int)ConsoleAppender::STDOUT_TARGET);
    EXPECT_EQ(ok, true);
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);

    EXPECT_EQ(OptionConverter::toTarget("STDERR_TARGET", &ok), (int)ConsoleAppender::STDERR_TARGET);
    EXPECT_EQ(ok, true);
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);
}

/******************************************************************************
 * Factory requires OptionConverter                                           */

static Appender *createDiyAppender()
{
    return new NullAppender;
}

static Filter *createDiyFilter()
{
    return new LevelRangeFilter;
}

static Layout *createDiyLayout()
{
    return new SimpleLayout;
}

TEST_F(Log4QtTest, Factory_createAppender)
{
    mpLoggingEvents->clearList();
    QObject *p_object = Factory::createAppender("Log4Qt::DebugAppender");
    EXPECT_EQ(p_object != 0, true);
    EXPECT_EQ(QString::fromLatin1(p_object->metaObject()->className()), "Log4Qt::DebugAppender");
    DebugAppender *debugAppender = qobject_cast<DebugAppender *>(p_object);
    if (debugAppender) {
        debugAppender->setLayout(new SimpleLayout());
        debugAppender->requiresLayout();
        debugAppender->append(LoggingEvent(test_logger(), Level::WARN_INT, "Message1"));
        debugAppender->asyncAppend(LoggingEvent(test_logger(), Level::WARN_INT, "Message2"));
        debugAppender->debug(qDebug() << "test");
    }
    delete p_object;
    p_object = Factory::createAppender("Log4Qt::NullAppender");
    EXPECT_EQ(p_object != 0, true);
    EXPECT_EQ(QString::fromLatin1(p_object->metaObject()->className()), "Log4Qt::NullAppender");
    NullAppender *nullAppender = qobject_cast<NullAppender *>(p_object);
    if (nullAppender) {
        nullAppender->setLayout(new SimpleLayout());
        nullAppender->append(LoggingEvent(test_logger(), Level::WARN_INT, "Message1"));
        nullAppender->asyncAppend(LoggingEvent(test_logger(), Level::WARN_INT, "Message2"));
        nullAppender->debug(qDebug() << "test");
    }
    delete p_object;
    p_object = Factory::createAppender("Log4Qt::FileAppender");
    EXPECT_EQ(p_object != 0, true);
    delete p_object;
    p_object = Factory::createAppender("Log4Qt::ListAppender");
    EXPECT_EQ(p_object != 0, true);
    delete p_object;
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);
    Factory::registerAppender("DiyAppender", createDiyAppender);
    Factory::registerFilter("DiyFilter", createDiyFilter);
    Factory::registerLayout("DiyLayout", createDiyLayout);
    Factory::unregisterAppender("DiyAppender");
    Factory::unregisterLayout("DiyLayout");
    Factory::unregisterFilter("DiyFilter");
    qDebug() << "Factory:" << *Factory::instance();
}

TEST_F(Log4QtTest, Factory_createFilter)
{
    mpLoggingEvents->clearList();
    QObject *p_object = Factory::createFilter("org.apache.log4j.varia.LevelRangeFilter");
    EXPECT_EQ(p_object != 0, true);
    EXPECT_EQ(QString::fromLatin1(p_object->metaObject()->className()), "Log4Qt::LevelRangeFilter");
    delete p_object;
    p_object = Factory::createFilter("org.apache.log4j.varia.DenyAllFilter");
    delete p_object;
    p_object = Factory::createFilter("org.apache.log4j.varia.LevelMatchFilter");
    delete p_object;
    p_object = Factory::createFilter("org.apache.log4j.varia.StringMatchFilter");
    delete p_object;
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);
}

TEST_F(Log4QtTest, Factory_createLayout)
{
    mpLoggingEvents->clearList();
    QObject *p_object = Factory::createLayout("Log4Qt::SimpleLayout");
    EXPECT_EQ(p_object != 0, true);
    EXPECT_EQ(QString::fromLatin1(p_object->metaObject()->className()), "Log4Qt::SimpleLayout");
    delete p_object;
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);
}

TEST_F(Log4QtTest, Factory_setObjectProperty)
{
    mpLoggingEvents->clearList();
    QObject *p_object = Factory::createAppender("Log4Qt::RollingFileAppender");

    Factory::setObjectProperty(p_object, "maximumFileSize", "7");
    RollingFileAppender *rollingFileAppender = qobject_cast<RollingFileAppender *>(p_object);
    if (rollingFileAppender) {
        rollingFileAppender->setLayout(new SimpleLayout());
        rollingFileAppender->append(LoggingEvent(test_logger(), Level::WARN_INT, "Message1"));
        // rollingFileAppender->asyncAppend(LoggingEvent(test_logger(), Level::WARN_INT, "Message2"));
        rollingFileAppender->rollOver();
        rollingFileAppender->debug(qDebug() << "test");
    }
    EXPECT_EQ(mpLoggingEvents->list().count(), 1);
    if (mpLoggingEvents->list().count() == 0) {
        EXPECT_EQ(p_object->property("maximumFileSize").toString(), "");
    }

    delete p_object;
}

/******************************************************************************
 * log4qt/varia                                                               */

TEST_F(Log4QtTest, ListAppender)
{
    Log4Qt::ListAppender appender;

    // Store messages
    EXPECT_EQ(appender.list().count(), 0);
    appender.doAppend(LoggingEvent(test_logger(), Level::WARN_INT, "Message1"));
    appender.doAppend(LoggingEvent(test_logger(), Level::WARN_INT, "Message2"));
    appender.doAppend(LoggingEvent(test_logger(), Level::WARN_INT, "Message3"));
    EXPECT_EQ(appender.list().count(), 3);

    // Delete oldest, if max is set
    appender.setMaxCount(2);
    EXPECT_EQ(appender.list().count(), 2);
    EXPECT_EQ(appender.list().at(0).message(), QString("Message2"));
    EXPECT_EQ(appender.list().at(1).message(), QString("Message3"));

    // Ignore new ones added
    appender.doAppend(LoggingEvent(test_logger(), Level::WARN_INT, "Message4"));
    EXPECT_EQ(appender.list().count(), 2);
    EXPECT_EQ(appender.list().at(0).message(), QString("Message2"));
    EXPECT_EQ(appender.list().at(1).message(), QString("Message3"));

    // Clear
    appender.clearList();
    EXPECT_EQ(appender.list().count(), 0);
}

TEST_F(Log4QtTest, DenyAllFilter)
{
    Log4Qt::DenyAllFilter filter;
    LoggingEvent event(test_logger(), Level::WARN_INT, "Message");
    EXPECT_EQ(filter.decide(event), Filter::DENY);
}

TEST_F(Log4QtTest, LevelMatchFilter)
{
    Log4Qt::LevelMatchFilter filter;
    filter.setLevelToMatch(Level::fromString("WARN"));
    filter.setAcceptOnMatch(false);
    LoggingEvent event(test_logger(), Level::fromString("TRACE"), "Message");

    QString decision = enumValueToKey(&filter, "Decision", filter.decide(event));
    EXPECT_EQ(decision, "NEUTRAL");
}

TEST_F(Log4QtTest, LevelRangeFilter)
{
    QString filter_min_level = "DEBUG";
    QString filter_max_level = "ERROR";
    Log4Qt::LevelRangeFilter filter;
    if (!filter_min_level.isEmpty())
        filter.setLevelMin(Level::fromString("DEBUG"));
    if (!filter_max_level.isEmpty())
        filter.setLevelMax(Level::fromString("ERROR"));
    filter.setAcceptOnMatch(false);
    LoggingEvent event(test_logger(), Level::fromString("TRACE"), "Message");

    QString decision = enumValueToKey(&filter, "Decision", filter.decide(event));
    EXPECT_EQ(decision, "DENY");
}

TEST_F(Log4QtTest, StringMatchFilter)
{
    Log4Qt::StringMatchFilter filter;
    filter.setStringToMatch("MESSAGE");
    filter.setAcceptOnMatch(false);
    LoggingEvent event(test_logger(), Level::WARN_INT, "This is a message");

    QString decision = enumValueToKey(&filter, "Decision", filter.decide(event));
    EXPECT_EQ(decision, "NEUTRAL");
}

/******************************************************************************
 * log4qt                                                                     */

TEST_F(Log4QtTest, AppenderSkeleton_threshold)
{
    resetLogging();

    Log4Qt::ListAppender *p_appender = new Log4Qt::ListAppender();
    Log4Qt::Logger *p_logger = test_logger();
    p_logger->addAppender(p_appender);

    // Threshold
    p_appender->setThreshold(Level::ERROR_INT);
    p_appender->doAppend(LoggingEvent(p_logger, Level::WARN_INT, "Warn"));
    p_appender->doAppend(LoggingEvent(p_logger, Level::ERROR_INT, "Error"));
    p_appender->doAppend(LoggingEvent(p_logger, Level::FATAL_INT, "Fatal"));
    EXPECT_EQ(p_appender->list().count(), 2);
    EXPECT_EQ(p_appender->list().at(0).level(), Level(Level::ERROR_INT));
    EXPECT_EQ(p_appender->list().at(1).level(), Level(Level::FATAL_INT));
}

TEST_F(Log4QtTest, AppenderSkeleton_filter)
{
    resetLogging();

    Log4Qt::ListAppender appender;
    QString filter1_level = "WARN";
    QString filter2_level = "WARN";

    if (!filter1_level.isEmpty()) {
        Log4Qt::LevelMatchFilter *p_filter = new Log4Qt::LevelMatchFilter();
        p_filter->setLevelToMatch(Level::fromString("WARN"));
        p_filter->setAcceptOnMatch(false);
        appender.addFilter(p_filter);
    }
    if (!filter2_level.isEmpty()) {
        Log4Qt::LevelMatchFilter *p_filter = new Log4Qt::LevelMatchFilter();
        p_filter->setLevelToMatch(Level::fromString("WARN"));
        p_filter->setAcceptOnMatch(false);
        appender.addFilter(p_filter);
    }

    appender.doAppend(LoggingEvent(test_logger(), Level::fromString("WARN"), "Message"));
    EXPECT_EQ(appender.list().count(), 0);
}

TEST_F(Log4QtTest, BasicConfigurator)
{
    LogManager::resetConfiguration();
    resetLogging();
    Log4Qt::BasicConfigurator::configure();

    Logger *p_logger = LogManager::rootLogger();
    EXPECT_EQ(p_logger->appenders().count(), 1);
    ConsoleAppender *p_appender = qobject_cast<ConsoleAppender *>(p_logger->appenders().at(0));
    EXPECT_EQ(p_appender != 0, true);
    EXPECT_EQ(p_appender->isActive(), true);
    EXPECT_EQ(!p_appender->isClosed(), true);
    EXPECT_EQ(p_appender->target(), QString::fromLatin1("STDOUT_TARGET"));
    PatternLayout *p_layout = qobject_cast<PatternLayout *>(p_appender->layout());
    EXPECT_EQ(p_layout != 0, true);
    EXPECT_EQ(p_layout->conversionPattern(), QString("%r [%t] %p %c %x - %m%n"));
}

TEST_F(Log4QtTest, FileAppender)
{
    resetLogging();

    QString dir(mTemporaryDirectory.path() + "/FileAppender");
    QString file("/log");

    Log4Qt::FileAppender appender1(new SimpleLayout(), dir + file, false);
    appender1.setName("Fileappender1");
    appender1.activateOptions();
    appender1.doAppend(LoggingEvent(test_logger(), Level::DEBUG_INT, QString("Message 0")));
    appender1.doAppend(LoggingEvent(test_logger(), Level::DEBUG_INT, QString("Message 1")));
    appender1.close();
    QStringList expected;
    QString result;
    expected << "log";
    if (!validateDirContents(dir, expected, result))
        FAIL();
    expected.clear();
    expected << "DEBUG - Message 0"
             << "DEBUG - Message 1";
    if (!validateFileContents(dir + file, expected, result))
        FAIL();

    Log4Qt::FileAppender appender2(new SimpleLayout(), dir + file, false);
    appender2.setName("Fileappender2");
    appender2.activateOptions();
    appender2.doAppend(LoggingEvent(test_logger(), Level::DEBUG_INT, QString("Message 2")));
    appender2.doAppend(LoggingEvent(test_logger(), Level::DEBUG_INT, QString("Message 3")));
    appender2.close();
    expected.clear();
    expected << "log";
    if (!validateDirContents(dir, expected, result))
        FAIL();
    expected.clear();
    expected << "DEBUG - Message 2"
             << "DEBUG - Message 3";
    if (!validateFileContents(dir + file, expected, result))
        FAIL();

    Log4Qt::FileAppender appender3(new SimpleLayout(), dir + file, true);
    appender3.setName("Fileappender3");
    appender3.activateOptions();
    appender3.doAppend(LoggingEvent(test_logger(), Level::DEBUG_INT, QString("Message 4")));
    appender3.doAppend(LoggingEvent(test_logger(), Level::DEBUG_INT, QString("Message 5")));
    appender3.close();
    expected.clear();
    expected << "log";
    if (!validateDirContents(dir, expected, result))
        FAIL();
    expected.clear();
    expected << "DEBUG - Message 2"
             << "DEBUG - Message 3"
             << "DEBUG - Message 4"
             << "DEBUG - Message 5";
    if (!validateFileContents(dir + file, expected, result))
        FAIL();

    EXPECT_EQ(mpLoggingEvents->list().count(), 0);

    Log4Qt::FileAppender appender4(new SimpleLayout(), "");
    appender4.activateOptions();

    Log4Qt::FileAppender appender5(new SimpleLayout(), dir + file, false, false);
    appender5.debug(qDebug() << "test");
    QFile fileAbc("/abc");
    appender5.removeFile(fileAbc);
    appender5.renameFile(fileAbc, "/bcd");
}

TEST_F(Log4QtTest, DailyRollingFileAppender)
{
    resetLogging();

    qDebug() << "The test is time based and takes approximately 3 minutes ...";

    QString dir(mTemporaryDirectory.path() + "/DailyRollingFileAppender");
    QString file("/log");

    // Using a RollingFileAppender with 2 files history and 3 messages per file
    Log4Qt::DailyRollingFileAppender appender;
    appender.setName("DailyRollingFileAppender");
    appender.setFile(dir + file);
    appender.setLayout(new SimpleLayout());
    appender.setDatePattern(DailyRollingFileAppender::MINUTELY_ROLLOVER);

    // Start on a full minute
    QDateTime now = QDateTime::currentDateTime();
    // QThread::sleep((60 - now.time().second()) * 1000);
    appender.activateOptions();
    Log4Qt::DailyRollingFileAppender appender1;
    appender1.setName("DailyRollingFileAppender1");
    appender1.setFile(dir + file + "1");
    appender1.setLayout(new SimpleLayout());
    appender1.setDatePattern(DailyRollingFileAppender::HOURLY_ROLLOVER);
    appender1.activateOptions();

    Log4Qt::DailyRollingFileAppender appender2;
    appender2.setName("DailyRollingFileAppender2");
    appender2.setFile(dir + file + "2");
    appender2.setLayout(new SimpleLayout());
    appender2.setDatePattern(DailyRollingFileAppender::HALFDAILY_ROLLOVER);
    appender2.activateOptions();

    Log4Qt::DailyRollingFileAppender appender3;
    appender3.setName("DailyRollingFileAppender3");
    appender3.setFile(dir + file + "3");
    appender3.setLayout(new SimpleLayout());
    appender3.setDatePattern(DailyRollingFileAppender::WEEKLY_ROLLOVER);
    //    appender3.activateOptions();

    Log4Qt::DailyRollingFileAppender appender4;
    appender4.setName("DailyRollingFileAppender4");
    appender4.setFile(dir + file + "4");
    appender4.setLayout(new SimpleLayout());
    appender4.setDatePattern(DailyRollingFileAppender::MONTHLY_ROLLOVER);
    appender4.activateOptions();

    qDebug() << "   1 / 7";
    appender.doAppend(LoggingEvent(test_logger(), Level::DEBUG_INT, QString("Message 0")));
    int i;
    for (i = 1; i < 1; i++) {
        QThread::sleep(1 * 1000);
        qDebug() << "  " << i + 1 << "/" << 7;
        appender.doAppend(LoggingEvent(test_logger(), Level::DEBUG_INT, QString("Message %1").arg(i)));
    }
    appender.asyncAppend(LoggingEvent(test_logger(), Level::DEBUG_INT, QString("Message async")));

    appender.debug(qDebug() << "test");

    appender.rollOver();
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);

    // Validate directory
    QStringList expected;
    QString result;
    expected << "log"
             << "log" + dailyRollingFileAppenderSuffix(now.addSecs(60))
             << "log" + dailyRollingFileAppenderSuffix(now.addSecs(120));
    if (!validateDirContents(dir, expected, result))
        FAIL();

    // Validate files
    expected.clear();
    expected << "DEBUG - Message 6";
    if (!validateFileContents(dir + file, expected, result))
        FAIL();
    expected.clear();
    expected << "DEBUG - Message 0"
             << "DEBUG - Message 1"
             << "DEBUG - Message 2";
    if (!validateFileContents(dir + file + dailyRollingFileAppenderSuffix(now.addSecs(60)), expected, result))
        FAIL();
    expected.clear();
    expected << "DEBUG - Message 3"
             << "DEBUG - Message 4"
             << "DEBUG - Message 5";
    if (!validateFileContents(dir + file + dailyRollingFileAppenderSuffix(now.addSecs(120)), expected, result))
        FAIL();
}

TEST_F(Log4QtTest, LoggingEvent_stream)
{
    resetLogging();
    qint64 timestamp = DateTime(QDateTime(QDate(2001, 9, 7), QTime(15, 7, 5, 9))).toMilliSeconds();
    QHash<QString, QString> properties;
    properties.insert("A", "a");
    properties.insert("B", "b");
    properties.insert("C", "c");

    LoggingEvent original(0, Level(Level::WARN_INT), "This is a message", "NDC", properties, "main", timestamp);
    QByteArray array;
    QBuffer buffer(&array);
    buffer.open(QIODevice::WriteOnly);
    QDataStream stream(&buffer);
    stream << original;
    buffer.close();

    buffer.open(QIODevice::ReadOnly);
    LoggingEvent streamed;
    stream >> streamed;
    buffer.close();

    EXPECT_EQ(original.level(), streamed.level());
    EXPECT_EQ(original.loggerName(), streamed.loggerName());
    EXPECT_EQ(original.message(), streamed.message());
    EXPECT_EQ(original.ndc(), streamed.ndc());
    EXPECT_EQ(original.properties().count(), streamed.properties().count());
    QStringList keys = original.properties().keys();
    QString key;
    Q_FOREACH (key, keys) {
        EXPECT_EQ(streamed.properties().contains(key), true);
        EXPECT_EQ(original.properties().value(key), streamed.properties().value(key));
    }
    EXPECT_EQ(original.sequenceNumber(), streamed.sequenceNumber());
    EXPECT_EQ(original.threadName(), streamed.threadName());
    EXPECT_EQ(original.timeStamp(), streamed.timeStamp());

    EXPECT_EQ(mpLoggingEvents->list().count(), 0);
}

TEST_F(Log4QtTest, LogManager_configureLogLogger)
{
    resetLogging();
    LogManager::logLogger()->removeAppender(mpLoggingEvents);

    LogManager::resetConfiguration();
    Log4Qt::Logger *p_logger = LogManager::logLogger();
    EXPECT_EQ(p_logger->appenders().count(), 2);
    ConsoleAppender *p_appender;
    TTCCLayout *p_layout;

    p_appender = qobject_cast<ConsoleAppender *>(p_logger->appenders().at(0));
    EXPECT_EQ(p_appender != 0, true);
    EXPECT_EQ(p_appender->isActive(), true);
    EXPECT_EQ(!p_appender->isClosed(), true);
    EXPECT_EQ(p_appender->target(), QString::fromLatin1("STDOUT_TARGET"));
    p_layout = qobject_cast<TTCCLayout *>(p_appender->layout());
    EXPECT_EQ(p_layout != 0, true);

    p_appender = qobject_cast<ConsoleAppender *>(p_logger->appenders().at(1));
    EXPECT_EQ(p_appender != 0, true);
    EXPECT_EQ(p_appender->isActive(), true);
    EXPECT_EQ(!p_appender->isClosed(), true);
    EXPECT_EQ(p_appender->target(), QString::fromLatin1("STDERR_TARGET"));
    p_layout = qobject_cast<TTCCLayout *>(p_appender->layout());
    EXPECT_EQ(p_layout != 0, true);

    TTCCLayout testLayout("yyyy-MM-dd hh:mm:ss.zzz");
    testLayout.setDateFormat(TTCCLayout::NONE);
    testLayout.setDateFormat(TTCCLayout::TIME_ABSOLUTE);
    testLayout.setDateFormat(TTCCLayout::DATE);
    testLayout.setDateFormat(TTCCLayout::TIME_RELATIVE);

    TTCCLayout testLayout1(TTCCLayout::ISO8601);
    testLayout1.debug(qDebug() << "test");
}

TEST_F(Log4QtTest, PropertyConfigurator_missing_appender)
{
    LogManager::resetConfiguration();
    resetLogging();
    mDefaultProperties.clear();
    mProperties.clear();

    mProperties.setProperty("log4j.logger.MissingAppender", "INHERITED, A");
    EXPECT_EQ(!PropertyConfigurator::configure(mProperties), true);
    EXPECT_EQ(ConfiguratorHelper::configureError().count(), 1);
    EXPECT_EQ(mpLoggingEvents->list().count(), 1);
}

TEST_F(Log4QtTest, PropertyConfigurator_unknown_appender_class)
{
    LogManager::resetConfiguration();
    resetLogging();
    mDefaultProperties.clear();
    mProperties.clear();

    mProperties.setProperty("log4j.logger.UnknownAppender", "INHERITED, A");
    mProperties.setProperty("log4j.appender.A", "org.apache.log4j.UnknownAppender");
    EXPECT_EQ(!PropertyConfigurator::configure(mProperties), true);
    EXPECT_EQ(ConfiguratorHelper::configureError().count(), 1);
    EXPECT_EQ(mpLoggingEvents->list().count(), 2); // Warning from Factory, Error PropertyConfigurator
}

TEST_F(Log4QtTest, PropertyConfigurator_missing_layout)
{
    resetLogging();
    mDefaultProperties.clear();
    mProperties.clear();

    mProperties.setProperty("log4j.logger.MissingLayout", "INHERITED, A");
    mProperties.setProperty("log4j.appender.A", "org.apache.log4j.ConsoleAppender");
    EXPECT_EQ(!PropertyConfigurator::configure(mProperties), true);
    EXPECT_EQ(ConfiguratorHelper::configureError().count(), 1);
    EXPECT_EQ(mpLoggingEvents->list().count(), 1);
}

TEST_F(Log4QtTest, PropertyConfigurator_unknown_layout_class)
{
    resetLogging();
    mDefaultProperties.clear();
    mProperties.clear();

    mProperties.setProperty("log4j.logger.UnknownLayout", "INHERITED, A");
    mProperties.setProperty("log4j.appender.A", "org.apache.log4j.ConsoleAppender");
    mProperties.setProperty("log4j.appender.A.layout", "org.apache.log4j.UnknownLayout");
    EXPECT_EQ(!PropertyConfigurator::configure(mProperties), true);
    EXPECT_EQ(ConfiguratorHelper::configureError().count(), 1);
    EXPECT_EQ(mpLoggingEvents->list().count(), 2); // Warning from Factory, Error PropertyConfigurator
}

TEST_F(Log4QtTest, PropertyConfigurator_reset)
{
    resetLogging();
    mDefaultProperties.clear();
    mProperties.clear();

    // - Create a logger with an appender
    // - If the reset flag is not set, configure must leave the appender
    // - If the reset flag is set to an invalid value, configure must raise an
    //   error and leave the appender
    // - If the reset flag is set, configure must remove the appender

    const QLatin1String key_reset("log4j.reset");
    test_logger()->addAppender(new Log4Qt::ListAppender);
    mProperties.setProperty(key_reset, "false");
    EXPECT_EQ(PropertyConfigurator::configure(mProperties), true);
    EXPECT_EQ(ConfiguratorHelper::configureError().count(), 0);
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);
    EXPECT_EQ(test_logger()->appenders().count(), 1);

    mProperties.setProperty(key_reset, "No boolean");
    EXPECT_EQ(!PropertyConfigurator::configure(mProperties), true);
    EXPECT_EQ(ConfiguratorHelper::configureError().count(), 1);
    EXPECT_EQ(mpLoggingEvents->list().count(), 1);
    EXPECT_EQ(test_logger()->appenders().count(), 1);

    resetLogging();
    mProperties.setProperty(key_reset, "true");
    EXPECT_EQ(PropertyConfigurator::configure(mProperties), true);
    EXPECT_EQ(ConfiguratorHelper::configureError().count(), 0);
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);
    EXPECT_EQ(test_logger()->appenders().count(), 0);
}

TEST_F(Log4QtTest, PropertyConfigurator_debug)
{
    resetLogging();
    mDefaultProperties.clear();
    mProperties.clear();

    // - Set the log logger level to INFO
    // - If debug is not set, configure must leave the log logger level
    //   unaltered
    // - If debug is set, but with no valid level string, configure must set
    //   the log logger level to DEBUG
    // - If debug is set to the level TRACE, configure must set the log logger
    //   level to TRACE

    const QLatin1String key_debug("log4j.Debug");
    Logger *p_logger = LogManager::logLogger();
    p_logger->setLevel(Level::INFO_INT);

    EXPECT_EQ(PropertyConfigurator::configure(mProperties), true);
    EXPECT_EQ(ConfiguratorHelper::configureError().count(), 0);
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);
    EXPECT_EQ(p_logger->level(), Level(Level::INFO_INT));

    mProperties.setProperty(key_debug, "true");
    EXPECT_EQ(PropertyConfigurator::configure(mProperties), true);
    EXPECT_EQ(ConfiguratorHelper::configureError().count(), 0);
    // EXPECT_EQ(mpLoggingEvents->list().count(), 1); // Warning from Level::fromString() & several debug messages
    EXPECT_EQ(p_logger->level(), Level(Level::DEBUG_INT));

    mpLoggingEvents->clearList();
    mProperties.setProperty(key_debug, "TRACE");
    EXPECT_EQ(PropertyConfigurator::configure(mProperties), true);
    EXPECT_EQ(ConfiguratorHelper::configureError().count(), 0);
    // EXPECT_EQ(mpLoggingEvents->list().count(), 1); // Warning from PropertyConfigurator & several debug/trace
    // messages
    EXPECT_EQ(p_logger->level(), Level(Level::TRACE_INT));
}

TEST_F(Log4QtTest, PropertyConfigurator_threshold)
{
    resetLogging();
    mDefaultProperties.clear();
    mProperties.clear();

    // - Set the repository threshold to INFO
    // - If the threshold is not set, configure must leave the repository
    //   threshold unaltered
    // - If the threshold is set to an invalid value, configure must raise an
    //   error and set the threshold to ALL
    // - If the threshold is set to WARN, configure must set the repository
    //   threshold to WARN

    const QLatin1String key_threshold("log4j.threshold");
    LogManager::loggerRepository()->setThreshold(Level::INFO_INT);

    EXPECT_EQ(PropertyConfigurator::configure(mProperties), true);
    EXPECT_EQ(ConfiguratorHelper::configureError().count(), 0);
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);
    EXPECT_EQ(LogManager::loggerRepository()->threshold(), Level(Level::INFO_INT));

    mProperties.setProperty(key_threshold, "Not a value");
    EXPECT_EQ(!PropertyConfigurator::configure(mProperties), true);
    EXPECT_EQ(ConfiguratorHelper::configureError().count(), 1);
    EXPECT_EQ(mpLoggingEvents->list().count(), 2); // Warning by Level, Error from OptionConverter
    EXPECT_EQ(LogManager::loggerRepository()->threshold(), Level(Level::ALL_INT));

    mpLoggingEvents->clearList();
    mProperties.setProperty(key_threshold, "WARN");
    EXPECT_EQ(PropertyConfigurator::configure(mProperties), true);
    EXPECT_EQ(ConfiguratorHelper::configureError().count(), 0);
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);
    EXPECT_EQ(LogManager::loggerRepository()->threshold(), Level(Level::WARN_INT));
}

TEST_F(Log4QtTest, PropertyConfigurator_handleQtMessages)
{
    resetLogging();
    mDefaultProperties.clear();
    mProperties.clear();

    // - Set handle Qt messages to true
    // - If handle Qt messages is not set, configure must leave handle Qt
    //   messages unaltered
    // - If handle Qt messages is set to an invalid value, configure must raise
    //   an error and set handle Qt messages to false
    // - If handle Qt messages is true, configure must set handle Qt messages
    //   to true

    const QLatin1String key_handle_qt_messages("log4j.handleQtMessages");
    LogManager::setHandleQtMessages(true);

    EXPECT_EQ(PropertyConfigurator::configure(mProperties), true);
    EXPECT_EQ(ConfiguratorHelper::configureError().count(), 0);
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);
    EXPECT_EQ(LogManager::handleQtMessages(), true);

    mProperties.setProperty(key_handle_qt_messages, "No boolean");
    EXPECT_EQ(!PropertyConfigurator::configure(mProperties), true);
    EXPECT_EQ(ConfiguratorHelper::configureError().count(), 1);
    EXPECT_EQ(mpLoggingEvents->list().count(), 1);
    EXPECT_EQ(LogManager::handleQtMessages(), false);

    mpLoggingEvents->clearList();
    mProperties.setProperty(key_handle_qt_messages, "true");
    EXPECT_EQ(PropertyConfigurator::configure(mProperties), true);
    EXPECT_EQ(ConfiguratorHelper::configureError().count(), 0);
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);
    EXPECT_EQ(LogManager::handleQtMessages(), true);
}

TEST_F(Log4QtTest, PropertyConfigurator_example)
{
    LogManager::resetConfiguration();
    resetLogging();
    mDefaultProperties.clear();
    mProperties.clear();

    QString file(mTemporaryDirectory.path() + "/RollingFileAppender/log");

    // Based on the JavaDoc example:
    // - A1: JavaDoc uses SyslogAppender, which is not available on all platforms
    // - A2: JavaDoc does not set a file, which causes error on activation
    // - A2: JavaDoc uses default values for file size and backup index. Use
    //       different values.
    // - A2 Layout: ContextPrinting uses default enabled. Use disabled instead.
    // - root: JavaDoc uses default level DEBUG. Use INFO instead.
    // - SECURITY: JavaDoc uses INHERIT. Use INHERITED instead
    // - log4j.logger.class.of.the.day: JavaDoc uses INHERIT. Use INHERITED
    //   instead

    // Appender A1: ConsoleAppender with PatternLayout
    mProperties.setProperty("log4j.appender.A1", "org.apache.log4j.ConsoleAppender");
    mProperties.setProperty("log4j.appender.A1.Target", "System.Out");
    mProperties.setProperty("log4j.appender.A1.layout", "org.apache.log4j.PatternLayout");
    mProperties.setProperty("log4j.appender.A1.layout.ConversionPattern", "%-4r %-5p %c{2} %M.%L %x - %m\n");
    // Appender A2: RollingFileAppender with TTCCLayout
    mProperties.setProperty("log4j.appender.A2", "org.apache.log4j.RollingFileAppender");
    mProperties.setProperty("log4j.appender.A2.File", file);
    mProperties.setProperty("log4j.appender.A2.MaxFileSize", "13MB");
    mProperties.setProperty("log4j.appender.A2.MaxBackupIndex", "7");
    mProperties.setProperty("log4j.appender.A2.layout", "org.apache.log4j.TTCCLayout");
    mProperties.setProperty("log4j.appender.A2.layout.ContextPrinting", "disabled");
    mProperties.setProperty("log4j.appender.A2.layout.DateFormat", "ISO8601");
    // Root Logger: Uses A2
    mProperties.setProperty("log4j.rootLogger", "INFO, A2");
    // Logger SECURITY: Uses A1
    mProperties.setProperty("log4j.logger.SECURITY", "INHERITED, A1");
    mProperties.setProperty("log4j.additivity.SECURITY", "false");
    // Logger SECURITY.access:
    mProperties.setProperty("log4j.logger.SECURITY.access", "WARN");
    // Logger class.of.the.day:
    mProperties.setProperty("log4j.logger.class.of.the.day", "INHERITED");

    // No warnings, no errors expected
    EXPECT_EQ(PropertyConfigurator::configure(mProperties), true);
    EXPECT_EQ(ConfiguratorHelper::configureError().count(), 0);
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);

    // Root logger
    Logger *p_logger;
    p_logger = LogManager::rootLogger();
    EXPECT_EQ(p_logger->level(), Level(Level::INFO_INT));
    EXPECT_EQ(p_logger->appenders().count(), 1);
    Log4Qt::RollingFileAppender *p_a2 = qobject_cast<Log4Qt::RollingFileAppender *>(p_logger->appenders().at(0));
    EXPECT_EQ(p_a2 != 0, true);
    EXPECT_EQ(p_a2->file(), file);
    EXPECT_EQ(p_a2->maximumFileSize(), Q_INT64_C(13 * 1024 * 1024));
    EXPECT_EQ(p_a2->maxBackupIndex(), 7);
    Log4Qt::TTCCLayout *p_a2layout = qobject_cast<Log4Qt::TTCCLayout *>(p_a2->layout());
    EXPECT_EQ(p_a2layout != 0, true);
    EXPECT_EQ(p_a2layout->contextPrinting(), false);
    EXPECT_EQ(p_a2layout->dateFormat(), QString::fromLatin1("ISO8601"));

    // Logger SECURITY
    EXPECT_EQ(LogManager::exists("SECURITY"), true);
    p_logger = LogManager::logger("SECURITY");
    EXPECT_EQ(p_logger->level(), Level(Level::NULL_INT));
    EXPECT_EQ(p_logger->appenders().count(), 1);
    Log4Qt::ConsoleAppender *p_a1 = qobject_cast<Log4Qt::ConsoleAppender *>(p_logger->appenders().at(0));
    EXPECT_EQ(p_a1 != 0, true);
    EXPECT_EQ(p_a1->target(), QString::fromLatin1("STDOUT_TARGET"));
    Log4Qt::PatternLayout *p_a1layout = qobject_cast<Log4Qt::PatternLayout *>(p_a1->layout());
    EXPECT_EQ(p_a1layout != 0, true);
    EXPECT_EQ(p_a1layout->conversionPattern(), QString::fromLatin1("%-4r %-5p %c{2} %M.%L %x - %m\n"));

    // Logger SECURITY::access
    EXPECT_EQ(LogManager::exists("SECURITY::access"), true);
    p_logger = LogManager::logger("SECURITY::access");
    EXPECT_EQ(p_logger->level(), Level(Level::WARN_INT));

    // Logger class::of::the::day
    EXPECT_EQ(LogManager::exists("class::of::the::day"), true);
}

TEST_F(Log4QtTest, RollingFileAppender)
{
    resetLogging();

    QString dir(mTemporaryDirectory.path() + "/RollingFileAppender");
    QString file("/log");

    // Using a RollingFileAppender with 2 files history and 3 messages per file
    Log4Qt::RollingFileAppender appender;
    appender.setName("RollingFileAppender");
    appender.setFile(dir + file);
    appender.setLayout(new SimpleLayout());
    appender.setMaxBackupIndex(2);
    appender.setMaximumFileSize(40);
    appender.activateOptions();

    // Output 9 messages
    int i;
    for (i = 0; i < 10; i++)
        appender.doAppend(LoggingEvent(test_logger(), Level::DEBUG_INT, QString("Message %1").arg(i)));

    // No warnings or errors expected
    EXPECT_EQ(mpLoggingEvents->list().count(), 0);

    // Validate diretcory
    QStringList expected;
    QString result;
    expected << "log"
             << "log.1"
             << "log.2";
    if (!validateDirContents(dir, expected, result))
        FAIL();

    // Validate files
    expected.clear();
    expected << "DEBUG - Message 9";
    if (!validateFileContents(dir + file, expected, result))
        FAIL();
    expected.clear();
    expected << "DEBUG - Message 6"
             << "DEBUG - Message 7"
             << "DEBUG - Message 8";
    if (!validateFileContents(dir + file + ".1", expected, result))
        FAIL();
    expected.clear();
    expected << "DEBUG - Message 3"
             << "DEBUG - Message 4"
             << "DEBUG - Message 5";
    if (!validateFileContents(dir + file + ".2", expected, result))
        FAIL();

    EXPECT_EQ(mpLoggingEvents->list().count(), 0);
}

TEST_F(Log4QtTest, LogError)
{
    Log4Qt::LogError logErrorNull;
    Log4Qt::LogError logError("abc", 0, "Test", "123456", LogError::LATIN1);

    logError.translatedMessage();
    qDebug() << logError.lastError() << logError;
    logError.setLastError(logErrorNull);
    QDataStream stream;
    stream << logError;
    stream >> logError;
}
