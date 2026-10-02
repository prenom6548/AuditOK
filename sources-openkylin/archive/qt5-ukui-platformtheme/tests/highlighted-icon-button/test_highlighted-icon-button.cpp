/*
 * Qt5-UKUI Unit Test for Highlighted Icon Button
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
#include <QPushButton>
#include <QToolButton>
#include <QMenu>
#include <QListWidget>

class TestHighlightedIconButton : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testUseIconHighlightEffectProperty();
    void testIconHighlightEffectModeProperty();
    void testMenuWithHighlightEffect();
    void testListWidgetWithHighlightEffect();

private:
    QPushButton *m_pushButton;
    QToolButton *m_toolButton;
    QMenu *m_menu;
    QListWidget *m_listWidget;
};

void TestHighlightedIconButton::initTestCase()
{
}

void TestHighlightedIconButton::cleanupTestCase()
{
}

void TestHighlightedIconButton::init()
{
    m_pushButton = new QPushButton();
    m_toolButton = new QToolButton();
    m_menu = new QMenu();
    m_listWidget = new QListWidget();
}

void TestHighlightedIconButton::cleanup()
{
    delete m_pushButton;
    delete m_toolButton;
    delete m_menu;
    delete m_listWidget;
}

void TestHighlightedIconButton::testUseIconHighlightEffectProperty()
{
    // Test default value (should be false)
    QVERIFY(m_pushButton->property("useIconHighlightEffect").toBool() == false);
    QVERIFY(m_toolButton->property("useIconHighlightEffect").toBool() == false);
    QVERIFY(m_menu->property("useIconHighlightEffect").toBool() == false);

    // Test setting to true
    m_pushButton->setProperty("useIconHighlightEffect", true);
    m_toolButton->setProperty("useIconHighlightEffect", true);
    m_menu->setProperty("useIconHighlightEffect", true);

    QVERIFY(m_pushButton->property("useIconHighlightEffect").toBool() == true);
    QVERIFY(m_toolButton->property("useIconHighlightEffect").toBool() == true);
    QVERIFY(m_menu->property("useIconHighlightEffect").toBool() == true);

    // Test setting back to false
    m_pushButton->setProperty("useIconHighlightEffect", false);
    m_toolButton->setProperty("useIconHighlightEffect", false);
    m_menu->setProperty("useIconHighlightEffect", false);

    QVERIFY(m_pushButton->property("useIconHighlightEffect").toBool() == false);
    QVERIFY(m_toolButton->property("useIconHighlightEffect").toBool() == false);
    QVERIFY(m_menu->property("useIconHighlightEffect").toBool() == false);
}

void TestHighlightedIconButton::testIconHighlightEffectModeProperty()
{
    // Test default value (should be 0)
    QVERIFY(m_pushButton->property("iconHighlightEffectMode").toInt() == 0);
    QVERIFY(m_toolButton->property("iconHighlightEffectMode").toInt() == 0);

    // Test setting different modes
    for (int mode = 0; mode < 5; ++mode) {
        m_pushButton->setProperty("iconHighlightEffectMode", mode);
        m_toolButton->setProperty("iconHighlightEffectMode", mode);
        
        QVERIFY(m_pushButton->property("iconHighlightEffectMode").toInt() == mode);
        QVERIFY(m_toolButton->property("iconHighlightEffectMode").toInt() == mode);
    }
}

void TestHighlightedIconButton::testMenuWithHighlightEffect()
{
    // Add actions to menu
    m_menu->addAction(QIcon::fromTheme("edit-find-symbolic"), "edit-find-symbolic");
    m_menu->addAction(QIcon::fromTheme("edit-cut-symbolic"), "edit-cut-symbolic");
    m_menu->addAction(QIcon::fromTheme("user-trash"), "user-trash");

    // Verify actions were added
    QVERIFY(m_menu->actions().size() == 3);

    // Test setting fillIconSymbolicColor property
    m_menu->setProperty("fillIconSymbolicColor", true);
    QVERIFY(m_menu->property("fillIconSymbolicColor").toBool() == true);

    m_menu->setProperty("fillIconSymbolicColor", false);
    QVERIFY(m_menu->property("fillIconSymbolicColor").toBool() == false);
}

void TestHighlightedIconButton::testListWidgetWithHighlightEffect()
{
    // Add items to list widget
    auto item1 = new QListWidgetItem(QIcon::fromTheme("window-close"), "window-close", m_listWidget);
    auto item2 = new QListWidgetItem(QIcon::fromTheme("window-close-symbolic"), "window-close-symbolic", m_listWidget);
    auto item3 = new QListWidgetItem(QIcon::fromTheme("user-trash"), "user-trash", m_listWidget);

    // Verify items were added
    QVERIFY(m_listWidget->count() == 3);

    // Test setting useIconHighlightEffect property
    m_listWidget->setProperty("useIconHighlightEffect", true);
    QVERIFY(m_listWidget->property("useIconHighlightEffect").toBool() == true);

    m_listWidget->setProperty("useIconHighlightEffect", false);
    QVERIFY(m_listWidget->property("useIconHighlightEffect").toBool() == false);
}

QTEST_MAIN(TestHighlightedIconButton)

#include "test_highlighted-icon-button.moc"