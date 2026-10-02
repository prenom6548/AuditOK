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
#include "left_widget.h"
#include <QStyleOption>
#include <QScrollBar>
#include <QPainter>
#include "custom_message_box.h"
#include "size_para.h"
#include "ukui_printer.h"
#include "common_ui.h"
#include "accessinfohelper.h"

#define g_printerManger UkuiPrinterManager::getInstance()
LeftWidget::LeftWidget(QWidget *parent)
            : QWidget(parent)
            , m_mainWin(parent)
            , m_currentSelectBtn(nullptr)
            , m_isUpdatingFlag(false)
{
    //全导航栏布局
    m_VLayoutNavigator = new QVBoxLayout(this);
    //左侧设备列表说明区布局
    m_HLayoutDeviceOP  = new QHBoxLayout();
    //左侧设备列表说明区布局
    m_HLayoutDeviceList  = new QHBoxLayout();
    //设备列表区布局
    m_VLayoutDeviceBtnList  = new QVBoxLayout();
    m_VLayoutDeviceBtnList->setContentsMargins(0,0,0,0);
    m_VLayoutDeviceBtnList->setSpacing(0);
    m_VLayoutDeviceBtnList->setMargin(0);

    initDeviceOperateSector();
    initDeviceListSector();

    m_VLayoutNavigator->setSpacing(0);
    m_VLayoutNavigator->setMargin(0);
    m_VLayoutNavigator->addSpacing(16);
    m_VLayoutNavigator->addLayout(m_HLayoutDeviceOP);
    m_VLayoutNavigator->addSpacing(8);
    m_VLayoutNavigator->addLayout(m_HLayoutDeviceList);
    m_VLayoutNavigator->addStretch();

    this->setLayout(m_VLayoutNavigator);
}

void LeftWidget::initDeviceOperateSector()
{
    m_deviceLabel = new QLabel(this);

    m_deviceLabel->setText(tr("Device List"));
    m_deviceLabel->setFixedHeight(24);
    m_deviceLabel->setForegroundRole(QPalette::PlaceholderText);

    m_addDeviceBtn             = new CustomPushButton(this);
    m_addDeviceBtn->setObjectName("addDeviceBtn");
    m_addDeviceBtn->setFixedSize(LARGE_BTN_SIZE);
    m_addDeviceBtn->setIcon(QIcon::fromTheme("list-add-symbolic"));//主题库的新建图标
    m_addDeviceBtn->setProperty("useIconHighlightEffect", 0x8);
    m_addDeviceBtn->setIconSize(QSize(16,16));
    m_addDeviceBtn->setTranslucent(true);

    m_removeDeviceBtn          = new CustomPushButton(this);
    m_removeDeviceBtn->setObjectName("removeDeviceBtn");
    m_removeDeviceBtn->setFixedSize(LARGE_BTN_SIZE);
    m_removeDeviceBtn->setIcon(QIcon::fromTheme("list-remove-symbolic"));//主题库的新建图标
    m_removeDeviceBtn->setProperty("useIconHighlightEffect", 0x8);
    m_removeDeviceBtn->setIconSize(QSize(16,16));

    m_removeDeviceBtn->setEnabled(false);//列表中无设备被选中时，不可点击
    m_removeDeviceBtn->setTranslucent(true);

    connect(m_addDeviceBtn,&QPushButton::clicked,this,[=](){
    emit addDevice();
    });
    connect(m_removeDeviceBtn,SIGNAL(clicked(bool)),this,SLOT(delDeviceSlot()));
    m_HLayoutDeviceOP->setSpacing(0);
    m_HLayoutDeviceOP->setMargin(0);
    m_HLayoutDeviceOP->addSpacing(14);
    m_HLayoutDeviceOP->addWidget(m_deviceLabel);
    m_HLayoutDeviceOP->addStretch();
    m_HLayoutDeviceOP->addWidget(m_addDeviceBtn);
    m_HLayoutDeviceOP->addSpacing(8);
    m_HLayoutDeviceOP->addWidget(m_removeDeviceBtn);
    m_HLayoutDeviceOP->addSpacing(17);

    KDK_ALL_INFO_FORMAT(m_addDeviceBtn,"");
    KDK_ALL_INFO_FORMAT(m_removeDeviceBtn,"");
}

void LeftWidget::initDeviceListSector()
{
    m_ScrollArea = new QScrollArea(this);
    m_ScrollArea->setFixedSize(300 - 12, 560 - 40 - 20);
    m_ScrollArea->setFrameShape(QFrame::NoFrame);

    QPalette pal = palette();
    pal.setColor(QPalette::Background, QColor(0,0,0,0));
    m_ScrollArea->setPalette(pal);

    m_deviceListWidget = new QWidget();
    m_deviceListWidget->setObjectName("deviceListWidget");

    m_ScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    (m_ScrollArea->verticalScrollBar())->setProperty("drawScrollBarGroove", false);

    m_ScrollArea->setWidget(m_deviceListWidget);

    m_ScrollArea->setProperty("drawScrollBarGroove", false);

    m_ScrollArea->setWidgetResizable(true);

    m_HLayoutDeviceList->setSpacing(0);
    m_HLayoutDeviceList->setMargin(0);
    m_HLayoutDeviceList->addSpacing(14);
    m_HLayoutDeviceList->addWidget(m_ScrollArea);
    m_HLayoutDeviceList->addSpacing(14);
}

void LeftWidget::updateDeviceList()
{
    m_deviceListWidget->hide();
    for(auto btn : m_deviceList) {
        btn->hide();
    }
    delete m_VLayoutDeviceBtnList;

    //设备列表区布局
    m_VLayoutDeviceBtnList  = new QVBoxLayout(m_deviceListWidget);
    m_VLayoutDeviceBtnList->setContentsMargins(0,0,0,0);
    m_VLayoutDeviceBtnList->setSpacing(0);
    m_VLayoutDeviceBtnList->setMargin(0);

    if(!m_deviceList.isEmpty())
    {
        for (auto btn : m_deviceList) {
            m_VLayoutDeviceBtnList->addWidget(btn);
            m_VLayoutDeviceBtnList->addSpacing(8);
            btn->freshStatus();
            btn->show();
        }
    }
    else
    {
        if(m_currentSelectBtn)
        {
            delete m_currentSelectBtn;
        }
        emit noBtnIsChecked();
        m_currentSelectBtn = nullptr;
    }

    if(m_currentSelectBtn == nullptr)
    {
        if(m_deviceList.size())
        {
            m_currentSelectBtn = m_deviceList.last();
        }
        else
        {
            m_currentSelectBtn = nullptr;
        }
        if(m_currentSelectBtn)
        {
            m_currentSelectBtn->setChecked(true);
        }
        else
        {
            emit noBtnIsChecked();
        }
    }
    else
    {
        bool tempFlag = false;
        for(auto it: m_deviceList)
        {
            it->setChecked(false);
            if(it->getPrinterName() == m_currentSelectBtn->getPrinterName())
            {
                m_currentSelectBtn = it;
                m_currentSelectBtn->setChecked(true);
                tempFlag = true;
            }
        }
        if(!tempFlag)
        {
            if(m_deviceList.size())
            {
                m_currentSelectBtn = m_deviceList.last();
            }
            else
            {
                m_currentSelectBtn = nullptr;
            }
            if(m_currentSelectBtn)
            {
                m_currentSelectBtn->setChecked(true);
            }
            else
            {
                emit noBtnIsChecked();
            }
        }
    }


    m_VLayoutDeviceBtnList->addStretch();
    m_deviceListWidget->setLayout(m_VLayoutDeviceBtnList);
    m_deviceListWidget->show();
}

DeviceListButton *LeftWidget::getCurrentSelectPrinter() const
{
    return m_currentSelectBtn;
}

void LeftWidget::delDeviceSlot()
{
    QString warningMsg = tr("Are you sure to delete \"%1\"?").arg(m_currentSelectBtn->getPrinterName());
    CustomMessageBox Msg(CustomMessageBox::Warning,"",warningMsg,CustomMessageBox::NoButton, this->m_mainWin);
    // Msg.setInformativeText(tr("This action will delete the job queue too!"));
    QPushButton *btnYes = Msg.addButton(tr("Delete"), CustomMessageBox::YesRole);
    QPushButton *btnNo  = Msg.addButton(tr("Cancel"), CustomMessageBox::NoRole);

    Msg.exec();
    if((QPushButton*)Msg.clickedButton() == btnYes)
    {
        emit delDevice(this->getCurrentSelectPrinter()->getPrinterName());
    }

}

void LeftWidget::updateListSector(const QList<UkuiUsbPrinter>& printersVector,const UkuiUsbPrinter& currentPrinter)
{
    if(m_isUpdatingFlag)
    {
        return;
    }
    else
    {
        m_isUpdatingFlag = true;
        if(m_printersVector == printersVector)
        {
            m_isUpdatingFlag = false;
            return;
        }
        else
        {
            for(auto printerBtn : m_deviceList)
            {
                if(printerBtn->getPrinterName() == currentPrinter.name)
                {
                    m_currentSelectBtn = printerBtn;
                }
                printerBtn->hide();
            }

            m_deviceList.clear();
            m_printersVector.clear();
            if(printersVector.size())
            {
                for (auto printerInfoStruct: printersVector) {
                    DeviceListButton *btn = new DeviceListButton(printerInfoStruct,m_deviceListWidget);
                    connect(btn,SIGNAL(thisBtnIsChecked(DeviceListButton*)),this,SLOT(updateBtnConnections(DeviceListButton*)),Qt::UniqueConnection);
                    connect(btn,SIGNAL(changeDefaultPrinter(QString)),this,SLOT(updateDefaultPrinter(QString)),Qt::UniqueConnection);
                    if(printerInfoStruct.is_default){
                        //默认插入最前
                        m_deviceList.insert(0,btn);
                    }else{
                        m_deviceList.append(btn);
                    }
                    m_printersVector.push_back(printerInfoStruct);
                    m_removeDeviceBtn->setEnabled(true);//列表中无设备被选中时，不可点击
                }
            }
            else
            {
                m_removeDeviceBtn->setEnabled(false);//列表中无设备被选中时，不可点击
            }
            updateDeviceList();
            if(1 == m_deviceList.size())//列表中只有一个时，设为默认
            {
                if(!m_currentSelectBtn->getDefaultStatus())
                {
                    updateDefaultPrinter(m_currentSelectBtn->getPrinterName());
                }
            }

        }
        m_isUpdatingFlag = false;
    }
}

void LeftWidget::addPrinterToListSector(const UkuiUsbPrinter& printersVector)
{
    for(auto btnItem : m_deviceList) {
        if((btnItem->getPrinterName()) ==printersVector.name)
        {
            m_currentSelectBtn = btnItem;
            m_currentSelectBtn->setChecked(true);
            m_currentSelectBtn->freshStatus();
            emit printerIsSelected(printersVector);

            return;
        }
    }

    DeviceListButton *btn = new DeviceListButton(printersVector,m_deviceListWidget);
    connect(btn,SIGNAL(thisBtnIsChecked(DeviceListButton*)),
            this,SLOT(updateBtnConnections(DeviceListButton*)),Qt::UniqueConnection);
    connect(btn,SIGNAL(changeDefaultPrinter(QString)),
            this,SLOT(updateDefaultPrinter(QString)),Qt::UniqueConnection);
    m_deviceList.append(btn);
    m_printersVector.append(printersVector);

    m_currentSelectBtn = btn;
    m_removeDeviceBtn->setEnabled(true);//列表中无设备被选中时，不可点击

    m_VLayoutDeviceBtnList->insertWidget((m_VLayoutDeviceBtnList->count()-1),btn);
    m_VLayoutDeviceBtnList->insertSpacing((m_VLayoutDeviceBtnList->count()-1),8);

    m_currentSelectBtn->setChecked(true);

//    emit m_currentSelectBtn->thisBtnIsChecked(m_currentSelectBtn);
    if(1 == m_deviceList.size())//列表中只有一个时，设为默认
    {
        updateDefaultPrinter(btn->getPrinterName());
    }
}

void LeftWidget::updateBtnConnections(DeviceListButton *btn)
{
    for(auto printer:m_printersVector)
    {
        if(printer.name == btn->getPrinterName())
        {
            m_currentSelectBtn = btn;
            emit printerIsSelected(printer);
            break;
        }
    }
}
void LeftWidget::updateDefaultPrinter(QString printerName)
{
    DeviceListButton* tempBtn = nullptr;
    bool tempFlag = false;
    for(auto btn : m_deviceList)
    {
        if(btn->getDefaultStatus())
        {
            tempBtn = btn;
            continue;
        }

        if(btn->getPrinterName() == printerName)
        {
            if(g_printerManger.setDefaultPrinter(printerName))
            {
                tempFlag = true;
                btn->setDefaultPrinter(true);
            }
            else
            {
                QString tipMsg = printerName;
                QString warningMsg = tr("Set Default Failed!");
                tipMsg.append(warningMsg);
                CustomMessageBox Msg(CustomMessageBox::Warning,tr("Warning"),tipMsg,CustomMessageBox::NoButton, this);
                Msg.addButton(tr("Close"), CustomMessageBox::NoRole);

                Msg.exec();
                btn->setDefaultPrinter(false);
            }
        }

    }

    if(tempFlag)
    {
        if(tempBtn!=nullptr)
        {
            tempBtn->setDefaultPrinter(false);
        }
    }
}

void LeftWidget::setQSSFontSize(QFont curFont)
{

}
