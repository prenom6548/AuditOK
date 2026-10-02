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

#include "tablewidgetview.h"
#include <QCheckBox>
#include <QLabel>
#include <QTextStream>
#include <QDebug>
#include <QScrollArea>
#include "../PluginService/ui_unit/xrandrobject.h"
#include "../PluginService/ui_unit/messagebox.h"
#include "pushbuttonicon.h"
namespace KInstaller {
const int tableWidth = KServer::GetScreenRect().width() * 0.6 ;
const int colWidth = KServer::GetScreenRect().width() * 0.6 / 9;
const QStringList colorNameStr = { QString("#EA5504"),QString("#00A0DA"),QString("#B062A3"),QString("#009944"),
 QString("#74C6BE"),QString("#4D4398"),QString("#FABE00"),QString("#D12E29"),QString("#601986"),QString("#B062A3")
                                 ,QString("#F18D00"),QString("#6DBB58")};

TableWidgetView::TableWidgetView(QWidget *parent):QWidget(parent),
    m_formatORcreatorTable(false), colorID(0), wtip(nullptr)
{
    this->setObjectName("TableWidgetView");
    initUI();
    addStyleSheet();
}

TableWidgetView::~TableWidgetView()
{

}

void TableWidgetView::clearTableWidget()
{
    if(tableWidget) {
        items.clear();
        tableWidget->clearContents();
    }
}

void TableWidgetView::initUI()
{
    widget = new QWidget(this);
    widget->setObjectName("widgetView");
    layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    widget->setLayout(layout);

}

//void TableWidgetView::setCreatePartTableBtnState(bool flag)
//{
//    if(flag) {
//        createPartTableBtn->setDisabled(flag);
//        createPartTableBtn->setStyleSheet("QPushButton#createPartTableBtn{background:rgba(61,107,229,1);}");
//    } else {
//        createPartTableBtn->setDisabled(flag);
//        createPartTableBtn->setStyleSheet("QPushButton#createPartTableBtn{background:rgba(61,107,229,0.15);}");
//    }
//}

void TableWidgetView::getItems(Device::Ptr dev)
{
    //测试硬盘分区数据
    m_dev = dev;

//    QWidget* tmpwidget = new QWidget(this);
    hlayout = new QHBoxLayout();
    hlayout->setMargin(0);
//    tmpwidget->setLayout(hlayout);
    QLabel* piclabel = new QLabel;
    piclabel->setPixmap(QIcon(":/res/pic/harddisk-32.svg").pixmap(28,28));
    hlayout->addWidget(piclabel);
    QLabel* disklabel = new QLabel();
    disklabel->setObjectName("disklabel");

    if(dev->isLvm==false){
   	 disklabel->setText(dev->m_path + " " + dev->m_model);
    }else{
	 disklabel->setText("lvm-vg  " +  dev->m_lvmSize);
    }

    hlayout->addWidget(disklabel, Qt::AlignLeft);
    hlayout->addStretch();

    createPartTableBtn = new QPushButton();
    createPartTableBtn->setObjectName("createPartTableBtn");
    createPartTableBtn->setFlat(true);
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
//    sizePolicy.setHeightForWidth(createPartTableBtn->sizePolicy().hasHeightForWidth());
    createPartTableBtn->setSizePolicy(sizePolicy);
    hlayout->addWidget(createPartTableBtn, Qt::AlignRight);

    connect(createPartTableBtn, &QPushButton::clicked, this,&TableWidgetView::showResult);

    connect(this, &TableWidgetView::signalGetCreateResult, [=]{emit signalCreateTable(m_dev);});

    layout->addLayout(hlayout);
    if(dev->isLvm==true){
    	createPartTableBtn->setVisible(false);
    	for(int j = 0; j < dev->partitions.length(); j++) {
        	Partition::Ptr partition = dev->partitions.at(j);
		//addLvmPartitionRow(partition);
        	addPartitionRow(partition);
    	}
    }else{
    	for(int j = 0; j < dev->partitions.length(); j++) {
        	Partition::Ptr partition = dev->partitions.at(j);
        	addPartitionRow(partition);
    	}
    }
    addTableWidget();
}

void TableWidgetView::getLvmItems(DeviceList devs)
{
    qDebug() << "getLvmItems";
}

void TableWidgetView::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}

void TableWidgetView::showResult()
{

    QString info(tr("This action will delect all partition,are you sure?"));

    KServer::MessageBox *message = new  KServer::MessageBox();
    message->setMessageInfo(info);
    if(message->exec() == QDialog::Accepted) {
        emit signalGetCreateResult();
//	emit signalLvmUpdate();
    }

}

bool TableWidgetView::eventFilter(QObject *watched, QEvent *event)
{
    if(tableWidget==watched&&event->type() == QEvent::KeyPress) {
            QKeyEvent *ev = (QKeyEvent*) event;
        switch (ev->key()) {
        case Qt::Key_Tab:
            QWidget::focusNextPrevChild(true);//保证tab按键不在表格内部循环选择而是直接跳到下一个焦点
            break;
        }
        }
    return QObject::eventFilter(watched,event);
}

void TableWidgetView::buttonAddClicked()
{
    qDebug() << Q_FUNC_INFO << m_dev;
    if(m_dev) {
	qDebug()<< "m_dev is exit"; 
        qDebug()<< "m_dev->m_path" << m_dev->m_path;
	qDebug()<< "m_curPartition->m_path" << m_curPartition->m_devPath;	
        emit signalAdd(m_dev, m_curPartition);
    }else{
	 qDebug()<< "m_dev is unexit";
    }
}
void TableWidgetView::buttonChangeClicked()
{
    if(m_curPartition->m_partitionPath != "" && m_curPartition->m_num !=-1) {
        if(m_curPartition) {
            emit signalChange(m_dev, m_curPartition);
        }
    }

}
void TableWidgetView::buttonDeleteClicked()
{
    qDebug() << "buttonDeleteClicked" ;
    if(m_curPartition) {
        emit signalDelete(m_dev, m_curPartition);
    }
}


void TableWidgetView::buttonLvmClicked()
{
    qDebug() << "buttonLvmClicked" ;
    if(m_curPartition) {
        emit signalLvm(m_dev, m_curPartition);
    }
}

void TableWidgetView::checkBoxChecked()
{

}
void TableWidgetView::addStyleSheet()
{
    QFile file(":/res/qss/TableWidgetView.css");
    qDebug() << Q_FUNC_INFO << file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();
    this->setStyleSheet(stylesheet);
}
void TableWidgetView::translateStr()
{
    //设备,类型,挂载点,格式化,大小,已用,已装系统
    header.clear();
    header << QObject::tr(" partition") << QObject::tr("type") << QObject::tr("mounted") << QObject::tr("size")
             << QObject::tr("used") << QObject::tr("system")  << QObject::tr("format") << "" << "" ;\

    createPartTableBtn->setText(tr("Create partition table"));
}
void TableWidgetView::addTableWidget()
{
    translateStr();
    tableWidget = new CBaseTableWidget(items.length() / 9 , 9, items, true, this);//构建表格
    tableWidget->installEventFilter(this);
    //隐藏tablewidget行标题
    tableWidget->setHeaderVisible(true);
    tableWidget->setHeaderLabels(header, true);
    tableWidget->verticalHeader()->hide();
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);    //x先自适应宽度
    tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
//    tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
//    tableWidget->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
    tableWidget->horizontalHeader()->setSectionResizeMode(8, QHeaderView::ResizeToContents);
    //    tableWidget->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);

    //添加列标题

    tableWidget->setColWidthRowHeight(colWidth, 32);
    tableWidget->setColumnWidth(1, 50);
    tableWidget->setColumnWidth(2, 50);

//    int w = tableWidget->columnWidth(0) * 9;
//    tableWidget->setFixedWidth(w);
//    tableWidget->horizontalHeaderItem(0)->setSizeHint(QSize(colWidth, 30));
//    tableWidget->resizeColumnToContents(0);
    layout->addWidget(tableWidget);
    connect(tableWidget, &CBaseTableWidget::currentCellChanged, this, &TableWidgetView::slotShowTableItem);
//    connect(tableWidget, &CBaseTableWidget::currentCellChanged, this, &TableWidgetView::showTableWidgetTip);
    connect(tableWidget, &CBaseTableWidget::clicked, [=]{emit signalSelectTable(this);});
    connect(tableWidget, &CBaseTableWidget::doubleClicked, this, &TableWidgetView::buttonChangeClicked);
    update();
//    layout->addStretch();
}

void TableWidgetView::showTableWidgetTip(int row, int col)
{
    if(col == 0) {
        wtip = new ArrowWidget();
        wtip->setObjectName("ArrowWidget");
        wtip->setAttribute(Qt::WA_StyledBackground);
        wtip->setDerection(ArrowWidget::Derection::right);
        wtip->setWindowFlags(wtip->windowFlags() | Qt::Dialog);
        wtip->setText(tableWidget->item(row, col)->text());//        w->setCenterWidget(textLabel);
        QPoint pos = tableWidget->mapToGlobal(QPoint());

        wtip->move(pos.x() + tableWidget->rowHeigh * row, pos.y() + tableWidget->columnWidth(col));
        wtip->show();
    } else {
        if(wtip != nullptr) {
            wtip->close();
        }
    }
}

//void TableWidgetView::updateTableShow()
//{
//    for(int i = 0; i < tableWidget->rows; i++) {
//        PushButtonIcon* prebtn1 = qobject_cast<PushButtonIcon*> (tableWidget->getItemWidget(i, 7));
//        if(prebtn1) {
//            prebtn1->showIcon();
//        }
//        PushButtonIcon* prebtn2 = qobject_cast<PushButtonIcon*> (tableWidget->getItemWidget(i, 8));
//        if(prebtn2) {
//            prebtn2->showIcon();
//        }
//    }
//}

//show button text
void TableWidgetView::slotShowTableItem(int curRow, int curCol, int preRow, int preCol)
{
   qDebug() << "curRow="<<curRow ;
       
    if(curRow != -1) {
        PushButtonIcon* prebtn1 = qobject_cast<PushButtonIcon*> (tableWidget->getItemWidget(preRow, 7));
        if(prebtn1) {
            prebtn1->showIcon();
        }
        PushButtonIcon* prebtn2 = qobject_cast<PushButtonIcon*> (tableWidget->getItemWidget(preRow, 8));
        if(prebtn2) {
            prebtn2->showIcon();
        }
        PushButtonIcon* btn1 = qobject_cast<PushButtonIcon*> (tableWidget->getItemWidget(curRow, 7));
        if(btn1) {
            btn1->showIconAndText();
        }
        PushButtonIcon* btn2 = qobject_cast<PushButtonIcon*> (tableWidget->getItemWidget(curRow, 8));
        if(btn2) {
            btn2->showIconAndText();
        }
        m_curPartition = m_dev->partitions.at(curRow);
	qDebug() << "m_curPartition->m_path" << m_curPartition->m_devPath; 
        m_curRow = curRow;

    }
}

void TableWidgetView::addPartitionRow(Partition::Ptr partition)
{
    if(colorID >= colorNameStr.length() - 1) {
        colorID = 0;
    }
    if(partition->m_type == PartitionType::Unallocated && partition->m_num < 0) {
	qDebug() << "partition->m_type == PartitionType::Unallocated && partition->m_num < 0" ;
	addFreeSpaceRow(partition);
    } else if (partition->fs == FSType::Vg) {
	qDebug() << "partition->fs == FSType::Vg" ;
	addFreeSpaceRow(partition);
	//addVgSpace(partition);
    } else {
        QString color = ":/res/pic/color/" + colorNameStr.at(colorID) + ".png";
        items.append(new CBaseTableItem(CBaseTableItem::ICON_TEXT, partition->m_partitionPath, color));//设备名称

        if(partition->m_type == PartitionType::Extended) {
            items.append(new CBaseTableItem(CBaseTableItem::TEXT, partition->m_name));//文件类型
        } else
            items.append(new CBaseTableItem(CBaseTableItem::TEXT, findNameByFSType(partition->fs)));//文件类型
        items.append(new CBaseTableItem(CBaseTableItem::TEXT, partition->m_mountPoint));//挂载点
        if(partition->m_type == PartitionType::Extended) {
            items.append(new CBaseTableItem(CBaseTableItem::TEXT, ""));//分区的总量
            items.append(new CBaseTableItem(CBaseTableItem::TEXT, ""));//已用分区空间
        } else {
            items.append(new CBaseTableItem(CBaseTableItem::TEXT, partition->getTotalSpace()));//分区的总量
            items.append(new CBaseTableItem(CBaseTableItem::TEXT, partition->getUsedSpace()));//已用分区空间
        }
        items.append(new CBaseTableItem(CBaseTableItem::TEXT, partition->m_OSname));//已装系统
//        QCheckBox* checkBox = new QCheckBox;
//        connect(checkBox, &QCheckBox::clicked, this, &TableWidgetView::checkBoxChecked);
        items.append(new CBaseTableItem(CBaseTableItem::TEXT, partition->blFormatPartition ? tr("yes"): tr("no") ));//格式化

        PushButtonIcon* delbtn = new PushButtonIcon(QString(":/res/pic/delete.png"), "", this);
        delbtn->setObjectName("delbtn");
        CBaseTableItem* delItem = new CBaseTableItem(CBaseTableItem::WIDGET,"",NULL, delbtn, NULL );
        if(partition->m_type == PartitionType::Unallocated || partition->m_type == PartitionType::Normal || partition->m_type == PartitionType::Logical) {
            PushButtonIcon* changebtn = new PushButtonIcon(QString(":/res/pic/change.svg"), "", this);
            changebtn->setObjectName("changebtn");
            connect(changebtn, &PushButtonIcon::clicked, this, &TableWidgetView::buttonChangeClicked);
            CBaseTableItem* changeItem = new CBaseTableItem(CBaseTableItem::WIDGET,"",NULL, changebtn, NULL );
            items.append(delItem);//删除
	     items.append(changeItem);//修改

        } else {
            items.append(new CBaseTableItem(CBaseTableItem::TEXT, ""));//修改
            items.append(delItem);//删除
        }

        connect(delbtn, &PushButtonIcon::clicked, this, &TableWidgetView::buttonDeleteClicked);
        colorID++;
    }

}


void TableWidgetView::addLvmPartitionRow(Partition::Ptr partition)
{
    if(colorID >= colorNameStr.length() - 1) {
        colorID = 0;
    }
    if(partition->fs == FSType::Vg ) {
        //addFreeSpaceRow(partition);
	addVgSpace(partition);
    } else {
        QString color = ":/res/pic/color/" + colorNameStr.at(colorID) + ".png";
        items.append(new CBaseTableItem(CBaseTableItem::ICON_TEXT, partition->m_partitionPath, color));//设备名称

        if(partition->m_type == PartitionType::Extended) {
            items.append(new CBaseTableItem(CBaseTableItem::TEXT, partition->m_name));//文件类型
        } else
            items.append(new CBaseTableItem(CBaseTableItem::TEXT, findNameByFSType(partition->fs)));//文件类型
        items.append(new CBaseTableItem(CBaseTableItem::TEXT, partition->m_mountPoint));//挂载点
        if(partition->m_type == PartitionType::Extended) {
            items.append(new CBaseTableItem(CBaseTableItem::TEXT, ""));//分区的总量
            items.append(new CBaseTableItem(CBaseTableItem::TEXT, ""));//已用分区空间
        } else {
            items.append(new CBaseTableItem(CBaseTableItem::TEXT, partition->getTotalSpace()));//分区的总量
            //items.append(new CBaseTableItem(CBaseTableItem::TEXT, QString::number(partition->m_lvmSize)));//分区的总量
            items.append(new CBaseTableItem(CBaseTableItem::TEXT, partition->getUsedSpace()));//已用分区空间
        }
        items.append(new CBaseTableItem(CBaseTableItem::TEXT, partition->m_OSname));//已装系统
        items.append(new CBaseTableItem(CBaseTableItem::TEXT, partition->blFormatPartition ? tr("yes"): tr("no") ));//格式化

        PushButtonIcon* delbtn = new PushButtonIcon(QString(":/res/pic/delete.png"), "", this);
        delbtn->setObjectName("delbtn");
        CBaseTableItem* delItem = new CBaseTableItem(CBaseTableItem::WIDGET,"",NULL, delbtn, NULL );

        if(partition->m_type == PartitionType::Unallocated || partition->m_type == PartitionType::Normal || partition->m_type == PartitionType::Logical) {
            PushButtonIcon* changebtn = new PushButtonIcon(QString(":/res/pic/change.svg"), "", this);
            changebtn->setObjectName("changebtn");
            connect(changebtn, &PushButtonIcon::clicked, this, &TableWidgetView::buttonChangeClicked);
            CBaseTableItem* changeItem = new CBaseTableItem(CBaseTableItem::WIDGET,"",NULL, changebtn, NULL );
            items.append(delItem);//删除
	   // items.append(changeItem);//修改
	    changebtn->setVisible(false);
	    delbtn->setVisible(false);
	    
	    QCheckBox* lvmbtn = new QCheckBox(this);
	    lvmbtn->setObjectName("lvmbtn");
	    CBaseTableItem* lvmItem = new CBaseTableItem(CBaseTableItem::WIDGET,"",NULL, lvmbtn, NULL );
	    items.append(lvmItem);
            connect(lvmbtn, &QCheckBox::clicked, this, &TableWidgetView::buttonLvmClicked);
	    lvmbtn->setChecked(true);
        } else {
            items.append(new CBaseTableItem(CBaseTableItem::TEXT, ""));//修改
            items.append(delItem);//删除
        }
        connect(delbtn, &PushButtonIcon::clicked, this, &TableWidgetView::buttonDeleteClicked);


        colorID++;
    }

}


void TableWidgetView::addFreeSpaceRow(Partition::Ptr partition)
{
    colorID++;
    if (partition->fs == FSType::Vg){
	 qDebug() << "partition->fs is FSType::Vg=" ;
    	items.append(new CBaseTableItem(CBaseTableItem::ICON_TEXT, tr("freespace")+"(lvm-vg)", ":/res/pic/color/0_0.png"));//设备名称
    }else{
	 qDebug() << "partition->fs is not FSType::Vg" ;
    	items.append(new CBaseTableItem(CBaseTableItem::ICON_TEXT, tr("freespace"), ":/res/pic/color/0_0.png"));//设备名称
    }
    items.append(new CBaseTableItem(CBaseTableItem::TEXT, ""));//文件类型
    items.append(new CBaseTableItem(CBaseTableItem::TEXT, ""));//挂载点
    items.append(new CBaseTableItem(CBaseTableItem::TEXT, partition->getTotalSpace()));//分区的总量
    items.append(new CBaseTableItem(CBaseTableItem::TEXT, ""));//已用分区空间
    items.append(new CBaseTableItem(CBaseTableItem::TEXT, ""));//已装系统
    items.append(new CBaseTableItem(CBaseTableItem::TEXT, tr("no")));//格式化
    items.append(new CBaseTableItem(CBaseTableItem::TEXT, ""));//格式化
    PushButtonIcon* addbtn = new PushButtonIcon(QString(":/res/pic/list-add-symbolic.svg"), "       ", this);
    addbtn->setObjectName("addbtn");
    CBaseTableItem* additem =new CBaseTableItem(CBaseTableItem::WIDGET, "", NULL, addbtn, NULL);
    items.append(additem);//add


    connect(addbtn, &PushButtonIcon::clicked, this, &TableWidgetView::buttonAddClicked);
}

void TableWidgetView::addVgSpace(Partition::Ptr partition)
{
    colorID++;
    items.append(new CBaseTableItem(CBaseTableItem::ICON_TEXT, tr("VG-freespace"), ":/res/pic/color/0_0.png"));//设备名称
    items.append(new CBaseTableItem(CBaseTableItem::TEXT, ""));//文件类型
    items.append(new CBaseTableItem(CBaseTableItem::TEXT, ""));//挂载点
    items.append(new CBaseTableItem(CBaseTableItem::TEXT, partition->getTotalSpace()));//分区的总量
    items.append(new CBaseTableItem(CBaseTableItem::TEXT, ""));//分区的总量
    items.append(new CBaseTableItem(CBaseTableItem::TEXT, ""));//已用分区空间
    items.append(new CBaseTableItem(CBaseTableItem::TEXT, ""));//已装系统
    items.append(new CBaseTableItem(CBaseTableItem::TEXT, ""));//格式化
    PushButtonIcon* addbtn = new PushButtonIcon(QString(":/res/pic/list-add-symbolic.svg"), "       ", this);
    addbtn->setObjectName("addbtn");
    CBaseTableItem* additem =new CBaseTableItem(CBaseTableItem::WIDGET, "", NULL, addbtn, NULL);
    items.append(additem);//add


    connect(addbtn, &PushButtonIcon::clicked, this, &TableWidgetView::buttonAddClicked);
}


void TableWidgetView::updateDiskTableWidgetInfo(Device::Ptr dev)
{
    items.clear();
    for(int j = 0; j < dev->partitions.length(); j++) {
        Partition::Ptr partition = dev->partitions.at(j);
        addPartitionRow(partition);
    }
    tableWidget->insertRows(items);
    tableWidget->viewport()->update();
    tableWidget->resize(tableWidget->columnCount() * tableWidget->columnWidth(0), (tableWidget->rowCount()) * tableWidget->rowHeight(0));

}

}
