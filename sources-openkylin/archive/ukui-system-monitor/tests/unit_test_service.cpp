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

#include "../src/service/servicedataworker.h"
#include "../src/service/servicemanager.h"
#include "../src/service/serviceinfo.h"

class ServiceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pServiceWorker = new ServiceDataWorker();
    }

    static void TearDownTestSuite()
    {
        delete m_pServiceWorker;
        m_pServiceWorker = nullptr;
    }

    static ServiceDataWorker *m_pServiceWorker;
};

ServiceDataWorker *ServiceTest::m_pServiceWorker = nullptr;

TEST_F(ServiceTest, refreshServiceList)
{
    m_pServiceWorker->refreshServiceList();
}

TEST_F(ServiceTest, ServiceManager)
{
    ServiceManager::instance()->isSysVInitEnabled("apt-p2p");
    ServiceManager::instance()->getServiceStartupMode("test id", "test state");
    ServiceManager::instance()->isServiceNoOp("test state");
    ServiceManager::instance()->isFinalState("test state");
    ServiceManager::instance()->isActiveState("test state");
    ServiceManager::instance()->isServiceAutoStartup("test id", "test state");
    ServiceManager::instance()->updateServiceInfo("/org/freedesktop/systemd1");
    ServiceManager::instance()->normalizeServiceId("test.service@");

    ServiceManager::instance()->startService("test.service");
    ServiceManager::instance()->stopService("test.service");
    ServiceManager::instance()->restartService("test.service");
    ServiceManager::instance()->setServiceStartupMode("test.service", true);
    ServiceManager::instance()->setServiceStartupMode("apt-p2p.service", true);

    ServiceManager::instance()->startService("apt-p2p.service");
    ServiceManager::instance()->stopService("apt-p2p.service");
    ServiceManager::instance()->restartService("apt-p2p.service");
}

TEST_F(ServiceTest, ServiceInfo)
{
    ServiceInfo info1("testname",
                     "testdescription",
                     "testloadState",
                     "testactiveState",
                     "testsubState",
                     "testfollowedBy",
                     "testserviceObjectPath",
                     1,
                     "testjobType",
                     "testjobObjectPath");
    ServiceInfo info2("testname",
                     "testdescription",
                     "testloadState",
                     "testactiveState",
                     "testsubState",
                     "testfollowedBy",
                     "testserviceObjectPath",
                     1,
                     "testjobType",
                     "testjobObjectPath");
    info1 == info2;
    info1.getDescription();
    info1.getState();
    info1.getSubState();
    info1.getFollowedBy();
    info1.getMainPID();
    info1.getJobId();
    info1.getJobType();
    info1.getJobObjectPath();
    info1.getIsCanReload();
    info1.getIsCanStart();
    info1.getIsCanStop();
    info1.getStartupType();

//    ServiceInfoList list;
//    QDBusMessage reply;
//    qvariant_cast<QDBusArgument>(reply.arguments()[0]) << list;
}


