/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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

#include "../../registeredSession/plugins/vino/vinointerface.h"

class VinoInterfaceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pVinoDbus = new VinoInterface();
    }

    static void TearDownTestSuite()
    {
        delete m_pVinoDbus;
        m_pVinoDbus = nullptr;
    }

    static VinoInterface *m_pVinoDbus;
};

VinoInterface *VinoInterfaceTest::m_pVinoDbus = nullptr;

TEST_F(VinoInterfaceTest, viewOnly)
{
    // get current value
    bool repeatKeyValue = m_pVinoDbus->getViewOnly();

    // call setKeyRepeat function to set different value
    m_pVinoDbus->setVinoKey(m_pVinoDbus->kVinoViewOnlyKey, !repeatKeyValue);

    // test the current value and the value you have set
    ASSERT_EQ(!repeatKeyValue, m_pVinoDbus->getViewOnly());

    // restore the value before test
    m_pVinoDbus->setVinoKey(m_pVinoDbus->kVinoViewOnlyKey, repeatKeyValue);
}

TEST_F(VinoInterfaceTest, noViewOnly)
{
    // get current value
    QString originKey = m_pVinoDbus->kVinoViewOnlyKey;
    m_pVinoDbus->mVinoGsettings = nullptr;

    bool repeatKeyValue = m_pVinoDbus->getViewOnly();

    // call setKeyRepeat function to set different value
    m_pVinoDbus->setVinoKey(m_pVinoDbus->kVinoViewOnlyKey, !repeatKeyValue);

    // test the current value and the value you have set
    ASSERT_EQ(!repeatKeyValue, m_pVinoDbus->getViewOnly());

    // restore the value before test
    m_pVinoDbus->kVinoViewOnlyKey = originKey;
}

TEST_F(VinoInterfaceTest, PromptEnabled)
{
    bool promptKeyValue = m_pVinoDbus->getPromptEnabled();

    m_pVinoDbus->setVinoKey(m_pVinoDbus->kVinoPromptKey, !promptKeyValue);

    ASSERT_EQ(promptKeyValue, m_pVinoDbus->getPromptEnabled());

    m_pVinoDbus->setVinoKey(m_pVinoDbus->kVinoPromptKey, promptKeyValue);
}

TEST_F(VinoInterfaceTest, noPromptEnabled)
{
    // get current value
    QString originKey = m_pVinoDbus->kVinoPromptKey;
    m_pVinoDbus->mVinoGsettings = nullptr;

    bool promptKeyValue = m_pVinoDbus->getPromptEnabled();

    m_pVinoDbus->setVinoKey(m_pVinoDbus->kVinoPromptKey, !promptKeyValue);

    ASSERT_EQ(promptKeyValue, m_pVinoDbus->getPromptEnabled());

    // restore the value before test
    m_pVinoDbus->kVinoPromptKey = originKey;
}

TEST_F(VinoInterfaceTest, Method)
{
    QString methodKeyValue = m_pVinoDbus->getMethod();

    m_pVinoDbus->setVinoKey(m_pVinoDbus->kAuthenticationKey, methodKeyValue);

    ASSERT_EQ(methodKeyValue, m_pVinoDbus->getMethod());

    m_pVinoDbus->setVinoKey(m_pVinoDbus->kAuthenticationKey, methodKeyValue);
}

TEST_F(VinoInterfaceTest, noMethod)
{
    // get current value
    QString originKey = m_pVinoDbus->kAuthenticationKey;
    m_pVinoDbus->mVinoGsettings = nullptr;

    QString methodKeyValue = m_pVinoDbus->getMethod();

    m_pVinoDbus->setVinoKey(m_pVinoDbus->kAuthenticationKey, methodKeyValue);

    ASSERT_EQ(methodKeyValue, m_pVinoDbus->getMethod());

    // restore the value before test
    m_pVinoDbus->kAuthenticationKey = originKey;
}


TEST_F(VinoInterfaceTest, Password)
{
    QString passwordKeyValue = m_pVinoDbus->getPassword();

    m_pVinoDbus->setVinoKey(m_pVinoDbus->kVncPwdKey, passwordKeyValue);

    ASSERT_EQ(passwordKeyValue, m_pVinoDbus->getPassword());

    m_pVinoDbus->setVinoKey(m_pVinoDbus->kVncPwdKey, passwordKeyValue);
}

TEST_F(VinoInterfaceTest, noPassword)
{
    // get current value
    QString originKey = m_pVinoDbus->kVncPwdKey;
    m_pVinoDbus->mVinoGsettings = nullptr;

    QString passwordKeyValue = m_pVinoDbus->getPassword();

    m_pVinoDbus->setVinoKey(m_pVinoDbus->kVncPwdKey, passwordKeyValue);

    ASSERT_EQ(passwordKeyValue, m_pVinoDbus->getPassword());

    // restore the value before test
    m_pVinoDbus->kVncPwdKey = originKey;
}

TEST_F(VinoInterfaceTest, VinoServerStatus)
{
    bool currentValue = m_pVinoDbus->getVinoServerStatus();
}

TEST_F(VinoInterfaceTest, VinoStatus)
{
    bool currentValue = m_pVinoDbus->getVinoStatus();

    m_pVinoDbus->setVinoKey(m_pVinoDbus->kVinoEnableKey, !currentValue);

    ASSERT_EQ(!currentValue, m_pVinoDbus->getVinoStatus());

    m_pVinoDbus->setVinoKey(m_pVinoDbus->kVinoEnableKey, currentValue);
}

TEST_F(VinoInterfaceTest, noVinoStatus)
{
    // get current value
    QString originKey = m_pVinoDbus->kVinoEnableKey;
    m_pVinoDbus->mVinoGsettings = nullptr;

    bool currentValue = m_pVinoDbus->getVinoStatus();

    m_pVinoDbus->setVinoKey(m_pVinoDbus->kVinoEnableKey, !currentValue);

    ASSERT_EQ(!currentValue, m_pVinoDbus->getVinoStatus());

    // restore the value before test
    m_pVinoDbus->kVinoEnableKey = originKey;
}

TEST_F(VinoInterfaceTest, XrdpStatus)
{
    bool currentValue = m_pVinoDbus->getXrdpStatus();

    m_pVinoDbus->setVinoKey(m_pVinoDbus->kXrdpEnableKey, !currentValue);

    ASSERT_EQ(!currentValue, m_pVinoDbus->getXrdpStatus());

    m_pVinoDbus->setVinoKey(m_pVinoDbus->kXrdpEnableKey, currentValue);
}

TEST_F(VinoInterfaceTest, noXrdpStatus)
{
    // get current value
    QString originKey = m_pVinoDbus->kXrdpEnableKey;
    m_pVinoDbus->mVinoGsettings = nullptr;

    int currentValue = m_pVinoDbus->getXrdpStatus();

    m_pVinoDbus->setVinoKey(m_pVinoDbus->kXrdpEnableKey, currentValue);

    // restore the value before test
    m_pVinoDbus->kXrdpEnableKey = originKey;
}

TEST_F(VinoInterfaceTest, noEnableStatus)
{
    bool currentValue = m_pVinoDbus->getEnableStatus();

    m_pVinoDbus->setVinoKey(m_pVinoDbus->kEnableKey, !currentValue);

    ASSERT_EQ(currentValue, m_pVinoDbus->getEnableStatus());

    m_pVinoDbus->setVinoKey(m_pVinoDbus->kEnableKey, currentValue);
}

TEST_F(VinoInterfaceTest, EnableStatus)
{
    // get current value
    QString originKey = m_pVinoDbus->kEnableKey;
    m_pVinoDbus->mVinoGsettings = nullptr;

    bool currentValue = m_pVinoDbus->getEnableStatus();

    m_pVinoDbus->setVinoKey(m_pVinoDbus->kEnableKey, !currentValue);

    ASSERT_EQ(currentValue, m_pVinoDbus->getEnableStatus());

    // restore the value before test
    m_pVinoDbus->kEnableKey = originKey;
}
