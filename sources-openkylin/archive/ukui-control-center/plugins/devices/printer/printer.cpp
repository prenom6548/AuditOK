/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "printer.h"

#include "ukcccommon.h"
using namespace ukcc;

#include <QtPrintSupport/QPrinterInfo>
#include <QProcess>
#include <cups/cups.h>

#include <QDebug>
#include <QMouseEvent>
#include <QThread>
#include <QDBusConnection>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

#include "usbthread.h"
#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

#define ITEMFIXEDHEIGH 58
#define UEVENT_BUFFER_SIZE 2048

#define PRINTER_PATH "/com/redhat/PrinterSpooler"
#define PRINTER_SERVICE  "com.redhat.PrinterSpooler"

Printer::Printer() : mFirstLoad(true)
{
    pluginName = tr("Printer");
    pluginType = DEVICES;
}

Printer::~Printer()
{
    if (!mFirstLoad) {
    }
}

QString Printer::plugini18nName()
{
    return pluginName;
}

int Printer::pluginTypes()
{
    return pluginType;
}

QWidget *Printer::pluginUi()
{
    if (mFirstLoad) {
        mFirstLoad = false;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);

        initUi(pluginWidget);

        // 在Qt中使用QFutureWatcher和QFuture
        auto *watcher  = new QFutureWatcher<void>();
        connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher]() {
        // 当耗时任务完成时，这里的代码会被执行
            this->initPrinterUi();
            watcher->deleteLater();
        });

        // 使用QtConcurrent运行耗时任务
        QFuture<void> future = QtConcurrent::run([=]{
            refreshPrinterDevSlot();
        });
        watcher->setFuture(future);

        if (!QDBusConnection::systemBus().connect(QString(), PRINTER_PATH, PRINTER_SERVICE, "PrinterAdded", this, SIGNAL(addsignal(QString)))) {
            qDebug()<<"failed to add D-Bus signal receiver(addprinter)";
        }
        if (!QDBusConnection::systemBus().connect(QString(), PRINTER_PATH, PRINTER_SERVICE, "PrinterRemoved", this, SIGNAL(removesignal(QString)))) {
            qDebug()<<"failed to add D-Bus signal receiver(removeprinter)";
        }

        // 监听打印机增删信号
         connect(this, &Printer::addsignal, [=](const QString &printerName){
             qDebug()<<"addPrinter : "<<printerName;
             QTimer::singleShot(1000, this, [=]() {
                 refreshPrinterDevSlot();
                 initPrinterUi();
             });
         });
         connect(this, &Printer::removesignal, [=](const QString &printerName){
             qDebug()<<"removePrinter : "<<printerName;
             refreshPrinterDevSlot();
             initPrinterUi();
         });
    }
    return pluginWidget;
}

const QString Printer::name() const
{
    return QStringLiteral("Printer");
}

bool Printer::isShowOnHomePage() const
{
    return true;
}

QIcon Printer::icon() const
{
    return QIcon::fromTheme("printer-symbolic");
}

bool Printer::isEnable() const
{
    return true;
}

void Printer::initUi(QWidget *widget)
{
    QVBoxLayout *verticaLyt = new QVBoxLayout(widget);
    verticaLyt->setSpacing(8);
    verticaLyt->setContentsMargins(0, 0, 0, 0);
    mPrinterLabel = new KLabel(widget);
    KDK_EXTEND_ALL_INFO_FORMAT(mPrinterLabel, name(), "", "");
    mPrinterLabel->setContentsMargins(16, 0, 0, 0);
    //~ contents_path /Printer/Printers
    mPrinterLabel->setText(tr("Printers"));
    mPrinterLabel->setAccessibleName(tr("Printers"));

    mPrinterWidget = new SettingGroup(widget);
    initComponent();
    mPrinterWidget->addWidget(mAddWidget);

    verticaLyt->addWidget(mPrinterLabel);
    verticaLyt->addWidget(mPrinterWidget);;
    verticaLyt->addStretch();
}

void Printer::initPrinterUi()
{
    mPrinterWidget->removeWidget(mAddWidget);
    clearAutoItem();
    for (auto printer : mPrinters) {
        UkccFrame *printerBtnFrame = new UkccFrame(mPrinterWidget);
        printerBtnFrame->setLineWidth(0);
        QHBoxLayout *hLyt = new QHBoxLayout(printerBtnFrame);
        hLyt->setContentsMargins(0, 0, 0, 0);
        PrinterBtn *mPriterBtn = new PrinterBtn(printer, printerBtnFrame);
        hLyt->addWidget(mPriterBtn);
        mPrinterWidget->addWidget(printerBtnFrame);
        connect(mPriterBtn, &QPushButton::clicked, this, [=]() {
            runExternalApp();
        });
    }
    mPrinterWidget->addWidget(mAddWidget);
}

void Printer::initComponent()
{
    mAddWidget = new UkccFrame;
    mAddWidget->setLineWidth(0);
    QHBoxLayout *hLyt = new QHBoxLayout(mAddWidget);
    hLyt->setContentsMargins(0, 0, 0, 0);
    //~ contents_path /Printer/Add
    tr("Add");
    AddButton *addBtn = new AddButton(pluginWidget);
    KDK_EXTEND_ALL_INFO_FORMAT(addBtn, name(), "", "");
    addBtn->setAccessibleName(tr("Add"));
    addBtn->setRadiusType(UkccFrame::BorderRadiusStyle::Around);
    hLyt->addWidget(addBtn);
    connect(addBtn, &AddButton::clicked, this, &Printer::runExternalApp);

}

void Printer::refreshPrinterDevSlot()
{
    cups_dest_t *dests;
    int num_dests = cupsGetDests(&dests);
    cups_dest_t *dest;
    int i;
    mPrinterList.clear();
    mPrinters.clear();
    for (i = num_dests, dest = dests; i > 0; i --, dest ++) {
        // 获取打印机状态，3为空闲，4为忙碌，5为不可用
        const char *value = cupsGetOption("printer-state", dest->num_options, dest->options);
        qDebug()<<dest->name<<"----------------"<<value;
        if (value == nullptr)
            continue;
        // 标志位flag用来判断该打印机是否可用
        bool flag = (atoi(value) == 5 ? true : false);

        if (flag) {
            if (mPrinterList.contains(QString(dest->name))) {
                mPrinterList.removeOne(QString(dest->name));
                for (int i = 0; i < mPrinters.count(); i++) {
                    if (mPrinters.at(i).name == QString(dest->name)) {
                        mPrinters.remove(i);
                        break;
                    }
                }

            }
        } else {
            if (!mPrinterList.contains(QString(dest->name))) {
                PrinterInfo printer;
                printer.name = QString(dest->name);
                mPrinterList.append(QString(dest->name));
                for (auto k = 0; k < dest->num_options; k++) {
                    if (QString::compare(dest->options[k].name, "device-uri") == 0) {
                        printer.uri = dest->options[k].value;
                    }
                }
                mPrinters.append(printer);
            }
        }
    }
    cupsFreeDests(num_dests, dests);
}

void Printer::runExternalApp()
{
    UkccCommon::buriedSettings(name(), "open kylin-printer", QString("clicked"));
    if (UkccCommon::isTablet()) {
        QDBusInterface ifc("com.kylin.AppManager",
                           "/com/kylin/AppManager",
                           "com.kylin.AppManager",
                           QDBusConnection::sessionBus());

        ifc.call("LaunchApp", "/usr/share/applications/kylin-printer.desktop");
    } else {
        QString cmd = "kylin-printer";
        QProcess process(this);
        process.startDetached(cmd);
    }
}

void Printer::clearAutoItem()
{
    if (mPrinterWidget->layout() != NULL) {
        QLayoutItem *item;
        while ((item = mPrinterWidget->layout()->takeAt(0)) != NULL)
        {
            delete item->widget();
            delete item;
            item = nullptr;
        }
    }
}

QString Printer::getPrinterInfo()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG","en_US");
    QProcess *process = new QProcess;
    process->setProcessEnvironment(env);
    process->start("lpstat -v");
    process->waitForFinished();

    QByteArray ba = process->readAllStandardOutput();

    delete process;
    return QString(ba.data());
}
