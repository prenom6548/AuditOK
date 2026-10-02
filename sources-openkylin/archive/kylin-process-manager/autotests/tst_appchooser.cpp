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

#include "appchooser.h"
#include <QDebug>
#include <QtTest>

class TestAppChooser : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void test_getDefaultApp();
    void test_getDefaultApp_edgeCases();
    void test_getAvailableAppList();

private:
    AppChooser m_appChooser;
};

void TestAppChooser::test_getDefaultApp()
{
    std::string defaultApp = m_appChooser.getDefaultAppForUrl("https://www.baidu.com");
    QVERIFY(!defaultApp.empty());

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // Test text file
    QTemporaryFile tempFile1(tempDir.path() + "/test.txt");

    defaultApp = m_appChooser.getDefaultAppForUrl("file:///tmp/test.txt");
    QVERIFY(!defaultApp.empty());

    defaultApp = m_appChooser.getDefaultAppForUrl("file:///tmp/picture.png");
    QVERIFY(!defaultApp.empty());
}

void TestAppChooser::test_getDefaultApp_edgeCases()
{
    // Test invalid URL
    std::string defaultApp = m_appChooser.getDefaultAppForUrl("invalid://url");
    QVERIFY(defaultApp.empty());

    // Test empty URL
    defaultApp = m_appChooser.getDefaultAppForUrl("");
    QVERIFY(defaultApp.empty());

    // Test unsupported MIME type
    defaultApp = m_appChooser.getDefaultAppForUrl("file:///tmp/test.unknown");
    QVERIFY(defaultApp.empty());
}

void TestAppChooser::test_getAvailableAppList()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // Test text file
    QTemporaryFile tempFile1(tempDir.path() + "/test.txt");
    tempFile1.open();
    tempFile1.write("test content");
    tempFile1.close();

    std::vector<std::string> appList =
        m_appChooser.getAvailableAppListForFile(tempFile1.fileName().toStdString());
    QVERIFY(!appList.empty());

    // Test image file
    QTemporaryFile tempFile2(tempDir.path() + "/picture.png");
    tempFile2.open();
    tempFile2.write("test content");
    tempFile2.close();

    appList = m_appChooser.getAvailableAppListForFile(tempFile2.fileName().toStdString());
    QVERIFY(!appList.empty());

    // Test invalid file
    appList = m_appChooser.getAvailableAppListForFile("/invalid/path");
    QVERIFY(appList.empty());

    // Test empty file
    QTemporaryFile tempFile3(tempDir.path() + "/empty.txt");
    tempFile3.open();
    tempFile3.close();

    appList = m_appChooser.getAvailableAppListForFile(tempFile3.fileName().toStdString());
    QVERIFY(!appList.empty());
}

QTEST_MAIN(TestAppChooser)

#include "tst_appchooser.moc"
