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
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#include "bookmarktest.h"
#include "bookmark.h"
#include "bookmarks-manager.h"
#include "indexerextractortestsconfig.h"

#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

using namespace UkuiFileMetadata;

QString testFilePath(const QString& name)
{
    return QLatin1String(INDEXER_TESTS_SAMPLE_FILES_PATH) + QLatin1String("/bookmarks/") + name;
}

void BookMarkTest::testParse()
{
    const BookMarksManager manager(testFilePath(QStringLiteral("test_bookmark.xbel")));

    QVERIFY(manager.isValid());
    QCOMPARE(manager.rootTitle(), QStringLiteral("Root Bookmarks"));
    QCOMPARE(manager.dbusName(), QStringLiteral("org.example.bookmarks"));

    const BookMarksGroup &root = manager.rootGroup();
    QCOMPARE(root.bookmarks().size(), 1);
    QCOMPARE(root.groups().size(), 1);

    const BookMark &topBookmark = root.bookmarks().constFirst();
    QCOMPARE(topBookmark.href(), QStringLiteral("file:///home/user/Documents"));
    QCOMPARE(topBookmark.title(), QStringLiteral("Docs"));
    QCOMPARE(topBookmark.description(), QStringLiteral("Documents folder"));
    QCOMPARE(topBookmark.showInToolbar(), QStringLiteral("yes"));
    QCOMPARE(topBookmark.iconName(), QStringLiteral("folder-documents"));
    QCOMPARE(topBookmark.iconHref(), QStringLiteral("file:///usr/share/icons/breeze/places/64/folder-documents.svg"));
    QCOMPARE(topBookmark.iconType(), QStringLiteral("image/svg+xml"));
    QCOMPARE(topBookmark.mimeType(), QStringLiteral("inode/directory"));

    const BookMarksGroup &folder = root.groups().constFirst();
    QCOMPARE(folder.title(), QStringLiteral("Favorites"));
    QCOMPARE(folder.folded(), QStringLiteral("no"));
    QCOMPARE(folder.toolbar(), QStringLiteral("yes"));
    QCOMPARE(folder.bookmarks().size(), 1);

    const BookMark &nestedBookmark = folder.bookmarks().constFirst();
    QCOMPARE(nestedBookmark.href(), QStringLiteral("https://example.com"));
    QCOMPARE(nestedBookmark.title(), QStringLiteral("Example"));
    QCOMPARE(nestedBookmark.description(), QStringLiteral("Example site"));

    const QHash<QString, QStringList> kdeMeta = topBookmark.kdeMetaData();
    QCOMPARE(kdeMeta.value(QStringLiteral("time_added")).size(), 1);
    QCOMPARE(kdeMeta.value(QStringLiteral("visit_count")).size(), 1);

    const QStringList lines = manager.textLines();
    auto containsAny = [&lines](const QStringList &candidates) {
        for (const QString &candidate : candidates) {
            if (lines.contains(candidate)) {
                return true;
            }
        }
        return false;
    };
    QVERIFY(lines.contains(QStringLiteral("metadata[https://example.org/bookmark-ext][file:///home/user/Documents].rating: favorite")));
    QVERIFY(lines.contains(QStringLiteral("metadata[https://example.org/bookmark-ext][file:///home/user/Documents].rating@score: 5")));
    QVERIFY(lines.contains(QStringLiteral("metadata[https://example.org/bookmark-ext][file:///home/user/Documents].flags.public: true")));
    QVERIFY(lines.contains(QStringLiteral("metadata[http://freedesktop.org][file:///home/user/Documents].groups.group: pluma")));
    QVERIFY(lines.contains(QStringLiteral("metadata[http://freedesktop.org][file:///home/user/Documents].applications.application@name: Firefox")));
    QVERIFY(lines.contains(QStringLiteral("metadata[http://freedesktop.org][file:///home/user/Documents].applications.application@exec: 'firefox-esr %u'")));
    QVERIFY(lines.contains(QStringLiteral("metadata[http://freedesktop.org][file:///home/user/Documents].applications.application@count: 1")));
    QVERIFY(lines.contains(QStringLiteral("icon-href: file:///usr/share/icons/breeze/places/64/folder-documents.svg")));
    QVERIFY(lines.contains(QStringLiteral("icon-type: image/svg+xml")));
    QVERIFY(containsAny({QStringLiteral("attribute[xbel].app:profile: work"),
                         QStringLiteral("attribute[xbel].profile: work")}));
    QVERIFY(containsAny({QStringLiteral("attribute[bookmark].app:priority: high"),
                         QStringLiteral("attribute[bookmark].priority: high")}));
    QVERIFY(containsAny({QStringLiteral("attribute[folder].app:color: blue"),
                         QStringLiteral("attribute[folder].color: blue")}));
    QVERIFY(containsAny({QStringLiteral("attribute[alias].app:aliasType: quicklink"),
                         QStringLiteral("attribute[alias].aliasType: quicklink")}));
    QVERIFY(containsAny({QStringLiteral("attribute[separator].app:marker: main"),
                         QStringLiteral("attribute[separator].marker: main")}));

    const QString href = QStringLiteral("file:///home/user/Documents");
    const QStringList freedesktopGroups = manager.freedesktopGroups(href);
    QVERIFY(freedesktopGroups.contains(QStringLiteral("pluma")));

    const QList<FreedesktopApplicationInfo> freedesktopApps = manager.freedesktopApplications(href);
    QVERIFY(!freedesktopApps.isEmpty());
    bool hasFirefox = false;
    bool hasPluma = false;
    for (const FreedesktopApplicationInfo &app : freedesktopApps) {
        if (app.name() == QStringLiteral("Firefox")
            && app.exec() == QStringLiteral("'firefox-esr %u'")
            && app.count() == 1) {
            hasFirefox = true;
        }
        if (app.name() == QStringLiteral("Pluma")
            && app.exec() == QStringLiteral("'pluma %u'")
            && app.count() == 1) {
            hasPluma = true;
        }
    }
    QVERIFY(hasFirefox);
    QVERIFY(hasPluma);
}

void BookMarkTest::testSettersWrite()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString source = testFilePath(QStringLiteral("test_bookmark.xbel"));
    const QString target = tempDir.path() + QLatin1String("/test_bookmark.xbel");
    QVERIFY(QFile::copy(source, target));

    BookMarksManager manager(target);
    QVERIFY(manager.isValid());

    manager.setFilePath(QString());
    QVERIFY(!manager.isValid());
    QVERIFY(!manager.lastError().isEmpty());

    manager.setFilePath(target);
    QVERIFY2(manager.isValid(), qPrintable(manager.lastError()));
    QCOMPARE(manager.lastError(), QString());
}

void BookMarkTest::testSave()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString source = testFilePath(QStringLiteral("test_bookmark.xbel"));
    const QString target = tempDir.path() + QLatin1String("/save_test_bookmark.xbel");
    QVERIFY(QFile::copy(source, target));

    BookMarksManager manager(target);
    QVERIFY(manager.isValid());

    const QString saveAsPath = tempDir.path() + QLatin1String("/saved_as.xbel");
    QVERIFY(manager.saveAs(saveAsPath));
    QVERIFY(QFile::exists(saveAsPath));

    QVERIFY(manager.save());
}

void BookMarkTest::testSignals()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString source = testFilePath(QStringLiteral("test_bookmark.xbel"));
    const QString target = tempDir.path() + QLatin1String("/signal_test_bookmark.xbel");
    QVERIFY(QFile::copy(source, target));

    BookMarksManager manager;
    QSignalSpy changedSpy(&manager, &BookMarksManager::changed);
    QSignalSpy errorSpy(&manager, &BookMarksManager::errorOccurred);
    int reentrantReadCount = 0;
    QObject::connect(&manager, &BookMarksManager::changed, &manager, [&manager, &reentrantReadCount]() {
        (void)manager.isValid();
        (void)manager.rootTitle();
        ++reentrantReadCount;
    });

    manager.setFilePath(target);
    QTRY_VERIFY(changedSpy.count() >= 1);
    QCOMPARE(errorSpy.count(), 0);
    QVERIFY(reentrantReadCount >= 1);

    const int changedCount = changedSpy.count();
    const int errorCount = errorSpy.count();
    manager.setFilePath(QString());
    QTRY_COMPARE(changedSpy.count(), changedCount + 1);
    QTRY_COMPARE(errorSpy.count(), errorCount + 1);
    QVERIFY(!manager.isValid());
    QVERIFY(manager.rootGroup().bookmarks().isEmpty());
}

void BookMarkTest::testReparseSamePathAfterFailure()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString target = tempDir.path() + QLatin1String("/broken_then_fixed.xbel");
    QFile file(target);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
    QVERIFY(file.write("<xbel><bookmark></xbel>") > 0);
    file.close();

    BookMarksManager manager(target);
    QVERIFY(!manager.isValid());
    QVERIFY(!manager.lastError().isEmpty());

    const QString source = testFilePath(QStringLiteral("test_bookmark.xbel"));
    QVERIFY(QFile::remove(target));
    QVERIFY(QFile::copy(source, target));

    manager.setFilePath(target);
    QVERIFY(manager.isValid());
    QCOMPARE(manager.lastError(), QString());
}

void BookMarkTest::testAutoReloadOnFileChange()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString source = testFilePath(QStringLiteral("test_bookmark.xbel"));
    const QString target = tempDir.path() + QLatin1String("/watch_test_bookmark.xbel");
    QVERIFY(QFile::copy(source, target));

    BookMarksManager manager(target);
    QVERIFY(manager.isValid());
    QCOMPARE(manager.rootTitle(), QStringLiteral("Root Bookmarks"));

    QFile file(target);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = QString::fromUtf8(file.readAll());
    file.close();
    content.replace(QStringLiteral("Root Bookmarks"), QStringLiteral("Updated Bookmarks"));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
    QVERIFY(file.write(content.toUtf8()) > 0);
    file.close();

    QTRY_COMPARE_WITH_TIMEOUT(manager.rootTitle(), QStringLiteral("Updated Bookmarks"), 3000);
}

void BookMarkTest::testAutoReloadOnInvalidFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString source = testFilePath(QStringLiteral("test_bookmark.xbel"));
    const QString target = tempDir.path() + QLatin1String("/watch_invalid_bookmark.xbel");
    QVERIFY(QFile::copy(source, target));

    BookMarksManager manager(target);
    QVERIFY(manager.isValid());

    QSignalSpy changedSpy(&manager, &BookMarksManager::changed);
    QSignalSpy errorSpy(&manager, &BookMarksManager::errorOccurred);

    QFile file(target);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
    QVERIFY(file.write("<xbel><bookmark></xbel>") > 0);
    file.close();

    QTRY_VERIFY_WITH_TIMEOUT(!manager.isValid(), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(changedSpy.count() >= 1, 3000);
    QTRY_VERIFY_WITH_TIMEOUT(errorSpy.count() >= 1, 3000);
    QVERIFY(manager.rootGroup().bookmarks().isEmpty());
}

void BookMarkTest::testAutoReloadOnFileDeleteAndRecreate()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString source = testFilePath(QStringLiteral("test_bookmark.xbel"));
    const QString target = tempDir.path() + QLatin1String("/watch_delete_recreate_bookmark.xbel");
    QVERIFY(QFile::copy(source, target));

    BookMarksManager manager(target);
    QVERIFY(manager.isValid());

    QSignalSpy changedSpy(&manager, &BookMarksManager::changed);
    QSignalSpy errorSpy(&manager, &BookMarksManager::errorOccurred);

    QVERIFY(QFile::remove(target));
    QTRY_VERIFY_WITH_TIMEOUT(!manager.isValid(), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(changedSpy.count() >= 1, 3000);
    QTRY_VERIFY_WITH_TIMEOUT(errorSpy.count() >= 1, 3000);

    const int changedCountAfterDelete = changedSpy.count();

    QVERIFY(QFile::copy(source, target));
    QTRY_VERIFY_WITH_TIMEOUT(manager.isValid(), 3000);
    QTRY_VERIFY_WITH_TIMEOUT(changedSpy.count() > changedCountAfterDelete, 3000);
    QCOMPARE(manager.rootTitle(), QStringLiteral("Root Bookmarks"));
}

void BookMarkTest::testFreedesktopMetadataCompatibility()
{
    const BookMarksManager manager(testFilePath(QStringLiteral("test_bookmark_freedesktop_metadata.xbel")));
    QVERIFY(manager.isValid());

    const QList<BookMark> bookmarks = manager.rootGroup().bookmarks();
    QCOMPARE(bookmarks.size(), 1);
    const BookMark bookmark = bookmarks.constFirst();
    QCOMPARE(bookmark.href(), QStringLiteral("file:///tmp/freedesktop-meta"));
    QCOMPARE(bookmark.iconName(), QStringLiteral("folder-remote"));
    QCOMPARE(bookmark.iconHref(), QStringLiteral("file:///usr/share/icons/hicolor/64x64/places/folder-remote.png"));
    QCOMPARE(bookmark.iconType(), QStringLiteral("image/png"));
    QCOMPARE(bookmark.mimeType(), QStringLiteral("inode/directory"));
    QCOMPARE(bookmark.privateFlag(), true);

    const QStringList lines = manager.textLines();
    QVERIFY(lines.contains(QStringLiteral("icon: folder-remote")));
    QVERIFY(lines.contains(QStringLiteral("icon-href: file:///usr/share/icons/hicolor/64x64/places/folder-remote.png")));
    QVERIFY(lines.contains(QStringLiteral("icon-type: image/png")));
    QVERIFY(lines.contains(QStringLiteral("mime-type: inode/directory")));
    QVERIFY(lines.contains(QStringLiteral("private: true")));

    const QString href = QStringLiteral("file:///tmp/freedesktop-meta");
    const QStringList groups = manager.freedesktopGroups(href);
    QCOMPARE(groups, QStringList{QStringLiteral("files")});

    const QList<FreedesktopApplicationInfo> apps = manager.freedesktopApplications(href);
    QCOMPARE(apps.size(), 2);
    QCOMPARE(apps.at(0).name(), QStringLiteral("Files"));
    QCOMPARE(apps.at(0).exec(), QStringLiteral("Files %u"));
    QCOMPARE(apps.at(0).count(), 3);
    QCOMPARE(apps.at(1).name(), QStringLiteral("Viewer"));
    QCOMPARE(apps.at(1).exec(), QStringLiteral("'viewer %u'"));
    QCOMPARE(apps.at(1).count(), 1);
}

void BookMarkTest::testEdgeCases()
{
    {
        const BookMarksManager manager(testFilePath(QStringLiteral("test_bookmark_ns_root.xbel")));
        QVERIFY(manager.isValid());
        QCOMPARE(manager.rootTitle(), QStringLiteral("NS Root"));
        QCOMPARE(manager.rootGroup().bookmarks().size(), 1);
    }

    {
        const BookMarksManager manager(testFilePath(QStringLiteral("test_bookmark_invalid_count.xbel")));
        QVERIFY(manager.isValid());
        const QList<FreedesktopApplicationInfo> apps = manager.freedesktopApplications(QStringLiteral("file:///tmp/count"));
        QCOMPARE(apps.size(), 2);
        QCOMPARE(apps.at(0).count(), -1);
        QCOMPARE(apps.at(1).count(), 1);
    }

    {
        const BookMarksManager manager(testFilePath(QStringLiteral("test_bookmark_group_duplicate_meta.xbel")));
        QVERIFY(manager.isValid());
        const QStringList custom = manager.customMetaLines();
        QCOMPARE(custom.count(QStringLiteral("metadata[unknown-owner].k: v")), 2);
    }

    {
        const BookMarksManager manager(testFilePath(QStringLiteral("test_bookmark_owner_variants.xbel")));
        QVERIFY(manager.isValid());

        const QList<BookMark> bookmarks = manager.rootGroup().bookmarks();
        QCOMPARE(bookmarks.size(), 1);
        const BookMark bookmark = bookmarks.constFirst();
        const QHash<QString, QStringList> kdeMeta = bookmark.kdeMetaData();
        QCOMPARE(kdeMeta.value(QStringLiteral("visit_count")).constFirst(), QStringLiteral("42"));

        const QString href = QStringLiteral("file:///tmp/owner-variants");
        const QStringList groups = manager.freedesktopGroups(href);
        QVERIFY(groups.contains(QStringLiteral("variant-group")));

        const QList<FreedesktopApplicationInfo> apps = manager.freedesktopApplications(href);
        QCOMPARE(apps.size(), 1);
        QCOMPARE(apps.constFirst().name(), QStringLiteral("VariantApp"));
        QCOMPARE(apps.constFirst().count(), 2);
    }
}

QTEST_GUILESS_MAIN(BookMarkTest)
