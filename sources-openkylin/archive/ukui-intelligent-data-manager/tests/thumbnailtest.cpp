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
#include <QImage>
#include <QStandardPaths>

#include "../thumbnail/thumbnailmanager.h"

class ThumbnailTest : public QObject
{
    Q_OBJECT
public:
    ThumbnailTest() {}

private Q_SLOTS:
    void initTestCase();
    void test1Thumbnail();
    void test2ThumbnailAgain();
    void cleanupTestCase();

private:
    ThumbnailManager *manager;
};

QTEST_MAIN(ThumbnailTest)
#include "thumbnailtest.moc"

void ThumbnailTest::initTestCase()
{
    QDir dir("/tmp");
    Q_ASSERT(dir.mkdir("ukui-idm-thumbnailtest"));
    dir.cd("ukui-idm-thumbnailtest");
    QImage image(QSize(500, 500), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::red);
    for (auto i = 0; i < 100; i++) {
        image.save(dir.absoluteFilePath(QString("%1.png").arg(i)));
    }

    manager = new ThumbnailManager;
}

void ThumbnailTest::test1Thumbnail()
{
    QEventLoop *loop = new QEventLoop;
    QDir dir("/tmp/ukui-idm-thumbnailtest");
    QStringList entries = dir.entryList(QDir::Files);
    QTimeLine timeLine;
    timeLine.setLoopCount(1);
    timeLine.setDuration(100 * 100);
    timeLine.setFrameRange(0, 100);
    connect(manager, &ThumbnailManager::thumbnailFinished, loop, [=, &entries](const QString &path, const QImage &image, const QString &errorMessage){
        if (entries.isEmpty()) {
            loop->quit();
            delete loop;
        }
        qDebug() << path << image.size();
        QVERIFY(image.size().isValid());
    });
    connect(&timeLine, &QTimeLine::frameChanged, this, [=, &entries]{
        if (entries.isEmpty())
            return;
        auto name = entries.takeFirst();
        QString path = dir.absoluteFilePath(name);
        manager->startPreview(path);
    });
    timeLine.start();
    loop->exec();
}

void ThumbnailTest::test2ThumbnailAgain()
{
    manager->setStrictedMode(true);
    test1Thumbnail();
}

void ThumbnailTest::cleanupTestCase()
{
    QDir dir("/tmp/ukui-idm-thumbnailtest");
    dir.removeRecursively();

    delete manager;
}
