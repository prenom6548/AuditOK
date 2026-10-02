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
#include <QTimer>
#include "device_list_button.h"
#include "size_para.h"
#include "libkysysinfo.h"

DeviceListButton::DeviceListButton(const UkuiUsbPrinter &printerStruct,QWidget *parent) :
    QPushButton(parent),//继承QToolButton构造函数
    m_isConnectedFlag(false),
    m_isDefaultFlag(printerStruct.is_default),
    m_printerStruct(printerStruct)
{
    initUI(printerStruct);
//    setQSSFontSize(currentFontSize);
}

void DeviceListButton::initUI(const UkuiUsbPrinter &printerStruct)
{
    m_HLwholeBtn = new QHBoxLayout();
    m_VLtextInfo = new QVBoxLayout();
    m_HLtextInfo = new QHBoxLayout();

    m_printerNameText = printerStruct.name;
    m_printerNameLabel = new CustomLabel(this);
    m_printerNameLabel->setObjectName("printerName");
    m_printerNameLabel->setFixedSize(196,24);

    m_printerNameLabel->setText(m_printerNameText);

    m_statusIcon = new QPushButton(this);
    m_statusIcon->setProperty("useIconHighlightEffect", 0x0);
    m_statusIcon->setIconSize(QSize(8,8));
    m_statusIcon->setFixedSize(8,8);
    if (printerStruct.connect_status == tr("Idle")) {
        m_statusIcon->setIcon(QIcon(":/pic/green.svg"));
    }
    else if (printerStruct.connect_status == tr("Printing")) {
        m_statusIcon->setIcon(QIcon(":/pic/orange.svg"));
    }
    else if (printerStruct.connect_status == tr("Stopped")) {
        m_statusIcon->setIcon(QIcon(":/pic/red.svg"));
    }
    else if (printerStruct.connect_status == tr("Unknow")) {
        m_statusIcon->setIcon(QIcon(":/pic/gray.svg"));
    }
    m_statusIcon->setFlat(true);
    m_statusIcon->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_connectStatus = new QLabel(this);
    m_connectStatus->setObjectName("connectStatus");
    m_connectStatus->setFixedSize(56,24);
    QString labelStyleOfConnectStatus ="QLabel#connectStatus{color:palette(PlaceholderText);}";
    m_connectStatus->setStyleSheet(labelStyleOfConnectStatus);

    m_connectStatus->setText(printerStruct.connect_status);

    m_isDefault = new QLabel(this);
    m_isDefault->setObjectName("m_isDefault");
    m_isDefault->setFixedSize(62,24);
    QString labelStyleOfIsDefault ="QLabel#m_isDefault{color:palette(PlaceholderText);}";
    m_isDefault->setStyleSheet(labelStyleOfIsDefault);
    m_isDefaultFlag = printerStruct.is_default;
    if(m_isDefaultFlag)
    {
        m_isDefault->setText(tr("Default"));
    }
    else
    {
        m_isDefault->setText("");
    }

    m_typeIcon = new QPushButton(this);
    m_typeIcon->setIconSize(LARGE_BTN_ICON_SIZE);
    m_typeIcon->setFixedSize(LARGE_BTN_ICON_SIZE);
    m_typeIcon->setStyleSheet(BTN_TO_LABEL_STYLE);
    m_typeIcon->setAttribute(Qt::WA_TransparentForMouseEvents, true);//不响应鼠标事件，直接穿透
    if (printerStruct.device_uri.contains("usb://",Qt::CaseInsensitive))
    {
        m_typeIcon->setIcon(QIcon::fromTheme("printer"));
    }
    else if (printerStruct.device_uri.contains("/usb",Qt::CaseInsensitive))
    {
        m_typeIcon->setIcon(QIcon::fromTheme("printer"));
    }
    else
    {
        m_typeIcon->setIcon(QIcon::fromTheme("printer-remote"));
    }

    auto font = m_connectStatus->font();
    font.setPixelSize(14);

    m_connectStatus->setFont(font);
    m_isDefault->setFont(font);

    m_HLtextInfo->setMargin(0);
    m_HLtextInfo->setSpacing(0);
    m_HLtextInfo->addSpacing(0);
    m_HLtextInfo->addWidget(m_statusIcon);
    m_HLtextInfo->addSpacing(4);
    m_HLtextInfo->addWidget(m_connectStatus);
    m_HLtextInfo->addSpacing(8);
    m_HLtextInfo->addWidget(m_isDefault);
    m_HLtextInfo->addStretch();

    m_VLtextInfo->setMargin(0);
    m_VLtextInfo->setSpacing(0);
    m_VLtextInfo->addSpacing(6);
    m_VLtextInfo->addWidget(m_printerNameLabel);
    m_VLtextInfo->addStretch();
    m_VLtextInfo->addLayout(m_HLtextInfo);
    m_VLtextInfo->addSpacing(6);


    m_HLwholeBtn->setMargin(0);
    m_HLwholeBtn->setSpacing(0);
    m_HLwholeBtn->addSpacing(8);
    m_HLwholeBtn->addWidget(m_typeIcon);
    m_HLwholeBtn->addLayout(m_VLtextInfo);
    m_HLwholeBtn->addStretch();

    this->setFixedSize(DEVICE_LIST_BTN_SIZE);
    this->setMouseTracking(true);

    this->setFocus();
    this->setFlat(true);
    this->setCheckable(true);
    this->setAutoExclusive(true);
    this->setLayout(m_HLwholeBtn);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->installEventFilter(this);

    m_menu = new QMenu(this);
    QList<QAction *> actions;
    m_actionSetDefault = new QAction(this);
    m_actionSetDefault->setText(tr("Set Default"));
    m_actionEnabled = new QAction(this);
    m_actionEnabled->setText(tr("Enabled"));
    m_actionEnabled->setCheckable(true);
    m_actionEnabled->setChecked(UkuiPrinterManager::getInstance().getPrinterEnabled(m_printerStruct.name));
    m_actionShared = new QAction(this);
    m_actionShared->setText(tr("Shared"));
    m_actionShared->setEnabled(m_printerStruct.make_and_model != "Remote Printer");
    m_actionShared->setCheckable(m_printerStruct.make_and_model != "Remote Printer");
    m_actionShared->setChecked(UkuiPrinterManager::getInstance().getPrinterShared(m_printerStruct.name));
    actions << m_actionSetDefault
            << m_actionEnabled
            << m_actionShared;
    m_menu->addActions(actions);

    connect(m_menu,SIGNAL(aboutToHide()),this,SLOT(menuHide()));

    connect(this,SIGNAL(toggled(bool)),this,SLOT(buttonCheckState(bool)));

    // connect(m_actionSetDefault, &QAction::triggered, this,[=](){
    //     changeDefaultPrinter(printerStruct.name);
    // });
    connect(m_menu, &QMenu::triggered, this, &DeviceListButton::triggerMenu);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu(const QPoint&)));
}

void DeviceListButton::triggerMenu(QAction *action)
{
    QString str = action->text();
    if(tr("Set Default") == str) {
        changeDefaultPrinter(m_printerStruct.name);
    }
    else if(tr("Enabled") == str) {
        UkuiPrinterManager::getInstance().setPrinterEnabled(m_printerStruct.name, m_actionEnabled->isChecked());
    }
    else if(tr("Shared") == str) {
        qDebug() << m_actionShared->isChecked();
        UkuiPrinterManager::getInstance().setPrinterShared(m_printerStruct.name, m_actionShared->isChecked());
    }
}
void DeviceListButton::buttonCheckState(bool isChecked)
{
    if(isChecked)
    {
        emit thisBtnIsChecked(this);
        m_printerNameLabel->setForegroundRole(QPalette::Light);
        QString labelStyleOfConnectStatus ="QLabel#connectStatus{color:rgba(220, 220, 220, 1) ;}";
        m_connectStatus->setStyleSheet(labelStyleOfConnectStatus);
        QString labelStyleOfIsDefault ="QLabel#m_isDefault{color:rgba(220, 220, 220, 1) ;}";
        m_isDefault->setStyleSheet(labelStyleOfIsDefault);
    }
    else
    {
        m_printerNameLabel->setForegroundRole(QPalette::WindowText);
        QString labelStyleOfConnectStatus ="QLabel#connectStatus{color:palette(PlaceholderText);}";
        m_connectStatus->setStyleSheet(labelStyleOfConnectStatus);
        QString labelStyleOfIsDefault ="QLabel#m_isDefault{color:palette(PlaceholderText);}";
        m_isDefault->setStyleSheet(labelStyleOfIsDefault);

        m_printerNameLabel->setText(m_printerNameText);
    }
}

void DeviceListButton::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton && !this->isChecked())
    {
        return;
    }
    if (e->button() == Qt::LeftButton && this->isChecked())
    {
        return;
    }
    return QPushButton::mousePressEvent(e);
}

bool DeviceListButton::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == this)
    {
        if(!this->isChecked())
        {
            if(event->type() == QEvent::HoverEnter) {
                if (QString(kdk_system_get_major_version()) != "V11") {
                    m_printerNameLabel->setForegroundRole(QPalette::Light);
                    m_connectStatus->setForegroundRole(QPalette::Light);
                    m_isDefault->setForegroundRole(QPalette::Light);
                    QString labelStyleOfConnectStatus ="QLabel#connectStatus{color:rgba(220, 220, 220, 1);}";
                    m_connectStatus->setStyleSheet(labelStyleOfConnectStatus);
                    QString labelStyleOfIsDefault ="QLabel#m_isDefault{color:rgba(220, 220, 220, 1);}";
                    m_isDefault->setStyleSheet(labelStyleOfIsDefault);
                }
            }

            if(event->type() == QEvent::HoverLeave) {
                if (QString(kdk_system_get_major_version()) != "V11") {
                    m_printerNameLabel->setForegroundRole(QPalette::WindowText);
                    m_connectStatus->setForegroundRole(QPalette::WindowText);
                    m_isDefault->setForegroundRole(QPalette::WindowText);
                    QString labelStyleOfConnectStatus ="QLabel#connectStatus{color:palette(PlaceholderText);}";
                    m_connectStatus->setStyleSheet(labelStyleOfConnectStatus);
                    QString labelStyleOfIsDefault ="QLabel#m_isDefault{color:palette(PlaceholderText);}";
                    m_isDefault->setStyleSheet(labelStyleOfIsDefault);
                }
            }
        }
    }
    return QPushButton::eventFilter(obj,event);
}

void DeviceListButton::menuHide()
{
    if(this->isChecked())
    {
        emit thisBtnIsChecked(this);
        m_printerNameLabel->setForegroundRole(QPalette::Light);
        QString labelStyleOfConnectStatus ="QLabel#connectStatus{color:rgba(220, 220, 220, 1);}";
        m_connectStatus->setStyleSheet(labelStyleOfConnectStatus);
        QString labelStyleOfIsDefault ="QLabel#m_isDefault{color:rgba(220, 220, 220, 1);}";
        m_isDefault->setStyleSheet(labelStyleOfIsDefault);
    }
    else
    {
        m_printerNameLabel->setForegroundRole(QPalette::WindowText);
        QString labelStyleOfConnectStatus ="QLabel#connectStatus{color:palette(PlaceholderText);}";
        m_connectStatus->setStyleSheet(labelStyleOfConnectStatus);
        QString labelStyleOfIsDefault ="QLabel#m_isDefault{color:palette(PlaceholderText);}";
        m_isDefault->setStyleSheet(labelStyleOfIsDefault);
    }
}

void DeviceListButton::setDefaultPrinter(bool is_Default)
{
    m_isDefaultFlag = is_Default;
    if(m_isDefaultFlag)
    {
        m_actionSetDefault->setEnabled(false);
        m_isDefault->setText(tr("Default"));
    }
    else
    {
        m_actionSetDefault->setEnabled(true);
        m_isDefault->setText("");
    }

    m_isDefault->hide();
    m_isDefault->show();
}

bool DeviceListButton::getDefaultStatus()
{
    return m_isDefaultFlag ;
}

void DeviceListButton::showMenu(const QPoint& point)
{
    m_actionEnabled->setChecked(UkuiPrinterManager::getInstance().getPrinterEnabled(m_printerStruct.name));
    if (m_actionShared->isEnabled()) {
        m_actionShared->setChecked(UkuiPrinterManager::getInstance().getPrinterShared(m_printerStruct.name));
    }

    m_menu->exec(mapToGlobal(point));
}

void DeviceListButton::freshStatus()
{
    if(m_isDefaultFlag)
    {
        m_actionSetDefault->setEnabled(false);
        m_isDefault->setText(tr("Default"));
    }
    else
    {
        m_actionSetDefault->setEnabled(true);
        m_isDefault->setText((""));
    }
}

const QString DeviceListButton::getPrinterName() const
{
    return m_printerNameText;
}

void DeviceListButton::setQSSFontSize(QFont curFont)
{
//    //带QSS的控件设置字体ui->xx->setFont(font);
//    double font14Size = curFont.pointSizeF()/15*14;
//    QFont font14 = curFont;
//    font14.setPointSizeF(font14Size);
//
//    m_isDefault->setFont(font14);
//    m_connectStatus->setFont(font14);
//    m_printerNameLabel->setFont(font14);
}
