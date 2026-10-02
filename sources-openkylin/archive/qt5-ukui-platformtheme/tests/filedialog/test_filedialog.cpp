/*
 * Qt5-UKUI Unit Test for File Dialog
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License.
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
#include <QFileDialog>
#include <QWidget>

class TestFileDialog : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testFileDialogCreation();
    void testFileDialogProperties();

private:
    QWidget *m_parent;
};

void TestFileDialog::initTestCase()
{
}

void TestFileDialog::cleanupTestCase()
{
}

void TestFileDialog::init()
{
    m_parent = new QWidget();
}

void TestFileDialog::cleanup()
{
    delete m_parent;
}

void TestFileDialog::testFileDialogCreation()
{
    QFileDialog dialog(m_parent);
    QVERIFY(dialog.parent() == m_parent);
}

void TestFileDialog::testFileDialogProperties()
{
    QFileDialog dialog;

    // Test default properties
    // 注意：isModal() 和 fileMode() 在某些 Qt 版本中默认值可能不同
    QVERIFY(dialog.acceptMode() == QFileDialog::AcceptOpen);

    // Test setting accept mode
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    QVERIFY(dialog.acceptMode() == QFileDialog::AcceptSave);

    // Test setting file mode
    dialog.setFileMode(QFileDialog::Directory);
    QVERIFY(dialog.fileMode() == QFileDialog::Directory);

    dialog.setFileMode(QFileDialog::ExistingFiles);
    QVERIFY(dialog.fileMode() == QFileDialog::ExistingFiles);
}

QTEST_MAIN(TestFileDialog)

#include "test_filedialog.moc"
