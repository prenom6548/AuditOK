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
 */
#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include "../src/util.h"

class utilsTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
    }

    static void TearDownTestSuite()
    {
    }
};

TEST_F(utilsTest, make_string)
{
    char *string = "test string";
    make_string(g_strdup(string));
}

TEST_F(utilsTest, formatProcessState)
{
    formatProcessState("R");
    formatProcessState("T");
    formatProcessState("Z");
    formatProcessState("D");
    formatProcessState("S");
}

TEST_F(utilsTest, getNiceLevel)
{
    getNiceLevel(-8);
    getNiceLevel(-3);
    getNiceLevel(2);
    getNiceLevel(6);
    getNiceLevel(8);
}

TEST_F(utilsTest, getNiceLevelWithPriority)
{
    getNiceLevelWithPriority(-8);
    getNiceLevelWithPriority(-3);
    getNiceLevelWithPriority(2);
    getNiceLevelWithPriority(6);
    getNiceLevelWithPriority(8);
}

TEST_F(utilsTest, formatDurationForDisplay)
{
    formatDurationForDisplay(0);
    formatDurationForDisplay(22);
    formatDurationForDisplay(5);

    formatDurationForDisplay(20241219);
}

TEST_F(utilsTest, getDeviceMountedPointPath)
{
    getDeviceMountedPointPath("/dev/test");
}

TEST_F(utilsTest, getFileContentsLineByLine)
{
    getFileContentsLineByLine("/proc/mounts");
}

TEST_F(utilsTest, getUsmVersion)
{
    getUsmVersion();
}
