/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#ifndef HARDWAREREADER_H
#define HARDWAREREADER_H

#include <QString>
#include <QFile>
#include <QDir>
#include <QDebug>

#define UPM_CPU_INFO_FILE         QStringLiteral("/proc/cpuinfo")

const QString DMI_PATH = "/sys/class/dmi/id/";
const QString DRM_PATH = "/sys/class/drm/";

class HardwareReader
{
public:
    HardwareReader();
    ~HardwareReader();

    static HardwareReader* instance();

    bool isPanguM900() const;
    bool isPgwDevice() const;
    bool isFlemingX();

    /**
     * @brief getDMIInfo
     * @param field 字段
     * bios_date        board_vendor       chassis_version  product_sku
        bios_vendor      board_version      modalias         product_uuid
        bios_version     chassis_asset_tag  power            product_version
        board_asset_tag  chassis_serial     product_family   subsystem
        board_name       chassis_type       product_name     sys_vendor
        board_serial     chassis_vendor     product_serial   uevent
     * @return
     */
    QString getDMIInfo(const QString field) const;

private:
    QString executeCommand(const QString &command) const;
    QString getSystemProductName();
    bool readFile(const QString &filePath, QString &content);
    void getCpuType();

    static HardwareReader* m_instance;
    QString m_cpuType;
};

class EdidComparator {

public:
    // 读取当前连接的EDID
    static QMap<QString, QByteArray> readCurrentEdids() {
        QMap<QString, QByteArray> edidMap;
        QDir drmDir("/sys/class/drm");

        foreach (const QString &card, drmDir.entryList({"card*"}, QDir::Dirs)) {
            QDir cardDir(drmDir.filePath(card));
            foreach (const QString &output, cardDir.entryList({"*-*"}, QDir::Dirs)) {
                QFile edidFile(cardDir.filePath(output + "/edid"));
                if (edidFile.open(QIODevice::ReadOnly)) {
                    QByteArray data = edidFile.readAll();
                    if (!data.isEmpty()) {
                        qInfo() << "output:" << output << " edid:" << data.toHex().toUpper();
                        edidMap.insert(output, data.toHex().toUpper());
                    }
                }
            }
        }
        return edidMap;
    }

    // 对比EDID关键字段
    static bool compareKeyFields(const QByteArray &current, const QByteArray &known) {
        if (current.size() < 16 || known.size() < 16) {
            qWarning() << "EDID长度不足，无法比较";
            return false;
        }

        // 厂商ID（字节8-9）
        // if (current.mid(16,4) != known.mid(16,4)) return false; // 2字节转4字符hex

        // 产品代码（字节10-11）
        if (current.mid(20,4) != known.mid(20,4)) {
            qDebug() << "比较产品代码:" << current.mid(20,4) << "vs" << known.mid(20,4);
            return false;
        }


        // 出厂周/年（字节16-17）
        // if (current.mid(32,4) != known.mid(32,4)) return false;

        return true;
    }

    // 完整EDID对比
    static bool compareFullEdid(const QByteArray &current, const QByteArray &known) {
        return (current == known);
    }

    static bool isSameScreen() {
        const QByteArray EDO_EDID = "00FFFFFFFFFFFF00148F28140000000015230104B51E1378"
                                     "02A521AE513CB9240A4F53000000010101010101010101010"
                                     "1010101010183DB4074B10838743004C14C2EBD1000001883D"
                                     "B4074B10878703004C14C2EBD10000018000000FE0045444F2"
                                     "053480A202020202020000000FE004545303051424136332E2"
                                     "0440A0002";
        const QByteArray EDO_EDID_1 = "00FFFFFFFFFFFF00148F28140000000015230104B51E1378"
                                     "02A521AE513CB9240A4F53000000010101010101010101010"
                                     "1010101010183DB4074B10838743004C14C2EBD1000001883D"
                                     "B4074B10878703004C14C2EBD10000018000000FE0045444F2"
                                     "053480A202020202020000000FE004545303051424136332E4"
                                     "40A200002";
        const QByteArray ATNA_EDID = "00ffffffffffff004c839f4100000000"
                                     "00210104b51e1378020cf1ae523cb923"
                                     "0c505400000001010101010101010101"
                                     "010101010101cafe4064b00818702008"
                                     "88002ebd1000001bcafe4064b0083877"
                                     "082088002ebd1000001b000000fe0053"
                                     "444320202020202020202020000000fe"
                                     "0041544e413430594b32302d302001cf"
                                     "02030f00e3058000e606050174600700"
                                     "00000000000000000000000000000000"
                                     "00000000000000000000000000000000"
                                     "00000000000000000000000000000000"
                                     "00000000000000000000000000000000"
                                     "00000000000000000000000000000000"
                                     "00000000000000000000000000000000"
                                     "000000000000000000000000000000b7";
                // 华星EDID
        const QByteArray HX_EDID = "00ffffffffffff000e77641400000000"
                                   "00230104b51e1378e32d0ea4554c9b25"
                                   "0d4e5300000001010101010101010101"
                                   "010101010101178840a0b0086e703020"
                                   "36002dbc10000018000000fd002878e6"
                                   "e646010a202020202020000000fe0043"
                                   "534f542054330a2020202020000000fc"
                                   "004d4e453030375a41332d370a200124"
                                   "7020790200200018408bf66414000000"
                                   "0000190c4d4e453030375a41332d370a"
                                   "21001dc70b5d07400b0807004dfa54cc"
                                   "e49b49020d023554d05fd05f00341378"
                                   "220014c1a10a053f0b9f002f001f0007"
                                   "076d00020005002b000c270028770000"
                                   "2700283b0000810014731a0000035128"
                                   "7800000000000078000000008d006690";
        // 天马EDID
        const QByteArray TM_EDID =  "00ffffffffffff0051a1562000000000"
                                    "16230104b51e13780766e5a3534c9824"
                                    "10505400000001010101010101010101"
                                    "010101010101988640a0b00859703020"
                                    "36002cbe10000018000000fd002878e3"
                                    "e345010a20202020202000000010000a"
                                    "202020202020202020202020000000fc"
                                    "00544c3134304144585032320a2001dd"
                                    "702079020020000c0000005620000000"
                                    "0016190021001dc80b5d07400b080740"
                                    "348a53c484984c821004455400000000"
                                    "00001378260009040000000000010000"
                                    "220014d5830a053f0b9f002f001f0007"
                                    "075800020005002b000c270028770000"
                                    "2700283b0000810015741a0000030128"
                                    "7800006041604178000000008d008390";
    
        qInfo() << "EDO_EDID :" << EDO_EDID;
        qInfo() << "ATNA_EDID :" << ATNA_EDID;

        // 获取当前EDID
        QMap<QString, QByteArray> currentEdids = EdidComparator::readCurrentEdids();

        // 对比逻辑
        foreach (const QByteArray &edid, currentEdids) {
            if (EdidComparator::compareFullEdid(edid.toUpper(), EDO_EDID)
                    || EdidComparator::compareFullEdid(edid.toLower(), ATNA_EDID)
                    || EdidComparator::compareFullEdid(edid.toUpper(), EDO_EDID_1)
                    || EdidComparator::compareKeyFields(edid.toLower(), HX_EDID)
                    || EdidComparator::compareKeyFields(edid.toLower(), TM_EDID)
                    )
            {
                qInfo() << "是同一屏幕";
                return true;
            }
        }
        return false;
    }
};

#endif // HARDWAREREADER_H
