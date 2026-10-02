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

#ifndef __TEST_MAIN_H__
#define __TEST_MAIN_H__

#include <QDebug>
#include <QObject>
#include <QtTest/QtTest>
#include "StatusManager/statusmanager.h"

class KylinStatusManagerTest : public QObject
{
    Q_OBJECT
public:
    KylinStatusManagerTest();
    ~KylinStatusManagerTest();
private slots:
    void testisAutoRotateEnabled();
    void testgetDeviceOrientation();
    void testisTabletMode();
    void testgetCurrentTabletMode();
    // 这个 Builtin 不能去掉，虚拟键盘已经在使用这个接口了。
    void testisBuiltinKeyboardAvailable();
    void testisAutoSwitchEnabled();
    void testisSupportedAutoRotation();

//    void testAutoRotateEnabled();
//    void testsetAutoSwitchEnabled();
//    void testtesttestsetDeviceOrientation();
//    void testsetTabletMode();
//    void testisTabletModeEnabled();
//    void testtesttestsetTabletModeEnabled();

//    void testget_auto_rotation();
//    void testget_current_rotation();
//    void testget_current_tabletmode();
//    void testget_inputmethod_mode();
//    void testget_modemonitor();
//    void testis_supported_autorotation();
//    void testtesttesttesttestset_auto_rotation();
//    void testset_modemonitor();
//    void testset_rotation();
//    void testset_tabletmode();

//    void testClassicalMode();

//    void testImmersiveMode();

//    void testOSAnimation();
//    void testOSFunction();

private:
    StatusManager *m_pstatusManager;
};

#endif //__TEST_MAIN_H__
