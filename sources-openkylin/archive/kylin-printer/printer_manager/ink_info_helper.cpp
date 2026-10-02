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
#include "ink_info_helper.h"
#include "ukui_printer.h"
#include "common.h"
#include "avahi_helper.h"

InkInfoHelper::InkInfoHelper(UkuiUsbPrinter &printer) :
    m_printer(printer)
{

}

InkInfoHelper::~InkInfoHelper()
{

}

// marker-colors (1setOf nameWithoutLanguage) = #000000,#FFFF00,#FF00FF,#00FFFF,#000000,#FFFF00,#FF00FF,#00FFFF
// marker-levels (1setOf integer) = 82,85,86,86,91,95,95,95
// marker-names (1setOf nameWithoutLanguage) = 黑色墨粉盒;SN806F5E0E6A070000,黄色墨粉盒;SN803B5A096B070000,品红色墨粉盒;SN8033581568070000,青色墨粉盒;SN80E2550269070000,黑色感光鼓;SN8085420F6D070000,黄色感光鼓;SN80FC4E0F6D070000,品红色感光鼓;SN80B5400F6D070000,青色感光鼓;SN809D4C0F6D070000
// marker-types (1setOf keyword) = toner,toner,toner,toner,opc,opc,opc,opc

// marker-colors (nameWithoutLanguage) = #000000
// marker-levels (integer) = 93
// marker-names (nameWithoutLanguage) = Black Cartridge
// marker-types (keyword) = toner-cartridge
void InkInfoHelper::getInkInfo(const QString &uri)
{
    if (!uri.contains("ipp://") && !uri.contains("ipps://")
     && !uri.contains("http://") && !uri.contains("https://")) {
        return ;
    }

    const QStringList markerOptions{"marker-colors", "marker-names", "marker-types", "marker-levels"};

    QMap<QString, QStringList> inkInfo;
    int minn = 0x3f3f3f3f;

    QStringList command;
    command << "ipptool" << "-t" << "-v" << uri << "/usr/share/cups/ipptool/ipp-backend.test";
    QString res;
    int msec = 1000;
    if (!execCommand(command, &res, nullptr, nullptr, &msec)) {
        return ;
    }
    auto list = res.split('\n');

    for (auto str : list) {
        str = str.trimmed();
        if (!str.contains(" = ")) {
            continue;
        }
        QString key = str.split(" = ").at(0).trimmed();
        for (auto markerOption : markerOptions) {
            if (key.contains(markerOption)) {

                QString value = str.split(" = ").at(1).trimmed() + ",";
                QStringList list = value.split(",");
                minn = qMin(minn, list.size());
                inkInfo.insert(markerOption, list);
            }
        }
    }

    if (minn == 0x3f3f3f3f || minn == 0) {
        return;
    }
    m_printer.inkOptions.clear();
    for (int i = 0; i < minn; i++) {
        InkProperty ink;
        if (inkInfo.contains("marker-colors")) {
            ink.color.setNamedColor(inkInfo["marker-colors"].at(i));
        }
        if (inkInfo.contains("marker-names")) {
            ink.name = inkInfo["marker-names"].at(i);
        }
        if (inkInfo.contains("marker-types")) {
            ink.type = inkInfo["marker-types"].at(i);
        }
        if (inkInfo.contains("marker-levels")) {
            ink.level = inkInfo["marker-levels"].at(i).toInt();
        }

        if (!ink.color.name().isEmpty() && !ink.name.isEmpty() && ink.level >= 0 && ink.level <= 100) {
            m_printer.inkOptions.append(ink);
        }
    }
}

void InkInfoHelper::dowork()
{
    QString uri = m_printer.device_uri;
    if (uri.contains("socket://")) {
        uri.replace("socket://", "ipp://");

        // 处理带有端口号的场景
        QStringList parts = uri.split(':');
        // 移除最后一个部分（端口号）
        if (parts.size() > 2) {
            parts.removeLast();
        }
        uri = parts.join(':');
    }

    getInkInfo(uri);
    if (m_printer.inkOptions.isEmpty()) {
        if (uri.contains("._ipp._tcp.local") || uri.contains("._ipps._tcp.local")) {
            QString host = AvahiHelper().search(uri);
            qDebug() << "use ip search again" << host;
            if (!host.isEmpty()) {
                if (uri.contains("ipp://")) {
                    getInkInfo(QString("ipp://%1").arg(host));
                } else if (uri.contains("ipps://")) {
                    getInkInfo(QString("ipps://%1").arg(host));
                }
            }
        }
    }

    emit succeed();
}
