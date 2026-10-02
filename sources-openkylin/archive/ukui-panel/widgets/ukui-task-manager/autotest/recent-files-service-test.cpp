/*
 *
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
 */

#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

#include "../recent-files-service.h"

using namespace TaskManager;

static QString createTempFile(const QString &path, const QByteArray &content = QByteArray("data"))
{
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write(content);
    file.close();
    return QUrl::fromLocalFile(path).toString();
}

class RecentFilesServiceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void desktopPathApplicationMatchesAppId();
    void displayNameOnlyApplicationDoesNotCreateBogusAppId();
    void entriesAreSortedNewestFirstAndTrimmedToTen();
    void pruneMissingLocalFilesForAppRemovesDeletedEntries();
};

void RecentFilesServiceTest::desktopPathApplicationMatchesAppId()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString fileUrl = createTempFile(dir.filePath("demo.txt"));

    QFile file(dir.filePath("recently-used.xbel"));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(QString(R"(<?xml version="1.0" encoding="UTF-8"?>
<xbel version="1.0" xmlns:bookmark="http://www.freedesktop.org/standards/desktop-bookmarks">
  <bookmark href="%1" modified="2026-05-27T01:55:42Z">
    <title>demo</title>
    <info>
      <metadata owner="http://freedesktop.org">
        <bookmark:applications>
          <bookmark:application name="/usr/share/applications/pluma.desktop" exec="&apos;pluma %U&apos;" modified="2026-05-27T01:55:42Z" count="1"/>
        </bookmark:applications>
      </metadata>
    </info>
  </bookmark>
</xbel>)").arg(fileUrl).toUtf8());
    file.close();

    RecentFilesService service(file.fileName());
    const auto entries = service.recentFilesForApp(QStringLiteral("pluma"));
    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.first().appId, QStringLiteral("pluma"));
    QCOMPARE(entries.first().desktopFile, QStringLiteral("/usr/share/applications/pluma.desktop"));
}

void RecentFilesServiceTest::displayNameOnlyApplicationDoesNotCreateBogusAppId()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString fileUrl = createTempFile(dir.filePath("demo.txt"));

    QFile file(dir.filePath("recently-used.xbel"));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(QString(R"(<?xml version="1.0" encoding="UTF-8"?>
<xbel version="1.0" xmlns:bookmark="http://www.freedesktop.org/standards/desktop-bookmarks">
  <bookmark href="%1" modified="2026-05-27T01:55:42Z">
    <info>
      <metadata owner="http://freedesktop.org">
        <bookmark:applications>
          <bookmark:application name="Pluma" exec="&apos;pluma %U&apos;" modified="2026-05-27T01:55:42Z" count="1"/>
        </bookmark:applications>
      </metadata>
    </info>
  </bookmark>
</xbel>)").arg(fileUrl).toUtf8());
    file.close();

    RecentFilesService service(file.fileName());
    QVERIFY(service.recentFilesForApp(QStringLiteral("pluma")).isEmpty());
    QVERIFY(service.recentFilesForApp(QStringLiteral("Pluma")).isEmpty());
}

void RecentFilesServiceTest::entriesAreSortedNewestFirstAndTrimmedToTen()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QFile file(dir.filePath("recently-used.xbel"));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));

    QByteArray xbel(R"(<?xml version="1.0" encoding="UTF-8"?>
<xbel version="1.0" xmlns:bookmark="http://www.freedesktop.org/standards/desktop-bookmarks">
)");
    for (int i = 0; i < 11; ++i) {
        const QString fileUrl = createTempFile(dir.filePath(QStringLiteral("demo-%1.txt").arg(i)));
        const QString title = QStringLiteral("demo-%1").arg(i);
        const QString stamp = QStringLiteral("2026-05-27T01:%1:42Z").arg(59 - i, 2, 10, QLatin1Char('0'));
        xbel += QString(R"(
  <bookmark href="%1" modified="%3">
    <title>%2</title>
    <info>
      <metadata owner="http://freedesktop.org">
        <bookmark:applications>
          <bookmark:application name="/usr/share/applications/pluma.desktop" exec="&apos;pluma %U&apos;" modified="%3" count="1"/>
        </bookmark:applications>
      </metadata>
    </info>
  </bookmark>)").arg(fileUrl, title, stamp).toUtf8();
    }
    xbel += "</xbel>";
    file.write(xbel);
    file.close();

    RecentFilesService service(file.fileName());
    const auto entries = service.recentFilesForApp(QStringLiteral("pluma"));
    QCOMPARE(entries.size(), 10);
    QCOMPARE(entries.first().bookmark.title(), QStringLiteral("demo-0"));
    QCOMPARE(entries.last().bookmark.title(), QStringLiteral("demo-9"));
}

void RecentFilesServiceTest::pruneMissingLocalFilesForAppRemovesDeletedEntries()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString filePath = dir.filePath("demo.txt");
    const QString fileUrl = createTempFile(filePath);

    QFile file(dir.filePath("recently-used.xbel"));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(QString(R"(<?xml version="1.0" encoding="UTF-8"?>
<xbel version="1.0" xmlns:bookmark="http://www.freedesktop.org/standards/desktop-bookmarks">
  <bookmark href="%1" modified="2026-05-27T01:55:42Z">
    <title>demo</title>
    <info>
      <metadata owner="http://freedesktop.org">
        <bookmark:applications>
          <bookmark:application name="/usr/share/applications/pluma.desktop" exec="&apos;pluma %U&apos;" modified="2026-05-27T01:55:42Z" count="1"/>
        </bookmark:applications>
      </metadata>
    </info>
  </bookmark>
</xbel>)").arg(fileUrl).toUtf8());
    file.close();

    RecentFilesService service(file.fileName());
    QCOMPARE(service.recentFilesForApp(QStringLiteral("pluma")).size(), 1);

    QVERIFY(QFile::remove(filePath));
    QVERIFY(service.pruneMissingLocalFilesForApp(QStringLiteral("pluma")));
    QVERIFY(service.recentFilesForApp(QStringLiteral("pluma")).isEmpty());
}

QTEST_MAIN(RecentFilesServiceTest)

#include "recent-files-service-test.moc"
