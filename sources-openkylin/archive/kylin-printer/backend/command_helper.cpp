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

#include "command_helper.h"
#include "device_monitor.h"
#include "event_notify_monitor.h"

CommandHelper::CommandHelper(const QString &command)
{
    init(command);
}

CommandHelper::~CommandHelper()
{

}

void CommandHelper::init(const QString &command)
{
    // usbadd_/devices/pci0000:00/0000:00:14.0/usb1/1/6
    // usbremove_/devices/pci0000:00/0000:00:14.0/usb1/1/6

    // addprinter_aaa
    // removeprinter_aaa

    QString message = command;
    if (message.contains("usbadd_")) {
        message.remove("usbadd_");
        DeviceMonitorBackend::getInstance()->usbDeviceAdd(message);
    } else if (message.contains("usbremove_")) {
        message.remove("usbremove_");
        DeviceMonitorBackend::getInstance()->usbDeviceRemove(message);
    } else if (message.contains("addprinter_")) {
        message.remove("addprinter_");
        EventNotifyMonitor::getInstance()->rhPrinterAdded(message);
    } else if (message.contains("removeprinter_")) {
        message.remove("removeprinter_");
        EventNotifyMonitor::getInstance()->rhPrinterRemoved(message);
    }
}
