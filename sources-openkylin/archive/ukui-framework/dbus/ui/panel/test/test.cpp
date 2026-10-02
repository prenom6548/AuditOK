/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "test.h"

void testLog(QString str, bool isSleep, int msSleep)
{
#if TEST_PRINT_LOG
    qDebug()<<str;
    if (isSleep) {
        QThread::msleep(msSleep);
    }
#endif
}

PanelConfigTest::PanelConfigTest()
{
    initEnum();
    testConnect();
}

PanelConfigTest::~PanelConfigTest()
{
    delete m_panelConfig;
    m_panelConfig = nullptr;
}

void PanelConfigTest::testConnect()
{
    m_panelConfig = new ConfigUkPanel();
    QSignalSpy spy(m_panelConfig, &ConfigUkPanel::connectStateChanged);

    QTRY_VERIFY(spy.count() > 0);
    QCOMPARE(spy[0].at(0).toBool(), true);
}

void testSetLockUnit(ConfigUkPanel* panel, bool expectedState) {

    panel->SetPanelLocked(expectedState);
    testLog(QString("panel testSetLockUnit %1").arg(expectedState), true, 1000);
    QSignalSpy spy(panel, &ConfigUkPanel::lockChanged);
    QTRY_VERIFY(spy.count() > 0);
    QCOMPARE(expectedState, spy[0].at(0).toBool());
}

void testSetMergeIconsUnit(ConfigUkPanel* panel, const int mergeIcons) {
    QSignalSpy spy(panel, &ConfigUkPanel::mergeIconsChanged);
    qDebug()<<"ready set:"<<mergeIcons;
    panel->SetMergedIconType(mergeIcons);
    QTRY_VERIFY(spy.count() > 0);
    QCOMPARE(spy[0].at(0).toInt(), mergeIcons);
}

void PanelConfigTest::testSetMergeIcons()
{
    int originState =  m_panelConfig->GetMergeIconsType();
    int lastKey;
    qDebug()<<originState;
    if (originState == 0) {
        lastKey = originState + 1;
        testLog(QString("调整初始值%1").arg( m_mergeEnum.valueToKey(1)),false, 3000);
        testSetMergeIconsUnit(m_panelConfig, lastKey);
    }

    for(int i = 0; i < m_mergeEnum.keyCount(); ++i) {
        QString key = m_mergeEnum.valueToKey(i);
        testLog(QString("开始测试合并任务栏上的图标：%1").arg(key),true, 1000);
        testSetMergeIconsUnit(m_panelConfig,  i);
        testLog(QString("测试合并任务栏上的图标：%1 over ret:%2").arg(key).arg(lastKey),true, 1000);
        lastKey = i;
    }

    qDebug()<<lastKey << originState;
    if (lastKey!=originState) {
        testLog(QString("还原初始值%1").arg( m_mergeEnum.valueToKey(1)),true, 3000);
        testSetMergeIconsUnit(m_panelConfig,  originState);
    }
}

void testSetPanelLocationUnit(ConfigUkPanel* panel, const int& location) {
    QSignalSpy spy(panel, &ConfigUkPanel::locationChanged);
    panel->SetPanelPosition(location);
    QTRY_VERIFY(spy.count() > 0);
    QCOMPARE(spy[0].at(0).toInt(), location);
}

void PanelConfigTest::testSetPanelLocation()
{
    if (m_panelConfig->IsPanelLocked()) {
        testSetLockUnit(m_panelConfig, false);
    }
    int originState =  m_panelConfig->GetPanelPosition();
    int lastKey;
    qDebug()<<originState;
    if (originState == 0) {
        testLog(QString("调整任务栏位置初始值%1").arg( m_locationEnum.valueToKey(1)),false, 1000);
        testSetPanelLocationUnit(m_panelConfig, 1);
    }

    for(int i = 0; i < m_locationEnum.keyCount(); ++i) {
        QString key = m_locationEnum.valueToKey(i);
        testLog(QString("开始测试任务栏位置%1").arg(key),true, 1000);
        testSetPanelLocationUnit(m_panelConfig, i);
        testLog(QString("开始测试任务栏位置%1 over ret:%2").arg(key).arg(lastKey),true, 1000);
        lastKey = i;
    }

    qDebug()<<lastKey << originState;
    if (lastKey!=originState) {
        testLog(QString("还原初始值%1").arg( m_locationEnum.valueToKey(1)),true, 1000);
        testSetPanelLocationUnit(m_panelConfig, originState);
    }
}

void testSetPanelSizeUnit(ConfigUkPanel* panel, const int& size) {
    panel->SetPanelDimensions(size);
    QSignalSpy spy(panel, &ConfigUkPanel::sizeChanged);

    QTRY_VERIFY(spy.count() > 0);
    QCOMPARE(spy[0].at(0).toInt(), size);
}


void PanelConfigTest::testSetPanelSize()
{
    if (m_panelConfig->IsPanelLocked()) {
        testSetLockUnit(m_panelConfig, false);
    }

    int originState =  m_panelConfig->GetPanelDimensions();
    int lastKey;
    qDebug()<<originState;

    if (originState == 0) {
        testLog(QString("adjust size to %1").arg( m_sizeEnum.valueToKey(1)),false, 3000);
        testSetPanelSizeUnit(m_panelConfig, 1);
    }

    for(int i = 0; i < m_sizeEnum.keyCount(); ++i) {
        QString key = m_sizeEnum.valueToKey(i);
        testLog(QString("adjust size: %1").arg(key),true, 1000);
        testSetPanelSizeUnit(m_panelConfig, i);
        testLog(QString("%1 over").arg(key),true, 3000);
        lastKey = i;
    }

    qDebug()<<lastKey << originState;
    if (lastKey!=originState) {
        testLog(QString("%1").arg( m_sizeEnum.valueToKey(1)),true, 3000);
        testSetPanelSizeUnit(m_panelConfig, originState);
    }
}

void testAutoHideSignalUnit(ConfigUkPanel* panel, bool expectedState) {
    testLog(QString("auto Hide %1").arg(expectedState), true, 2000);
    panel->SetAutoHidden(expectedState);

    QSignalSpy spy(panel, &ConfigUkPanel::autoHideChanged);
    QTRY_VERIFY(spy.count() > 0);
    QCOMPARE(expectedState, spy[0].at(0).toBool());
}

void PanelConfigTest::testSetAuotHide()
{
    bool originState =  m_panelConfig->IsAutoHidden();
    qDebug()<<originState;

    testAutoHideSignalUnit(m_panelConfig, !originState);  // 测试状态切换
    testAutoHideSignalUnit(m_panelConfig, originState);   // 测试状态恢复
}


void PanelConfigTest::testSetLock()
{
    bool originState =  m_panelConfig->IsPanelLocked();
    qDebug()<<__FUNCTION__<<originState;

    testSetLockUnit(m_panelConfig, !originState);
    testSetLockUnit(m_panelConfig, originState);
}

void testSetOnAllscreensUnit(ConfigUkPanel* panel, bool expectedState) {
    testLog(QString("panel testSetOnAllscreensUnit %1").arg(expectedState), true, 2000);
    panel->SetShowPanelOnAllScreens(expectedState);

    QSignalSpy spy(panel, &ConfigUkPanel::showOnAllScreenChanged);
    QTRY_VERIFY(spy.count() > 0);
    QCOMPARE(expectedState, spy[0].at(0).toBool());
}

void PanelConfigTest::testSetOnAllscreens()
{
    bool originState =  m_panelConfig->IsShowPanelOnAllScreens();
    qDebug()<<originState;

    testSetOnAllscreensUnit(m_panelConfig, !originState);
    testSetOnAllscreensUnit(m_panelConfig, originState);
}

void testShowSystemTryOnAllPanelsUnit(ConfigUkPanel* panel, bool expectedState) {
    testLog(QString("panel testShowSystemTryOnAllPanelsUnit %1").arg(expectedState), true, 2000);
    panel->SetShowSystemTrayOnAllPanels(expectedState);

    QSignalSpy spy(panel, &ConfigUkPanel::systemTrayOnAllPanels);
    QTRY_VERIFY(spy.count() > 0);
    QCOMPARE(expectedState, spy[0].at(0).toBool());
}

void PanelConfigTest::testsetShowSystemTryOnAllPanels()
{
    if (!m_panelConfig->IsShowPanelOnAllScreens()) {
         testSetOnAllscreensUnit(m_panelConfig, true);
    }
    bool originState =  m_panelConfig->IsShowSystemTrayOnAllPanels();
    qDebug()<<originState;

    testShowSystemTryOnAllPanelsUnit(m_panelConfig, !originState);
    testShowSystemTryOnAllPanelsUnit(m_panelConfig, originState);
}

void testTaskBarIconsShowedOnUnit(ConfigUkPanel* panel, const int& value)
{
    QSignalSpy spy(panel, &ConfigUkPanel::taskBarIconsShowOnChanged);
    panel->SetTaskBarIconDisplayLocation(value);
    QTRY_VERIFY(spy.count() > 0);
    QCOMPARE(spy[0].at(0).toInt(), value);
}

void PanelConfigTest::testsetTaskBarIconsShowedOn()
{
    int originState =  m_panelConfig->GetTaskBarIconDisplayLocation();
    int lastKey;
    qDebug()<<originState;

    if (originState == 0) {
        testLog(QString("adjust size to %1").arg(m_taskBarShowOnEnum.valueToKey(1)),false, 3000);
        testTaskBarIconsShowedOnUnit(m_panelConfig, 1);
    }

    for(int i = 0; i < m_taskBarShowOnEnum.keyCount(); ++i) {
        QString key = m_taskBarShowOnEnum.valueToKey(i);
        testLog(QString("adjust size: %1").arg(key),true, 1000);
        testTaskBarIconsShowedOnUnit(m_panelConfig, i);
        testLog(QString("%1 over").arg(key),true, 3000);
        lastKey = i;
    }

    qDebug()<<lastKey << originState;
    if (lastKey!=originState) {
        testLog(QString("%1").arg(m_taskBarShowOnEnum.valueToKey(1)),true, 3000);
        testTaskBarIconsShowedOnUnit(m_panelConfig, originState);
    }
}

void PanelConfigTest::initEnum()
{
    m_mergeEnum = QMetaEnum::fromType<GeneralConfigDefine::MergeIcons>();
    m_sizeEnum = QMetaEnum::fromType<GeneralConfigDefine::PanelSizePolicy>();
    m_locationEnum = QMetaEnum::fromType<GeneralConfigDefine::PanelLocation>();
    m_taskBarShowOnEnum = QMetaEnum::fromType<GeneralConfigDefine::TaskBarIconsShowedOn>();
}


QTEST_MAIN(PanelConfigTest)

#include "moc_test.cpp" 
