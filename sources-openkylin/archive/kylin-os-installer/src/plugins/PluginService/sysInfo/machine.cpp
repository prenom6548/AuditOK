/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "machine.h"

#include <sys/utsname.h>
#include <QString>

namespace KServer {

MachineArch getComputerArch()
{
    struct utsname computr;
    QString computrarch;
    if(uname(&computr) != 0) {
        return MachineArch::UnKnown;
    }else{
        computrarch = computr.machine;
    }
    if(computrarch.startsWith("x86", Qt::CaseInsensitive)) {
        return MachineArch::X86;//x86 i386
    }
    if(computrarch.startsWith("arm", Qt::CaseInsensitive)) {
        return MachineArch::ARM;//arm64
    }
    if(computrarch.startsWith("sw", Qt::CaseInsensitive)) {
        return MachineArch::SW;//
    }
    if(computrarch.startsWith("mips", Qt::CaseInsensitive)) {
        return MachineArch::LOONGSON;//mips64
    }
    return MachineArch::UnKnown;
}

QString getMachineArchName()
{
    switch (getComputerArch()) {
    case MachineArch::ARM:
        return QString("arm");
    case MachineArch::LOONGSON:
        return QString("loongson");
    case MachineArch::SW:
        return QString("sw");
    case MachineArch::X86:
        return QString("x86");
    default:
        return QString("unknown");
    }
}



}
