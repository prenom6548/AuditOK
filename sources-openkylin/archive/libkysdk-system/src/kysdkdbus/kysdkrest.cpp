/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2023-2025, KylinSoft Co., Ltd.
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
 * Authors: ZhiMin Shao <shaozhimin@kylinos.cn>
 *
 */

#include "kysdkrest.h"
#include "../systeminfo/libkysysinfo.h" // 分辨率

KySdkRest::KySdkRest(QObject *parent) : QObject(parent)
{

}
QStringList KySdkRest::getSysLegalResolution() const
{
    QStringList res = {};
    char **power = kdk_system_get_resolving_power();
    size_t count = 0;
    while(power[count])
    {
        res << QString::fromLocal8Bit(power[count]);
        count ++;
    }
    kdk_resolving_freeall(power);
    return res;
}

KySdkPrint::KySdkPrint(){}

QStringList KySdkPrint::getPrinterList() const
{
    QStringList res = {};
    char **printers = kdk_printer_get_list();
    size_t index = 0;
    while (printers[index])
    {
        res << QString::fromLocal8Bit(printers[index]);
        index ++;
    }
    return res;
}

QStringList KySdkPrint::getPrinterAvailableList() const
{
    QStringList res = {};
    char **printers = kdk_printer_get_available_list();
    size_t index = 0;
    while (printers[index])
    {
        res << QString::fromLocal8Bit(printers[index]);
        index ++;
    }
    return res;
}

void KySdkPrint::setPrinterOptions(int number_up, const QString media, const QString number_up_layout, const QString sides) const
{
    std::string med = media.toStdString();
    std::string nul = number_up_layout.toStdString();
    std::string sid = sides.toStdString();
    kdk_printer_set_options(number_up, med.c_str(), nul.c_str(), sid.c_str());
}

int KySdkPrint::getPrinterPrintLocalFile (const QString printername, const QString filepath) const
{
    std::string printers = printername.toStdString();
    std::string fp = filepath.toStdString();
    return kdk_printer_print_local_file(printers.c_str(), fp.c_str());
}

int KySdkPrint::getPrinterCancelAllJobs(const QString printername) const
{
    std::string printers = printername.toStdString();
    return kdk_printer_cancel_all_jobs(printers.c_str());
}

int KySdkPrint::getPrinterStatus(const QString printername) const
{
    std::string printers = printername.toStdString();
    return kdk_printer_get_status(printers.c_str());
}

QString KySdkPrint::getPrinterFilename(const QString url) const
{
    std::string urlpath = url.toStdString();
    
    return QString::fromLocal8Bit(kdk_printer_get_filename(urlpath.c_str()));
}
int KySdkPrint::getPrinterJobStatus(const QString printername, int jobid) const
{
    std::string printers = printername.toStdString();
    return kdk_printer_get_job_status(printers.c_str(), jobid);
}

int KySdkPrint::getPrinterDownloadRemoteFile(const QString url, const QString filepath) const
{
    std::string urlpath = url.toStdString();
    std::string fp = filepath.toStdString();
    return kdk_printer_print_download_remote_file(urlpath.c_str(), fp.c_str());
}
