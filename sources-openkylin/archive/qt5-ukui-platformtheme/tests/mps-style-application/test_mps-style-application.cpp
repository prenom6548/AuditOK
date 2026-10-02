/*
 * Qt5-UKUI Unit Test for MPS Style Application
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include <QTest>
#include <QMainWindow>
#include <QApplication>
#include <QStyleFactory>

class TestMpsStyleApplication : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testMpsStyleCreation();
    void testMainWindowWithStyle();
    void testStyleProperty();

private:
    QMainWindow *m_window;
};

void TestMpsStyleApplication::initTestCase()
{
}

void TestMpsStyleApplication::cleanupTestCase()
{
}

void TestMpsStyleApplication::init()
{
    m_window = new QMainWindow();
}

void TestMpsStyleApplication::cleanup()
{
    delete m_window;
}

void TestMpsStyleApplication::testMpsStyleCreation()
{
    // Test that we can create a main window
    QVERIFY(m_window != nullptr);
    QVERIFY(m_window->isVisible() == false);
}

void TestMpsStyleApplication::testMainWindowWithStyle()
{
    // Test setting different styles
    QStringList styles = QStyleFactory::keys();
    QVERIFY(styles.size() > 0);
    
    // Test that we can apply a style
    QApplication::setStyle("fusion");
    QVERIFY(QApplication::style() != nullptr);
    
    // Reset to default
    QApplication::setStyle("");
}

void TestMpsStyleApplication::testStyleProperty()
{
    // Test window properties related to styling
    m_window->setProperty("useSystemStyleBlur", false);
    QVERIFY(m_window->property("useSystemStyleBlur").toBool() == false);
    
    m_window->setProperty("useSystemStyleBlur", true);
    QVERIFY(m_window->property("useSystemStyleBlur").toBool() == true);
}

QTEST_MAIN(TestMpsStyleApplication)

#include "test_mps-style-application.moc"