/*
 * UKUI Intelligent Data Manager
 *
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include <QtTest>

#include <QObject>
#include <QDir>
#include <QFile>

#include "fileinfomanager.h"
#include "fileinfo.h"

class FileInfoManagerTests : public QObject
{
    Q_OBJECT
public:
    FileInfoManagerTests() {}

private Q_SLOTS:
    void initTestCase();
    void test1CreateFileInfos();
    void test2AddFileToWatcher();
    void test3RemoveFileFromWatcher();
    void test4AddFilesToWatcher();
    void test5WatcherFileChangeTest();
    void test6RemoveFilesFromWatcher();
    void cleanupTestCase();

private:
    QStringList paths;
    QList<QSharedPointer<UKUI::IDM::FileInfo>> infos;
};

QTEST_MAIN(FileInfoManagerTests)
#include "fileinfomanagertests.moc"

void FileInfoManagerTests::initTestCase()
{
    QDir d;
    bool ok = d.mkpath("/tmp/.ukui-idm-infomanager-tests");
    Q_ASSERT(ok);
    d.cd("/tmp/.ukui-idm-infomanager-tests");

    for (int i = 1; i <= 10; i++) {
        QString filePath = d.absoluteFilePath(QString::number(i));
        QFile file(filePath);
        ok = file.open(QFile::WriteOnly|QFile::Text);
        Q_ASSERT(ok);
        file.write("test");
        file.close();
        paths << filePath;
    }

    for (int i = 11; i <= 20; i++) {
        QDir subDir;
        QString dirPath = d.absoluteFilePath(QString::number(i));
        bool ok = subDir.mkpath(dirPath);
        Q_ASSERT(ok);
        paths << dirPath;
    }

    UKUI::IDM::FileInfoManager::globalInstance();
}

void FileInfoManagerTests::test1CreateFileInfos()
{
    for (auto path : paths) {
        auto info = UKUI::IDM::FileInfo::fromPath(path, false);
        infos << info;
    }
}

void FileInfoManagerTests::test2AddFileToWatcher()
{
    for (auto info : infos) {
        bool ok = false;
        UKUI::IDM::FileInfoManager::globalInstance()->addFileToWatcher(info->uri(), &ok);
        QCOMPARE(ok, true);
    }
}

void FileInfoManagerTests::test3RemoveFileFromWatcher()
{
    for (auto info : infos) {
        bool ok = false;
        UKUI::IDM::FileInfoManager::globalInstance()->removeFileFromWatcher(info->uri(), &ok);
        QCOMPARE(ok, true);
    }
}

void FileInfoManagerTests::test4AddFilesToWatcher()
{
    QStringList urls;
    for (auto info : infos) {
        urls << info->uri();
    }

    QStringList failedUrls = UKUI::IDM::FileInfoManager::globalInstance()->addFilesToWatcher(urls);
    bool hasFailedUrls = !failedUrls.isEmpty();
    QCOMPARE(hasFailedUrls, false);
}

void FileInfoManagerTests::test5WatcherFileChangeTest()
{
    auto infomanager = UKUI::IDM::FileInfoManager::globalInstance();
    bool hasPaths = !paths.isEmpty();
    if (hasPaths) {
        auto path = paths.takeFirst();

//        qDebug()<<"test file open and read";
        QFile file(path);
        QSignalSpy spy(infomanager, &UKUI::IDM::FileInfoManager::fileChanged);
        file.open(QFile::ReadWrite|QFile::Text);
//        if (!spy.wait())
//            QFAIL("failed to monitor file open");

        qDebug() << file.readAll();

        qDebug()<<"test file modify";
        file.write("test write");
        file.flush();
        if (!spy.wait())
            QFAIL("failed to monitor file modify");

//        qDebug()<<"test file close";
        file.close();
//        if (!spy.wait())
//            QFAIL("failed to monitor file close");

        qDebug()<<"test file delete";
        file.remove();
        if (!spy.wait())
            QFAIL("failed to monitor file delete");
    } else {
        QSKIP("no file can be tested");
    }
}

void FileInfoManagerTests::test6RemoveFilesFromWatcher()
{
    QStringList urls;
    for (auto path : paths) {
        auto info = UKUI::IDM::FileInfo::fromPath(path, false);
        urls << info->uri();
    }

    QStringList failedUrls = UKUI::IDM::FileInfoManager::globalInstance()->removeFilesFromWatcher(urls);
    bool hasFailedUrls = !failedUrls.isEmpty();
    QCOMPARE(hasFailedUrls, false);
}

void FileInfoManagerTests::cleanupTestCase()
{
    QDir d("/tmp/.ukui-idm-infomanager-tests");
    bool ok = d.removeRecursively();
    QCOMPARE(ok, true);
}
