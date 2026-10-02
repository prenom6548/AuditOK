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

#include "test_config-scrollbar-animation-helper.h"

#include "../../../../../ukui-styles/qt5-config-style-ukui/animations/config-scrollbar-animation-helper.h"
#include "../../../../../libqt5-ukui-style/animations/animator-iface.h"

#include <QScrollBar>
#include <QWidget>
#include <QDebug>

TestConfigScrollBarAnimationHelper::TestConfigScrollBarAnimationHelper(QObject *parent)
    : QObject(parent)
    , m_testWidget(nullptr)
{
}

TestConfigScrollBarAnimationHelper::~TestConfigScrollBarAnimationHelper()
{
}

void TestConfigScrollBarAnimationHelper::initTestCase()
{
}

void TestConfigScrollBarAnimationHelper::cleanupTestCase()
{
}

void TestConfigScrollBarAnimationHelper::init()
{
    m_testWidget = new QScrollBar();
}

void TestConfigScrollBarAnimationHelper::cleanup()
{
    if (m_testWidget)
    {
        delete m_testWidget;
        m_testWidget = nullptr;
    }
}

void TestConfigScrollBarAnimationHelper::testConstructor()
{
    UKUIConfigStyleSpace::ConfigScrollBarAnimationHelper *helper = new UKUIConfigStyleSpace::ConfigScrollBarAnimationHelper();
    QVERIFY(helper != nullptr);
    QVERIFY(helper->parent() == nullptr);
    delete helper;

    UKUIConfigStyleSpace::ConfigScrollBarAnimationHelper *helper2 = new UKUIConfigStyleSpace::ConfigScrollBarAnimationHelper(this);
    QVERIFY(helper2 != nullptr);
    QVERIFY(helper2->parent() == this);
    delete helper2;
}

void TestConfigScrollBarAnimationHelper::testDestructor()
{
    UKUIConfigStyleSpace::ConfigScrollBarAnimationHelper *helper = new UKUIConfigStyleSpace::ConfigScrollBarAnimationHelper();
    delete helper;
    QVERIFY(true);
}

void TestConfigScrollBarAnimationHelper::testRegisterWidget()
{
    UKUIConfigStyleSpace::ConfigScrollBarAnimationHelper *helper = new UKUIConfigStyleSpace::ConfigScrollBarAnimationHelper();
    bool result = helper->registerWidget(m_testWidget);
    QVERIFY(result == true);
    delete helper;
}

void TestConfigScrollBarAnimationHelper::testRegisterWidgetNull()
{
    UKUIConfigStyleSpace::ConfigScrollBarAnimationHelper *helper = new UKUIConfigStyleSpace::ConfigScrollBarAnimationHelper();
    bool result = helper->registerWidget(nullptr);
    QVERIFY(result == false);
    delete helper;
}

void TestConfigScrollBarAnimationHelper::testUnregisterWidget()
{
    UKUIConfigStyleSpace::ConfigScrollBarAnimationHelper *helper = new UKUIConfigStyleSpace::ConfigScrollBarAnimationHelper();
    bool registerResult = helper->registerWidget(m_testWidget);
    QVERIFY(registerResult == true);

    bool unregisterResult = helper->unregisterWidget(m_testWidget);
    QVERIFY(unregisterResult == true);
    delete helper;
}

void TestConfigScrollBarAnimationHelper::testUnregisterWidgetNotRegistered()
{
    UKUIConfigStyleSpace::ConfigScrollBarAnimationHelper *helper = new UKUIConfigStyleSpace::ConfigScrollBarAnimationHelper();
    QWidget *anotherWidget = new QWidget();
    bool unregisterResult = helper->unregisterWidget(anotherWidget);
    QVERIFY(unregisterResult == false);
    delete anotherWidget;
    delete helper;
}

void TestConfigScrollBarAnimationHelper::testAnimator()
{
    UKUIConfigStyleSpace::ConfigScrollBarAnimationHelper *helper = new UKUIConfigStyleSpace::ConfigScrollBarAnimationHelper();
    bool registerResult = helper->registerWidget(m_testWidget);
    QVERIFY(registerResult == true);

    AnimatorIface *animator = helper->animator(m_testWidget);
    QVERIFY(animator != nullptr);

    delete helper;
}

void TestConfigScrollBarAnimationHelper::testAnimatorNotRegistered()
{
    UKUIConfigStyleSpace::ConfigScrollBarAnimationHelper *helper = new UKUIConfigStyleSpace::ConfigScrollBarAnimationHelper();
    AnimatorIface *animator = helper->animator(m_testWidget);
    QVERIFY(animator == nullptr);
    delete helper;
}

#include "test_config-scrollbar-animation-helper.moc"
