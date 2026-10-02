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

#include "../src/filesystem/filesystemworker.h"
#include "../src/filesystem/datacdrom.h"

#include "kt-test-utils/cpp-stub-ext/stubext.h"
using namespace stub_ext;

class FilesystemTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        m_pFilesystemWorker = new FileSystemWorker();
    }

    static void TearDownTestSuite()
    {
        delete m_pFilesystemWorker;
        m_pFilesystemWorker = nullptr;
    }

    static FileSystemWorker *m_pFilesystemWorker;
};

FileSystemWorker *FilesystemTest::m_pFilesystemWorker = nullptr;

TEST_F(FilesystemTest, FileSystemWorker)
{
    m_pFilesystemWorker->onFileSystemListChanged();

    QList<QString> filesytemList = m_pFilesystemWorker->diskDevNameList();

    FileSystemData fsData;
    m_pFilesystemWorker->getDiskInfo(filesytemList.count() > 0 ? filesytemList.at(0): "/dev/test", fsData);

    m_pFilesystemWorker->getDiskInfo("/dev/test", fsData);

    m_pFilesystemWorker->updateDiskInfo(filesytemList.count() > 0 ? filesytemList.at(0):"/dev/test", fsData);

    m_pFilesystemWorker->isDeviceContains("/dev/test");

    m_pFilesystemWorker->addDiskInfo("/dev/test", fsData);

    m_pFilesystemWorker->diskInfoList();
}

TEST_F(FilesystemTest, FileSystemData)
{
    FileSystemData fsData;

    m_pFilesystemWorker->onFileSystemListChanged();

    fsData.setDevName("/dev/test");

    fsData.deviceName();

    fsData.mountDir();
    fsData.diskType();
    fsData.totalCapacity();
    fsData.freeCapacity();
    fsData.availCapacity();
    fsData.usedCapactiy();
    fsData.usedPercentage();
    fsData.totalCapacityValue();
    fsData.freeCapacityValue();
    fsData.avalidCapacityValue();
    fsData.usedCapacityValue();
}

TEST_F(FilesystemTest, DataCDROM)
{
    QString strDevId = "test";
    DataCDROM cdrom(strDevId);

    StubExt st;
    st.set_lamda(&DataCDROM::open,[](){return true;});
    st.set_lamda(&DataCDROM::execSCSI,[](){return true;});

    cdrom.getCDROMInfo();

    cdrom.getCDROMCapacity();

    cdrom.getCDROMUsedCapacity();

    cdrom.checkRWSupport();

    cdrom.checkMediumType();

    cdrom.cdRomGetTrackNum();

    cdrom.DVDRWCapacity();

    cdrom.cdRomCapacity();

}

