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
#include <QString>
#include <QStringList>
#include <QPushButton>
#include <QEventLoop>

#include "pop_window_manager.h"
#include "device_monitor.h"
#include "system_notification.h"
#include "common.h"
#include "find_and_match_ppds.h"
#include "ukui_printer.h"
#include "ukui_apt.h"
#include "package_helper.h"
#include "information_collector.h"
#include "find_ppd_worker.h"

PopWindowManager::PopWindowManager()
{
    connect(DeviceMonitorBackend::getInstance(), &DeviceMonitorBackend::findUsbConnect,
            this, &PopWindowManager::addNewPrinterDevice, Qt::QueuedConnection);
    connect(DeviceMonitorBackend::getInstance(), &DeviceMonitorBackend::findUsbDisconnect,
            this, &PopWindowManager::modifyInfoList, Qt::QueuedConnection);
    connect(this, &PopWindowManager::installPrinter,
            this, &PopWindowManager::getDevInfo);
    connect(this, &PopWindowManager::showNextRequest,
            this, &PopWindowManager::showNext);
}

void PopWindowManager::addNewPrinterDevice(DeviceInformation devInfo)
{
    if (m_devMp.contains(devInfo.sysPath)) {
        return;
    }

    // 判断一下是否重复了
    QList<UkuiUsbPrinter> printers;
    UkuiPrinterManager::getInstance().enumPrinters(printers);
    foreach (auto printer, printers) {
        if (printer.device_uri == devInfo.uri) {
            qDebug() << "In Installing: " << devInfo.name;
            qDebug()<<"The Printer: "<<devInfo.uri << " has already installed!";
            return ;
        }
    }

    m_devMp.insert(devInfo.sysPath, devInfo);

    QString desc = devInfo.vendor + " " + devInfo.model;
    SystemNotification().sendMessage(tr("Do you want to add a printer?"), tr("The printer was detected:%1").arg(desc), -1, QStringList() << QString("kylin-printer-add-printer:%1").arg(devInfo.sysPath) << tr("Add"));
}

void PopWindowManager::getDevInfo(QString sysPath)
{
    if (!m_devMp.contains(sysPath)) {
        return ;
    }
    DeviceInformation devInfo = m_devMp[sysPath];

    if (m_infoList.isEmpty()) {
        m_infoList.enqueue(devInfo);
        showPopWindow(m_infoList.last());
    }
    else {
        // 先判断之前有没有处理过
        for (int i = 0; i < m_infoList.size(); ++i) {
            if (devInfo.sysPath == m_infoList.at(i).sysPath) {
                return ;
            }
        }
        m_infoList.enqueue(devInfo);
    }
    return ;
}

void PopWindowManager::modifyInfoList(QString path)
{
    if (m_devMp.isEmpty()) {
        return ;
    }
    m_devMp.remove(path);
    return ;
}

void PopWindowManager::showNext()
{
    if (m_infoList.isEmpty()) {
        return ;
    }
    m_infoList.dequeue();
    if (m_infoList.isEmpty()) {
        qDebug() << "popwindow show all finished!";
        return ;
    }
    
    showPopWindow(m_infoList.first());
    return ;
}

void PopWindowManager::showPopWindow(const DeviceInformation &devInfo)
{
    qDebug() << "Installing: " <<devInfo.name;

    DeviceInformation info(devInfo);
    // 判断一下是否重复了
    QList<UkuiUsbPrinter> printers;
    UkuiPrinterManager::getInstance().enumPrinters(printers);
    foreach (auto printer, printers) {
        if (printer.device_uri == info.uri) {
            qDebug() << "In Installing: " << info.name;
            qDebug()<<"The Printer: "<<info.uri << " has already installed!";
            showNextRequest();
            return ;
        }
    }

    QMap<QString, QString> messages;
    messages.insert("uri", devInfo.uri);
    messages.insert("pid", devInfo.PID);
    messages.insert("vid", devInfo.VID);
    messages.insert("vendor", devInfo.vendor);
    messages.insert("model", devInfo.model);

    QString desc = getNameFromDeviceInformation(info);
    int nameCount=1;
    QString textDesc(desc);
    while(1) {
        auto res = UkuiPrinterManager::getInstance().printerNameCheck(textDesc, false, printers);

        if (res.status == CommonStatusFlag::ERROR_EXIST_SAME_PRINTER) {
            textDesc = QString("%1-%2").arg(desc).arg(nameCount);
            nameCount++;
        } else {
            break ;
        }
    }
    desc = textDesc;
    // SystemNotification().sendMessage(tr("Installing..."), tr("The printer was detected:%1").arg(desc), -1);
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
        qDebug() << "Install failed";
        pkgHelper->finished();
        m_pkgHelperFlag = false;
    });
    pkgHelper->startWorker();
    loop.exec();

    if (!m_pkgHelperFlag) {
        qDebug () << "搜索驱动失败，使用本地驱动!";
    }

    if (m_needInstall == true && !info.packageLevel.isEmpty() && info.packageLevel != "1") {
        QString tipStr;
        if (info.packageLevel == "2") {

        } else {
            m_needInstall = false;
            if (info.packageLevel == "3") {
                // tipStr = "该设备型号驱动适配中。";
                tipStr = tr("The device driver is being adapted.");
            } else {
                // tipStr = "由于商业协议限制，请联系设备制造商以获取该设备型号驱动程序。";
                tipStr = tr("Due to commercial agreement restrictions, please contact the device manufacturer to obtain drivers for this device model.");
            }
            SystemNotification().sendMessage(tipStr, tr("%1 installation failed").arg(desc), -1);
            
            messages.insert("result", "auto fail");
            messages.insert("resultString", QString("the packageLevel is %1!").arg(info.packageLevel));
            InformationCollector::getInstance().addMessageMap("add_printer", messages);
            return;
        }
    }

    if (m_needInstall) {
        m_installOk = 1;
        QEventLoop loop;
        connect(this, &PopWindowManager::installEnd, &loop, &QEventLoop::quit);
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
        if (m_installOk == 1) {
            // QString msg = "安装驱动失败！";
            // SystemNotification().sendMessage(msg);
            // return;
        }
        aptHelper->finished();
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
            QString msg = tr("No printer driver for %1.").arg(desc);
            SystemNotification().sendMessage(msg, tr("%1 installation failed").arg(desc), -1);

            messages.insert("result", "auto fail");
            messages.insert("resultString", QString("No printer driver"));
            InformationCollector::getInstance().addMessageMap("add_printer", messages);
            return;
        } else {
            ppd = ppdList.at(0);
        }
    }

    QString errorString;
    ret = UkuiPrinterManager::getInstance().addPrinter(info.uri, desc, ppd, "", errorString);
    if (ret == false) {
        SystemNotification().sendMessage(errorString, tr("%1 installation failed").arg(desc), -1);
        messages.insert("result", "auto fail");
        messages.insert("resultString", "add printer failed!");
        messages.insert("cupsErrorString", errorString);
        InformationCollector::getInstance().addMessageMap("add_printer", messages);
        return;
    }
    info.displayName = desc;
    info.userChoosePpdName = ppd;

    SystemNotification().sendMessage("", tr("%1 successful installation").arg(desc), -1, QStringList() << QString("kylin-printer-print-test-page:%1").arg(info.serialize()) << tr("Print test page"));
    
    emit showNextRequest();
    messages.insert("result", "auto success");
    InformationCollector::getInstance().addMessageMap("add_printer", messages);
    return ;
}

PopWindowManager *PopWindowManager::getInstance()
{
    static PopWindowManager *instance = nullptr;
    static QThread *thread;
    if (nullptr == instance) {
        instance = new PopWindowManager();
        thread = new QThread();
        thread->setObjectName("PopWindowManager");
        instance->moveToThread(thread);

        thread->start();
    }
    return instance;
}

PopWindowManager::~PopWindowManager()
{

}
