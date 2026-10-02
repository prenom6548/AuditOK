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
#include <QDebug>
#include <QFont>
#include <QVBoxLayout>
#include <QPushButton>
#include "custom_message_box.h"
#include <QByteArray>
#include "mainwindow.h"
#include "find_and_match_ppds.h"
#include "base_notify_dialog.h"
#include "pop_window_manager.h"
#include "daemonipcdbus.h"
#include "size_para.h"
#include "global_variable.h"
#include "windowmanage.hpp"
#include "waiting_dialog.h"
#include "information_collector.h"
#include "troubleshooting_dialog.h"

#define g_popWindow PopWindowManager::getInstance()

MainWindow::MainWindow(QWidget *parent) :
            CustomWidget(parent, false),
            m_dbus(nullptr),
            m_AddWindow(nullptr),
            m_propertyDialog(nullptr),
            m_jobListDialog(nullptr),
            m_allPrintersVector()
{
    kabase::WindowManage::removeHeader(this);
    setWidgetName(tr("Printer"));
    setLayoutType(kdk::HorizontalType);
    this->layout()->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    iconBar()->setFixedWidth(300);
    sideBar()->setFixedSize(300,560);
    baseBar()->setFixedSize(600,560);
    windowButtonBar()->menuButton()->show();
    windowButtonBar()->maximumButton()->hide();
    windowButtonBar()->minimumButton()->show();
}

void MainWindow::init()
{
    initModule();
    initQSS();
    initConnect();

    qDebug() << "主窗口初始化";
    g_printerManger.enumPrinters(m_allPrintersVector);
    QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    // this->move((availableGeometry.width()-this->width())/2, (availableGeometry.height()-this->height())/2);
    this->installEventFilter(this);
    if(m_allPrintersVector.size())
    {
        qDebug() << "打印机数量";
        m_currentSelectedPrinter = m_allPrintersVector.at(m_allPrintersVector.size() - 1);
        emit updateListSector(m_allPrintersVector, m_currentSelectedPrinter);
    }
    emit getCurFontSize();
    m_timer = new QTimer(this);
    m_timer->setInterval(5000);
    connect(m_timer,SIGNAL(timeout()),this,SLOT(updateImmediately()));
    m_timer->start();
    handleTabletMode(UkuiSettingsMonitor::instance().tabletMode());
}

MainWindow::~MainWindow()
{
    // qDebug() << "主窗口析构";
    // if(m_dbus != nullptr)
    //     m_dbus->deleteLater();
}

void MainWindow::topWindow()
{
    qDebug()<<"Set mainwindow to toplevel.";
    if(!this->isActiveWindow())
    {
        this->hide();
        this->show();
    }
    this->show();
    this->raise();
    this->activateWindow();
}
void MainWindow::onBtnClicked()
{
    qDebug() << "add Device!";
    QPoint temp = QPoint(this->mapToGlobal(QPoint(0,0)).x() + this->width()/2,this->mapToGlobal(QPoint(0,0)).y() + this->height()/2);
    if(m_AddWindow == nullptr)
    {
        qDebug() << "New Add Window";
        m_AddWindow = new AddPrinterWindow(temp,this);
        connect(m_AddWindow,SIGNAL(needUpdateDests(QString)),this,SLOT(updateMainWindowByAdd(QString)));
    }
    emit m_AddWindow->showThis(temp);
}

void MainWindow::initModule()
{


    //Dbus
    m_dbus = DaemonIpcDbus::getInstance();

    m_leftWidget = new LeftWidget(this);
    m_leftWidget->setObjectName("leftWidget");

    m_rightWidget= new RightWidget(this);
    m_rightWidget->setObjectName("rightWidget");
    m_rightWidget->hideExhbitionSector();


    m_menu = new MenuModule(this);
    this->windowButtonBar()->menuButton()->setMenu(m_menu);
    connect(m_menu, &MenuModule::menuModuleClose,this, &MainWindow::close);
}

void MainWindow::initConnect()
{
    connect(m_dbus, SIGNAL(show()), this, SLOT(topWindow()));

    connect(m_leftWidget,&LeftWidget::addDevice,this,&MainWindow::onBtnClicked);
    connect(this,SIGNAL(updateListSector(QList<UkuiUsbPrinter>,UkuiUsbPrinter)),
            m_leftWidget,SLOT(updateListSector(QList<UkuiUsbPrinter>,UkuiUsbPrinter)));
    connect(this,SIGNAL(addPrinterToListSector(UkuiUsbPrinter)),m_leftWidget,SLOT(addPrinterToListSector(UkuiUsbPrinter)));

    connect(m_leftWidget,SIGNAL(printerIsSelected(const UkuiUsbPrinter&)),
            this,SLOT(updateRightWidgetByCheckBtn(const UkuiUsbPrinter&)));
    connect(m_leftWidget,SIGNAL(printerIsSelected(const UkuiUsbPrinter&)),
            this,SLOT(updateCurrentSelectedPrinter(const UkuiUsbPrinter&)));
    connect(m_leftWidget,SIGNAL(delDevice(QString)),
            this,SLOT(updateMainWindowByDel(QString)));
    connect(m_leftWidget,&LeftWidget::noBtnIsChecked,[=](){
        m_rightWidget->hideExhbitionSector();
    });
    connect(this,SIGNAL(updateRightWidget(const UkuiUsbPrinter&)),
            m_rightWidget,SLOT(updateExhbitionSector(const UkuiUsbPrinter&)));
    connect(m_rightWidget->m_deviceExhbitionMyWidget, &MainWinPropertyWidget::ppdDialogShowRequest,
            this, &MainWindow::ppdDialogShow);
    connect(this,SIGNAL(showPPDSelectDialog()),this,SLOT(ppdDialogShow()));
    connect(m_rightWidget->m_deviceExhbitionMyWidget,SIGNAL(sendOutCurrentPinterInfo(const UkuiUsbPrinter&)),
            this,SLOT(propertyDialogShow(const UkuiUsbPrinter&)));
    connect(m_rightWidget->m_deviceExhbitionMyWidget,SIGNAL(sendOutCurrentPinterName(const QString&)),
            this,SLOT(jobListDialogShow(const QString&)));
    connect(m_rightWidget->m_deviceExhbitionMyWidget,SIGNAL(currentPinterInfoPrintTest(const UkuiUsbPrinter&)),
            this,SLOT(printTestPage(const UkuiUsbPrinter&)));
    connect(m_rightWidget->m_deviceExhbitionMyWidget,SIGNAL(currentPinterInfoTroubleshooting(const UkuiUsbPrinter&)),
            this,SLOT(troubleshooting(const UkuiUsbPrinter&)));
    connect(m_rightWidget->m_deviceExhbitionMyWidget,SIGNAL(updateWholeMainWin(QString)),
            this,SLOT(updateByRename(QString)));

    connect(this,SIGNAL(clearRightWidget()),m_rightWidget,SLOT(hideExhbitionSector()));

    connect(g_popWindow,&PopWindowManager::deviceViewShowRequest,[=](QString &strName){
        if(isHidden()== true){
            show();
        }
        updateMainWindowByAdd(strName);
    });
    connect(this,SIGNAL(updateMainWin()),this,SLOT(updateImmediately()));
    connect(this,SIGNAL(modifyPPD(QString)),this,SLOT(updateByChangePPD(QString)));
}
void MainWindow::initQSS()
{
    m_leftLayout.reset(new QHBoxLayout());
    m_leftLayout->setContentsMargins(0,0,0,0);
    m_leftLayout->setSpacing(0);
    m_leftLayout->addWidget(m_leftWidget);
    sideBar()->setLayout(m_leftLayout.get());

    m_rightLayout.reset(new QHBoxLayout());
    m_rightLayout->setMargin(0);
    m_rightLayout->setSpacing(0);
    m_rightLayout->addWidget(m_rightWidget);
    baseBar()->setLayout(m_rightLayout.get());
}

void MainWindow::updateRightWidgetByCheckBtn(const UkuiUsbPrinter& printer)
{
    qDebug() << "当前选中打印机名称：" << (printer.name);
    InformationCollector::getInstance().addMessage("Currently selected printer name: " + printer.name);
    qDebug() << "当前选中打印机驱动：" << (printer.make_and_model);
    emit updateRightWidget(printer);
}

void MainWindow::updateMainWindowByAdd(QString printerName)
{
    m_allPrintersVector.clear();
    g_printerManger.enumPrinters(m_allPrintersVector);

    QList<UkuiUsbPrinter>::iterator printer;

    for(printer = m_allPrintersVector.begin();printer != m_allPrintersVector.end();printer++)
    {
        if(printer->name == printerName)
        {
            emit addPrinterToListSector(*printer);
            break;
        }
    }

    emit updateListSector(m_allPrintersVector, m_currentSelectedPrinter);
    emit getCurFontSize();
}

void MainWindow::updateMainWindowByDel(QString printerName)
{
    m_allPrintersVector.clear();
    g_printerManger.enumPrinters(m_allPrintersVector);
    qDebug() <<"打印机" << printerName << "删除按钮被点下";
    QList<UkuiUsbPrinter>::iterator printer;
    for(printer = m_allPrintersVector.begin(); printer != m_allPrintersVector.end(); printer++)
    {
        qDebug() << "遍历打印机列表中，当前打印机名称为："
                 <<  (printer->name);
        if(printer->name == printerName)
        {
            if(g_printerManger.removePrinter(printerName))
            {
                m_allPrintersVector.erase(printer);
                break;
            }
            else
            {
                QString showMsg = printerName;
                QString warningMsg = tr("Delete Failed!");
                showMsg.append(warningMsg);
                CustomMessageBox Msg(CustomMessageBox::Warning,tr("Warning"),showMsg,CustomMessageBox::NoButton, this);
                Msg.addButton(tr("Close"), CustomMessageBox::NoRole);
                Msg.exec();
                break;
            }

        }
    }
    if(m_allPrintersVector.size() != 0)
    {
        m_currentSelectedPrinter = m_allPrintersVector.at(m_allPrintersVector.size() - 1);
    }
    emit updateListSector(m_allPrintersVector,m_currentSelectedPrinter);
    emit getCurFontSize();
}

void MainWindow::ppdDialogShow(const QString &str)
{
    m_pDebInstallWindow = new DebInstallWindow(this);

    connect(m_pDebInstallWindow,SIGNAL(modifyPPDName(QString)),this,SLOT(modifyPrinterPPD(QString)));
    connect(m_pDebInstallWindow,SIGNAL(getCurFontSize()),this,SIGNAL(getCurFontSize()));
    m_pDebInstallWindow->onShow(str);
}
void MainWindow::updateByChangePPD(QString ppdName)
{
    modifyPrinterPPD(ppdName);
    emit getCurFontSize();
}
void MainWindow::modifyPrinterPPD(QString ppd)
{
    bool ret = false;
    QString errorString;
    ret = g_printerManger.addPrinter(QString()
                                     ,m_currentSelectedPrinter.name,ppd
                                     ,m_currentSelectedPrinter.location, errorString);
    if (ret == false) {
        //弹出一个提示窗口，然后返回(安装失败，请重试)
        CustomMessageBox Msg(CustomMessageBox::Critical,tr("Warning"),tr("Add printer failed，please retry after a while."),CustomMessageBox::NoButton, this);
        Msg.addButton(tr("Close"), CustomMessageBox::YesRole);
        Msg.exec();
        emit showPPDSelectDialog();
        return;
    }
    updateMainWindowByAdd(m_currentSelectedPrinter.name);
    emit getCurFontSize();
    return;
}
void MainWindow::updateCurrentSelectedPrinter(const UkuiUsbPrinter&printer)
{
    m_currentSelectedPrinter = printer;
}
void MainWindow::propertyDialogShow(const UkuiUsbPrinter& currentPrinter)
{
    if (m_isProcessing) {
        return; // 如果正在处理，直接返回
    }

    m_isProcessing = true;

    m_allPrintersVector.clear();
    g_printerManger.enumPrinters(m_allPrintersVector);

    for(auto printer : m_allPrintersVector)
    {
        if(printer.name == currentPrinter.name)
        {
            m_currentSelectedPrinter = printer;
        }
    }

    m_propertyDialog = new PropertyWindow(m_currentSelectedPrinter,this);
    connect(m_propertyDialog, &PropertyWindow::destroyed, this, [=](){
        m_propertyDialog = nullptr;
    });
    m_propertyDialog->onShow();

    m_isProcessing = false;
}
void MainWindow::jobListDialogShow(const QString& currentPrinterName)
{
    m_allPrintersVector.clear();
    g_printerManger.enumPrinters(m_allPrintersVector);

    for(auto printer : m_allPrintersVector)
    {
        if(printer.name == currentPrinterName)
        {
            m_currentSelectedPrinter = printer;
        }
    }

    m_jobListDialog = JobManagerWindow::getInstance(m_currentSelectedPrinter.name,UkuiQueueStatus::UKUI_PRINTER_QUEUE_ALL,this);
    m_jobListDialog->showThis();
}
void MainWindow::printTestPage(const UkuiUsbPrinter& printer)
{
    //TODO:打印测试页要传出的参数，调用哪个接口
    qDebug() <<"打印机" << printer.name << "打印测试页！";

    bool res = false;
    res = g_printerManger.printTestPage(printer.name);
    if(res)
    {
        WaitingDialog dialog(tr("Try to connect the Printer..."), 5000, this);
        dialog.exec();
    }
    else
    {
        CustomMessageBox Msg(CustomMessageBox::Critical,tr("Error"),tr("Open Printer Failed,Please Del And Add Printer, Then Try Again!"),CustomMessageBox::NoButton, this);
        Msg.addButton(tr("Sure"), CustomMessageBox::YesRole);
        Msg.exec();
    }
    emit updateRightWidget(printer);
}

void MainWindow::troubleshooting(const UkuiUsbPrinter&printer)
{
    TroubleshootingDialog *d = new TroubleshootingDialog(printer, this);
    d->onShow();
}

void MainWindow::updateImmediately()
{
    m_allPrintersVector.clear();
    g_printerManger.enumPrinters(m_allPrintersVector);
    emit updateListSector(m_allPrintersVector,m_currentSelectedPrinter);
}

void MainWindow::updateByRename(QString printerName)
{
    m_allPrintersVector.clear();
    g_printerManger.enumPrinters(m_allPrintersVector);
    for (auto &cur : m_allPrintersVector)
    {
        if(cur.name == printerName)
        {
            m_currentSelectedPrinter = cur;
        }
    }
    emit addPrinterToListSector(m_currentSelectedPrinter);
    updateImmediately();
    emit getCurFontSize();
}

MainWindow *MainWindow::getInstance()
{
    static MainWindow *mainWinInstance = nullptr;
    if (nullptr == mainWinInstance){
        mainWinInstance = new MainWindow();
    }

    return mainWinInstance;
}

void MainWindow::setQSSFontSize(QFont currentFont)
{

}

/************************************************
* 函数名称：keyPressEvent
* 功能描述：F1键打开用户手册对应页
* 输入参数：QKeyEvent *event键盘事件
* 输出参数：
* 修改日期：2021.07.03
* 修改内容：
*   创建  HZH
*
*************************************************/
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_F1)
    {
        DaemonIpcDbus *ipcDbus = DaemonIpcDbus::getInstance();
        if(!ipcDbus->daemonIsNotRunning()){
            ipcDbus->showGuide("kylin-printer");
        }
    }
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    return CustomWidget::eventFilter(watched, event);
}

void MainWindow::enableMaximum()
{
    sideBar()->setMaximumHeight(QWIDGETSIZE_MAX);
    baseBar()->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    setWindowState(Qt::WindowMaximized);
    this->layout()->setSizeConstraint(QLayout::SizeConstraint::SetMaximumSize);
    if (isHidden() == false) {
        hide();
        show();
    }
}

void MainWindow::disableMaximum()
{
    sideBar()->setFixedSize(300,560);
    baseBar()->setMaximumSize(baseBar()->minimumSize());
    this->layout()->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    setWindowState(Qt::WindowNoState);
    if (isHidden() == false) {
        hide();
        show();
    }
}

void MainWindow::handlePADMode()
{
    // enableMaximum();
}

void MainWindow::handlePCMode()
{
    // disableMaximum();
}
