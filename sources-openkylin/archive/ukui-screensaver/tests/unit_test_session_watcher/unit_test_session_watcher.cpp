/*
 * Copyright (C) 2025 KylinSoft Co., Ltd.
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

#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include "../../src/lock-backend/sessionwatcher.h"

#include "stubext.h"
using namespace stub_ext;

class SessionWatcherTest : public testing::Test
{
protected:
    QSharedPointer<GSettingsHelper> m_pGSettingsHelperDbus;
    QSharedPointer<SessionWatcher> m_pSessionWatcherDbus;

    void SetUp() override
    {
        m_pGSettingsHelperDbus = QSharedPointer<GSettingsHelper>(new GSettingsHelper());
        m_pSessionWatcherDbus = QSharedPointer<SessionWatcher>(new SessionWatcher(m_pGSettingsHelperDbus));
    }

    void TearDown() override
    {
        m_pSessionWatcherDbus.reset();
        m_pGSettingsHelperDbus.reset();
    }
};

TEST_F(SessionWatcherTest, GetCurUserName)
{
    m_pSessionWatcherDbus->onStatusChanged(0);
    m_pSessionWatcherDbus->onStatusChanged(1);
    m_pSessionWatcherDbus->onStatusChanged(2);
    m_pSessionWatcherDbus->onStatusChanged(3);
}
