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
#include "custom_message_box.h"
#include <QEvent>
#include <QMouseEvent>

#include "find_and_match_ppds.h"
#include "ukui_apt.h"
#include "base_notify_dialog.h"

#include "ukui_printer.h"

#include "network_device_monitor.h"
#include "global_variable.h"
#include "common.h"

#include "custom_tab_bar.h"
#include "custom_message_box.h"
#include "choose_ppd_combo_box.h"
#include "add_printer_window.h"
#include "ui_add_printer_window.h"
#include "information_collector.h"
#include "waiting_dialog.h"
#include "common_ui.h"
#include "lpinfo_browser.h"
#include "package_helper.h"
#include "upload_helper.h"
#include "gsettings_monitor.h"
#include "smbclient_helper.h"
#include "smb_auth_info_dialog.h"
#include "auto_search_result_model.h"
#include "manual_search_result_model.h"
#include "find_ppd_worker.h"
#include "serial_config_helper.h"
#include <driver-acquirer-widget/webdriverinstallwidget.h>

AddPrinterWindow::AddPrinterWindow(QPoint pos, QWidget *parent)
    : CustomDialog(parent, true)
    , ui(new Ui::AddPrinterWindow)
    , m_parentCenterPos(pos)
{
    initUi();
    initConnect();

    // NetworkDeviceMonitor::getInstance()->start();
}

void AddPrinterWindow::initConnect()
{
    connect(this, &AddPrinterWindow::showThis, this, &AddPrinterWindow::onShow);

    connect(ui->bar, &kdk::KTabBar::currentChanged, [=](int index) {
        switch (index) {
        case 0:
            onAutoClicked();
            break;
        case 1:
            onManualClicked();
            break;
        default:
            break;
        }
    });

    connect(m_refreshBtn, &QPushButton::clicked, this, &AddPrinterWindow::onRefreshClicked);

    connect(m_autoTableView, &QTableView::clicked, [=](const QModelIndex &index) {
        DeviceInformation info = m_autoSearchResultModel->getInfo(index);
        onDeviceUpdated(info);
    });

    connect(ui->manualTreeView, &QTreeView::clicked, [=](const QModelIndex &index) {
        DeviceInformation info = m_manualSearchResultModel->getInfo(index);
        onDeviceUpdated(info);
    });

    connect(m_autoTableView, &QTableView::customContextMenuRequested, [=](const QPoint& pos) {
        int row = m_autoTableView->indexAt(pos).row();
        if (row < 0 || m_autoSearchResultModel->rowCount() < row) {
            // clearDeviceComponent();
        }
    });

    connect(ui->manualTreeView, &QTreeView::customContextMenuRequested, [=](const QPoint& pos) {
        int row = ui->manualTreeView->indexAt(pos).row();
        if (row < 0 || m_manualSearchResultModel->rowCount() < row) {
            // clearDeviceComponent();
        }
    });

    connect(m_nameEdit, &QLineEdit::textChanged, this, &AddPrinterWindow::onNameEditChanged);
    connect(m_addrEdit, &QLineEdit::textChanged, this, &AddPrinterWindow::onAddrChanged);
    connect(m_addrEdit, &QLineEdit::returnPressed, this, &AddPrinterWindow::onSearchClicked);
    connect(m_searchBtn, &QPushButton::clicked, this, &AddPrinterWindow::onSearchClicked);
    connect(m_forwardBtn, &QPushButton::clicked, this, &AddPrinterWindow::onForwardClicked);
    connect(m_drvComboBox, &ChoosePpdComboBox::currentPpdChanged, [=](bool isDefault, const QString &ppdName) {
        m_drvComboBoxCheck = !isDefault;
        updateForwardButtonStatus();
    });

    connect(NetworkDeviceMonitor::getInstance(), &NetworkDeviceMonitor::getDeviceFromIP_cb, this, &AddPrinterWindow::handleDeviceFromIP,Qt::UniqueConnection);
}

AddPrinterWindow::~AddPrinterWindow()
{
    delete ui;
}

void AddPrinterWindow::initUi()
{
    ui->setupUi(this->mainWidget());

    m_centralWidget = ui->centralWidget;
    m_manualWidget = ui->manualWidget;
    m_addrEdit = ui->addrEdit;
    m_searchBtn = ui->searchBtn;
    m_protocolLabel = ui->protocolLabel;
    m_protocolComboBox = ui->protocolComboBox;

    m_devList = ui->devList;
    m_loadingPic = ui->loadingPic;
    m_refreshBtn = ui->refreshBtn;
    m_autoWidget = ui->autoWidget;
    m_autoTableView = ui->autoTableView;

    m_nameLabel = ui->nameLabel;
    m_netLabel = ui->netLabel;
    m_locationLabel = ui->locationLabel;
    m_drvLabel = ui->drvLabel;

    m_nameEdit = ui->nameEdit;
    m_locationEdit = ui->locationEdit;
    m_drvComboBox = ui->drvComboBox;

    m_forwardBtn = ui->forwardBtn;

    this->mainWidget()->setFixedSize(560,576);
    this->layout()->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);
    setWindowTitle(tr("Add Printer"));

    initSwitchArea();
    initCentralWidget();
    initDeviceComponents();
    initBottomArea();
}

void AddPrinterWindow::initSwitchArea()
{
    ui->bar->setTabBarStyle(kdk::KTabBarStyle::SegmentDark);
    ui->bar->addTab(tr("Auto"));
    ui->bar->addTab(tr("Manual"));
}

void AddPrinterWindow::initCentralWidget()
{
    initAutoWidget();
    initManualWidget();
}

void AddPrinterWindow::initAutoWidget()
{
    m_devList->setText(tr("Device List"));

    // 设置表头隐藏
    m_autoTableView->horizontalHeader()->setHidden(true);
    m_autoTableView->verticalHeader()->setHidden(true);
    // 隔行变色
    m_autoTableView->setAlternatingRowColors(true);
    // 不显示网格线
    m_autoTableView->setShowGrid(false);
    // 禁用双击编辑
    m_autoTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_autoTableView->setColumnWidth(0, 494);
    m_autoTableView->horizontalHeader()->setDefaultSectionSize(494);
    m_autoTableView->verticalHeader()->setDefaultSectionSize(36);
    // 设置右键触发customContextMenuRequested信号
    m_autoTableView->setContextMenuPolicy(Qt::CustomContextMenu);

    m_autoSearchSortFilterProxyModel = new SortFilterProxyModel(this);
    m_autoSearchResultModel = new AutoSearchResultModel(this);
    m_autoSearchSortFilterProxyModel->setSourceModel(m_autoSearchResultModel);
    m_autoTableView->setModel(m_autoSearchSortFilterProxyModel);

    ui->manualTreeView->header()->setHidden(true);
    m_manualSearchResultModel = new ManualSearchResultModel();
    ui->manualTreeView->setModel(m_manualSearchResultModel);
}

void AddPrinterWindow::initManualWidget()
{
    m_protocolLabel->setText(tr("Protocol"));
    m_netLabel->setText(tr("Address"));
    QRegExp rx_ip("^(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9])\\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\\.(25[0-5]|"
                  "2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[0-9])$");
    m_addrEdit->setValidator(new QRegExpValidator(rx_ip, this));
    m_addrEdit->setClearButtonEnabled(true);

    m_searchBtn->setText(tr("Search"));
    m_searchBtn->setEnabled(false);


    m_protocolComboBox->addItem("socket");
    m_protocolComboBox->addItem("ipp");
    m_protocolComboBox->addItem("http");
    m_protocolComboBox->addItem("smb");
    m_protocolComboBox->addItem("lpd");
    m_protocolComboBox->setCurrentIndex(0);
}

void AddPrinterWindow::initDeviceComponents()
{
    m_nameLabel->setText(tr("name"));
    m_locationLabel->setText(tr("location"));
    m_drvLabel->setText(tr("driver"));
    ui->baudRateLabel->setText(tr("baud rate"));
    ui->baudRateComboBox->addItem(tr("default") + (" (9600)"));
    ui->baudRateComboBox->addItem("1200");
    ui->baudRateComboBox->addItem("1400");
    ui->baudRateComboBox->addItem("4800");
    ui->baudRateComboBox->addItem("9600");
    ui->baudRateComboBox->addItem("19200");
    ui->baudRateComboBox->addItem("38400");
    ui->baudRateComboBox->addItem("57600");
    ui->baudRateComboBox->addItem("115200");
}

void AddPrinterWindow::initBottomArea()
{
    m_forwardBtn->setText(tr("forward"));
}

void AddPrinterWindow::clearDeviceArea()
{
    clearDeviceComponent();
    m_autoSearchResultModel->clearAllPrinter();
    m_manualSearchResultModel->clearAllPrinter();
}

void AddPrinterWindow::clearDeviceComponent()
{
    m_nameEdit->setEnabled(false);
    m_locationEdit->setEnabled(false);
    m_drvComboBox->setEnabled(false);
    m_locationEdit->clear();
    m_nameEdit->clear();
    m_drvComboBox->clear();
    m_deviceInfo = DeviceInformation();
    m_nameEditCheck = false;
    m_drvComboBoxCheck = false;
    updateForwardButtonStatus();
    m_autoTableView->clearSelection();
    hideBaudRate();
}

void AddPrinterWindow::showBaudRate()
{
    this->mainWidget()->setFixedSize(560, 576 + 36 + 8);
    ui->baudRateComboBox->setCurrentIndex(0);
    ui->baudRateWidget->show();
}

void AddPrinterWindow::hideBaudRate()
{
    ui->baudRateWidget->hide();
    this->mainWidget()->setFixedSize(560,576);
}

void AddPrinterWindow::onForwardClicked()
{
    qDebug() << __FUNCTION__;
    if (m_drvComboBox->isEmpty()) {
        // 弹出一个提示窗口，然后返回(安装失败，请重试)
        CustomMessageBox Msg(CustomMessageBox::Critical, tr("Error"), tr("Add printer failed: no PPD selected!"),
                             CustomMessageBox::NoButton, this);
        Msg.exec();
        return;
    }
    m_printers.clear();
    UkuiPrinterManager::getInstance().enumPrinters(m_printers);
    m_nameEditCheck = checkPrinterNameIsLegal(true, m_nameEdit->text());
    updateForwardButtonStatus();
    if (m_nameEditCheck == false) {
        return;
    }

    if (m_deviceInfo.uri.contains("serial:/")) {
        QString baudRate = ui->baudRateComboBox->currentText();
        if (baudRate.contains("(9600)")) {
            baudRate = "9600";
        }
        m_deviceInfo.uri = SerialConfigHelper::modifyBaudrate(m_deviceInfo.uri, baudRate);
    }

    bool ret = false;
    QMap<QString, QString> messages;
    messages.insert("uri", m_deviceInfo.uri);
    messages.insert("pid", m_deviceInfo.PID);
    messages.insert("vid", m_deviceInfo.VID);
    messages.insert("vendor", m_deviceInfo.vendor);
    messages.insert("model", m_deviceInfo.model);
    messages.insert("ppd", m_drvComboBox->currentText());
    QString errorString;
    m_deviceInfo.displayName = m_nameEdit->text();
    m_deviceInfo.userChoosePpdName = m_drvComboBox->currentText();

    if (m_drvComboBox->currentText() == "everywhere") {

        int exitCode;
        QStringList command;
        // lpadmin -p pr -E -v cups-brf:/ -m drv:///sample.drv/zebra.ppd -L aaa
        command << "lpadmin" << "-p" << QString("\"%1\"").arg(m_nameEdit->text()) << "-E" << "-v" << QString("\"%1\"").arg(m_deviceInfo.uri) << "-m" << "everywhere" << "-D" << QString("\"%1 added by kylin-printer.\" ").arg(m_nameEdit->text());

        if (!m_locationEdit->text().isEmpty()) {
            command << "-L" << m_locationEdit->text();
        }
        QString res;
        execCommand(command, nullptr, &res, &exitCode, nullptr);
        qDebug() << res << exitCode;
        if (exitCode == 0) {
            ret = true;
        }
    }
    else {
        ret = UkuiPrinterManager::getInstance().addPrinter(m_deviceInfo.uri, m_nameEdit->text(), m_drvComboBox->currentText(), m_locationEdit->text(), errorString);
    }

    if (ret == false) {
        messages.insert("result", "fail");
        messages.insert("resultString", "add printer failed!");
        messages.insert("cupsErrorString", errorString);
        InformationCollector::getInstance().addMessageMap("add_printer", messages);
        //弹出一个提示窗口，然后返回(安装失败，请重试)
        CustomMessageBox Msg(CustomMessageBox::Critical, tr("Error"), tr("Add printer failed，please retry after a while."),
                             CustomMessageBox::NoButton, this);
        QPushButton *btnYes = Msg.addButton(tr("Confirm"), CustomMessageBox::YesRole);
        Msg.exec();
        return;
    }
    messages.insert("result", "success");
    InformationCollector::getInstance().addMessageMap("add_printer", messages);

    CustomMessageBox msg1(QIcon::fromTheme("ukui-dialog-success"),tr("Hint"), tr("Add printer successfully，printer a test page？"), CustomMessageBox::NoButton, this);
    QPushButton *btnYes = msg1.addButton(tr("Print test page"), CustomMessageBox::YesRole);
    QPushButton *btnNo  = msg1.addButton(tr("Confirm"), CustomMessageBox::NoRole);
    msg1.exec();
    if ((QPushButton*)msg1.clickedButton() == btnYes) {
        // TODO:打开打印测试页窗口
        UkuiPrinterManager::getInstance().printTestPage(m_nameEdit->text());
        CustomMessageBox msg2(CustomMessageBox::Question,tr("Hint"), tr("Is the test page printed successfully?"), CustomMessageBox::NoButton, this);
        QPushButton *btnYes = msg2.addButton(tr("Success"), CustomMessageBox::YesRole);
        QPushButton *btnNo  = msg2.addButton(tr("Fail"), CustomMessageBox::NoRole);
        msg2.exec();
        if ((QPushButton*)msg2.clickedButton() == btnYes) {
            // TODO 通知主窗体显示这台打印机？
            UploadHelper *u = new UploadHelper(m_deviceInfo, 1);
            u->startWorker();
            emit needUpdateDests(m_nameEdit->text());
            close();
        } else if ((QPushButton*)msg2.clickedButton() == btnNo) {
            CustomMessageBox msg3(CustomMessageBox::Warning,tr("Hint"), tr("Test print failed.Please check whether the printer is connected or modify the driver and try again."), CustomMessageBox::NoButton, this);
            QPushButton *btnYes = msg3.addButton(tr("Confirm"), CustomMessageBox::YesRole);
            msg3.exec();
            if ((QPushButton*)msg3.clickedButton() == btnNo) {
                // TODO:删除当前打印机
                // UkuiPrinterManager::getInstance().removePrinter(m_nameEdit->text());
                close();
            } else if ((QPushButton*)msg3.clickedButton() == btnYes) {
                // TODO:确定显示到哪个页面？
                return;
            }
        }

    } else if ((QPushButton*)msg1.clickedButton() == btnNo) {
        // TODO:发送信号，通知主窗体显示这次添加的打印机
        emit needUpdateDests(m_nameEdit->text());
        close();
    }
}

void AddPrinterWindow::onSearchClicked()
{
    qDebug() << __FUNCTION__ << m_addrEdit->text();
    QString ipAddr = m_addrEdit->text();
    m_manualSearchResultModel->clearAllPrinter();

    QPoint windowCenter = QPoint(this->mapToGlobal(QPoint(0,0)).x() + this->width()/2,this->mapToGlobal(QPoint(0,0)).y() + this->height()/2);

    WaitingDialog dialog(this);
    dialog.hide();
    dialog.labelSetText(tr("Searching printers..."));
    dialog.show();

    dialog.move(windowCenter - dialog.rect().center());

    connect(NetworkDeviceMonitor::getInstance(), &NetworkDeviceMonitor::getDeviceFromIP_cb, &dialog, &QDialog::close);
    

    ProtocolType type;

    switch (m_protocolComboBox->currentIndex())
    {
    case 0:
        type = ProtocolType::SOCKET;
        break;
    case 1:
        type = ProtocolType::IPP;
        break;
    case 2:
        type = ProtocolType::HTTP;
        break;
    case 3:
        type = ProtocolType::SMB;
        break;
    case 4:
        type = ProtocolType::LPD;
        break;
    default:
        break;
    }
    SMBAuthData data;
    int smbStatus = 0;
    QString smbErrMsg{QString()};

    switch(type)
    {
        case ProtocolType::SMB:{
            data.host = ipAddr;
            SmbClientHelper *smbHelper = new SmbClientHelper(data);
            connect(smbHelper, &SmbClientHelper::finished, &dialog, &QDialog::close);
            connect(smbHelper, &SmbClientHelper::succeed, [=, &smbStatus, &smbErrMsg](int status, QString errorMsg) {
                smbStatus = status;
                smbErrMsg = errorMsg;
                smbHelper->finished();
            });

            smbHelper->startWorker();
            break;
        }
            
        default:
        emit NetworkDeviceMonitor::getInstance()->getDeviceFromIP_async(ipAddr, 9100, type);
        break;
    }
    
    dialog.exec();
    if (type == ProtocolType::SMB) {
        if (smbStatus == 0) {
            for (auto smb : data.smbPrinterList) {
                handleDeviceFromIP(smb);
            }
        } else if (smbStatus == -2){
            SmbAuthInfoDialog authDia(data, this);
            int res = authDia.exec();
            if (res == QDialog::Accepted) {
                for (auto smb : data.smbPrinterList) {
                    handleDeviceFromIP(smb);
                }
            }
        } else {
            CustomMessageBox Msg(CustomMessageBox::Warning, tr("Warning"), smbErrMsg, CustomMessageBox::NoButton, this);
            QPushButton *btnYes = Msg.addButton(tr("Confirm"), CustomMessageBox::YesRole);
            Msg.exec();
        }
    }
}

void AddPrinterWindow::handleDeviceFromIP(DeviceInformation &info)
{
    if (info.deviceType == DeviceType::NONE) {
        CustomMessageBox Msg(CustomMessageBox::Warning, tr("Warning"), tr("Can not find this Printer!"), CustomMessageBox::NoButton, this);
        QPushButton *btnYes = Msg.addButton(tr("Confirm"), CustomMessageBox::YesRole);
        Msg.exec();
        return ;
    }

    onManualDeviceAdd(info);
    return ;
}

void AddPrinterWindow::onRefreshClicked()
{
    if (usbThread != nullptr) {
        usbThread->blockSignals(true);
        usbThread->finished();
        usbThread = nullptr;
    }
    m_refreshBtn->refreshStart();
    clearDeviceArea();
    usbThread = new getUsbPrinterThread();
    connect(usbThread, &getUsbPrinterThread::recvUsbPrinter, this, &AddPrinterWindow::onAutoDeviceAdd);
    connect(usbThread, &getUsbPrinterThread::finished, [=]() {
        m_refreshBtn->refreshFinish();
        usbThread = nullptr;
    });

    usbThread->startWorker();
} 

void AddPrinterWindow::onAutoClicked()
{
    m_centralWidget->setCurrentWidget(m_autoWidget);
    clearDeviceArea();
    onRefreshClicked();
}

void AddPrinterWindow::onManualClicked()
{
    m_centralWidget->setCurrentWidget(m_manualWidget);
    clearDeviceArea();
}

void AddPrinterWindow::onAutoDeviceAdd(DeviceInformation& device)
{        
    m_autoSearchResultModel->addPrinter(device);
    m_autoSearchSortFilterProxyModel->sort(0, Qt::AscendingOrder);
}

void AddPrinterWindow::onManualDeviceAdd(DeviceInformation& device)
{
    qDebug() << device;
    qDebug() << "IN";

    m_manualSearchResultModel->addPrinter(device);
}

void AddPrinterWindow::onDeviceUpdated(DeviceInformation& info)
{
    clearDeviceComponent();
    if (info.uri.contains("serial:/")) {
        showBaudRate();
    }

    QString desc = getPrinterNameFromDeviceInformation(info);
    
    m_printers.clear();
    UkuiPrinterManager::getInstance().enumPrinters(m_printers);

    int nameCount=1;
    QString textDesc(desc);
    while(1) {
        auto res = UkuiPrinterManager::getInstance().printerNameCheck(textDesc, false, m_printers);

        if (res.status == CommonStatusFlag::ERROR_EXIST_SAME_PRINTER) {
            textDesc = QString("%1-%2").arg(desc).arg(nameCount);
            nameCount++;
        } else {
            break ;
        }
    }
    desc = textDesc;

    m_nameEdit->setEnabled(true);
    m_locationEdit->setEnabled(true);
    m_drvComboBox->setEnabled(true);
    m_nameEdit->setText(desc);
    m_nameEditCheck = checkPrinterNameIsLegal(false, m_nameEdit->text());
    updateForwardButtonStatus();

    m_locationEdit->setText("");
    m_drvComboBox->clear();
    m_deviceInfo = info;

    if (info.deviceType == DeviceType::DRIVERLESS) {
        m_drvComboBox->addItem("everywhere");
        return ;
    }

    if (info.uri == "cups-brf:/") {
        m_drvComboBox->addItem("drv:///indexbraille.drv/4WavPRO.ppd");
        m_drvComboBox->addItem("drv:///indexbraille.drv/4x4PRO.ppd");
        m_drvComboBox->addItem("drv:///indexbraille.drv/BasicD.ppd");
        m_drvComboBox->addItem("drv:///indexbraille.drv/BasicS.ppd");
        m_drvComboBox->addItem("drv:///indexbraille.drv/Everest.ppd");
        return ;
    }

    if (info.uri == "cups-pdf:/") {
        m_drvComboBox->addItem("lsb/usr/cups-pdf/CUPS-PDF_noopt.ppd");
        return ;
    }

    if (info.vendor.isEmpty() && info.model.isEmpty()) {
        CustomMessageBox Msg1(CustomMessageBox::Warning, tr("Warning"), tr("Automatic driver configuration failed, please select the driver manually."), CustomMessageBox::NoButton, this);
        QPushButton *btnYes = Msg1.addButton(tr("Confirm"), CustomMessageBox::YesRole);
        Msg1.exec();
        if ((QPushButton *)Msg1.clickedButton() == btnYes) {
        }
        else {
        }
        return ;
    }

    //自己画一个窗口，
    m_installOk = 1;
    QPoint windowCenter = QPoint(this->mapToGlobal(QPoint(0,0)).x() + this->width()/2,this->mapToGlobal(QPoint(0,0)).y() + this->height()/2);

    BaseNotifyDialog dialog(this);
    dialog.hide();
    dialog.dialogSetType(BaseNotifyDialogType::WAITTING);
    // dialog.labelSetText(tr("正在搜索驱动..."));
    dialog.labelSetText(tr("Searching for driver..."));
    dialog.show();
    dialog.move(windowCenter - dialog.rect().center());


    PackageHelper* pkgHelper = new PackageHelper(m_deviceInfo);
    m_needInstall = false;
    m_pkgHelperFlag = false;
    connect(pkgHelper, &PackageHelper::finished, &dialog, &QDialog::close);
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
    dialog.exec();

    if (!m_pkgHelperFlag) {
        CustomMessageBox Msg1(CustomMessageBox::Warning, tr("Warning"), tr("Failed to search for driver. Do you want to use local driver?"), CustomMessageBox::NoButton, this);
        QPushButton *btnYes = Msg1.addButton(tr("Confirm"), CustomMessageBox::YesRole);
        QPushButton *btnNo  = Msg1.addButton(tr("Cancel"), CustomMessageBox::NoRole);
        Msg1.exec();
        if ((QPushButton *)Msg1.clickedButton() == btnYes) {
        }
        else {
            m_autoTableView->clearSelection();
            clearDeviceComponent();
            return;
        }
    }

    if (m_needInstall == true && !m_deviceInfo.packageLevel.isEmpty() && m_deviceInfo.packageLevel != "1") {

        QString tipStr;
        if (m_deviceInfo.packageLevel == "2") {
            // tipStr = "检测到驱动程序来源于互联网，是否继续安装？";
            tipStr = tr("It is detected that the driver comes from the Internet. Do you want to continue the installation?");
            CustomMessageBox Msg(CustomMessageBox::Warning, tr("Warning"), tipStr, CustomMessageBox::NoButton, this);
            QPushButton *btnYes = Msg.addButton(tr("Install"), CustomMessageBox::YesRole);
            QPushButton *btnNo  = Msg.addButton(tr("Cancel"), CustomMessageBox::NoRole);
            Msg.exec();
            if ((QPushButton*)Msg.clickedButton() == btnYes) {

            } else {
                m_autoTableView->clearSelection();
                clearDeviceComponent();
                return;
            }
        } else {
            m_needInstall = false;
            if (m_deviceInfo.packageLevel == "3") {
                // tipStr = "该设备型号驱动适配中。";
                tipStr = tr("The device driver is being adapted.");
            } else {
                // tipStr = "由于商业协议限制，请联系设备制造商以获取该设备型号驱动程序。";
                tipStr = tr("Due to commercial agreement restrictions, please contact the device manufacturer to obtain drivers for this device model.");
            }
            CustomMessageBox Msg(
                CustomMessageBox::Warning, tr("Warning"), tipStr, CustomMessageBox::NoButton, this);

            QPushButton *btnYes = Msg.addButton(tr("Confirm"), CustomMessageBox::YesRole);
            Msg.exec();
            return;
        }
    }

    if (m_needInstall) {
        m_installOk = -1;
        BaseNotifyDialog dialog2(this);
        dialog2.hide();
        dialog2.dialogSetType(BaseNotifyDialogType::WAITTING);
        dialog2.labelSetText(tr("Installing printer driver..."));
        dialog2.show();
        dialog2.move(windowCenter - dialog2.rect().center());
        connect(this, &AddPrinterWindow::installEnd, &dialog2, &BaseNotifyDialog::close);
        AptUtilHelper *aptHelper = new AptUtilHelper(m_deviceInfo);
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
        int res = dialog2.exec();
        if (m_installOk == -1) {
            aptHelper->canceled();
        }
        aptHelper->finished();
        qDebug() << res;
        if (m_installOk == 1) {
            if (m_deviceInfo.digitId.size()) {
                QStringList digitId;
                qDebug() << "弹窗前" << m_deviceInfo.digitId;
                if(m_deviceInfo.digitId.contains(",")){
                    digitId = m_deviceInfo.digitId.split(",");
                }else{
                    digitId.append(m_deviceInfo.digitId); 
                }

                WebDriverInstallWidget webdriverwidget(digitId, this);
                webdriverwidget.exec();
                
                bool check = packageListNeedInstall(m_deviceInfo.packageInfoList);
                QString msg;
                if (check) {
                    msg = tr("No corresponding driver was detected. Do you want to use the local driver?");
                } else {
                    msg = tr("The corresponding driver has been detected. Do you want to continue installing the printer?");
                    m_installOk = 0;
                }
                {
                CustomMessageBox Msg2(CustomMessageBox::Warning, tr("Warning"), msg, CustomMessageBox::NoButton, this);
                QPushButton *btnYes = Msg2.addButton(tr("Confirm"), CustomMessageBox::YesRole);
                QPushButton *btnNo  = Msg2.addButton(tr("Cancel"), CustomMessageBox::NoRole);
                Msg2.exec();
                if ((QPushButton *)Msg2.clickedButton() == btnYes) {
                }
                else {
                    m_autoTableView->clearSelection();
                    clearDeviceComponent();
                    return;
                }
                }

            } else {
                CustomMessageBox Msg1(CustomMessageBox::Warning, tr("Warning"), tr("Driver installation failed. Do you want to use local driver?"), CustomMessageBox::NoButton, this);
                QPushButton *btnYes = Msg1.addButton(tr("Confirm"), CustomMessageBox::YesRole);
                QPushButton *btnNo  = Msg1.addButton(tr("Cancel"), CustomMessageBox::NoRole);
                Msg1.exec();
                if ((QPushButton *)Msg1.clickedButton() == btnYes) {
                }
                else {
                    m_autoTableView->clearSelection();
                    clearDeviceComponent();
                    return;
                }
            }
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

    worker->setInput(m_deviceInfo.vendor, m_deviceInfo.model, m_deviceInfo.suggestPpdName);


    BaseNotifyDialog dialog3(this);
    dialog3.hide();
    dialog3.dialogSetType(BaseNotifyDialogType::WAITTING);
    dialog3.labelSetText(tr("Searching for driver..."));
    dialog3.show();
    dialog3.move(windowCenter - dialog3.rect().center());
    connect(worker, &FindPpdWorker::finished, &dialog3, &QDialog::close);
    connect(worker, &FindPpdWorker::result, [=, &ret, &ppd, &ppdList](int res, QString exactMatchPPD, QStringList matchResultList) {
        ret = res;
        ppd = exactMatchPPD;
        ppdList = matchResultList;
        worker->finished();
    });

    worker->startWorker();
    dialog3.exec();

    if (ret != 0) {
        qDebug() << ret;
    }

    if (ppd.compare("") != 0) {
        qDebug("完全匹配");
        m_drvComboBox->addItem(ppd);
    } else {
        QStringList ppds = ppdList;
        qDebug() << ppds.size();
        if (ppds.size() == 0) {
            return;
        }
        m_drvComboBox->addItems(ppds);
    }
}

void AddPrinterWindow::onAddrChanged(const QString& ipAddr)
{
    qDebug() << ipAddr;
    m_searchBtn->setEnabled(ipIsValid(ipAddr));
}

void AddPrinterWindow::onNameEditChanged() {
    m_nameEditCheck = checkPrinterNameIsLegal(false, m_nameEdit->text());
    updateForwardButtonStatus();
}

bool AddPrinterWindow::checkPrinterNameIsLegal(bool hasMessageBox, const QString &printerName)
{
    if (printerName == "") {
        return false;
    }

    auto res = UkuiPrinterManager::getInstance().printerNameCheck(printerName, false, m_printers);

    switch (res.status)
    {
    case (CommonStatusFlag::SUCCESS):
        return true;
    case (CommonStatusFlag::ERROR_EXIST_SAME_PRINTER):
    case (CommonStatusFlag::FAIL):{
        if (hasMessageBox == true) {
            QString warningMsg = res.message;
            CustomMessageBox Msg(CustomMessageBox::Warning, tr("Warning"), warningMsg, CustomMessageBox::NoButton, this);
            Msg.addButton(tr("Close"), CustomMessageBox::NoRole);
            Msg.exec();
        break;
        }
    }
    default:
        qDebug() << "AddPrinterWindow::checkPrinterNameIsLegal :undefined error!";
        break;
    }

    return false;
}



void AddPrinterWindow::onShow(QPoint pos)
{
    m_nameEdit->setEnabled(false);
    m_locationEdit->setEnabled(false);
    m_drvComboBox->setEnabled(false);
    m_parentCenterPos = pos;

    qDebug() << "Set addPrinterWindow to toplevel.";
    move(pos - rect().center());
    if (!isActiveWindow()) {
        hide();
        show();
    }
    show();
    raise();
    activateWindow();
    // Clear all other area... and then refresh all
    m_addrEdit->clear();
    ui->bar->setCurrentIndex(0);
    onRefreshClicked();
    hideBaudRate();
}

void AddPrinterWindow::setQSSFontSize(QFont curFont)
{

}

bool AddPrinterWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_autoTableView) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            QPoint p = mouseEvent->pos();
            qDebug() << p;
            QModelIndex index = m_autoTableView->indexAt(p);
            if (!index.isValid()) {
                // clearDeviceComponent();
            } else {
                qDebug() << index.row();
            }
        }
    }
    return CustomDialog::eventFilter(watched, event);
}

void AddPrinterWindow::updateForwardButtonStatus()
{
    if (m_nameEditCheck == true && m_drvComboBoxCheck == true) {
        m_forwardBtn->setEnabled(true);
    } else {
        m_forwardBtn->setEnabled(false);
    }
}

getUsbPrinterThread::getUsbPrinterThread(QObject* parent)
    : QObject(parent)
{
    m_thread = new QThread;
    moveToThread(m_thread);
    connect(m_thread, &QThread::started, this, &getUsbPrinterThread::startSearch);
    connect(this, &getUsbPrinterThread::finished, m_thread, &QThread::quit);
    connect(this, &getUsbPrinterThread::finished, this, &getUsbPrinterThread::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

void getUsbPrinterThread::startSearch()
{
    QMap<QString, DeviceInformation> prts = LpinfoBrowser::findDirectPrinter();
    QList<UkuiUsbPrinter> printers;
    UkuiPrinterManager::getInstance().enumUsbPrinters(printers);
    foreach (auto prt, prts) {
        if (prt.deviceType == DeviceType::USB) {
            if (UkuiPrinterManager::getInstance().printerIsInstalled(prt.uri, false, printers))
                continue;
        }
        emit recvUsbPrinter(prt);
    }
    prts = LpinfoBrowser::findNetWorkPrinter();
    foreach (auto prt, prts) {
        if (prt.deviceType == DeviceType::USB) {
            if (UkuiPrinterManager::getInstance().printerIsInstalled(prt.uri, false, printers))
                continue;
        }
        emit recvUsbPrinter(prt);
    }
    emit finished();
}
