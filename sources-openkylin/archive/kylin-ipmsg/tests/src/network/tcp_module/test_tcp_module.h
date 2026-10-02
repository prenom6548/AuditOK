/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef TEST_TCP_MODULE_H
#define TEST_TCP_MODULE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "network/tcp_module.h"

class TestTcpModule : public ::testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    void testCoverageAdditions();

    TcpModule* m_tcpModule;
};

#endif // TEST_TCP_MODULE_H
