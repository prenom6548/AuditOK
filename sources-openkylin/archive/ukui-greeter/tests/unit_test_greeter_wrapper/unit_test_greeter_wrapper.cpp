/*
 * Copyright (C) 2024 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include <QDebug>
#include "../../ukui-greeter/greeter/greeterwrapper.h"
#include "../../ukui-greeter/common/configuration.h"
#include "../../ukui-greeter/common/globalv.h"

#include "stubext.h"
using namespace stub_ext;

class GreeterWrapperTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        StubExt st;
        st.set_lamda(&GreeterWrapper::connectToDaemonSync,[](){return true;});
        m_pGreeterWrapperDbus = new GreeterWrapper();
    }

    static void TearDownTestSuite()
    {
        delete m_pGreeterWrapperDbus;
        m_pGreeterWrapperDbus = nullptr;
    }

    static GreeterWrapper *m_pGreeterWrapperDbus;
};

GreeterWrapper *GreeterWrapperTest::m_pGreeterWrapperDbus = nullptr;

TEST_F(GreeterWrapperTest, greeterWrapper)
{
    StubExt st;
    st.set_lamda(&GreeterWrapper::connectToDaemonSync,[](){return true;});
    GreeterWrapper::instance()->setLang(getenv("LANG"));
    GreeterWrapper::instance()->lang();
    GreeterWrapper::instance()->session();
    GreeterWrapper::instance()->userName();
    GreeterWrapper::instance()->setUserName(getenv("USER"));
    GreeterWrapper::instance()->userLoggedIn();
//    m_pGreeterWrapperDbus->setrootWindow();
    GreeterWrapper::instance()->getdrawBackgroundIsStarted();
    GreeterWrapper::instance()->getEnsureShareDir(getenv("USER"));
    GreeterWrapper::instance()->closeNM();
    GreeterWrapper::instance()->startSession();
    GreeterWrapper::instance()->showKsplashAnimation();
}
