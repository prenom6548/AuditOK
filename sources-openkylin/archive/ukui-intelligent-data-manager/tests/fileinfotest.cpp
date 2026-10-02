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

#include "fileinfo.h"
#include "fileinfojob.h"

#include <QDir>
#include <QDebug>

#include <QObject>
#include <QtTest>

#include <QEventLoop>

#include <QThreadPool>

class FileInfoTest : public QObject
{
    Q_OBJECT
public:
    explicit FileInfoTest(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();
    void testSharedPointer();
    void testQueryFiles();
    void testThreadSafety1();
    void testCancel();
    void cleanupTestCase();

private:
    QStringList paths;
    QList<UKUI::IDM::FileInfoJob*> jobs;
    QThreadPool *threadPool;
};

QTEST_MAIN(FileInfoTest)
#include "fileinfotest.moc"

FileInfoTest::FileInfoTest(QObject *parent) : QObject(parent)
{

}

void FileInfoTest::initTestCase()
{
    threadPool = new QThreadPool;
    QDir dir("/bin");
    auto infoList = dir.entryInfoList();
    for (auto info : infoList) {
        paths << info.filePath();
    }
}

void FileInfoTest::testSharedPointer()
{
    QSharedPointer<UKUI::IDM::FileInfo> info1 = UKUI::IDM::FileInfo::fromPath("/home");
    QSharedPointer<UKUI::IDM::FileInfo> info2 = UKUI::IDM::FileInfo::fromUri("file:///home");
    QCOMPARE(info1.data(), info2.data());
}

void FileInfoTest::testQueryFiles()
{
    UKUI::IDM::FileInfoJob *j = UKUI::IDM::FileInfoJob::fromPaths(paths, false);

    QEventLoop *l = new QEventLoop;
    connect(j, &UKUI::IDM::FileInfoJob::finished, j, [j, l]{
        auto infos = j->getInfos();
        for (auto info : infos) {
            QCOMPARE(info->isValid(), true);
            //qDebug()<<info->uri()<<info->iconName()<<info->isValid();
        }
        delete j;
        l->quit();
    });

    j->start();

    l->exec();
    delete l;
}

void FileInfoTest::testThreadSafety1()
{
    for (int i = 0; i < threadPool->maxThreadCount(); i++) {
        UKUI::IDM::FileInfoJob *j = UKUI::IDM::FileInfoJob::fromPaths(paths, false);
        connect(j, &UKUI::IDM::FileInfoJob::finished, j, [j]{
            auto infos = j->getInfos();
            for (auto info : infos) {
                QCOMPARE(info->isValid(), true);
                //qDebug()<<info->uri()<<info->iconName()<<info->isValid();
            }
            delete j;
        });
        threadPool->start(j);
    }

    while (threadPool->activeThreadCount()) {
        for (auto path : paths) {
            auto info = UKUI::IDM::FileInfo::fromPath(path);
            info->dateString();
            info->isValid();
            info->iconName();
            info->uri();
            info->path();
        }
    }

    threadPool->waitForDone();
}

void FileInfoTest::testCancel()
{
    for (int i = 0; i < threadPool->maxThreadCount(); i++) {
        UKUI::IDM::FileInfoJob *j = UKUI::IDM::FileInfoJob::fromPaths(paths, false);
        connect(j, &UKUI::IDM::FileInfoJob::finished, this, [=]{
            qDebug()<<"job finished"<<j;
            auto infos = j->getInfos();
            for (auto info : infos) {
                QCOMPARE(info->isValid(), true);
                //qDebug()<<info->uri()<<info->iconName()<<info->isValid();
            }
            jobs.removeOne(j);
            delete j;
        });
        connect(j, &UKUI::IDM::FileInfoJob::started, this, [j]{
            qDebug()<<"job started"<<j;
        });
        connect(j, &UKUI::IDM::FileInfoJob::cancelled, this, [=]{
            jobs.removeOne(j);
            qDebug()<<"job cancelled"<<j;
            delete j;
        });
        jobs<<j;
        threadPool->start(j);
    }

    while(true) {
        QTest::qWait(1000);
        if (jobs.isEmpty())
            break;

        jobs.first()->cancel();
    }

    threadPool->waitForDone();
}

void FileInfoTest::cleanupTestCase()
{
    threadPool->waitForDone();
    delete threadPool;
}
