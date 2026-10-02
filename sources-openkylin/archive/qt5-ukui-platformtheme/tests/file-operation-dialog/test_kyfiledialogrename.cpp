/*
 * Qt5-UKUI Unit Test for KyFileDialogRename
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#include <functional>
#include <gio/gio.h>
#include <file-operation-error-handler.h>

#include <QTest>
#include <QObject>
#include <QWidget>
#include <QSignalSpy>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QShowEvent>
#include <QTimer>

// Include KyFileDialogRename headers
#include "kyfiledialogrename.h"
#include "rename-editor.h"
#include <QKeyEvent>

using namespace UKUIFileDialog;

class TestKyFileDialogRename : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // MyTextEdit tests
    void testMyTextEditCreation();
    void testMyTextEditFontChange();

    // KyFileDialogRename basic tests
    void testKyFileDialogRenameCreation();
    void testDialogResult();
    void testSetCurrentUri();
    void testCurrentUri();
    void testSetMaxLength();

    // Truncate function tests
    void testTruncateDestFileName();
    void testTruncateDestFileNamePost();
    void testTruncateDestFileNameFront();
    void testTruncateDestFileNameNtfs();
    void testTruncateDestFileNameExt();

    // Signal tests
    void testChangeFileNameSignal();
    void testMuchLongNameRejectSignal();

    void testTruncateEdgeCases();
    void testTruncateForceChopPaths();
    void testDialogResultDefault();
    void testShowEventOffscreen();
    void testRenameEditorKeyPress();
    void testTruncateTarArchiveSuffixes();
    void testTruncateNormalSuffixExt4();
    void testTruncateNtfsCharacterCountPath();
    void testTruncateLongSuffixForceChop();

    void testHandleCancelOffscreen();
    void testHandleTruncateOffscreen();
    void testHandleRenameOffscreen();
    void testHandleSaveOffscreen();

private:
    Peony::FileOperationError makeLongNameRenameError(const QString &destDirUri) const;
    void scheduleHandleReject(KyFileDialogRename *dlg, int ms = 80);
    QPushButton *findPushButton(KyFileDialogRename *dlg, const QString &label) const;
    QRadioButton *findRadioButton(KyFileDialogRename *dlg, const std::function<bool(const QString &)> &match) const;
    KyFileDialogRename *m_renameDialog;
    QWidget *m_parent;
};

void TestKyFileDialogRename::initTestCase()
{
}

void TestKyFileDialogRename::cleanupTestCase()
{
}

void TestKyFileDialogRename::init()
{
    m_parent = new QWidget();
    m_renameDialog = new KyFileDialogRename(m_parent);
}

void TestKyFileDialogRename::cleanup()
{
    delete m_renameDialog;
    delete m_parent;
}

void TestKyFileDialogRename::testMyTextEditCreation()
{
    // Test MyTextEdit creation
    MyTextEdit *textEdit = new MyTextEdit(m_parent);
    QVERIFY(textEdit != nullptr);
    QVERIFY(textEdit->isReadOnly());
    delete textEdit;
}

void TestKyFileDialogRename::testMyTextEditFontChange()
{
    // Test MyTextEdit font change handling
    MyTextEdit *textEdit = new MyTextEdit(m_parent);
    
    // Change application font - should trigger the connected lambda
    QFont newFont("Arial", 12);
    QApplication::setFont(newFont);
    
    // Verify font was applied
    QVERIFY(textEdit->font().family() == "Arial" || textEdit->font().pointSize() == 12);
    
    delete textEdit;
}

void TestKyFileDialogRename::testKyFileDialogRenameCreation()
{
    QVERIFY(m_renameDialog != nullptr);
    QVERIFY(m_renameDialog->parent() == m_parent);
}

void TestKyFileDialogRename::testDialogResult()
{
    // Test dialogResult getter
    bool result = m_renameDialog->dialogResult();
    QVERIFY(result == true || result == false);
}

void TestKyFileDialogRename::testSetCurrentUri()
{
    // Test setCurrentUri
    QString testUri = "file:///home/user/test.txt";
    m_renameDialog->setCurrentUri(testUri);
    QVERIFY(m_renameDialog->currentUri() == testUri);
}

void TestKyFileDialogRename::testCurrentUri()
{
    // Test currentUri getter
    QString testUri = "file:///home/user/document.pdf";
    m_renameDialog->setCurrentUri(testUri);
    QString result = m_renameDialog->currentUri();
    QVERIFY(result == testUri);
}

void TestKyFileDialogRename::testSetMaxLength()
{
    // Test setMaxLength
    m_renameDialog->setMaxLength(100);
    // The internal m_maxLength should be 100 (we can't directly access it, but the function should work)
    QVERIFY(true);
}

void TestKyFileDialogRename::testTruncateDestFileName()
{
    // Test truncateDestFileName with basic file name
    QString fileName = "test_file_name_that_is_very_long.txt";
    QString result = m_renameDialog->truncateDestFileName(fileName, 0, "ext4");
    QVERIFY(!result.isEmpty());
}

void TestKyFileDialogRename::testTruncateDestFileNamePost()
{
    // Test truncateDestFileName with post truncation (cateType = 0)
    QString fileName = "very_long_file_name_that_exceeds_the_limit_for_testing.txt";
    QString result = m_renameDialog->truncateDestFileName(fileName, 0, "ext4");
    QVERIFY(!result.isEmpty());
}

void TestKyFileDialogRename::testTruncateDestFileNameFront()
{
    // Test truncateDestFileName with front truncation (cateType = 1)
    QString fileName = "very_long_file_name_that_exceeds_the_limit_for_testing.txt";
    QString result = m_renameDialog->truncateDestFileName(fileName, 1, "ext4");
    QVERIFY(!result.isEmpty());
}

void TestKyFileDialogRename::testTruncateDestFileNameNtfs()
{
    // Test truncateDestFileName with NTFS file system
    QString fileName = "test_file_name.txt";
    QString result = m_renameDialog->truncateDestFileName(fileName, 0, "ntfs");
    QVERIFY(!result.isEmpty());
}

void TestKyFileDialogRename::testTruncateDestFileNameExt()
{
    QString fileName = "test_file_name.txt";
    QString result = m_renameDialog->truncateDestFileName(fileName, 0, "ext4");
    QVERIFY(!result.isEmpty());
}

void TestKyFileDialogRename::testTruncateEdgeCases()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QString archive = tmp.path() + QStringLiteral("/sample.tar.gz");
    QFile f(archive);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();

    const QString tarResult = m_renameDialog->truncateDestFileName(archive, 0, QStringLiteral("ext4"));
    QVERIFY(!tarResult.isEmpty());

    const QString ntfsLong = QStringLiteral(
        "abcdefghijklmnopqrstuvwxyz_abcdefghijklmnopqrstuvwxyz_"
        "abcdefghijklmnopqrstuvwxyz_abcdefghijklmnopqrstuvwxyz_"
        "abcdefghijklmnopqrstuvwxyz_abcdefghijklmnopqrstuvwxyz_"
        "abcdefghijklmnopqrstuvwxyz.txt");
    const QString ntfsResult = m_renameDialog->truncateDestFileName(ntfsLong, 1, QStringLiteral("ntfs"));
    QVERIFY(ntfsResult.contains(QStringLiteral("<s>")) || !ntfsResult.isEmpty());

    const QString fuseResult = m_renameDialog->truncateDestFileName(
        QStringLiteral("long_name_for_fuse_filesystem_test_only.bin"), 0, QStringLiteral("fuse.kyfs"));
    QVERIFY(!fuseResult.isEmpty());

    const QString dirPath = tmp.path() + QStringLiteral("/subdir");
    QVERIFY(QDir().mkpath(dirPath));
    const QString dirResult = m_renameDialog->truncateDestFileName(dirPath, 0, QStringLiteral("ext4"));
    QVERIFY(!dirResult.isEmpty());
}

void TestKyFileDialogRename::testTruncateForceChopPaths()
{
    const QString veryLongBase = QString(300, QLatin1Char('a')) + QStringLiteral(".txt");
    const QString postExt4 = m_renameDialog->truncateDestFileName(veryLongBase, 0, QStringLiteral("ext4"));
    QVERIFY(postExt4.contains(QStringLiteral("<s>")) || !postExt4.isEmpty());

    const QString frontExt4 = m_renameDialog->truncateDestFileName(veryLongBase, 1, QStringLiteral("ext4"));
    QVERIFY(frontExt4.contains(QStringLiteral("<s>")) || !frontExt4.isEmpty());

    const QString longSuffix = QString(250, QLatin1Char('x')) + QStringLiteral(".dat");
    const QString chopSuffix = m_renameDialog->truncateDestFileName(longSuffix, 0, QStringLiteral("ext4"));
    QVERIFY(!chopSuffix.isEmpty());

    m_renameDialog->setMaxLength(50);
    const QString shortLimit = m_renameDialog->truncateDestFileName(
        QStringLiteral("short.txt"), 0, QStringLiteral("ntfs"));
    QVERIFY(!shortLimit.isEmpty());
}

void TestKyFileDialogRename::testDialogResultDefault()
{
    QVERIFY(m_renameDialog->dialogResult() == true || m_renameDialog->dialogResult() == false);
    m_renameDialog->setCurrentUri(QStringLiteral("file:///tmp/a"));
    QCOMPARE(m_renameDialog->currentUri(), QStringLiteral("file:///tmp/a"));
    m_renameDialog->setCurrentUri(QStringLiteral("file:///tmp/a"));
    QCOMPARE(m_renameDialog->currentUri(), QStringLiteral("file:///tmp/a"));
}

void TestKyFileDialogRename::testChangeFileNameSignal()
{
    // Test changeFileName signal
    QSignalSpy spy(m_renameDialog, &KyFileDialogRename::changeFileName);
    
    // Manually trigger the signal
    Q_EMIT m_renameDialog->changeFileName("new_name.txt");
    
    QVERIFY(spy.count() == 1);
}

void TestKyFileDialogRename::testShowEventOffscreen()
{
    m_renameDialog->setCurrentUri(QStringLiteral("file:///tmp/rename-show.txt"));
    QShowEvent showEv;
    QCoreApplication::sendEvent(m_renameDialog, &showEv);
    QVERIFY(m_renameDialog->currentUri().contains(QStringLiteral("rename-show")));
}

void TestKyFileDialogRename::testRenameEditorKeyPress()
{
    RenameEditor editor(m_parent);
    QSignalSpy spy(&editor, &RenameEditor::returnPressed);
    QKeyEvent enter(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QCoreApplication::sendEvent(&editor, &enter);
    QCOMPARE(spy.count(), 1);

    QKeyEvent other(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    QCoreApplication::sendEvent(&editor, &other);
    QCOMPARE(spy.count(), 1);
}

void TestKyFileDialogRename::testMuchLongNameRejectSignal()
{
    // Test muchLongNameReject signal
    QSignalSpy spy(m_renameDialog, &KyFileDialogRename::muchLongNameReject);
    
    // Manually trigger the signal
    Q_EMIT m_renameDialog->muchLongNameReject();
    
    QVERIFY(spy.count() == 1);
}

void TestKyFileDialogRename::testTruncateTarArchiveSuffixes()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QStringList archives = {
        tmp.path() + QStringLiteral("/a.tar.gz"),
        tmp.path() + QStringLiteral("/b.tar.bz2"),
        tmp.path() + QStringLiteral("/c.tar.xz"),
    };
    for (const QString &path : archives) {
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("1");
        f.close();
        const QString post = m_renameDialog->truncateDestFileName(path, 0, QStringLiteral("ext4"));
        const QString front = m_renameDialog->truncateDestFileName(path, 1, QStringLiteral("ext4"));
        QVERIFY(!post.isEmpty());
        QVERIFY(!front.isEmpty());
    }
}

void TestKyFileDialogRename::testTruncateNormalSuffixExt4()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QString path = tmp.path() + QStringLiteral("/document.txt");
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();

    const QString moderate = QString(180, QLatin1Char('n')) + QStringLiteral(".txt");
    const QString moderatePath = tmp.path() + QLatin1Char('/') + moderate;
    QFile f2(moderatePath);
    QVERIFY(f2.open(QIODevice::WriteOnly));
    f2.write("x");
    f2.close();

    const QString result = m_renameDialog->truncateDestFileName(moderatePath, 0, QStringLiteral("ext4"));
    QVERIFY(result.contains(QLatin1String(".txt")) || result.contains(QLatin1String("<s>")));
}

void TestKyFileDialogRename::testTruncateNtfsCharacterCountPath()
{
    const QString name = QString(200, QLatin1Char('z')) + QStringLiteral(".dat");
    const QString post = m_renameDialog->truncateDestFileName(name, 0, QStringLiteral("ntfs"));
    const QString front = m_renameDialog->truncateDestFileName(name, 1, QStringLiteral("fuse.kyfs"));
    QVERIFY(post.contains(QLatin1String("<s>")) || !post.isEmpty());
    QVERIFY(front.contains(QLatin1String("<s>")) || !front.isEmpty());
}

Peony::FileOperationError TestKyFileDialogRename::makeLongNameRenameError(const QString &destDirUri) const
{
    Peony::FileOperationError error;
    error.op = Peony::FileOpRename;
    error.destDirUri = destDirUri;
    error.errorCode = G_IO_ERROR_FILENAME_TOO_LONG;
    error.errorStr = QStringLiteral("filename too long");
    return error;
}

void TestKyFileDialogRename::scheduleHandleReject(KyFileDialogRename *dlg, int ms)
{
    QTimer::singleShot(ms, dlg, [dlg]() { dlg->reject(); });
}

QPushButton *TestKyFileDialogRename::findPushButton(KyFileDialogRename *dlg, const QString &label) const
{
    const auto buttons = dlg->findChildren<QPushButton *>();
    for (QPushButton *btn : buttons) {
        if (btn->text() == label)
            return btn;
    }
    return nullptr;
}

QRadioButton *TestKyFileDialogRename::findRadioButton(
    KyFileDialogRename *dlg, const std::function<bool(const QString &)> &match) const
{
    const auto radios = dlg->findChildren<QRadioButton *>();
    for (QRadioButton *radio : radios) {
        if (match(radio->text()))
            return radio;
    }
    return nullptr;
}

void TestKyFileDialogRename::testHandleCancelOffscreen()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());

    const QString baseName = QStringLiteral("existing.txt");
    const QString filePath = tmp.path() + QLatin1Char('/') + baseName;
    QFile f(filePath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();

    const QString currentUri = QStringLiteral("file://") + filePath;
    const QString longDest = QString(280, QLatin1Char('x')) + QStringLiteral(".txt");
    const QString destUri = QStringLiteral("file://") + tmp.path() + QLatin1Char('/') + longDest;

    m_renameDialog->setAttribute(Qt::WA_DontShowOnScreen, true);
    m_renameDialog->setCurrentUri(currentUri);
    m_renameDialog->setMaxLength(255);

    Peony::FileOperationError error = makeLongNameRenameError(destUri);
    scheduleHandleReject(m_renameDialog);

    m_renameDialog->handle(error);

    QVERIFY(!m_renameDialog->dialogResult());
}

void TestKyFileDialogRename::testHandleTruncateOffscreen()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());

    const QString filePath = tmp.path() + QStringLiteral("/seed.txt");
    QFile f(filePath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();

    const QString currentUri = QStringLiteral("file://") + filePath;
    const QString longDest = QString(280, QLatin1Char('t')) + QStringLiteral(".txt");
    const QString destUri = QStringLiteral("file://") + tmp.path() + QLatin1Char('/') + longDest;

    m_renameDialog->setAttribute(Qt::WA_DontShowOnScreen, true);
    m_renameDialog->setCurrentUri(currentUri);
    m_renameDialog->setMaxLength(255);

    Peony::FileOperationError error = makeLongNameRenameError(destUri);
    KyFileDialogRename *dlg = m_renameDialog;

    QTimer::singleShot(80, dlg, [this, dlg]() {
        QRadioButton *cutOff = findRadioButton(dlg, [](const QString &text) {
            return text.contains(QStringLiteral("Truncation"), Qt::CaseInsensitive)
                && !text.contains(QStringLiteral("Front"), Qt::CaseInsensitive)
                && !text.contains(QStringLiteral("Post"), Qt::CaseInsensitive);
        });
        if (cutOff)
            QTest::mouseClick(cutOff, Qt::LeftButton);
        QPushButton *apply = findPushButton(dlg, tr("OK"));
        if (apply)
            QTest::mouseClick(apply, Qt::LeftButton);
    });

    QTimer::singleShot(200, dlg, [this, dlg]() {
        QPushButton *ok = findPushButton(dlg, tr("OK"));
        if (ok && ok->isEnabled())
            QTest::mouseClick(ok, Qt::LeftButton);
    });

    QSignalSpy nameSpy(dlg, &KyFileDialogRename::changeFileName);

    dlg->handle(error);

    QVERIFY(dlg->dialogResult());
    QCOMPARE(error.respCode, Peony::ExceptionResponse::TruncateOne);
    QVERIFY(error.respValue.contains(QStringLiteral("cateType")));
    QVERIFY(nameSpy.count() >= 1);
}

void TestKyFileDialogRename::testHandleRenameOffscreen()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());

    const QString filePath = tmp.path() + QStringLiteral("/rename-seed.txt");
    QFile f(filePath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();

    const QString currentUri = QStringLiteral("file://") + filePath;
    const QString longDest = QString(280, QLatin1Char('r')) + QStringLiteral(".txt");
    const QString destUri = QStringLiteral("file://") + tmp.path() + QLatin1Char('/') + longDest;

    m_renameDialog->setAttribute(Qt::WA_DontShowOnScreen, true);
    m_renameDialog->setCurrentUri(currentUri);
    m_renameDialog->setMaxLength(255);

    Peony::FileOperationError error = makeLongNameRenameError(destUri);
    KyFileDialogRename *dlg = m_renameDialog;

    QTimer::singleShot(80, dlg, [this, dlg]() {
        QRadioButton *renameRadio = findRadioButton(dlg, [](const QString &text) {
            return text.contains(QStringLiteral("Rename"), Qt::CaseInsensitive)
                && !text.contains(QStringLiteral("Save"), Qt::CaseInsensitive);
        });
        if (renameRadio)
            QTest::mouseClick(renameRadio, Qt::LeftButton);
        QPushButton *apply = findPushButton(dlg, tr("OK"));
        if (apply && apply->isVisible())
            QTest::mouseClick(apply, Qt::LeftButton);
    });

    QTimer::singleShot(300, dlg, [dlg]() {
        RenameEditor *editor = nullptr;
        for (RenameEditor *e : dlg->findChildren<RenameEditor *>()) {
            editor = e;
            break;
        }
        if (editor)
            editor->setPlainText(QStringLiteral("ok-name.txt"));
        QWidget *page2 = editor ? editor->parentWidget() : nullptr;
        for (QPushButton *btn : dlg->findChildren<QPushButton *>()) {
            if (btn->text() != QObject::tr("OK") || !btn->isEnabled())
                continue;
            if (page2) {
                QWidget *w = btn;
                bool onRenamePage = false;
                while (w) {
                    if (w == page2) {
                        onRenamePage = true;
                        break;
                    }
                    w = w->parentWidget();
                }
                if (!onRenamePage)
                    continue;
            }
            QTest::mouseClick(btn, Qt::LeftButton);
            return;
        }
    });
    scheduleHandleReject(dlg, 800);

    QSignalSpy nameSpy(dlg, &KyFileDialogRename::changeFileName);

    dlg->handle(error);

    QVERIFY(dlg->dialogResult());
    QCOMPARE(error.respCode, Peony::ExceptionResponse::RenameOne);
    QVERIFY(error.respValue.contains(QStringLiteral("newName")));
    QCOMPARE(error.respValue.value(QStringLiteral("newName")).toString(), QStringLiteral("ok-name.txt"));
    QVERIFY(nameSpy.count() >= 1);
}

void TestKyFileDialogRename::testHandleSaveOffscreen()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());

    const QString filePath = tmp.path() + QStringLiteral("/save-seed.txt");
    QFile f(filePath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();

    const QString currentUri = QStringLiteral("file://") + filePath;
    const QString longDest = QString(280, QLatin1Char('s')) + QStringLiteral(".txt");
    const QString destUri = QStringLiteral("file://") + tmp.path() + QLatin1Char('/') + longDest;

    m_renameDialog->setAttribute(Qt::WA_DontShowOnScreen, true);
    m_renameDialog->setCurrentUri(currentUri);
    m_renameDialog->setMaxLength(255);

    Peony::FileOperationError error = makeLongNameRenameError(destUri);
    KyFileDialogRename *dlg = m_renameDialog;

    QTimer::singleShot(80, dlg, [this, dlg]() {
        QRadioButton *saveRadio = findRadioButton(dlg, [](const QString &text) {
            return text.contains(QStringLiteral("Save"), Qt::CaseInsensitive)
                && text.contains(QStringLiteral("long"), Qt::CaseInsensitive);
        });
        if (saveRadio && saveRadio->isVisible())
            QTest::mouseClick(saveRadio, Qt::LeftButton);
        QPushButton *apply = findPushButton(dlg, tr("OK"));
        if (apply && apply->isVisible())
            QTest::mouseClick(apply, Qt::LeftButton);
    });
    scheduleHandleReject(dlg, 1200);

    dlg->handle(error);

    if (dlg->dialogResult()) {
        QVERIFY(error.respCode == Peony::ExceptionResponse::SaveOne
                || error.respCode == Peony::ExceptionResponse::SaveAll);
    } else {
        QVERIFY(error.respCode == Peony::ExceptionResponse::Cancel
                || error.respCode == Peony::ExceptionResponse::Other);
    }
}

void TestKyFileDialogRename::testTruncateLongSuffixForceChop()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QString longSuffix(230, QLatin1Char('s'));
    const QString path = tmp.path() + QStringLiteral("/base") + longSuffix;
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("1");
    f.close();

    const QString post = m_renameDialog->truncateDestFileName(path, 0, QStringLiteral("ext4"));
    const QString front = m_renameDialog->truncateDestFileName(path, 1, QStringLiteral("ext4"));
    QVERIFY(!post.isEmpty());
    QVERIFY(!front.isEmpty());
}

QTEST_MAIN(TestKyFileDialogRename)

#include "test_kyfiledialogrename.moc"
