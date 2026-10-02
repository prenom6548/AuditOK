/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: xibowen <xibowen@kylinos.cn>
 *
 */

#ifndef TEST_CONFIG_CHECKBOX_ANIMATOR_H
#define TEST_CONFIG_CHECKBOX_ANIMATOR_H

#include <QObject>
#include <QCheckBox>
#include <QTest>
#include <QSignalSpy>
#include "config-checkbox-animator.h"

using namespace UKUIConfigStyleSpace;

class TestConfigCheckBoxAnimator : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testDestructor();
    void testBindWidget();
    void testBindWidgetWithDoNotAnimate();
    void testBindWidgetWithNonCheckbox();
    void testUnboundWidget();
    void testUnboundWidgetWithoutBind();
    void testBoundedWidget();
    void testValue();
    void testValueInvalidProperty();
    void testIsRunning();
    void testIsRunningInvalidProperty();
    void testSetAnimatorStartValue();
    void testSetAnimatorStartValueInvalidProperty();
    void testSetAnimatorEndValue();
    void testSetAnimatorEndValueInvalidProperty();
    void testSetAnimatorDuration();
    void testSetAnimatorDurationInvalidProperty();
    void testSetAnimatorDirectionForward();
    void testSetAnimatorDirectionForwardInvalidProperty();
    void testStartAnimator();
    void testStartAnimatorInvalidProperty();
    void testStopAnimator();
    void testStopAnimatorInvalidProperty();
    void testCurrentAnimatorTime();
    void testCurrentAnimatorTimeInvalidProperty();
    void testSetAnimatorCurrentTime();
    void testSetAnimatorCurrentTimeInvalidProperty();
    void testTotalAnimationDuration();
    void testTotalAnimationDurationInvalidProperty();
    void testSetExtraProperty();
    void testGetExtraProperty();

private:
    ConfigCheckBoxAnimator *m_animator = nullptr;
    QCheckBox *m_checkBox = nullptr;
};

#endif // TEST_CONFIG_CHECKBOX_ANIMATOR_H
