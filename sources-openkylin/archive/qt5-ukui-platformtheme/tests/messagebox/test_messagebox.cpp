/*
 * Qt5-UKUI Unit Test for Message Box
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
 * Authors: xibowen <xibowen@kylinos.cn>
 *
 */

#include <QTest>
#include <QMessageBox>
#include <QMetaEnum>

class TestMessageBox : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testMessageBoxCreation();
    void testStandardButtons();
    void testIconTypes();
    void testDetailedText();

private:
    QWidget *m_parent;
};

void TestMessageBox::initTestCase()
{
}

void TestMessageBox::cleanupTestCase()
{
}

void TestMessageBox::init()
{
    m_parent = new QWidget();
}

void TestMessageBox::cleanup()
{
    delete m_parent;
}

void TestMessageBox::testMessageBoxCreation()
{
    QMessageBox box(m_parent);
    QVERIFY(box.parent() == m_parent);
    QVERIFY(box.windowTitle().isEmpty());
}

void TestMessageBox::testStandardButtons()
{
    QMetaEnum metaEnum = QMetaEnum::fromType<QMessageBox::StandardButtons>();
    
    // Test that we can iterate through standard buttons
    int buttonCount = 0;
    for (auto i = 0; i < metaEnum.keyCount(); ++i) {
        if (metaEnum.value(i) >= QMessageBox::FirstButton
                && metaEnum.value(i) <= QMessageBox::LastButton) {
            buttonCount++;
        }
    }
    QVERIFY(buttonCount > 0);
}

void TestMessageBox::testIconTypes()
{
    QMessageBox box;
    
    // Test all icon types
    box.setIcon(QMessageBox::NoIcon);
    QVERIFY(box.icon() == QMessageBox::NoIcon);
    
    box.setIcon(QMessageBox::Information);
    QVERIFY(box.icon() == QMessageBox::Information);
    
    box.setIcon(QMessageBox::Warning);
    QVERIFY(box.icon() == QMessageBox::Warning);
    
    box.setIcon(QMessageBox::Critical);
    QVERIFY(box.icon() == QMessageBox::Critical);
    
    box.setIcon(QMessageBox::Question);
    QVERIFY(box.icon() == QMessageBox::Question);
}

void TestMessageBox::testDetailedText()
{
    QMessageBox box;
    
    // Test setting detailed text
    QString detailedText = "This is detailed text for testing.";
    box.setDetailedText(detailedText);
    QVERIFY(box.detailedText() == detailedText);
    
    // Test clearing detailed text
    box.setDetailedText("");
    QVERIFY(box.detailedText().isEmpty());
    
    // Test with long text
    QString longText;
    for (int i = 0; i < 100; ++i) {
        longText += "This is line " + QString::number(i) + " of detailed text. ";
    }
    box.setDetailedText(longText);
    QVERIFY(box.detailedText() == longText);
}

QTEST_MAIN(TestMessageBox)

#include "test_messagebox.moc"