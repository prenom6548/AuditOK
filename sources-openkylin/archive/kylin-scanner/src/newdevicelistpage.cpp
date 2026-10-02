/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
 */
#include "newdevicelistpage.h"
#include "Qt/windowmanage.hpp"
#include <QHeaderView>
#include "buriedpoint.hpp"
#include <libkydatacollect.h>
#include "saneobject.h"

newDeviceListPage::newDeviceListPage(QList<DeviceInformation> devInfoList, QWidget *parent)
    : QDialog(parent),
      m_VLayout(new QVBoxLayout()),
      m_titleHLayout(new QHBoxLayout()),
      m_closeButton(new QPushButton()),
      m_mainWidget(new QStackedWidget()),
      m_mainVLayout(new QVBoxLayout()),
      m_mainVLayout2(new QVBoxLayout()),
      m_mainVWidget1(new QWidget()),
      m_mainVWidget2(new QWidget()),
      m_labelbuttonLayout(new QHBoxLayout()),
      m_deviceLabel(new QLabel()),
      m_reDetectButton(new QPushButton()),
      m_stackDeviceList(new QStackedWidget()),
      m_noDeviceWidget(new NoDeviceWidget()),
      m_buttonHLayout(new QHBoxLayout()),
      m_cancelButton(new QPushButton()),
      m_nextStepButton(new QPushButton()),
      m_selectedDeviceName(new QLabel()),
      m_selectedDriverName(new CustomLabel()),
      m_cancelButton2(new QPushButton()),
      m_beforeButton(new QPushButton()),
      m_installButton(new QPushButton()),
      m_buttonHLayout2(new QHBoxLayout()),
      m_devInfoList(devInfoList)
{
    WindowManage::removeHeader(this);
    this->setWindowModality(Qt::ApplicationModal);

    QList<QString> header = {tr("Name"), tr("Symbol"), tr("Type")};
    m_deviceList = new CustomTableWidget(header, this);
    init();
    initConnect();
    handleData();
    this->show();
    for (int i = 0; i < devInfoList.length(); ++i)
    {
        if (devInfoList.at(i).type == DeviceType::PRINTER)
        {
            QMessageBox::information(this, tr("Alert"), tr("You need to manually confirm whether the printer devices included in the list are scanning and printing all-in-one machines. If they are not scanning and printing all-in-one machines, ignore them."));
            break;
        }
    }
}

void newDeviceListPage::init()
{
    m_closeButton->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_closeButton->setToolTip(tr("Close"));
    m_closeButton->setFixedSize(30, 30);
    m_closeButton->setIconSize(QSize(16, 16));
    m_closeButton->setProperty("isWindowButton", 0x2);
    m_closeButton->setProperty("useIconHighlightEffect", 0x8);
    m_closeButton->setFlat(true);

    m_titleHLayout->setSpacing(0);
    m_titleHLayout->addStretch();
    m_titleHLayout->addWidget(m_closeButton);
    m_titleHLayout->setContentsMargins(0, 4, 4, 4);
    m_titleHLayout->setAlignment(Qt::AlignCenter);

    m_deviceLabel->setText(tr("Device List"));
    m_reDetectButton->setFixedSize(24, 24);
    m_reDetectButton->setIcon(QIcon::fromTheme("view-refresh-symbolic"));
    m_reDetectButton->setIconSize(QSize(16, 16));

    m_labelbuttonLayout->addWidget(m_deviceLabel);
    m_labelbuttonLayout->addStretch();
    m_labelbuttonLayout->addWidget(m_reDetectButton);

    m_stackDeviceList->addWidget(m_noDeviceWidget);
    m_stackDeviceList->addWidget(m_deviceList);
    m_stackDeviceList->setCurrentIndex(0);

    m_cancelButton->setText(tr("Cancel"));
    m_nextStepButton->setText(tr("Next"));
    m_nextStepButton->setEnabled(false);

    m_buttonHLayout->addStretch();
    m_buttonHLayout->addWidget(m_cancelButton);
    m_buttonHLayout->addSpacing(10);
    m_buttonHLayout->addWidget(m_nextStepButton);

    m_mainVLayout->setSpacing(0);
    m_mainVLayout->addLayout(m_labelbuttonLayout);
    m_mainVLayout->addSpacing(7);
    m_mainVLayout->addWidget(m_stackDeviceList);
    m_mainVLayout->addSpacing(24);
    m_mainVLayout->addLayout(m_buttonHLayout);
    m_mainVLayout->setContentsMargins(24, 0, 24, 24);
    m_mainVWidget1->setLayout(m_mainVLayout);

    m_mainWidget->addWidget(m_mainVWidget1);

    m_cancelButton2->setText(tr("Cancel"));
    m_beforeButton->setText(tr("Before"));
    m_installButton->setText(tr("Install"));

    m_buttonHLayout2->addStretch();
    m_buttonHLayout2->addWidget(m_cancelButton2);
    m_buttonHLayout2->addSpacing(10);
    m_buttonHLayout2->addWidget(m_beforeButton);
    m_buttonHLayout2->addSpacing(10);
    m_buttonHLayout2->addWidget(m_installButton);

    m_mainVLayout2->setSpacing(0);
    m_mainVLayout2->addWidget(m_selectedDeviceName);
    m_mainVLayout2->addSpacing(12);
    m_mainVLayout2->addWidget(m_selectedDriverName);
    m_mainVLayout2->addSpacing(22);
    m_mainVLayout2->addLayout(m_buttonHLayout2);
    m_mainVLayout->setContentsMargins(24, 0, 24, 24);

    m_mainVWidget2->setLayout(m_mainVLayout2);
    m_mainWidget->addWidget(m_mainVWidget2);
    m_mainWidget->setCurrentIndex(0);

    m_VLayout->addLayout(m_titleHLayout);
    m_VLayout->addSpacing(6);
    m_VLayout->addWidget(m_mainWidget);

    this->setLayout(m_VLayout);
    this->setFixedSize(560, 420);
    if (m_nextStepButton->isEnabled())
    {
        m_nextStepButton->setFocus();
        m_nextStepButton->setDefault(true);
    }
    else
    {
        m_cancelButton->setFocus();
        m_cancelButton->setDefault(true);
    }
}

void newDeviceListPage::initConnect()
{
    connect(m_cancelButton, &QPushButton::clicked, this, &newDeviceListPage::close);
    connect(m_cancelButton, &QPushButton::clicked, this, &newDeviceListPage::deleteLater);
    connect(m_closeButton, &QPushButton::clicked, m_cancelButton, &QPushButton::click);
    connect(m_cancelButton2, &QPushButton::clicked, this, &newDeviceListPage::close);
    connect(m_cancelButton2, &QPushButton::clicked, this, &newDeviceListPage::deleteLater);
    connect(m_deviceList, &CustomTableWidget::itemSelectionChanged, this, [=]()
            { m_nextStepButton->setEnabled(true); });
    connect(m_beforeButton, &QPushButton::clicked, this, [=]()
            {
        m_mainWidget->setCurrentIndex(0);
        this->setFixedSize(560, 420);

        QWidget *widget = nullptr;
        QWidgetList widgetList = QApplication::allWidgets();
        for (int i=0; i<widgetList.length(); ++i) {
            if (widgetList.at(i)->objectName() == "MainWindow") {
                widget = widgetList.at(i);
            }
        }
        if (widget) {
            QRect rect = widget->geometry();
            int x = rect.x() + rect.width() / 2 - this->width() / 2;
            int y = rect.y() + rect.height() / 2 - this->height() / 2;
            this->move(x, y);
        } });
    connect(m_nextStepButton, &QPushButton::clicked, this, &newDeviceListPage::nextButtonClickSlot);
    connect(m_installButton, &QPushButton::clicked, this, &newDeviceListPage::installDriverSlot);
    connect(m_reDetectButton, &QPushButton::clicked, this, &newDeviceListPage::reDetectDevicesSlot);
}

void newDeviceListPage::handleData()
{
    for (int i = 0; i < m_devInfoList.length(); ++i)
    {
        QString name = m_devInfoList.at(i).vendor + " " + m_devInfoList.at(i).model;
        QString symbol;
        QString type;
        QString lever = QString::number(m_devInfoList.at(i).packageLevel);
        if (m_devInfoList.at(i).type == DeviceType::SCANNER)
        {
            type = tr("Scanner");
        }
        else if (m_devInfoList.at(i).type == DeviceType::PRINTER)
        {
            type = tr("Printer");
        }
        if (m_devInfoList.at(i).serial != "")
        {
            symbol = m_devInfoList.at(i).serial;
        }
        else
        {
            symbol = m_devInfoList.at(i).host;
        }
        m_nameToLever.insert(name, lever);
        QStringList content = QStringList() << name << symbol << type;
        m_deviceList->insertItem(i, content);
        m_stackDeviceList->setCurrentIndex(1);
    }
}

void newDeviceListPage::nextButtonClickSlot()
{
    QStringList names = m_deviceList->getSelectedRow();
    QString devName = QString(tr("Device Name:")) + names.at(0);
    QString lever = m_nameToLever.value(names.at(0));
    m_selectedDeviceName->setText(devName);
    bool hasDriver = false;
    int index;
    QString driverName = QString(tr("Driver Name:"));
    // 判断该设备在云端是否有驱动可以自动安装
    for (int i = 0; i < m_devInfoList.length(); ++i)
    {
        if (names.at(1) == m_devInfoList.at(i).serial || names.at(1) == m_devInfoList.at(i).host)
        {
            index = i;
            if (m_devInfoList.at(i).packageNameList.length() == 0)
            {
                break;
            }
            if (m_devInfoList.at(i).packageNameList.at(0) != "")
            {
                hasDriver = m_devInfoList.at(i).installable;
                m_waitForInstallOne = m_devInfoList.at(i);
                for (int j = 0; j < m_devInfoList.at(i).packageNameList.length(); j++)
                {
                    driverName += m_devInfoList.at(i).packageNameList.at(j) + "; ";
                }
                break;
            }
        }
    }
    // 运行驱动寻找程序
    if (hasDriver)
    {
        QMap<QString, QString> buried_data;
        buried_data.insert("functionName", QString::fromStdString(kabase::BuriedPoint::convertPTtoString(kabase::BuriedPoint::PT::KylinScannerDriverCanAutoInstall)));
        buried_data.insert("action", "has driver, auto install");
        buried_data.insert("function", "in newdevicelistpage.cpp function nextButtonClickSlot()");

        int cursor{0};
        KCustomProperty property[buried_data.size()];
        for (auto iter = buried_data.begin(); iter != buried_data.end(); iter++)
        {
            property[cursor].key = strdup(iter.key().toLocal8Bit().data());
            property[cursor].value = strdup(iter.value().toLocal8Bit().data());
            cursor++;
        }
        KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
        kdk_dia_append_custom_property(node, property, buried_data.size());
        kdk_dia_upload_default(node, "driver_can_auto_install", const_cast<char *>("mainPage"));
        kdk_dia_data_free(node);

        // 判断是不是外来驱动
        if (lever.toInt() == 2)
        {
            QMessageBox::information(this, tr("Alert"), tr("This driver is from third party, may cause some unmetable result."));
        }

        if (lever.toInt() == 3)
        {
            QMessageBox::information(this, tr("Alert"), tr("This driver is provided by the manufacturer. Please contact the corresponding manufacturer of the device to obtain the driver program."));
        }

        // 找到了对应驱动
        m_selectedDriverName->setText(driverName);
        m_mainWidget->setCurrentIndex(1);
        m_installButton->setFocus();
        this->setFixedSize(425, 210);

        QWidget *widget = nullptr;
        QWidgetList widgetList = QApplication::allWidgets();
        for (int i = 0; i < widgetList.length(); ++i)
        {
            if (widgetList.at(i)->objectName() == "MainWindow")
            {
                widget = widgetList.at(i);
            }
        }
        if (widget)
        {
            QRect rect = widget->geometry();
            int x = rect.x() + rect.width() / 2 - this->width() / 2;
            int y = rect.y() + rect.height() / 2 - this->height() / 2;
            this->move(x, y);
        }

    }else {
        this->close();
        QString type;
        if(m_devInfoList.at(index).type == DeviceType::PRINTER){
            type = "printer";
        }else if(m_devInfoList.at(index).type == DeviceType::SCANNER){
            type = "scanner";
        }

//        WebDriverInstallWidget *widget = new WebDriverInstallWidget(type, m_devInfoList.at(index).digitId);
//        widget->show();
        QMap<QString, QString> buried_data;
        buried_data.insert("functionName", QString::fromStdString(kabase::BuriedPoint::convertPTtoString(kabase::BuriedPoint::PT::KylinScannerNoDriver)));
        buried_data.insert("action", "no driver, manual install");
        buried_data.insert("function", "in newdevicelistpage.cpp function nextButtonClickSlot()");

        int cursor{0};
        KCustomProperty property[buried_data.size()];
        for (auto iter = buried_data.begin(); iter != buried_data.end(); iter++) {
            property[cursor].key = strdup(iter.key().toLocal8Bit().data());
            property[cursor].value = strdup(iter.value().toLocal8Bit().data());
            cursor++;
        }
        KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
        kdk_dia_append_custom_property(node, property, buried_data.size());
        kdk_dia_upload_default(node, "NoDriverDevice", const_cast<char *>("mainPage"));
        kdk_dia_data_free(node);

        // 未找到对应驱动程序
        m_msg = new QMessageBox();
        m_msg->setFixedSize(450, 210);
        this->close();
        QString tipsStr = tr("No available drivers, do you want to manually add drivers?");
        m_msg->setText(tipsStr);
        m_msg->setIcon(QMessageBox::Warning);
        QPushButton *addButton = new QPushButton(m_msg);
        addButton->setText(tr("Add"));
        addButton->setProperty("isImportant", true);
        m_msg->addButton(tr("Cancel"), QMessageBox::RejectRole);
        m_msg->addButton(addButton, QMessageBox::AcceptRole);

        connect(m_msg, &QMessageBox::accepted, this, &newDeviceListPage::installHandlerSlot);
        connect(m_msg, &QMessageBox::rejected, [=](){this->close();});
        m_msg->show();

        QWidget *widget = nullptr;
        QWidgetList widgetList = QApplication::allWidgets();
        for (int i=0; i<widgetList.length(); ++i) {
            if (widgetList.at(i)->objectName() == "MainWindow") {
                widget = widgetList.at(i);
            }
        }
        if (widget) {
            QRect rect = widget->geometry();
            int x = rect.x() + rect.width()/2 - m_msg->width()/2;
            int y = rect.y() + rect.height()/2 - m_msg->height()/2;
            m_msg->move(x,y);
        }
    }
}

void newDeviceListPage::installDriverSlot()
{
    QWidget *widget = nullptr;
    QWidgetList widgetList = QApplication::allWidgets();
    for (int i=0; i<widgetList.length(); ++i) {
        if (widgetList.at(i)->objectName() == "MainWindow") {
            widget = widgetList.at(i);
        }
    }

    m_waittingDialog = new WaittingDialog(widget);
    this->close();
    m_waittingDialog->setText(tr("Installing driver..."));
    m_waittingDialog->show();
    m_aptHelper = new AptUtilHelper(DeviceInformation(m_waitForInstallOne));

    QObject::connect(m_aptHelper, &AptUtilHelper::succeed, this, &newDeviceListPage::showSuccessDialog);
    QObject::connect(m_aptHelper, &AptUtilHelper::failed, this, &newDeviceListPage::showFailDialog);

    m_aptHelper->startWorker();
}

void newDeviceListPage::showSuccessDialog()
{
    m_waittingDialog->close();
    qDebug() << "Install succeed";

    m_msg = new QMessageBox();
    m_msg->setFixedSize(450, 210);
    QString tipsStr = tr("Installation successful. Do you want to use this scanner now? Click 'Use' to reset udev (administrator password required) and restart the scanning application to refresh the device list. If you still want to continue operating the scanned image, you will need to manually restart and use the scanning function.");
    m_msg->setText(tipsStr);
    m_msg->setIcon(QMessageBox::Icon::Information);
    QPushButton *cancleBtn = new QPushButton;
    QPushButton *useBtn = new QPushButton;
    cancleBtn = m_msg->addButton(tr("Cancel"), QMessageBox::RejectRole);
    useBtn = m_msg->addButton(tr("Use"), QMessageBox::AcceptRole);

    connect(useBtn, &QPushButton::clicked, g_user_signal, &GlobalUserSignal::refreshListInFilePage);
    connect(cancleBtn, &QPushButton::clicked, g_user_signal, &GlobalUserSignal::updateSettingBtnSignal);
    connect(cancleBtn, &QPushButton::clicked, m_msg, &QMessageBox::close);
    connect(cancleBtn, &QPushButton::clicked, m_msg, &QMessageBox::deleteLater);

    m_msg->show();

    QWidget *widget = nullptr;
    QWidgetList widgetList = QApplication::allWidgets();
    for (int i=0; i<widgetList.length(); ++i) {
        if (widgetList.at(i)->objectName() == "MainWindow") {
            widget = widgetList.at(i);
        }
    }
    if (widget) {
        QRect rect = widget->geometry();
        int x = rect.x() + rect.width()/2 - m_msg->width()/2;
        int y = rect.y() + rect.height()/2 - m_msg->height()/2;
        m_msg->move(x,y);
    }
}

void newDeviceListPage::showFailDialog(QString err)
{
    m_waittingDialog->close();
    qDebug() << "Install failed " << err;

    m_msg = new QMessageBox();
    m_msg->setFixedSize(450, 210);
    QString tipsStr = tr("Installation failed.") + "\n" + err;
    m_msg->setText(tipsStr);
    m_msg->setIcon(QMessageBox::Critical);
    m_msg->addButton(tr("Ok"), QMessageBox::AcceptRole);
    connect(m_msg, &QMessageBox::accepted, [=](){this->close();m_msg->deleteLater();});
    m_msg->show();

    QWidget *widget = nullptr;
    QWidgetList widgetList = QApplication::allWidgets();
    for (int i=0; i<widgetList.length(); ++i) {
        if (widgetList.at(i)->objectName() == "MainWindow") {
            widget = widgetList.at(i);
        }
    }
    if (widget) {
        QRect rect = widget->geometry();
        qDebug() << "!!!!!!!!!!!!!!rect: " <<rect;
        int x = rect.x() + rect.width()/2 - m_msg->width()/2;
        int y = rect.y() + rect.height()/2 - m_msg->height()/2;
        qDebug() << "!!!!!!!!!!!!!!msg_size: " << m_msg->size();
        qDebug() << "x,y :" << x << " / "<< y;
        m_msg->move(x,y);
    }
}

void newDeviceListPage::reDetectDevicesSlot()
{
    this->close();
    m_waittingDialog = new WaittingDialog(this);
    m_waittingDialog->show();

    m_deviceFinder = new deviceFinder();
    QObject::connect(m_deviceFinder, &deviceFinder::succeed, this, [=](){
        m_devInfoList = m_deviceFinder->getList();
        m_deviceList->clear();
        m_waittingDialog->close();
        m_waittingDialog->deleteLater();
        handleData();
        this->show();
    });
    m_deviceFinder->startWorker();
}

void newDeviceListPage::installHandlerSlot()
{
    QString dlgTitle = tr("Select a directory");
    QString userRootDIr = QDir::homePath();
    QString selectedDir = QFileDialog::getOpenFileName(this, dlgTitle, userRootDIr, "*.deb");

    m_waittingDialog = new WaittingDialog();
    this->close();
    m_waittingDialog->setText(tr("Installing driver..."));
    m_waittingDialog->show();

    QWidget *widget = nullptr;
    QWidgetList widgetList = QApplication::allWidgets();
    for (int i=0; i<widgetList.length(); ++i) {
        if (widgetList.at(i)->objectName() == "MainWindow") {
            widget = widgetList.at(i);
        }
    }
    if (widget) {
        QRect rect = widget->geometry();
        int x = rect.x() + rect.width()/2 - m_waittingDialog->width()/2;
        int y = rect.y() + rect.height()/2 - m_waittingDialog->height()/2;
        m_waittingDialog->move(x,y);
    }

    m_debHelper = new DebUtilHelper(selectedDir);
    QObject::connect(m_debHelper, &DebUtilHelper::succeed, this, &newDeviceListPage::showSuccessDialog);
    QObject::connect(m_debHelper, &DebUtilHelper::failed, this, &newDeviceListPage::showFailDialog);
    m_debHelper->startWorker();
}

