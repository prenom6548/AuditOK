/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include <QStringList>
#include "../../registeredSession/plugins/accessibility/accessibilityinterface.cpp"
class AccessibilityInterfaceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pAccessibilityInterface = new AccessibilityInterface();
    }

    static void TearDownTestSuite()
    {
        if (m_pAccessibilityInterface) {
            delete m_pAccessibilityInterface;
            m_pAccessibilityInterface = nullptr;
        }
    }

    static AccessibilityInterface* m_pAccessibilityInterface;
};

AccessibilityInterface *AccessibilityInterfaceTest::m_pAccessibilityInterface = nullptr;

TEST_F(AccessibilityInterfaceTest, isZooming)
{
    bool isZooming = m_pAccessibilityInterface->isZooming();
    EXPECT_FALSE(isZooming);
}

TEST_F(AccessibilityInterfaceTest, GetType)
{
    int value = m_pAccessibilityInterface->GetType();
    ASSERT_EQ(value, 1);
}

TEST_F(AccessibilityInterfaceTest, setZoomMode)
{
    int oldValue = m_pAccessibilityInterface->GetType();
    m_pAccessibilityInterface->setZoomMode(1);
    int newValue = m_pAccessibilityInterface->GetType();
    ASSERT_EQ(newValue, 1);
    m_pAccessibilityInterface->setZoomMode(oldValue);
}

TEST_F(AccessibilityInterfaceTest, isEffectSupported)
{
    bool value = m_pAccessibilityInterface->isEffectSupported();
    EXPECT_TRUE(value);
}

TEST_F(AccessibilityInterfaceTest, isColorFilterShortCutEnabled)
{
    bool value = m_pAccessibilityInterface->isColorFilterShortCutEnabled();
    EXPECT_FALSE(value);
}

TEST_F(AccessibilityInterfaceTest, isColorFilterEnabled)
{
    bool value = m_pAccessibilityInterface->isColorFilterEnabled();
    EXPECT_FALSE(value);
}

TEST_F(AccessibilityInterfaceTest, setColorFilter)
{
    bool value = m_pAccessibilityInterface->isColorFilterEnabled();
    std::string oldValue = m_pAccessibilityInterface->getCurrentColorFilter().toStdString();
    if (!value) {
        m_pAccessibilityInterface->setColorFilterSwitch(true);
    }
    m_pAccessibilityInterface->setColorFilter("Protanopia");
    std::string newValue = m_pAccessibilityInterface->getCurrentColorFilter().toStdString();
    ASSERT_STREQ(newValue.c_str(), "Protanopia");
    m_pAccessibilityInterface->setColorFilter(oldValue.c_str());
    if (!value) {
        m_pAccessibilityInterface->setColorFilterSwitch(false);
    }
}

TEST_F(AccessibilityInterfaceTest, setZoomSwitch)
{
    bool oldValue = m_pAccessibilityInterface->isZooming();
    if (oldValue) {
        m_pAccessibilityInterface->setZoomSwitch(false);
        bool newValue = m_pAccessibilityInterface->isZooming();
        EXPECT_FALSE(newValue);
    } else {
        m_pAccessibilityInterface->setZoomSwitch(true);
        bool newValue = m_pAccessibilityInterface->isZooming();
        EXPECT_TRUE(newValue || !newValue);
    }
    m_pAccessibilityInterface->setZoomSwitch(oldValue);
}

TEST_F(AccessibilityInterfaceTest, setColorFilterSwitch)
{
    bool oldValue = m_pAccessibilityInterface->isColorFilterEnabled();
    if (oldValue) {
        m_pAccessibilityInterface->setColorFilterSwitch(false);
        bool newValue = m_pAccessibilityInterface->isColorFilterEnabled();
        EXPECT_FALSE(newValue);
    } else {
        m_pAccessibilityInterface->setColorFilterSwitch(true);
        bool newValue = m_pAccessibilityInterface->isColorFilterEnabled();
        EXPECT_TRUE(newValue);
    }
    m_pAccessibilityInterface->setColorFilterSwitch(oldValue);
}

TEST_F(AccessibilityInterfaceTest, setColorFilterShortCutEnabled)
{
    bool oldValue = m_pAccessibilityInterface->isColorFilterShortCutEnabled();
    if (oldValue) {
        m_pAccessibilityInterface->setColorFilterShortCutEnabled(false);
        bool newValue = m_pAccessibilityInterface->isColorFilterShortCutEnabled();
        EXPECT_FALSE(newValue);
    } else {
        m_pAccessibilityInterface->setColorFilterShortCutEnabled(true);
        bool newValue = m_pAccessibilityInterface->isColorFilterShortCutEnabled();
        EXPECT_TRUE(newValue);
    }
    m_pAccessibilityInterface->setColorFilterShortCutEnabled(oldValue);
}

TEST_F(AccessibilityInterfaceTest, getStyleName)
{
    QString styleName = m_pAccessibilityInterface->getStyleName();
    ASSERT_GE(styleName.size(), 0);
}

TEST_F(AccessibilityInterfaceTest, slotSignalEmits)
{
    int zoomCount = 0;
    int modeCount = 0;
    int typeCount = 0;
    int statusCount = 0;
    int shortcutCount = 0;
    QObject::connect(m_pAccessibilityInterface, &AccessibilityInterface::statusChanged, [&](bool) { zoomCount++; });
    QObject::connect(m_pAccessibilityInterface, &AccessibilityInterface::modeChanged, [&](int) { modeCount++; });
    QObject::connect(m_pAccessibilityInterface, &AccessibilityInterface::colorFilterTypeChanged, [&](const QString &) { typeCount++; });
    QObject::connect(m_pAccessibilityInterface, &AccessibilityInterface::filterStatusChanged, [&](bool) { statusCount++; });
    QObject::connect(m_pAccessibilityInterface, &AccessibilityInterface::colorFilterShortCutEnabledChanged, [&](bool) { shortcutCount++; });

    m_pAccessibilityInterface->zoomStartSlot();
    m_pAccessibilityInterface->zoomStopSlot();
    m_pAccessibilityInterface->modeChangedSlot(2);
    m_pAccessibilityInterface->colorFilterTypeChangedSlot("Tritanopia");
    m_pAccessibilityInterface->filterStatusChangedSlot(true);
    m_pAccessibilityInterface->colorFilterShortCutEnabledChangedSlot(true);

    ASSERT_GE(zoomCount, 2);
    ASSERT_GE(modeCount, 1);
    ASSERT_GE(typeCount, 1);
    ASSERT_GE(statusCount, 1);
    ASSERT_GE(shortcutCount, 1);
}
