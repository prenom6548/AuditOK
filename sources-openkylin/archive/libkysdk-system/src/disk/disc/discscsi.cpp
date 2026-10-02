/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#include "discscsi.h"

std::string commandString(const unsigned char command)
{
    if (command == MMC_BLANK)
        return "BLANK";
    if (command == MMC_CLOSE_TRACK_SESSION)
        return "CLOSE TRACK/SESSION";
    if (command == MMC_ERASE)
        return "ERASE";
    if (command == MMC_FORMAT_UNIT)
        return "FORMAT UNIT";
    if (command == MMC_GET_CONFIGURATION)
        return "GET CONFIGURATION";
    if (command == MMC_GET_EVENT_STATUS_NOTIFICATION)
        return "GET EVENT STATUS NOTIFICATION";
    if (command == MMC_GET_PERFORMANCE)
        return "GET PERFORMANCE";
    if (command == MMC_INQUIRY)
        return "INQUIRY";
    if (command == MMC_LOAD_UNLOAD_MEDIUM)
        return "LOAD/UNLOAD MEDIUM";
    if (command == MMC_MECHANISM_STATUS)
        return "MECHANISM STATUS";
    if (command == MMC_MODE_SELECT)
        return "MODE SELECT";
    if (command == MMC_MODE_SENSE)
        return "MODE SENSE";
    if (command == MMC_PAUSE_RESUME)
        return "PAUSE/RESUME";
    if (command == MMC_PLAY_AUDIO_10)
        return "PLAY AUDIO (10)";
    if (command == MMC_PLAY_AUDIO_12)
        return "PLAY AUDIO (12)";
    if (command == MMC_PLAY_AUDIO_MSF)
        return "PLAY AUDIO (MSF)";
    if (command == MMC_PREVENT_ALLOW_MEDIUM_REMOVAL)
        return "PREVENT ALLOW MEDIUM REMOVAL";
    if (command == MMC_READ_10)
        return "READ (10)";
    if (command == MMC_READ_12)
        return "READ (12)";
    if (command == MMC_READ_BUFFER)
        return "READ BUFFER";
    if (command == MMC_READ_BUFFER_CAPACITY)
        return "READ BUFFER CAPACITY";
    if (command == MMC_READ_CAPACITY)
        return "READ CAPACITY";
    if (command == MMC_READ_CD)
        return "READ CD";
    if (command == MMC_READ_CD_MSF)
        return "READ CD MSF";
    if (command == MMC_READ_DISC_INFORMATION)
        return "READ DISC INFORMATION";
    if (command == MMC_READ_DVD_STRUCTURE)
        return "READ DVD STRUCTURE";
    if (command == MMC_READ_FORMAT_CAPACITIES)
        return "READ FORMAT CAPACITIES";
    if (command == MMC_READ_SUB_CHANNEL)
        return "READ SUB-CHANNEL";
    if (command == MMC_READ_TOC_PMA_ATIP)
        return "READ TOC/PMA/ATIP";
    if (command == MMC_READ_TRACK_INFORMATION)
        return "READ TRACK INFORMATION";
    if (command == MMC_REPAIR_TRACK)
        return "REPAIR TRACK";
    if (command == MMC_REPORT_KEY)
        return "REPORT KEY";
    if (command == MMC_REQUEST_SENSE)
        return "REQUEST SENSE";
    if (command == MMC_RESERVE_TRACK)
        return "RESERVE TRACK";
    if (command == MMC_SCAN)
        return "SCAN";
    if (command == MMC_SEEK_10)
        return "SEEK (10)";
    if (command == MMC_SEND_CUE_SHEET)
        return "SEND CUE SHEET";
    if (command == MMC_SEND_DVD_STRUCTURE)
        return "SEND DVD STRUCTURE";
    if (command == MMC_SEND_KEY)
        return "SEND KEY";
    if (command == MMC_SEND_OPC_INFORMATION)
        return "SEND OPC INFORMATION";
    if (command == MMC_SET_SPEED)
        return "SET SPEED";
    if (command == MMC_SET_READ_AHEAD)
        return "SET READ AHEAD";
    if (command == MMC_SET_STREAMING)
        return "SET STREAMING";
    if (command == MMC_START_STOP_UNIT)
        return "START STOP UNIT";
    if (command == MMC_STOP_PLAY_SCAN)
        return "STOP PLAY/SCAN";
    if (command == MMC_SYNCHRONIZE_CACHE)
        return "SYNCHRONIZE CACHE";
    if (command == MMC_TEST_UNIT_READY)
        return "TEST UNIT READY";
    if (command == MMC_VERIFY_10)
        return "VERIFY (10)";
    if (command == MMC_WRITE_10)
        return "WRITE (10)";
    if (command == MMC_WRITE_12)
        return "WRITE (12)";
    if (command == MMC_WRITE_AND_VERIFY_10)
        return "WRITE AND VERIFY (10)";
    if (command == MMC_WRITE_BUFFER)
        return "WRITE BUFFER";

    return "unknown";
}

DiscScsi::DiscScsi(std::string device)
{
    mDevice = device;
    initMembers();
}

DiscScsi::~DiscScsi()
{
    if (-1 != mDeviceHandle)
        ::close(mDeviceHandle);
}

unsigned char &DiscScsi::operator[](ulong i)
{
    if (mSgIo.cmd_len < i + 1)
        mSgIo.cmd_len = i + 1;
    return mCmd.cmd[i];
}

int DiscScsi::transport(TransportDirection dir, void *buffer, ulong length)
{
    int ret;

    if (-1 == mDeviceHandle)
    {
        std::cout << "open " << mDevice << " failed...";
        return -1;
    }

    mSgIo.interface_id = 'S';
    mSgIo.mx_sb_len = sizeof(struct request_sense);
    mSgIo.cmdp = mCmd.cmd;
    mSgIo.sbp = (unsigned char *)&mSense;
    mSgIo.flags = SG_FLAG_LUN_INHIBIT | SG_FLAG_DIRECT_IO;
    mSgIo.dxferp = buffer;
    mSgIo.dxfer_len = length;
    mSgIo.timeout = 5000;
    switch (dir)
    {
    case TR_DIR_READ:
        mSgIo.dxfer_direction = SG_DXFER_FROM_DEV;
        break;
    case TR_DIR_WRITE:
        mSgIo.dxfer_direction = SG_DXFER_TO_DEV;
        break;
    default:
        mSgIo.dxfer_direction = SG_DXFER_NONE;
        break;
    }

    ret = ::ioctl(mDeviceHandle, SG_IO, &mSgIo);
    if ((mSgIo.info & SG_INFO_OK_MASK) != SG_INFO_OK)
        ret = -1;

    if (-1 == ret)
    {
        debugError(mCmd.cmd[0],
                   mSense.error_code,
                   mSense.sense_key,
                   mSense.asc,
                   mSense.ascq);

        int errCode =
            (mSense.error_code << 24 & 0xF000) |
            (mSense.sense_key << 16 & 0x0F00) |
            (mSense.asc << 8 & 0x00F0) |
            (mSense.ascq & 0x000F);
        return errCode != 0 ? errCode : 1;
    }

    return 0;
}

void DiscScsi::initMembers()
{
    ::memset(&mSgIo, 0, sizeof(struct sg_io_hdr));
    ::memset(&mSense, 0, sizeof(struct request_sense));
    ::memset(&mCmd, 0, sizeof(struct cdrom_generic_command));

    mCmd.quiet = 1;
    mCmd.sense = &mSense;

    mDeviceHandle = -1;
    if (!mDevice.empty())
        mDeviceHandle = ::open(mDevice.c_str(), O_NONBLOCK | O_RDONLY | O_CLOEXEC);
    if (mDeviceHandle != -1)
    {
        int fdFlags = fcntl(mDeviceHandle, F_GETFD);
        if (fdFlags != -1)
            fcntl(mDeviceHandle, F_SETFD, fdFlags | FD_CLOEXEC);
    }
}

std::string DiscScsi::senseKeyToString(int key)
{
    switch (key)
    {
    case 0x0:
        return "NO SENSE (2)";
    case 0x1:
        return "RECOVERED ERROR (1)";
    case 0x2:
        return "NOT READY (2)";
    case 0x3:
        return "MEDIUM ERROR (3)";
    case 0x4:
        return "HARDWARE ERROR (4)";
    case 0x5:
        return "ILLEGAL REQUEST (5)";
    case 0x6:
        return "UNIT ATTENTION (6)";
    case 0x7:
        return "DATA PROTECT (7)";
    case 0x8:
        return "BLANK CHECK (8)";
    case 0x9:
        return "VENDOR SPECIFIC (9)";
    case 0xA:
        return "COPY ABORTED (A)";
    case 0xB:
        return "ABORTED COMMAND (B)";
    case 0xC:
        return "0xC is obsolete... ??";
    }

    return "unknown";
}

void DiscScsi::debugError(int command, int errorCode, int senseKey, int asc, int ascq)
{
    ::printf("(DiscScsi) failed: \n");
    ::printf("\tcommond:    %s (%x)\n", commandString(command).c_str(), command);
    ::printf("\terror code: %x\n", errorCode);
    ::printf("\tsense key: %s\n", senseKeyToString(senseKey).c_str());
    ::printf("\tasc: %x\n", asc);
    ::printf("\tascq: %x\n", ascq);
}
