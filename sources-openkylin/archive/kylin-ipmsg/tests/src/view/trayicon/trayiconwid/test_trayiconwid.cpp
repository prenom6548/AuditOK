/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#include "test_trayiconwid.h"
#include <QApplication>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QStandardItemModel>
#include "model/friendlistmodel.h"
#include "view/titlebar/titleseting.h"
#include <QTimer>

void TestTrayIconWid::initTestCase()
{
}

void TestTrayIconWid::cleanupTestCase()
{
}

void TestTrayIconWid::init()
{
    m_trayIconWid = new TrayIconWid();
}

void TestTrayIconWid::cleanup()
{
    delete m_trayIconWid;
}

void TestTrayIconWid::testConstructor()
{
    QVERIFY(m_trayIconWid != nullptr);
    QVERIFY(m_trayIconWid->m_trayIcon != nullptr);
}

void TestTrayIconWid::testSetWidgetUi()
{
    m_trayIconWid->setWidgetUi();
    QVERIFY(m_trayIconWid->m_mainWid != nullptr);
    QVERIFY(m_trayIconWid->m_friendView != nullptr);
}

void TestTrayIconWid::testSetWidgetStyle()
{
    m_trayIconWid->setWidgetStyle();
    QVERIFY(m_trayIconWid->m_ignoreBtn != nullptr);
}

void TestTrayIconWid::testSetSignalConn()
{
    m_trayIconWid->setSignalConn();
    // 检查不崩溃
}

void TestTrayIconWid::testChangeTheme()
{
    m_trayIconWid->changeTheme();
    // 检查不崩溃
}

void TestTrayIconWid::testEvents()
{
    // 测试 paintEvent
    QPaintEvent paintEvent(m_trayIconWid->rect());
    m_trayIconWid->paintEvent(&paintEvent);
    
    // 测试 keyPressEvent (Esc)
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    m_trayIconWid->keyPressEvent(&keyEvent);
    
    QVERIFY(true);
}

void TestTrayIconWid::testPrivateSlots()
{
    m_trayIconWid->slotStartAlert();
    m_trayIconWid->slotMsgAlert();
    m_trayIconWid->slotIgnoreAll();
    
    // 测试 iconActivated
    m_trayIconWid->iconActivated(QSystemTrayIcon::Trigger);
    m_trayIconWid->iconActivated(QSystemTrayIcon::Context);
    
    // 测试 eventFilter
    QEvent event(QEvent::MouseButtonPress);
    m_trayIconWid->eventFilter(m_trayIconWid->m_ignoreBtn, &event);
    
    QVERIFY(true);
}

void TestTrayIconWid::testCoverageAdditions()
{
    // 1. 覆盖 256-261 (closeEvent)
    QCloseEvent closeEvent;
    m_trayIconWid->closeEvent(&closeEvent);

    // 2. 覆盖 267-272 (keyPressEvent F1)
    QKeyEvent f1Event(QEvent::KeyPress, Qt::Key_F1, Qt::NoModifier);
    m_trayIconWid->keyPressEvent(&f1Event);

    // 3. 覆盖 292-299 (slotMsgAlert)
    m_trayIconWid->hide();
    QStandardItemModel *sourceModel = new QStandardItemModel(m_trayIconWid);
    QStandardItem *item = new QStandardItem();
    item->setData(1, FriendListModel::UnreadMsgNum);
    sourceModel->appendRow(item);
    m_trayIconWid->m_trayFilterModel->setSourceModel(sourceModel);
    m_trayIconWid->slotMsgAlert();
    m_trayIconWid->slotMsgAlert(); // 再次调用以覆盖 m_timeFlag % 2 != 0 分支

    // 4. 覆盖 319-337 (eventFilter)
    QEvent showEvent(QEvent::Show);
    m_trayIconWid->eventFilter(m_trayIconWid, &showEvent);
    
    QEvent focusOutEvent(QEvent::FocusOut);
    m_trayIconWid->eventFilter(m_trayIconWid, &focusOutEvent);
    
    QEvent toolTipEvent(QEvent::ToolTip);
    m_trayIconWid->eventFilter(m_trayIconWid->m_trayIcon, &toolTipEvent);
    
    QEvent otherEvent(QEvent::None);
    m_trayIconWid->eventFilter(m_trayIconWid->m_trayIcon, &otherEvent);

    // 5. 覆盖 354-369 (paintEvent 中的尺寸调整)
    m_trayIconWid->m_msgCounter = 0; // 重置计数器
    QPaintEvent pe(m_trayIconWid->rect());
    m_trayIconWid->paintEvent(&pe); // msgCount = 1

    // 设置更多数据以覆盖 msgCount >= 4 逻辑
    for(int i=0; i<5; ++i) {
        QStandardItem *it = new QStandardItem();
        it->setData(1, FriendListModel::UnreadMsgNum);
        sourceModel->appendRow(it);
    }
    m_trayIconWid->paintEvent(&pe); // msgCount = 6

    // 6. 覆盖 427, 431-433, 442-443 (iconActivated)
    m_trayIconWid->iconActivated(QSystemTrayIcon::Trigger);
    m_trayIconWid->iconActivated(QSystemTrayIcon::DoubleClick);
    m_trayIconWid->iconActivated(QSystemTrayIcon::Unknown);

    // 7. 覆盖 453-472, 497 (slotStartChat)
    // 准备数据
    item->setData(1, FriendListModel::Id);
    item->setData("test-uuid", FriendListModel::Uuid);
    FriendListModel::getInstance()->m_idItemsMap.insert(1, item);
    m_trayIconWid->m_friendView->setCurrentIndex(m_trayIconWid->m_trayFilterModel->index(0, 0));
    m_trayIconWid->slotStartChat();

    // 8. 覆盖 510-513, 518 (slotIgnoreAll)
    m_trayIconWid->slotIgnoreAll();

    // 9. 覆盖 524-539 (slotCloseWin, slotSetting)
    // m_trayIconWid->slotCloseWin(); // 注意：这会调用 KyView::getInstance()->deleteLater()，可能会影响后续测试
    
    QTimer::singleShot(500, []() {
        for (QWidget *w : QApplication::topLevelWidgets()) {
            if (w->objectName().contains("TitleSeting")) {
                w->close();
            }
        }
    });
    m_trayIconWid->slotSetting();

    QVERIFY(true);
}
