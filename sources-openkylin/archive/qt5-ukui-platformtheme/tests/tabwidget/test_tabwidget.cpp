/*
 * Qt5-UKUI Unit Test for TabWidget
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
#include <QTabWidget>
#include <QLabel>
#include <QStackedWidget>

class TestTabWidget : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testTabWidgetCreation();
    void testAddTabs();
    void testCurrentIndex();
    void testTabPosition();
    void testTabsClosable();

private:
    QTabWidget *m_tabWidget;
};

void TestTabWidget::initTestCase()
{
}

void TestTabWidget::cleanupTestCase()
{
}

void TestTabWidget::init()
{
    m_tabWidget = new QTabWidget();
}

void TestTabWidget::cleanup()
{
    delete m_tabWidget;
}

void TestTabWidget::testTabWidgetCreation()
{
    QVERIFY(m_tabWidget != nullptr);
    QVERIFY(m_tabWidget->count() == 0);
    QVERIFY(m_tabWidget->currentIndex() == -1);
}

void TestTabWidget::testAddTabs()
{
    // Add first tab
    QLabel *label1 = new QLabel("Tab 1");
    m_tabWidget->addTab(label1, "Tab 1");
    QVERIFY(m_tabWidget->count() == 1);
    QVERIFY(m_tabWidget->tabText(0) == "Tab 1");
    
    // Add second tab
    QLabel *label2 = new QLabel("Tab 2");
    m_tabWidget->addTab(label2, "Tab 2");
    QVERIFY(m_tabWidget->count() == 2);
    QVERIFY(m_tabWidget->tabText(1) == "Tab 2");
    
    // Add third tab
    QLabel *label3 = new QLabel("Tab 3");
    m_tabWidget->addTab(label3, "Tab 3");
    QVERIFY(m_tabWidget->count() == 3);
    QVERIFY(m_tabWidget->tabText(2) == "Tab 3");
}

void TestTabWidget::testCurrentIndex()
{
    // Add some tabs
    m_tabWidget->addTab(new QLabel("Tab 1"), "Tab 1");
    m_tabWidget->addTab(new QLabel("Tab 2"), "Tab 2");
    m_tabWidget->addTab(new QLabel("Tab 3"), "Tab 3");
    
    // Test current index
    QVERIFY(m_tabWidget->currentIndex() == 0);
    
    // Change current index
    m_tabWidget->setCurrentIndex(1);
    QVERIFY(m_tabWidget->currentIndex() == 1);
    
    m_tabWidget->setCurrentIndex(2);
    QVERIFY(m_tabWidget->currentIndex() == 2);
    
    // Test current widget
    QVERIFY(m_tabWidget->currentWidget() != nullptr);
}

void TestTabWidget::testTabPosition()
{
    // Test default position
    QVERIFY(m_tabWidget->tabPosition() == QTabWidget::North);
    
    // Test all positions
    m_tabWidget->setTabPosition(QTabWidget::South);
    QVERIFY(m_tabWidget->tabPosition() == QTabWidget::South);
    
    m_tabWidget->setTabPosition(QTabWidget::East);
    QVERIFY(m_tabWidget->tabPosition() == QTabWidget::East);
    
    m_tabWidget->setTabPosition(QTabWidget::West);
    QVERIFY(m_tabWidget->tabPosition() == QTabWidget::West);
    
    // Reset to North
    m_tabWidget->setTabPosition(QTabWidget::North);
    QVERIFY(m_tabWidget->tabPosition() == QTabWidget::North);
}

void TestTabWidget::testTabsClosable()
{
    // Test default value
    QVERIFY(m_tabWidget->tabsClosable() == false);
    
    // Test setting to true
    m_tabWidget->setTabsClosable(true);
    QVERIFY(m_tabWidget->tabsClosable() == true);
    
    // Test setting to false
    m_tabWidget->setTabsClosable(false);
    QVERIFY(m_tabWidget->tabsClosable() == false);
}

QTEST_MAIN(TestTabWidget)

#include "test_tabwidget.moc"