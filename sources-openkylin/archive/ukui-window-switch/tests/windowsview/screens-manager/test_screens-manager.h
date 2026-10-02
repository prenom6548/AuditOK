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
 *
 *
 */

#ifndef TEST_SCREENS_MANAGER_H
#define TEST_SCREENS_MANAGER_H

#include <QObject>

class TestScreensManager : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testScreenProperties();
    void testManagerInitializesPrimaryScreen();
    void testEnableChangedSignals();
    void testPrimaryScreenChangedEnablesTarget();
    void testScreenRemovedSignalAndListUpdate();
    void testInternalGeometryChangeTriggersRefresh();
    void testCheckScreensGeometryStatusReenablesScreen();
    void testCheckScreensGeometryStatusDisablesDuplicate();
    void testOnScreenAddedTracksPrimaryScreen();
    void testPrimaryScreen();
    void testDestructor();
    void testEnhancedBranchCoverage();
};

#endif // TEST_SCREENS_MANAGER_H
