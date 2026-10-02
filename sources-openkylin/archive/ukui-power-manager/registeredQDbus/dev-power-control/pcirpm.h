/*
 * Copyright 2023 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PCIRPMDEVICE_H
#define PCIRPMDEVICE_H

#include <QMap>
#include "devrpminterface.h"
extern "C"{
#include "pci/pci.h"
}

#define PCI_DEVICE_PATH "/sys/bus/pci/devices/"
#define PCI_BLOCK_DEVICE_PATH "/sys/block/"

#include <QMap>
#include <cstdint>

// PCI BaseClass + SubClass 映射表（QT QMap格式）
static const QMap<uint16_t, QString> PCI_CLASS_MAP = {
    // 0x00 - Unclassified
    {0x0000, "Non-VGA compatible unclassified device"},
    {0x0001, "VGA compatible unclassified device"},

    // 0x01 - Mass Storage Controller
    {0x0100, "SCSI storage controller"},
    {0x0101, "IDE interface"},
    {0x0102, "Floppy disk controller"},
    {0x0103, "IPI controller"},
    {0x0104, "RAID bus controller"},
    {0x0105, "ATA controller"},
    {0x0106, "SATA controller"},
    {0x0107, "SAS controller"},
    {0x0108, "NVMe controller"},
    {0x0180, "Mass storage controller"},

    // 0x02 - Network Controller
    {0x0200, "Ethernet controller"},
    {0x0201, "Token Ring controller"},
    {0x0202, "FDDI controller"},
    {0x0203, "ATM controller"},
    {0x0204, "ISDN controller"},
    {0x0205, "WorldFip controller"},
    {0x0206, "PICMG controller"},
    {0x0207, "Fibre Channel controller"},
    {0x0208, "Asynchronous transfer mode controller"},
    {0x0209, "InfiniBand controller"},
    {0x020A, "Fabric interface controller"},
    {0x0280, "Network controller"},

    // 0x03 - Display Controller
    {0x0300, "VGA compatible controller"},
    {0x0301, "XGA controller"},
    {0x0302, "3D controller"},
    {0x0380, "Display controller"},

    // 0x04 - Multimedia Controller
    {0x0400, "Video controller"},
    {0x0401, "Audio device"},
    {0x0402, "Voice processing controller"},
    {0x0403, "Multimedia communication controller"},
    {0x0480, "Multimedia controller"},

    // 0x05 - Memory Controller
    {0x0500, "RAM controller"},
    {0x0501, "FLASH controller"},
    {0x0580, "Memory controller"},

    // 0x06 - Bridge
    {0x0600, "Host bridge"},
    {0x0601, "ISA bridge"},
    {0x0602, "EISA bridge"},
    {0x0603, "MCA bridge"},
    {0x0604, "PCI bridge"},
    {0x0605, "PCMCIA bridge"},
    {0x0606, "NuBus bridge"},
    {0x0607, "CardBus bridge"},
    {0x0608, "RACEway bridge"},
    {0x0609, "PCI-to-PCI subordinate bridge"},
    {0x060A, "InfiniBand bridge"},
    {0x0680, "Bridge"},

    // 0x07 - Simple Communication Controller
    {0x0700, "Generic serial controller"},
    {0x0701, "Multiport serial controller"},
    {0x0702, "16550 multiport serial controller"},
    {0x0703, "Generic parallel controller"},
    {0x0704, "ECP/EPP parallel controller"},
    {0x0705, "Enhanced serial controller"},
    {0x0780, "Communication controller"},

    // 0x08 - Base System Peripheral
    {0x0800, "Interrupt controller"},
    {0x0801, "DMA controller"},
    {0x0802, "System timer"},
    {0x0803, "RTC controller"},
    {0x0804, "PCI hot-plug controller"},
    {0x0880, "System peripheral"},

    // 0x09 - Input Controller
    {0x0900, "Keyboard controller"},
    {0x0901, "Digitizer controller"},
    {0x0902, "Mouse controller"},
    {0x0903, "Scanner controller"},
    {0x0904, "Gameport controller"},
    {0x0980, "Input controller"},

    // 0x0A - Docking Station
    {0x0A00, "Docking station"},
    {0x0A80, "Other docking station"},

    // 0x0B - Processor
    {0x0B00, "386 processor"},
    {0x0B01, "486 processor"},
    {0x0B02, "Pentium processor"},
    {0x0B03, "Coprocessor"},
    {0x0B80, "Processor"},

    // 0x0C - Serial Bus Controller
    {0x0C00, "FireWire (IEEE1394) controller"},
    {0x0C01, "ACCESS.bus controller"},
    {0x0C02, "SSA controller"},
    {0x0C03, "USB controller"},
    {0x0C04, "Fibre Channel serial controller"},
    {0x0C05, "SMBus"},
    {0x0C06, "I2C controller"},
    {0x0C07, "SPI controller"},
    {0x0C08, "UART controller"},
    {0x0C80, "Serial bus controller"},

    // 0x0D - Wireless Controller
    {0x0D00, "Bluetooth controller"},
    {0x0D01, "NFC controller"},
    {0x0D10, "RFID controller"},
    {0x0D80, "Wireless controller"},

    // 0x0E - Intelligent Controller
    {0x0E00, "Intelligent controller"},

    // 0x0F - Satellite Communication
    {0x0F00, "Satellite communication controller"},
    {0x0F01, "Satellite device"},
    {0x0F02, "Satellite network controller"},
    {0x0F03, "Satellite voice controller"},
    {0x0F04, "Satellite data controller"},
    {0x0F80, "Satellite communication device"},

    // 0x10 - Encryption Controller
    {0x1000, "Network encryption controller"},
    {0x1001, "Encryption processor"},
    {0x1080, "Encryption controller"},

    // 0x11 - Data Acquisition
    {0x1100, "ADC/DAC controller"},
    {0x1101, "Signal processing controller"},
    {0x1102, "Multisensor controller"},
    {0x1180, "Data acquisition controller"},

    // 0xFF - Unassigned
    {0xFF00, "Unassigned class (vendor specific)"}
};

class PciRpmDevice : public QObject, public DevRpmInterface
{
    Q_OBJECT
public:
    explicit PciRpmDevice(QObject *parent = nullptr);

private:
    void getDevicePowerInfo();
    void getAtaDevicePowerInfo(const QString &parentDir);
    void getBlockDevicePowerInfo();
    bool deviceHasRuntimePM(const QString &devPath);
    QString lookupPciDeviceName(const QString &devPath);
    bool ignoreDeviceList(QString deviceName);
    QString getPciClassName(uint16_t classCode) {
        return PCI_CLASS_MAP.value(classCode, "Unknown PCI Class");
    }

public:
    bool setPowerStat(int deviceIndex, const QString &stat);
    bool setAllDevicePowerStat(const QString &stat);
};

#endif // PCIRPMDEVICE_H
