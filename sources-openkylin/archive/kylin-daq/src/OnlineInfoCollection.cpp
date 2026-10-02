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
#include "OnlineInfoCollection.h"

OnlineInfoCollection::OnlineInfoCollection(QObject *parent)
    :InfoCollectionBase(parent)
    , bootDateTimeCmd("date -d \"$(awk -F. '{print $1}' /proc/uptime) second ago\" +\"%Y-%m-%d %H:%M:%S\"")
    , shutdownDateTimeCmd("date -d \"$(last -x --time-format iso shutdown|head -1|awk '{printf $5}')\" +\"%Y-%m-%d %H:%M:%S\"")
    , penultimateBootDateTimeCmd("date -d \"$(last --time-format iso |grep down|head -1|awk '{printf $3}')\" +\"%Y-%m-%d %H:%M:%S\"")
{


}
void OnlineInfoCollection::getInfo(OnlineInfo &onlineInfo)
{
    QString bootDateTimeStr;
    QString shutdownDateTimeStr;
    QString penultimateBootDateTimeStr;
    getOnePopenResult(bootDateTimeCmd,bootDateTimeStr);
    getOnePopenResult(shutdownDateTimeCmd,shutdownDateTimeStr);
    getOnePopenResult(penultimateBootDateTimeCmd,penultimateBootDateTimeStr);
    QDateTime bootDateTime = QDateTime::fromString(bootDateTimeStr,KylinDaqCommon::gs_timeFormat2);
    QDateTime shutdownDateTime= QDateTime::fromString(shutdownDateTimeStr,KylinDaqCommon::gs_timeFormat2);
    QDateTime penultimateBootDateTime= QDateTime::fromString(penultimateBootDateTimeStr,KylinDaqCommon::gs_timeFormat2);
    if (bootDateTime.isValid())
    {
        onlineInfo.bootDateTime = bootDateTime.toString(KylinDaqCommon::gs_timeFormat);
    }
    if (shutdownDateTime.isValid())
    {
        onlineInfo.shutdownDateTime = shutdownDateTime.toString(KylinDaqCommon::gs_timeFormat);
    }
    if (shutdownDateTime.isValid() && penultimateBootDateTime.isValid())
    {
        onlineInfo.lastRunDateTimeInterval = QString("%1 - %2")
            .arg(shutdownDateTime.toString(KylinDaqCommon::gs_timeFormat))
            .arg(penultimateBootDateTime.toString(KylinDaqCommon::gs_timeFormat));
        onlineInfo.lastRunDateTimeQuantum = penultimateBootDateTime.msecsTo(shutdownDateTime);
    }
    if (bootDateTime.isValid() && shutdownDateTime.isValid() )
    {
        onlineInfo.lastShutdownDateTimeInterval = QString("%1 - %2")
            .arg(bootDateTime.toString(KylinDaqCommon::gs_timeFormat))
            .arg(shutdownDateTime.toString(KylinDaqCommon::gs_timeFormat));
        onlineInfo.lastShutdownDateTimeQuantum = shutdownDateTime.msecsTo(bootDateTime);
    }
}
