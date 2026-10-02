/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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

#include "main.h"

/*
curl --insecure -X POST -F "file=@result.zip" -F "package=kylin-status-manager" -F "username=sunjunhao" https://dashboard-api.kylin.com/kylin_dashboard/upload/upload_coverage_result
*/

KylinStatusManagerTest::KylinStatusManagerTest()
{
    m_pstatusManager = new StatusManager();
}

KylinStatusManagerTest::~KylinStatusManagerTest()
{
    delete m_pstatusManager;
    m_pstatusManager = nullptr;
}

void KylinStatusManagerTest::testisAutoRotateEnabled()
{
    if (false == m_pstatusManager->isOrientationSensorAvailable()) {
        QCOMPARE(0, 0);
    }

    m_pstatusManager->setAutoRotateEnabled(true, false);
}

void KylinStatusManagerTest::testgetDeviceOrientation()
{

}

void KylinStatusManagerTest::testisTabletMode()
{
    bool originStatus =  m_pstatusManager->isTabletModeEnabled();
    m_pstatusManager->setTabletModeEnabled(true);
    QCOMPARE(true, m_pstatusManager->isTabletModeEnabled());

    m_pstatusManager->setTabletModeEnabled(false);
    QCOMPARE(false, m_pstatusManager->isTabletModeEnabled());

    m_pstatusManager->setTabletModeEnabled(originStatus);
}

void KylinStatusManagerTest::testgetCurrentTabletMode()
{
    bool originEnableStatus =  m_pstatusManager->isTabletModeEnabled();
    bool originStatus =  m_pstatusManager->isTabletMode();

    m_pstatusManager->setTabletModeEnabled(true);
    m_pstatusManager->setTabletMode(true);
    QCOMPARE(true, m_pstatusManager->isTabletMode());

    m_pstatusManager->setTabletMode(false);
    QCOMPARE(false, m_pstatusManager->isTabletMode());

    m_pstatusManager->setTabletModeEnabled(originEnableStatus);
    m_pstatusManager->setTabletMode(originStatus);
}

void KylinStatusManagerTest::testisBuiltinKeyboardAvailable()
{

}

void KylinStatusManagerTest::testisAutoSwitchEnabled()
{
    bool originEnableStatus =  m_pstatusManager->isAutoSwitchEnabled();

    m_pstatusManager->setAutoSwitchEnabled(true);
    QCOMPARE(true, m_pstatusManager->isAutoSwitchEnabled());

    m_pstatusManager->setAutoSwitchEnabled(false);
    QCOMPARE(false, m_pstatusManager->isAutoSwitchEnabled());

    m_pstatusManager->setAutoSwitchEnabled(originEnableStatus);

}

void KylinStatusManagerTest::testisSupportedAutoRotation()
{
    bool originEnableStatus = m_pstatusManager->isAutoRotateEnabled();

    m_pstatusManager->setAutoRotateEnabled(false, true);
    QCOMPARE(false, m_pstatusManager->isAutoRotateEnabled());

    m_pstatusManager->setAutoRotateEnabled(true, true);
    QCOMPARE(true, m_pstatusManager->isAutoRotateEnabled());

    m_pstatusManager->setAutoRotateEnabled(originEnableStatus, true);
}

QTEST_MAIN(KylinStatusManagerTest)
