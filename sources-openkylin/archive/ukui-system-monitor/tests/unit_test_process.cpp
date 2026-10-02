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

#include "../src/process/process_monitor.h"
using namespace sysmonitor::process;
#include "../src/process/process_list.h"

class ProcessTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
    }

    static void TearDownTestSuite()
    {
    }
};

TEST_F(ProcessTest, processListTest)
{
    auto *monitor = ProcessMonitor::instance();
    monitor->startMonitorJob();
    monitor->onChangeRefreshFilter(PROCESSTYPE_ALL);

    getProcessPriorityStub(20);

    monitor->processList()->getProcessById(1);
    monitor->processList()->getProcessByAllSetId(1);
    monitor->processList()->containsById(1);

    qreal mergeBps = 0;
    monitor->processList()->mergeSubProcNetIO(2334, mergeBps);

    qreal ptotalCpu = 0.;
    monitor->processList()->mergeSubProcCpu(2334, ptotalCpu);

    qreal mergeDiskIO = 0;
    qreal mergePreCount = 0;
    monitor->processList()->mergeSubProcDiskIO(2334, mergeDiskIO, mergePreCount);

    qreal mergeMemery = 0;
    monitor->processList()->mergeSubProcMemery(2334, mergeMemery);

    monitor->processList()->getPIDList();

    monitor->updateProcMonitorInfo();

}

TEST_F(ProcessTest, processDataTest)
{
    const ProcessData data1;
    ProcessData data2(data1);
}

