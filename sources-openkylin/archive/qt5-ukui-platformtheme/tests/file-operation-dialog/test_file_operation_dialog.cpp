/*
 * Unit tests for qt5-ukui-filedialog/file-operation-dialog/rename-editor.cpp and
 * kyfileoperationdialog.cpp. kyfiledialogrename.cpp is compiled into test-settings
 * (Peony / gsettings / full filedialog stack).
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <QtTest>
#include <QSignalSpy>

#include "../../qt5-ukui-filedialog/file-operation-dialog/rename-editor.h"
#include "../../qt5-ukui-filedialog/file-operation-dialog/kyfileoperationdialog.h"

class TestFileOperationDialog : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testRenameEditorConstructAndReturn()
    {
        UKUIFileDialog::RenameEditor editor;
        editor.resize(200, 40);
        QSignalSpy spy(&editor, &UKUIFileDialog::RenameEditor::returnPressed);
        QTest::keyClick(&editor, Qt::Key_Return);
        QCOMPARE(spy.count(), 1);
        spy.clear();
        QTest::keyClick(&editor, Qt::Key_Enter);
        QCOMPARE(spy.count(), 1);
    }

    void testRenameEditorDelegatesOtherKeys()
    {
        UKUIFileDialog::RenameEditor editor;
        editor.show();
        QSignalSpy spy(&editor, &UKUIFileDialog::RenameEditor::returnPressed);
        QTest::keyClick(&editor, Qt::Key_A);
        QCOMPARE(spy.count(), 0);
        QVERIFY(editor.toPlainText().contains(QLatin1Char('a')));
    }

    void testKyFileOperationDialogConstruct()
    {
        UKUIFileDialog::KyFileOperationDialog dlg(nullptr);
        (void)dlg.windowTitle();
    }
};

QTEST_MAIN(TestFileOperationDialog)

#include "test_file_operation_dialog.moc"
