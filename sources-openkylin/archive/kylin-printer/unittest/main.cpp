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

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QLibraryInfo>
#include <QLocale>
#include <QMetaType>
#include <QStandardPaths>
#include <QTranslator>

//#include <ukui-log4qt.h>

#include <fcntl.h>
#include <sys/inotify.h>
#include <syslog.h>

#include "../printer_manager/ukuiPrinter.h"
const static QString g_locale = "zh_CN";
static bool getChineseOptionKey(ppd_file_t* ppd, const QString &keyword,QString &display)
{
    QString trans = QString("%1.%2").arg(g_locale).arg("Translation");
    ppd_attr_t *attr = ppdFindAttr(ppd,trans.toLocal8Bit().data(),keyword.toLocal8Bit().data());
    if(attr==nullptr){
        return false;
    }
    display = attr->text;
    return true;
}
static bool getChineseOptionChoice(ppd_file_t* ppd, const QString &keyword ,const QString &choice,QString &display)
{
    QString key = QString("%1.%2").arg(g_locale).arg(keyword);
    ppd_attr_t *attr = ppdFindAttr(ppd,key.toLocal8Bit().data(),choice.toLocal8Bit().data());
    if(attr==nullptr){
        return false;
    }
    display = attr->text;
    return true;
}

int main(int argc, char* argv[])
{
    // 统一日志输出
    // initUkuiLog4qt("kylin-printer-unittest");
    QApplication app(argc, argv);
    // bool r = UkuiPrinterManager::getInstance().setDefaultPrinter("Brother+DCP-7195DW");
    // qDebug() << r;
    QList<UkuiUsbPrinter> printers;
    //UkuiPrinterManager::getInstance().enumPrinters(printers);
    // UkuiPrinterManager::getInstance().setPaperSize("ipp://localhost/printers/Brother-DCP-7195DW","custom_215x345mm_215x345mm");
    // UkuiPrinterManager::getInstance().setLocation("ipp://localhost/printers/Brother-DCP-7195DW","XXXY");

    // Check different and conflict...
    // bool checkConflict(ppd,)

    // Two Files
    // File1:orig ppd, File2:modified ppd
    // Read from file1, and write to file2 line by line...
    // And then add ppd to printer...
    // OPEN PPD
//    UkuiUsbPrinter printer = printers.at(0);
//    printer.mediaSize.current = "A6";
//    printer.duplex.current = "None";
//    printer.location = "TEST1";

//    UkuiPrinterManager::getInstance().setPrinterConfig(printer);
    //UkuiPrinterManager::getInstance().duplicatePrinter("CANONLBP251","CNLBP251-CNLBP251");
//    QMap<QString,QString> conflicts;
//    conflict("PageSize","A6",conflicts);
//    qDebug() << conflicts;

    //UkuiPrinterManager::getInstance().enumPrinters(printers);
    UkuiUsbPrinter printer,printer2;
    UkuiPrinterManager::getInstance().startModifyAttr("Cumtenn",printer);
    UkuiPrinterManager::getInstance().endModifyAttr("Cumtenn",false);
    UkuiPrinterManager::getInstance().startModifyAttr("GreatWall",printer2);
    UkuiPrinterManager::getInstance().endModifyAttr("GreatWall",false);
//    const char* tmpFileName = cupsGetPPD2(CUPS_HTTP_DEFAULT, "Cumtenn");
//    ppd_file_t* ppd = ppdOpenFile(tmpFileName);
//    ppdMarkDefaults(ppd);
//    ppd_attr_t* attr;
//    attr = ppdFindAttr(ppd,"zh_CN.Translation","Duplex");
//    qDebug() << attr->text;
//    attr = ppdFindAttr(ppd,"zh_CN.Duplex","DuplexNoTumble");
//    qDebug() << attr->text;
//    ppdClose(ppd);
//    unlink(tmpFileName);
    return app.exec();
    QMap<QString,QString> conflicts;
    QList<conflictPair> conflictResults;

    UkuiPrinterManager::getInstance().startModifyAttr("CanonLBP251",printer);
    UkuiPrinterManager::getInstance().modifyPrinterAttr("PageSize","A6",conflicts);
    qDebug() << conflicts;
    for(auto it = conflicts.begin();it!=conflicts.end();it++){
        UkuiPrinterManager::getInstance().getConflictResults(it.key(),conflictResults);
        qDebug() << conflictResults;
    }
    conflicts.clear();
    conflictResults.clear();
    UkuiPrinterManager::getInstance().modifyPrinterAttr("MediaType","ENVOLPE",conflicts);
    qDebug() << conflicts;
    for(auto it = conflicts.begin();it!=conflicts.end();it++){
        UkuiPrinterManager::getInstance().getConflictResults(it.key(),conflictResults);
        qDebug() << conflictResults;
    }
    UkuiPrinterManager::getInstance().modifyPrinterAttr("PageSize","A4",conflicts);
    qDebug() << conflicts;
    for(auto it = conflicts.begin();it!=conflicts.end();it++){
        UkuiPrinterManager::getInstance().getConflictResults(it.key(),conflictResults);
        qDebug() << conflictResults;
    }
    UkuiPrinterManager::getInstance().endModifyAttr("CANONLBP251",false);
    return app.exec();
}
