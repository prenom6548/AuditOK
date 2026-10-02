/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Ding Jing dingjing@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or (at your option)
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

#include "usbmanage.h"
#include <QDebug>

UsbManage::UsbManage(QObject *parent) : QObject(parent)
{
    qInfo() << "Start listening for USB devices";
    mGvolumeMonitor = g_volume_monitor_get();

    g_signal_connect(mGvolumeMonitor, "volume-added", G_CALLBACK(volumeConnectedCallback), this);
    g_signal_connect(mGvolumeMonitor, "volume-removed", G_CALLBACK(volumeDisconnectedCallback), this);
}

UsbManage::~UsbManage()
{
    qInfo() << "Stop listening for USB devices";
    if (mGvolumeMonitor) {
        g_object_unref(mGvolumeMonitor);
    }
}

void UsbManage::volumeConnectedCallback(GVolumeMonitor *monitor, GVolume *drive, gpointer pThis)
{
    g_return_if_fail(drive);
    g_return_if_fail(pThis);

    UsbManage *dm = static_cast<UsbManage *>(pThis);

    QString volumeName = g_volume_get_name(drive);

    Q_EMIT dm->sigVolumeConnected(volumeName);

    Q_UNUSED(monitor);
}

void UsbManage::volumeDisconnectedCallback(GVolumeMonitor *monitor, GVolume *drive, gpointer pThis)
{
    g_return_if_fail(drive);
    g_return_if_fail(pThis);

    QString volumeName = g_volume_get_name(drive);
    UsbManage *dm = static_cast<UsbManage *>(pThis);

    Q_EMIT dm->sigVolumeDisconnected(volumeName);

    Q_UNUSED(dm);
    Q_UNUSED(monitor);
}
