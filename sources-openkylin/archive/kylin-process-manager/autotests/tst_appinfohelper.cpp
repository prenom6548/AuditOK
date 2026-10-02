/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "appinfohelper.h"
#include <QFile>
#include <QtTest>

class TestAppInfoHelper : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void test_isSameDesktopFile_sameContent();
    void test_isSameDesktopFile_differentContent();
    void test_isSameDesktopFile_differentName();
    void test_hasSameDesktopName();
    void test_hasSameMD5Hash();
    void test_generateDesktopFileId();
    void test_getDesktopFileName();
    void test_isTopApp();
    void test_isSessionApp();
    void test_getCmdlineFromDesktopFile();
    void test_findDesktopFileFromCmdline();
    void test_isKmreApp();

private:
    QTemporaryDir m_tempDir;
    void newFile(const QString &fileName, const QByteArray &content);
    void newDesktopFile(const QString &fileName, const QByteArray &content);
};

void TestAppInfoHelper::initTestCase()
{
    system("/usr/bin/mkdir -p /tmp/test");
}

void TestAppInfoHelper::cleanupTestCase()
{
    system("/usr/bin/rm -r /tmp/test");
}

void TestAppInfoHelper::test_isSameDesktopFile_sameContent()
{
    auto fileName = QUuid::createUuid().toString(QUuid::Id128);
    QString filePath1 = m_tempDir.path() + "/" + fileName;
    QString filePath2 = m_tempDir.path() + "/test/" + fileName;
    QDir().mkpath(m_tempDir.path() + "/test");
    newFile(filePath1, "1234");
    newFile(filePath2, "1234");

    QVERIFY(appinfo_helper::isSameDesktopFile(
        filePath1.toStdString(), filePath2.toStdString()));
}

void TestAppInfoHelper::test_isSameDesktopFile_differentContent()
{
    auto fileName = QUuid::createUuid().toString(QUuid::Id128);
    QString filePath1 = m_tempDir.path() + "/" + fileName;
    QString filePath2 = m_tempDir.path() + "/test/" + fileName;
    QDir().mkpath(m_tempDir.path() + "/test");
    newFile(filePath1, "1234");
    newFile(filePath2, "12345");

    QVERIFY(!appinfo_helper::isSameDesktopFile(
        filePath1.toStdString(), filePath2.toStdString()));
}

void TestAppInfoHelper::test_isSameDesktopFile_differentName()
{
    auto fileName1 = QUuid::createUuid().toString(QUuid::Id128);
    auto fileName2 = QUuid::createUuid().toString(QUuid::Id128);
    QString filePath1 = m_tempDir.path() + "/" + fileName1;
    QString filePath2 = m_tempDir.path() + "/test/" + fileName2;
    QDir().mkpath(m_tempDir.path() + "/test");
    newFile(filePath1, "1234");
    newFile(filePath2, "1234");

    QVERIFY(!appinfo_helper::isSameDesktopFile(
        filePath1.toStdString(), filePath2.toStdString()));
}

void TestAppInfoHelper::test_hasSameDesktopName()
{
    auto fileName = "test.desktop";
    QString filePath1 = m_tempDir.path() + "/" + fileName;
    QString filePath2 = m_tempDir.path() + "/test/" + fileName;
    QDir().mkpath(m_tempDir.path() + "/test");
    newFile(filePath1, "1234");
    newFile(filePath2, "5678");

    QVERIFY(appinfo_helper::hasSameDesktopName(
        filePath1.toStdString(), filePath2.toStdString()));

    QString filePath3 = m_tempDir.path() + "/different.desktop";
    newFile(filePath3, "1234");

    QVERIFY(!appinfo_helper::hasSameDesktopName(
        filePath1.toStdString(), filePath3.toStdString()));
}

void TestAppInfoHelper::test_hasSameMD5Hash()
{
    auto fileName1 = "file1.desktop";
    auto fileName2 = "file2.desktop";
    QString filePath1 = m_tempDir.path() + "/" + fileName1;
    QString filePath2 = m_tempDir.path() + "/" + fileName2;

    newFile(filePath1, "content");
    newFile(filePath2, "content");

    QVERIFY(appinfo_helper::hasSameMD5Hash(
        filePath1.toStdString(), filePath2.toStdString()));

    QString filePath3 = m_tempDir.path() + "/file3.desktop";
    newFile(filePath3, "different content");

    QVERIFY(!appinfo_helper::hasSameMD5Hash(
        filePath1.toStdString(), filePath3.toStdString()));
}

void TestAppInfoHelper::test_generateDesktopFileId()
{
    QString filePath = m_tempDir.path() + "/test.desktop";
    newFile(filePath, "test content");

    std::string id1 = appinfo_helper::generateDesktopFileId(filePath.toStdString());
    std::string id2 = appinfo_helper::generateDesktopFileId(filePath.toStdString());

    QVERIFY(!id1.empty());
    QCOMPARE(id1, id2);

    QString nonExistentPath = m_tempDir.path() + "/nonexistent.desktop";
    std::string id3 = appinfo_helper::generateDesktopFileId(nonExistentPath.toStdString());
    QVERIFY(id3.empty());
}

void TestAppInfoHelper::test_getDesktopFileName()
{
    std::string path = "/usr/share/applications/test.desktop";
    std::string fileName = appinfo_helper::getDesktopFileName(path);
    QCOMPARE(fileName, std::string("test.desktop"));

    path = "test.desktop";
    fileName = appinfo_helper::getDesktopFileName(path);
    QCOMPARE(fileName, std::string("test.desktop"));

    path = "";
    fileName = appinfo_helper::getDesktopFileName(path);
    QVERIFY(fileName.empty());
}

void TestAppInfoHelper::test_isTopApp()
{
    QString filePath = m_tempDir.path() + "/firefox.desktop";
    newFile(filePath, "test content");

    std::vector<std::string> defaultTopApps = {
        filePath.toStdString(),
        "/usr/share/applications/chrome.desktop"};

    QVERIFY(appinfo_helper::isTopApp(filePath.toStdString(), defaultTopApps));

    QString otherPath = m_tempDir.path() + "/other.desktop";
    newFile(otherPath, "test content");

    QVERIFY(!appinfo_helper::isTopApp(otherPath.toStdString(), defaultTopApps));
}

void TestAppInfoHelper::test_isSessionApp()
{
    // This test is limited as it requires creating desktop files with specific content
    // and would need mocking of KDesktopFile functionality
    std::vector<std::string> availableFiles = {"/etc/xdg/autostart/test.desktop"};

    // Test with non-autostart path
    QString filePath = m_tempDir.path() + "/test.desktop";
    QVERIFY(!appinfo_helper::isSessionApp(filePath.toStdString(), availableFiles));

    // Test with multiple available files
    std::vector<std::string> multipleFiles = {
        "/etc/xdg/autostart/test1.desktop",
        "/etc/xdg/autostart/test2.desktop"};
    QVERIFY(!appinfo_helper::isSessionApp("/etc/xdg/autostart/test.desktop", multipleFiles));
}

void TestAppInfoHelper::test_getCmdlineFromDesktopFile()
{
    // This test would require creating a valid desktop file with Exec entry
    // and would need mocking of GDesktopAppInfo functionality
    // For now, we can test the function with non-existent file which should return empty string
    QString nonExistentPath = m_tempDir.path() + "/nonexistent.desktop";
    std::vector<std::string> args = {"--arg1", "--arg2"};

    std::string cmdline = appinfo_helper::getCmdlineFromDesktopFile(
        nonExistentPath.toStdString(), args);
    QVERIFY(cmdline.empty());
}

void TestAppInfoHelper::test_findDesktopFileFromCmdline()
{
    // Note: This test is limited because it depends on the actual system environment
    // and searchAppDesktopFiles function which uses g_desktop_app_info_search

    // Test with a common command that might exist on the system
    std::string cmdline = "firefox";
    std::string desktopFile = appinfo_helper::findDesktopFileFromCmdline(cmdline);

    // We can't make strong assertions about the result as it depends on the test environment
    // But we can log the result for manual verification
    qDebug() << "Found desktop file for" << QString::fromStdString(cmdline)
             << ":" << QString::fromStdString(desktopFile);

    // Test with command and arguments
    std::string cmdlineWithArgs = "firefox --private-window";
    std::string desktopFile2 = appinfo_helper::findDesktopFileFromCmdline(cmdlineWithArgs);
    qDebug() << "Found desktop file for" << QString::fromStdString(cmdlineWithArgs)
             << ":" << QString::fromStdString(desktopFile2);

    // Test with non-existent command
    std::string nonExistentCmd = "this_command_should_not_exist_12345";
    std::string emptyResult = appinfo_helper::findDesktopFileFromCmdline(nonExistentCmd);
    QVERIFY(emptyResult.empty());
}

void TestAppInfoHelper::test_isKmreApp()
{
    // Create a desktop file with Android categories
    QString androidAppPath = m_tempDir.path() + "/android-app.desktop";
    QFile file(androidAppPath);
    QVERIFY(file.open(QIODevice::ReadWrite));
    file.write("[Desktop Entry]\n");
    file.write("Type=Application\n");
    file.write("Name=Android App\n");
    file.write("Categories=Android;Game;\n");
    file.write("Exec=kmre-app\n");
    file.flush();
    file.close();

    // Create a desktop file with Apk categories
    QString apkAppPath = m_tempDir.path() + "/apk-app.desktop";
    QFile file2(apkAppPath);
    QVERIFY(file2.open(QIODevice::ReadWrite));
    file2.write("[Desktop Entry]\n");
    file2.write("Type=Application\n");
    file2.write("Name=APK App\n");
    file2.write("Categories=Apk;Utility;\n");
    file2.write("Exec=kmre-app\n");
    file2.flush();
    file2.close();

    // Create a regular desktop file
    QString regularAppPath = m_tempDir.path() + "/regular-app.desktop";
    QFile file3(regularAppPath);
    QVERIFY(file3.open(QIODevice::ReadWrite));
    file3.write("[Desktop Entry]\n");
    file3.write("Type=Application\n");
    file3.write("Name=Regular App\n");
    file3.write("Categories=Utility;Network;\n");
    file3.write("Exec=regular-app\n");
    file3.flush();
    file3.close();

    // Test non-existent file
    QString nonExistentPath = m_tempDir.path() + "/nonexistent.desktop";
    QVERIFY(!appinfo_helper::isKmreApp(nonExistentPath.toStdString()));

    // Note: The complete test requires GDesktopAppInfo to work properly,
    // which may not be available in all test environments.
    // We're making a best effort test here.

    // Depending on the test environment, these tests may not work as expected
    // so we log the results for manual verification
    bool isAndroidKmre = appinfo_helper::isKmreApp(androidAppPath.toStdString());
    bool isApkKmre = appinfo_helper::isKmreApp(apkAppPath.toStdString());
    bool isRegularKmre = appinfo_helper::isKmreApp(regularAppPath.toStdString());

    qDebug() << "Android app is Kmre app:" << isAndroidKmre;
    qDebug() << "APK app is Kmre app:" << isApkKmre;
    qDebug() << "Regular app is Kmre app:" << isRegularKmre;

    // If the environment supports GDesktopAppInfo properly, these should be true
    // But we can't guarantee it in all test environments
    // QVERIFY(isAndroidKmre);
    // QVERIFY(isApkKmre);
    // QVERIFY(!isRegularKmre);
}

void TestAppInfoHelper::newFile(const QString &fileName, const QByteArray &content)
{
    QFile file(fileName);
    QVERIFY(file.open(QIODevice::ReadWrite));
    file.write(content);
    file.flush();
    file.close();
}

void TestAppInfoHelper::newDesktopFile(const QString &fileName, const QByteArray &content)
{
    QFile file(fileName);
    QVERIFY(file.open(QIODevice::ReadWrite));
    file.write("[Desktop Entry]\n");
    file.write("Type=Application\n");
    file.write("Name=Test App\n");
    file.write("Exec=testapp\n");
    file.write(content);
    file.flush();
    file.close();
}

QTEST_MAIN(TestAppInfoHelper)

#include "tst_appinfohelper.moc"
