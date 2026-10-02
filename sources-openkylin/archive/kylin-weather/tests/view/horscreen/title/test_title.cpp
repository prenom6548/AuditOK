/*
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
 */
#include "test_title.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QSignalSpy>
#include <QMouseEvent>
#include "global/corevar.h"
#include "global/viewvar.h"
#include "controller/Interaction.h"
#include "view/horscreen/title.h"
#include "view/horscreen/horscreen.h"

TestTitle::TestTitle()
    : m_title(nullptr)
{
}

void TestTitle::initTestCase()
{
}

void TestTitle::cleanupTestCase()
{
}

void TestTitle::init()
{
    m_title = new title();
}

void TestTitle::cleanup()
{
    if (m_title) {
        delete m_title;
        m_title = nullptr;
    }
}

void TestTitle::testConstructor()
{
    QVERIFY(m_title != nullptr);
    QVERIFY(m_title->m_searchLineEdit != nullptr);
}

void TestTitle::testSlotSearchLineEditClear()
{
    CoreVar::CURRENT_TITLE_SEARCH = true;
    m_title->slotSearchLineEditClear();
    QCOMPARE(CoreVar::CURRENT_TITLE_SEARCH, false);
}

void TestTitle::testSlotChangeLineEditStyle()
{
    m_title->slotChangeLineEditStyle();
    QVERIFY(m_title->m_searchLineEdit->isEnabled());
}

void TestTitle::testSlotChangePCMode()
{
    m_title->slotChangePCMode();
    QCOMPARE(m_title->height(), 40);
}

void TestTitle::testSlotChangeHScreenMode()
{
    m_title->slotChangeHScreenMode();
    QCOMPARE(m_title->height(), 64);
}

void TestTitle::testSlotChangeVScreenMode()
{
    m_title->slotChangeVScreenMode();
    QCOMPARE(m_title->height(), 64);
}

void TestTitle::testGetSearchLinePositionX()
{
    m_title->show();
    int x = m_title->getSearchLinePositionX();
    QVERIFY(x >= 0);
}

void TestTitle::testGetSearchLinePositionY()
{
    m_title->show();
    int y = m_title->getSearchLinePositionY();
    QVERIFY(y >= 0);
}

void TestTitle::testEventFilter()
{
    QSignalSpy spy(m_title, SIGNAL(sigLineEditGetFocus()));
    QFocusEvent event(QEvent::FocusIn);
    QApplication::sendEvent(m_title->m_searchLineEdit, &event);
    QCOMPARE(spy.count(), 1);
}

void TestTitle::testKeyPressEvent()
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_E, Qt::ControlModifier);
    QApplication::sendEvent(m_title, &event);
    QVERIFY(m_title->m_searchLineEdit->hasFocus());
}

void TestTitle::testSendSearchInput()
{
    QSignalSpy spy(m_title, SIGNAL(sigTitleSearchCity()));
    m_title->m_searchLineEdit->setText("Beijing");
    
    // Qt 的信号可能是异步处理的，或者 textChanged 直接触发
    if (spy.isEmpty()) {
        QTest::qWait(100);
    }
    
    QVERIFY(spy.count() >= 1);
    
    // 测试重复输入（不应再次触发信号）
    int count = spy.count();
    m_title->m_searchLineEdit->setText("Beijing");
    QCOMPARE(spy.count(), count);
}

void TestTitle::testButtonConnections()
{
    // 需要初始化 horscreen::mutual 避免崩溃
    horscreen dummy;
    horscreen::mutual = &dummy;
    
    // 测试刷新按钮
    QSignalSpy spyRefresh(m_title, SIGNAL(sigRefreshWeather()));
    m_title->btnRefresh->click();
    QCOMPARE(spyRefresh.count(), 1);
    
    // 测试最大化/还原按钮
    QSignalSpy spyRecovery(m_title, SIGNAL(recovery()));
    m_title->btnMaximum->click();
    QCOMPARE(spyRecovery.count(), 1);
    
    // 最小化和关闭按钮调用的是 horscreen::mutual 的方法，通过不崩溃来验证
    m_title->btnMinimize->click();
    m_title->btnCancel->click();
    
    horscreen::mutual = nullptr;
}
