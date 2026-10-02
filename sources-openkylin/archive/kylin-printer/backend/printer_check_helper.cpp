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

#include <QEventLoop>
#include "ukui_printer.h"
#include "printer_check_helper.h"
#include "device_information.h"
#include "lpinfo_browser.h"
#include "package_helper.h"
#include "ukui_apt.h"
#include "find_ppd_worker.h"
#include "system_notification.h"
#include "custom_message_box.h"
#include "event_notify_monitor.h"

 PrinterCheckHelper::PrinterCheckHelper()
{

}

PrinterCheckHelper::~PrinterCheckHelper()
{

}

PrinterCheckHelper *PrinterCheckHelper::getInstance()
{
    static PrinterCheckHelper *mainWinInstance = nullptr;
    if (nullptr == mainWinInstance){
        mainWinInstance = new PrinterCheckHelper();
    }

    connect(EventNotifyMonitor::getInstance(), &EventNotifyMonitor::rhPrinterAdded,
            mainWinInstance, &PrinterCheckHelper::dowork);

    return mainWinInstance;
}

void PrinterCheckHelper::sendMessage(bool repairSucceed, const QString &msg, const QString &printername) {
    if (repairSucceed) {
        // QString title = "完成打印机驱动修复";
        QString title = tr("Complete printer driver repair");
        // QString text = QString("“%1” 打印机驱动修复已完成").arg(printername);
        QString text = tr("\"%1\" Printer driver repair completed").arg(printername);
        SystemNotification().sendMessage(text, title, 5000);
    } else {
        QString title = msg;
        QString text = tr("\"%1\" printer driver repair failed").arg(printername);
        SystemNotification().sendMessage(text, title, 5000);
    }
}

void PrinterCheckHelper::dowork(const QString &printername)
{
    m_printername = printername;
    m_installOk = -1;
    m_needInstall = false;
    m_pkgHelperFlag = false;
    m_needmessage = false;

    qDebug() << "im in PrinterCheckHelper!";
    UkuiUsbPrinter selectPrinter = UkuiPrinterManager::getInstance().getPrinterFromName(m_printername); 

    if (selectPrinter.name.isEmpty() || selectPrinter.device_uri.isEmpty()) {
        return;
    }

    if (selectPrinter.info == selectPrinter.name + " added by kylin-printer.") {
        return;
    }

    DeviceInformation info = LpinfoBrowser::getDeviceFromUri(selectPrinter.device_uri);

    QEventLoop loop;
    PackageHelper* pkgHelper = new PackageHelper(info);
    m_needInstall = false;
    m_pkgHelperFlag = false;
    connect(pkgHelper, &PackageHelper::finished, &loop, &QEventLoop::quit);
    connect(pkgHelper, &PackageHelper::succeed, [=](bool needInstall) {
        m_needInstall = needInstall;
        pkgHelper->finished();
        m_pkgHelperFlag = true;
    });
    connect(pkgHelper, &PackageHelper::failed, [=]() {
        pkgHelper->finished();
        m_pkgHelperFlag = false;
    });
    pkgHelper->startWorker();
    loop.exec();

    if (!m_pkgHelperFlag) {
        qDebug () << "搜索驱动失败!";
        return;
    }

    if (m_needInstall == true && !info.packageLevel.isEmpty() && info.packageLevel != "1") {
        QString tipStr;
        if (info.packageLevel == "2") {

        } else {
            return;
        }
    }

    if (m_needInstall) {
        m_needmessage = true;
    }


    PPDsAndAttr ppd;
    QList<PPDsAndAttr> ppdList;
    if (!m_needmessage) {

        int ret;

        FindPpdWorker *worker = new FindPpdWorker();

        if (m_installOk == 0) {
            worker->setStatus(true);
        } else {
            worker->setStatus(false);
        }

        worker->setInput(info.vendor, info.model, info.suggestPpdName);

        QEventLoop loop2;
        connect(worker, &FindPpdWorker::finished, &loop2, &QEventLoop::quit);
        connect(worker, &FindPpdWorker::resultPPDs, [=, &ret, &ppd, &ppdList](int res, PPDsAndAttr exactMatchPPD, QList<PPDsAndAttr> matchResultList) {
            ret = res;
            ppd = exactMatchPPD;
            ppdList = matchResultList;
            worker->finished();
        });

        worker->startWorker();
        loop2.exec();

        if (ret != 0) {
            qDebug() << ret;
        }

        if (ppd.ppdname.isEmpty()) {
            return;
        }

        // nick name 匹配;
        qDebug() << selectPrinter.make_and_model;
        qDebug() << ppd.origin_ppd_make_and_model;


        if (selectPrinter.make_and_model != ppd.origin_ppd_make_and_model) {
            m_needmessage = true;
        }
    }

    if (!m_needmessage) {
        return;
    }

    CustomMessageBox Msg1(CustomMessageBox::Warning, tr("Warning"), tr("The printer \"%1\" has an incompatible print driver installed. Do you want to repair it?").arg(m_printername), CustomMessageBox::NoButton, nullptr);
    Msg1.setInformativeText(tr("Installing an incompatible print driver may result in abnormal printing or printing failure. It is recommended to repair it."));
    QPushButton *btnYes = Msg1.addButton(tr("Repair"), CustomMessageBox::YesRole);
    QPushButton *btnNo  = Msg1.addButton(tr("Not repaired"), CustomMessageBox::NoRole);
    Msg1.exec();
    if ((QPushButton *)Msg1.clickedButton() == btnYes) {
        if (m_needInstall) {
            m_installOk = 1;
            QEventLoop loop;
            connect(this, &PrinterCheckHelper::installEnd, &loop, &QEventLoop::quit);
            AptUtilHelper *aptHelper = new AptUtilHelper(info);
            connect(aptHelper, &AptUtilHelper::succeed, [ = ]() {
                qDebug() << "Install succeed";
                m_installOk = 0;
                emit installEnd();
            });
            connect(aptHelper, &AptUtilHelper::failed, [ = ]() {
                qDebug() << "Install failed";
                m_installOk = 1;
                emit installEnd();
            });
            aptHelper->startWorker();
            loop.exec();
            aptHelper->finished();

            if (m_installOk != 0) {
                sendMessage(false, tr("Driver installation failed!"),m_printername);
                return;
            }
        }

        QString ppd;
        QStringList ppdList;
        int ret;

        FindPpdWorker *worker = new FindPpdWorker();

        if (m_installOk == 0) {
            worker->setStatus(true);
        } else {
            worker->setStatus(false);
        }

        worker->setInput(info.vendor, info.model, info.suggestPpdName);

        QEventLoop loop2;
        connect(worker, &FindPpdWorker::finished, &loop2, &QEventLoop::quit);
        connect(worker, &FindPpdWorker::result, [=, &ret, &ppd, &ppdList](int res, QString exactMatchPPD, QStringList matchResultList) {
            ret = res;
            ppd = exactMatchPPD;
            ppdList = matchResultList;
            worker->finished();
        });

        worker->startWorker();
        loop2.exec();

        if (ret != 0) {
            qDebug() << ret;
        }

        if (ppd.isEmpty()) {
            if (ppdList.isEmpty()) {
                sendMessage(false, tr("No printer driver for %1.").arg(m_printername) ,m_printername);
                return;
            } else {
                ppd = ppdList.at(0);
            }
        }
        QString errorString;
        bool res = UkuiPrinterManager::getInstance().addPrinter(QString()
                                                                ,m_printername,ppd
                                                                ,selectPrinter.location, errorString);

        if (res == false) {
            sendMessage(false, errorString ,m_printername);
            return;
        }
        sendMessage(true, "" ,m_printername);
        return;
    }
    return;
}
