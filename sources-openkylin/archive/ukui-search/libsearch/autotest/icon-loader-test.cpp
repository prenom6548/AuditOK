/*
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "icon-loader-test.h"
#include <QImage>
#include <QTemporaryDir>
#include <QTest>
#include <icon-loader.h>
#include <QDebug>

void IconLoaderTest::initTestCase()
{
    Q_INIT_RESOURCE(resource1);
}

void IconLoaderTest::testLoadIconQt()
{
    const QString resourcePath = QStringLiteral(":/res/icons/unknown.svg");

    QIcon icon = UkuiSearch::IconLoader::loadIconQt(QStringLiteral("noIcon"), QIcon(resourcePath));
    QVERIFY(!icon.pixmap(QSize(16, 16)).isNull());

    icon = UkuiSearch::IconLoader::loadIconQt(resourcePath);
    QVERIFY(!icon.pixmap(QSize(16, 16)).isNull());
}

void IconLoaderTest::testLoadIconQtFallbackBranches()
{
    const QIcon fallback(QStringLiteral("kylin-search"));

    QVERIFY(UkuiSearch::IconLoader::loadIconQt(QString(), fallback).cacheKey() == fallback.cacheKey());
    QVERIFY(UkuiSearch::IconLoader::loadIconQt(QStringLiteral("/definitely/not/here.png"), fallback).cacheKey() == fallback.cacheKey());
    QVERIFY(UkuiSearch::IconLoader::loadIconQt(QStringLiteral("/definitely/not/here.png")).isNull());
}

void IconLoaderTest::testLoadIconQtExistingFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = dir.filePath(QStringLiteral("icon.png"));
    QImage image(8, 8, QImage::Format_ARGB32);
    image.fill(Qt::red);
    QVERIFY(image.save(path));

    const QIcon icon = UkuiSearch::IconLoader::loadIconQt(path);
    QVERIFY(!icon.isNull());
}

void IconLoaderTest::testLoadIconXdgFallbackBranches()
{
    const QIcon fallback(QStringLiteral("kylin-search"));

    QVERIFY(UkuiSearch::IconLoader::loadIconXdg(QString(), fallback).cacheKey() == fallback.cacheKey());
    QVERIFY(UkuiSearch::IconLoader::loadIconXdg(QStringLiteral("/definitely/not/here.png"), fallback).cacheKey() == fallback.cacheKey());
}

void IconLoaderTest::testLoadIconXdgExistingFileFallback()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = dir.filePath(QStringLiteral("xdg-icon.png"));
    QImage image(8, 8, QImage::Format_ARGB32);
    image.fill(Qt::blue);
    QVERIFY(image.save(path));

    const QIcon fallback(QStringLiteral("kylin-search"));
    const QIcon icon = UkuiSearch::IconLoader::loadIconXdg(path, fallback);

    QVERIFY(!icon.isNull());
}

void IconLoaderTest::testLoadIconQtUnreadableFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("unreadable.png"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("not an image");
    file.close();
    file.setPermissions(QFileDevice::WriteOwner);  // not readable

    QIcon icon = UkuiSearch::IconLoader::loadIconQt(path);
    QVERIFY(icon.isNull());  // unreadable → returns null
}

void IconLoaderTest::testLoadIconXdgUnreadableFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("xdg-unreadable.png"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("not an image");
    file.close();
    file.setPermissions(QFileDevice::WriteOwner);  // not readable

    QIcon icon = UkuiSearch::IconLoader::loadIconXdg(path);
    QVERIFY(icon.isNull());
}

QTEST_MAIN(IconLoaderTest)
