/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "custompartitionframe.h"
#include <QGridLayout>
#include <QCheckBox>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QListView>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QScrollBar>

#include "device.h"

#include "partman/libparted_unit.h"
#include "./delegate/custom_partition_delegate.h"
#include "./delegate/customize_device_model.h"
#include "partman/partition_server.h"
#include "../PluginService/ui_unit/xrandrobject.h"

#include "../PluginService/ui_unit/xrandrobject.h"
namespace KInstaller {
using namespace KServer;
const int tableWidth = KServer::GetScreenRect().width() * 0.6 ;
CustomPartitionFrame::CustomPartitionFrame(CustomPartitiondelegate* customDelegate, QWidget *parent) : QWidget(parent),
    m_delegate(customDelegate),
    m_createPartFrame(nullptr),
    m_modifyPartFrame(nullptr),
    m_bootstr(""),
    layout1Width(0)
{
//    m_modifyPartFrame->hide();
//    m_createPartFrame->hide();
    initUI();
    addStyleSheet();
    initAllConnect();
    translateStr();
    this->setFocusPolicy(Qt::NoFocus);

    //this->grabKeyboard();
}

CustomPartitionFrame::~CustomPartitionFrame()
{
}

void CustomPartitionFrame::initUI()
{
    gridLayout = new QGridLayout(this);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(gridLayout);
    gridLayout->setColumnStretch(0, 2);
    gridLayout->setColumnStretch(1, 5);
    gridLayout->setColumnStretch(2, 2);

    QHBoxLayout* hlayout = new QHBoxLayout;
    m_bootlabel = new QLabel;
    hlayout->addWidget(m_bootlabel);

    m_bootCombox = new QComboBox;
    hlayout->addWidget(m_bootCombox);
    m_bootCombox->setView(new QListView());
    m_bootCombox->setObjectName("bootCombox");
    m_bootCombox->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    m_bootCombox->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    m_bootCombox->installEventFilter(this);
    hlayout->addStretch();
    m_ToolTip = new QLabel(this);
    m_ToolTip->setFixedHeight(40);
    m_ToolTip->setFixedWidth(500);
    m_ToolTip->setVisible(false);

    m_lvmBtn = new QPushButton;
    m_lvmBtn->setObjectName("lvmBtn");
    hlayout->addWidget(m_lvmBtn);
    m_lvmBtn->setFlat(true);
    hlayout->addSpacerItem(new QSpacerItem(12, 10, QSizePolicy::Fixed, QSizePolicy::Preferred));
    m_lvmBtn->setVisible(false);

    m_revertBtn = new QPushButton;
    m_revertBtn->setObjectName("revertBtn");
    hlayout->addWidget(m_revertBtn);
    m_revertBtn->setFlat(true);
    m_revertBtn->setIcon(QIcon(":/res/pic/revert.svg"));
    hlayout->addSpacerItem(new QSpacerItem(12, 10, QSizePolicy::Fixed, QSizePolicy::Preferred));

    addTableWidget();
    addPartProgress();

    gridLayout->addWidget(tableViewWidget, 3, 1, 1, 1);
    gridLayout->addLayout(hlayout, 4, 1, 1, 1);
//    gridLayout->setRowStretch(5, 1);
}
void CustomPartitionFrame::addStyleSheet()
{
    QFile file(":/res/qss/CustomPartitionFrame.css");
    qDebug() << Q_FUNC_INFO << file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

void CustomPartitionFrame::initAllConnect()
{
    connect(m_delegate, &CustomPartitiondelegate::deviceRefreshed, this, &CustomPartitionFrame::repaintDevice);
    connect(m_bootCombox, &QComboBox::currentTextChanged, this, &CustomPartitionFrame::slotSelectBootCombox);
    connect(m_revertBtn, &QPushButton::clicked, this, &CustomPartitionFrame::revertPartition);
    connect(m_lvmBtn, &QPushButton::clicked, this, &CustomPartitionFrame::createLvmPartition);
    connect(this, &CustomPartitionFrame::updateLvm, this, &CustomPartitionFrame::lvmPartition);
}
//添加tabelwidget控件
void CustomPartitionFrame::addTableWidget()
{
    tableViewWidget = new QWidget;
    tableViewWidget->setObjectName("tableLayoutWidget");
    tableViewWidget->installEventFilter(this);
    //tableViewWidget->setFocusPolicy(Qt::NoFocus);
    pScroll = new QScrollArea();
    pScroll->setFocusPolicy(Qt::NoFocus);
    pScroll->setObjectName("pScroll");
    pScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pScroll->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    
    pScroll->verticalScrollBar()->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);

    pScroll->setFrameShape(QFrame::NoFrame);
    pScroll->setStyleSheet("QScrollArea{background-color:transparent;");
    pScroll->viewport()->setStyleSheet("background-color:transparent;");
    psWidget = new QWidget();
    psWidget->setObjectName("psWidget");
    vtlayout = new QVBoxLayout;
    vtlayout->setSpacing(10);
    psWidget->setLayout(vtlayout);
    pScroll->setWidget(psWidget);
    pScroll->setWidgetResizable(true);

    pScroll->adjustSize();
    psWidget->adjustSize();
//    psWidget->setMaximumHeight(500);

    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->addWidget(pScroll);
    vlayout->addStretch();
    tableViewWidget->setLayout(vlayout);
}

void CustomPartitionFrame::updateScrollAreaWidget()
{
    if(psWidget->height() > 500) {
        int width = psWidget->width();
        int height = psWidget->height();
        psWidget->setMinimumSize(tableWidth, height);
        tableViewWidget->setMinimumSize(tableWidth - 5, height);
//        m_colorProgressFrame->setMinimumSize(tableWidth, height);
//        pScroll->setMinimumSize(width, height);
    }

    int nHeightTotal = 0;//解决bug163173自定义安装界面，文字重叠
    for(TableWidgetView* table : m_tableViews) {
        nHeightTotal += table->height() + 30;
    }
    if(nHeightTotal)
    {
        if(nHeightTotal > KServer::GetScreenRect().height()*0.5)
        {
            pScroll->setMinimumSize(tableWidth, KServer::GetScreenRect().height()*0.5);
            psWidget->setMinimumSize(tableWidth, nHeightTotal);
        }
        else
        {
            pScroll->setMinimumSize(tableWidth, nHeightTotal);
            psWidget->setMinimumSize(tableWidth, nHeightTotal);
        }
    }

    tableViewWidget->adjustSize();
    pScroll->adjustSize();
    psWidget->adjustSize();
}

void CustomPartitionFrame::newTableViews(DeviceList devs)
{
    vtlayout->setContentsMargins(0, 0, 0, 0);
    int widgetH = 0;
    for(int i = 0; i < devs.length(); i++) {
        //将隐藏listwidget控件显示,显示tablewidget控件
        TableWidgetView* tableView = new TableWidgetView(this);
        tableView->setObjectName("TableWidgetView");
        tableView->getItems(devs.at(i));
        tableView->createPartTableBtn->installEventFilter(this);
        vtlayout->addWidget(tableView, Qt::AlignTop | Qt::AlignCenter);
        vtlayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding));
//        vtlayout->addStretch();
        m_tableViews.append(tableView);
        tableView->tableWidget->setFixedWidth(tableWidth - 5);

        int w = tableView->tableWidget->columnWidth(0) * 9;
        int h = tableView->tableWidget->rowHeight(0) * (tableView->tableWidget->rowCount() + 3);
        widgetH += h;
//        tableView->setFixedWidth(w);
        tableView->setMinimumHeight(h);
//        tableView->setMaximumHeight(h);


        qDebug() << "signalAdd" <<connect(tableView, &TableWidgetView::signalAdd, this, &CustomPartitionFrame::slotCreatePartition);
        qDebug() << "signalChange" <<connect(tableView, &TableWidgetView::signalChange, this, &CustomPartitionFrame::slotModifyPartition);
        qDebug() << "signalDelete" <<connect(tableView, &TableWidgetView::signalDelete, this, &CustomPartitionFrame::slotDeletePartition);
	qDebug() << "signalCreatorTable" <<connect(tableView, &TableWidgetView::signalCreateTable, [=](Device::Ptr dev){
                        emit signalCreatePartTable(dev);
                        emit updateLvm();
                        });

        qDebug() << "signalLvm" <<connect(tableView, &TableWidgetView::signalLvm, this,&CustomPartitionFrame::setLvmPartition);

        connect(tableView, &TableWidgetView::signalSelectTable, this, &CustomPartitionFrame::updateTableView);
        tableView->adjustSize();
    }
//    vtlayout->addStretch();
    vtlayout->addSpacerItem(new QSpacerItem(10, 25, QSizePolicy::Minimum, QSizePolicy::Expanding));
    tableViewWidget->adjustSize();
}

void CustomPartitionFrame::newLvmTableViews(DeviceList devs)
{
     qDebug() << "111111115";
}


void CustomPartitionFrame::updateTableView(TableWidgetView* tablewidget)
{
    for(TableWidgetView* table : m_tableViews) {
        if(table != tablewidget){
            for(int i = 0; i < table->tableWidget->rowCount(); i++) {
                table->tableWidget->clearSelection();
//                table->updateTableShow();

            }
        }
    }
    m_colorProgressFrame->setDevice(tablewidget->m_dev);
}

void CustomPartitionFrame::showEvent(QShowEvent *event)
{
    emit reSetFocusPolicy();
}

void CustomPartitionFrame::addPartProgress()
{
    m_colorProgressFrame = new DiskPartitionColorProgress();
    gridLayout->addItem(new QSpacerItem(10, 30, QSizePolicy::Expanding, QSizePolicy::Preferred), 0, 1, 1, 1);
    gridLayout->addWidget(m_colorProgressFrame, 1, 1, 1, 1);
    gridLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Preferred), 2, 1, 1, 1);
   // m_colorProgressFrame->setFocusPolicy(Qt::NoFocus);
}

void CustomPartitionFrame::slotCreatePartition(Device::Ptr dev, Partition::Ptr part)
{
    qDebug() << Q_FUNC_INFO << dev << part->m_devPath;
    // slotCreatePartition m_blCreatorPartition=true;


    QString dev_path = part->m_devPath;
    if(!m_delegate->isPartitionTableMatchDe(dev_path)) {
	if(dev_path!="lvm-vg"){
                qDebug() << "isPartitionTableMatchDe is false;";
                return;
        }
    }
    bool bl = true;
    if (m_delegate->canAddPrimary(part)) {
        bl = true;
    } else if(!m_delegate->canAddPrimary(part) && m_delegate->canAddLogical(part)) {
        bl = false;
    }else {
        qDebug() << GetPartTableNameByType(dev->m_partTableType);
        return;
    }
    m_createPartFrame = new CreatePartitionFrame(m_delegate,&devlist,m_pFrame);
    connect(m_createPartFrame, &CreatePartitionFrame::finished, [=]{repaintDevice();m_createPartFrame->close();});
    m_createPartFrame->setNewPartition(dev, part, bl, m_SCreenWidth, m_SCreenHeight);
    m_createPartFrame->show();
    connect(m_createPartFrame, &CreatePartitionFrame::createLvm, this, &CustomPartitionFrame::lvmPartition);
}

void CustomPartitionFrame::slotDeletePartition(Device::Ptr dev, Partition::Ptr part)
{
    qDebug() << "slotDeletePartition" ;
    MessageBox message;

    if(part->m_OSname.isEmpty())
        message.setMessageInfo(tr("remove this partition?"));
    else
        message.setMessageInfo(tr("This is a system partition,remove this partition?"));
    if (message.exec() == QDialog::Rejected && message.execid == 1){
        qDebug() << "Delete Partition cancel" ;
        return;
    }

    if(part) {
        m_delegate->deletePartition(part);
        m_delegate->refreshShow();
	if(part->fs == FSType::Pv){
                qDebug() <<"part->fs == FSType::Pv11111111";
                m_delegate->getLvmOperations();
        }
        repaintDevice();
    }
}

void CustomPartitionFrame::setLvmPartition(Device::Ptr dev, Partition::Ptr part)
{
        qDebug() << "lvmPartition" ;
        if(part) {
                m_delegate->lvmUpdatePartition(part);
   //           m_delegate->refreshShow();
     //         repaintDevice();

        }

}


void CustomPartitionFrame::slotRestorePartition()
{
    qDebug() << "slotRestorePartitin" ;
}
void CustomPartitionFrame::slotModifyPartition(Device::Ptr dev, Partition::Ptr part)
{
    qDebug() << "slotModifyPartition" ;
    QString dev_path = part->m_devPath;
    if(!m_delegate->isPartitionTableMatchDe(dev_path) && !part->m_partitionPath.contains("lvm-lv")) {
        qDebug() << "isPartitionTableMatchDe is false;";
        return;
    } else {
        m_modifyPartFrame = new ModifyPartitionFrame(m_delegate,m_pFrame);
        m_modifyPartFrame->devlst=&devlist;//把硬盘列表传过去
        connect(m_modifyPartFrame, &ModifyPartitionFrame::finished, [=]{repaintDevice();m_modifyPartFrame->close();emit modifyPartFinished();});
        m_modifyPartFrame->setModifyPartition(part, m_SCreenWidth, m_SCreenHeight);
        m_modifyPartFrame->show();
	connect(m_modifyPartFrame, &ModifyPartitionFrame::createLvm, this, &CustomPartitionFrame::lvmPartition);
    }
}

void CustomPartitionFrame::repaintDevice()
{
    qDebug() << Q_FUNC_INFO ;
    devlist = m_delegate->virtualDevices();

    for (QLayoutItem* item = vtlayout->takeAt(0);
          item != nullptr;
          item = vtlayout->takeAt(0)) {
       delete item->widget();
       delete item;
       item = nullptr;
     }
    if(vtlayout)
        delete vtlayout;
    if(psWidget)
        delete psWidget;

    psWidget = new QWidget();
    psWidget->setObjectName("psWidget");
    vtlayout = new QVBoxLayout;
    vtlayout->setSpacing(10);
    psWidget->setLayout(vtlayout);
    pScroll->setWidget(psWidget);
    pScroll->setWidgetResizable(true);
    pScroll->adjustSize();
    psWidget->adjustSize();

    m_tableViews.clear();
    newTableViews(devlist);
    if(devlist.count() > 0) {
        m_colorProgressFrame->setDevice(devlist.at(0));
    }

    if(m_bootCombox->count() <= 0) {
        int i = 0;
        for(Device::Ptr dev : devlist) {
            QString strRealData(QString("%1 %2").arg(dev->m_path).arg(dev->m_model));//硬盘全称
//            QString strShow = m_bootCombox->view()->fontMetrics().elidedText(strRealData,Qt::ElideRight,m_bootCombox->width()-20);//当文字过长时，末尾显示省略号
            QString strShow = m_bootCombox->view()->fontMetrics().elidedText(strRealData,Qt::ElideRight,300-6-2-9-10);//当文字过长时，末尾显示省略号.由于m_bootCombox宽度是在css中设置导致这里获取的值不对，改为直接写宽度值
            m_bootCombox->addItem(strShow,strRealData);
            qDebug() << "Device Name Show:" <<strShow<<"Device Name:"<<strRealData;
        }
        m_bootCombox->setCurrentIndex(0);
    }
    updateScrollAreaWidget();

    update();
    qDebug() << Q_FUNC_INFO<< "signalFinishedLoadDisk1" ;

    emit signalFinishedLoadDisk();
    this->activateWindow();//实现重新设置焦点顺序后仍然在原位置并可tab走焦
}

void CustomPartitionFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
        this->repaintDevice();
    } else {
        QWidget::changeEvent(event);
    }
}

void CustomPartitionFrame::translateStr()
{
    m_bootlabel->setText(tr("Device for boot loader path:"));//引导加载程序安装设备：
    m_revertBtn->setText(tr("Revert"));//还原
    m_lvmBtn->setText(tr("Create lvm"));//创建lvm
}

void CustomPartitionFrame::slotSelectBootCombox(QString bootstr)
{
    QString currentData = m_bootCombox->currentData().toString();//获取完整值
    m_delegate->setBootLoaderPath(currentData);
    m_bootstr = currentData;

    //悬浮提示完整信息
    if(currentData != m_bootCombox->currentText())
    {
        m_ToolTip->setText(currentData);
    }
    else
    {
        m_ToolTip->clear();
    }
}

void CustomPartitionFrame::revertPartition()
{
    m_delegate->resetOperations();
    repaintDevice();
}

void CustomPartitionFrame::lvmPartition()
{
   m_delegate->getLvmOperations();
   repaintDevice();
}

void CustomPartitionFrame::createLvmPartition()
{
   m_delegate->addLvmPartition();

   repaintDevice();
}

void CustomPartitionFrame::keyPressEvent(QKeyEvent *event)
{
    this->setFocus();
    qDebug() << Q_FUNC_INFO << "keyPressEvent";
    if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        emit enterpressed();
    }
    else if(event->key() == Qt::Key_Backspace)
    {
        qDebug()<< "Custom partition back to main partition";
        emit backspacepressed();
    }
    else if(event->key() == Qt::Key_Escape || event->key() == Qt::Key_Control)
        ;
    else
    {
        QWidget::keyPressEvent(event);
    }
}

bool CustomPartitionFrame::eventFilter(QObject *watched, QEvent *event)
{
    if(watched->objectName() == "bootCombox") {
        if(event->type() == QEvent::Enter) {
            if(!m_ToolTip->text().isEmpty())
            {
                m_ToolTip->move(m_bootCombox->x()+22, m_bootCombox->y() - m_bootCombox->height());
                m_ToolTip->setVisible(true);
            }
        } else if(event->type() == QEvent::Leave){
            if(!m_ToolTip->text().isEmpty()) {
                m_ToolTip->setVisible(false);
            }
        }
    }
    if(/*watched->objectName() == "tableLayoutWidget"*/1) {
        if(event->type() == QEvent::KeyPress) {
            QKeyEvent *ev = (QKeyEvent*) event;
            QScrollBar *vScrollBar=pScroll->verticalScrollBar();
            if(vScrollBar==NULL)
            {
                return QWidget::eventFilter(watched, event);
            }
            switch (ev->key()) {
            case Qt::Key_Home:
                vScrollBar->setValue(0);
                return true;
                break;
            case Qt::Key_End:
                vScrollBar->setValue(vScrollBar->maximum());
                return true;
                break;
            case Qt::Key_PageUp:
                vScrollBar->setValue(0);
                return true;
                break;
            case Qt::Key_PageDown:
                vScrollBar->setValue(vScrollBar->maximum());
                return true;
                break;
            case Qt::Key_Up:
                vScrollBar->setValue(vScrollBar->value()-10);
                return true;
                break;
            case Qt::Key_Down:
                vScrollBar->setValue(vScrollBar->value()+10);
                return true;
                break;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

}
