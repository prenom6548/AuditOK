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
#include "BasicInfoCollection.h"

BasicInfoCollection::BasicInfoCollection(QObject *parent)
    :InfoCollectionBase(parent)
    ,kernelVersionCmd("uname -r 2>/dev/null")
    ,gccVersionCmd("dpkg --status gcc 2>/dev/null|grep 'Version'|awk '{print $2}'")
    ,glibcVersiCmd("dpkg --status glibc 2>/dev/null|grep 'Version'|awk '{print $2}'")
    ,gdbVersionCmd("dpkg --status gdb 2>/dev/null|grep 'Version'|awk '{print $2}'")
    ,qt5VersionCmd("dpkg --status libqt5core5a 2>/dev/null|grep 'Version'|awk '{print $2}'")
    ,qt4VersionCmd("dpkg --status libqt4-dev 2>/dev/null|grep 'Version'|awk '{print $2}'")
    ,python3VersionCmd("dpkg --status python3 2>/dev/null|grep 'Version'|awk '{print $2}'")
    ,python2VersionCmd("dpkg --status python2 2>/dev/null|grep 'Version'|awk '{print $2}'")
    ,javaVersionCmd("java --version 2>/dev/null|head --lines=1")
    ,grubVersionCmd("dpkg --status grub-common  2>/dev/null|grep 'Version'|awk '{print $2}'")
    ,cupsVersionCmd("dpkg --status cups 2>/dev/null|grep 'Version'|awk '{print $2}'")
    ,libjbigVersionCmd("dpkg --status libjbig-dev 2>/dev/null|grep 'Version'|awk '{print $2}'")
    ,libpngVersionCmd("dpkg --status libpng-dev 2>/dev/null|grep 'Version'|awk '{print $2}'")
    ,libjpegCmd("dpkg --status libjpeg-dev 2>/dev/null|grep 'Version'|awk '{print $2}'")
    ,kylinDaqVersionCmd("dpkg --status kylin-daq 2>/dev/null|grep 'Version'|awk '{print $2}'")
{

}

void BasicInfoCollection::getInfo(BasicInfo &basicInfo)
{
    getOnePopenResult(kernelVersionCmd,basicInfo.kernelVersion);
    getOnePopenResult(gccVersionCmd,basicInfo.gccVersion);
    getOnePopenResult(glibcVersiCmd,basicInfo.glibcVersi);
    getOnePopenResult(gdbVersionCmd,basicInfo.gdbVersion);
    getOnePopenResult(qt5VersionCmd,basicInfo.qt5Version);
    getOnePopenResult(qt4VersionCmd,basicInfo.qt4Version);
    getOnePopenResult(python3VersionCmd,basicInfo.python3Version);
    getOnePopenResult(python2VersionCmd,basicInfo.python2Version);
    getOnePopenResult(javaVersionCmd,basicInfo.javaVersion);
    getOnePopenResult(grubVersionCmd,basicInfo.grubVersion);
    getOnePopenResult(cupsVersionCmd,basicInfo.cupsVersion);
    getOnePopenResult(libjbigVersionCmd,basicInfo.libjbigVersion);
    getOnePopenResult(libpngVersionCmd,basicInfo.libpngVersion);
    getOnePopenResult(libjpegCmd,basicInfo.libjpeg);
    getOnePopenResult(kylinDaqVersionCmd,basicInfo.kylinDaqVersion);
}
