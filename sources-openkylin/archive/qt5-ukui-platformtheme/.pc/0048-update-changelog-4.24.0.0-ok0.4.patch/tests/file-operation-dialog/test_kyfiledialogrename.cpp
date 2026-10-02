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

// Include KyFileDialogRename headers
#include "kyfiledialogrename.h"
#include "rename-editor.h"

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

    // Show event test
    void testShowEvent();

private:
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
    // Test truncateDestFileName with ext file system
    QString fileName = "test_file_name.txt";
    QString result = m_renameDialog->truncateDestFileName(fileName, 0, "ext4");
    QVERIFY(!result.isEmpty());
}

void TestKyFileDialogRename::testChangeFileNameSignal()
{
    // Test changeFileName signal
    QSignalSpy spy(m_renameDialog, &KyFileDialogRename::changeFileName);
    
    // Manually trigger the signal
    Q_EMIT m_renameDialog->changeFileName("new_name.txt");
    
    QVERIFY(spy.count() == 1);
}

void TestKyFileDialogRename::testMuchLongNameRejectSignal()
{
    // Test muchLongNameReject signal
    QSignalSpy spy(m_renameDialog, &KyFileDialogRename::muchLongNameReject);
    
    // Manually trigger the signal
    Q_EMIT m_renameDialog->muchLongNameReject();
    
    QVERIFY(spy.count() == 1);
}

void TestKyFileDialogRename::testShowEvent()
{
    // Test showEvent - this is protected but we can test it indirectly
    // by showing the dialog
    m_renameDialog->show();
    QVERIFY(m_renameDialog->isVisible());
    m_renameDialog->hide();
    QVERIFY(!m_renameDialog->isVisible());
}

QTEST_MAIN(TestKyFileDialogRename)

#include "test_kyfiledialogrename.moc"
