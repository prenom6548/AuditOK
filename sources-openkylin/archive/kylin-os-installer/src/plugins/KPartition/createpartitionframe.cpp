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

#include "createpartitionframe.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QRect>
#include <QEvent>
#include <QDebug>
#include <QListView>
#include <QTextStream>
#include <QGridLayout>
#include <QScrollBar>
#include <QTimer>
#include "../PluginService/ksystemsetting_unit.h"
#include "../PluginService/ui_unit/xrandrobject.h"
#include "../PluginService/sysInfo/kernelfilesystemfs.h"
#include "qscrollbar.h"
#include "../PluginService/sysInfo/machine.h"
#include "qmessagebox.h"
#include "partman/partition_unit.h"

using namespace KServer;
namespace KInstaller {
using namespace Partman;
CreatePartitionFrame::CreatePartitionFrame(CustomPartitiondelegate* delegate,
                                            DeviceList *pdevlst,
                                           QWidget *parent) : QDialog(parent),
    m_delegate(delegate),
    devlst(pdevlst),
    mountFileStr(""),
    m_tTip(nullptr)
{

    this->setObjectName("CreatePartitionFrame");
    initUI();
    addStyleSheet();
    initAllConnect();
    initBackground();

    this->setWindowFlags(Qt::Sheet|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);//保证界面显示在最上层且设置为窗口模式和应用模态，这样焦点就不会跑到下层界面上
    //setWindowModality(Qt::ApplicationModal);
//    this->setFocus();
    //this->setWindowFlags(/*Qt::Window|*/Qt::FramelessWindowHint);


}
void CreatePartitionFrame::initBackground()
{
//    // 无边框
//    setWindowFlags(Qt::FramelessWindowHint |
//                   Qt::CustomizeWindowHint|
////                   Qt::WindowStaysOnTopHint |
//                   Qt::Tool |
//                   Qt::Dialog);

    //全屏
    showFullScreen();

     //设置背景blue色
    QRect rect = KServer::GetScreenRect();
    QPixmap pixmap = QPixmap(":/data/png/sp2-bk@1x.png").scaled(rect.width(), rect.height(), Qt::KeepAspectRatioByExpanding);
    QPalette pal(this->palette());
    pal.setBrush(backgroundRole(), QBrush(pixmap));
    this->setAutoFillBackground(true);
    this->setPalette(pal);
}

void CreatePartitionFrame::setNewPartition(Device::Ptr dev,
                                           Partition::Ptr partition,bool pritypebl,int SCreenWidth,int SCreenHeight)
{
    m_partition = partition;
    m_dev = dev;

    if(partition->fs == FSType::Vg){
        qDebug() << "fileFormat->setEnabled(false)";

        for(int i = 0; i < fileFormat->count(); i++ ) {
                if(fileFormat->itemText(i) == "lvm-pv" || fileFormat->itemText(i) == "efi") {
                        fileFormat->removeItem(i);
                }
        }
        for(int j = 0; j < mountFile->count(); j++ ) {
                if(mountFile->itemText(j) == "/boot") {
                        mountFile->removeItem(j);
                }
        }
    }

    sizeStr = GetMebiByateValue(m_partition->getByteLength());
    sizeStr.chop(4);
    sizeEdit->setValue(sizeStr);
    
    root_default_size=dev->m_root_default_size;
    data_default_size=dev->m_data_default_size;
    swap_default_size=dev->m_swap_default_size;
    backup_default_size=dev->m_backup_default_size;

    fileFormatStr = m_partition->m_fsname;
    mountFileStr = m_partition->m_mountPoint;
    priTypebl = pritypebl;
    startPosbl = true;
    primaryPart->setChecked(priTypebl);
    lvmPart->setChecked(!priTypebl);
    startPosition->setChecked(startPosbl);
    endPosition->setChecked(!startPosbl);
    fileFormat->setCurrentIndex(0);
    mountFile->setCurrentIndex(0);

    if(getExtendedPartitionIndex(dev->partitions) > -1) {
        primaryPart->setEnabled(false);
        priTypebl = false;
        lvmPart->setChecked(true);
    } else {
        primaryPart->setEnabled(true);
    }

    if(m_dev->m_partTableType == PartTableType::MsDos) {
        //lvmPart->setVisible(true);
        lvmPart->setEnabled(true);
    } else {
        //lvmPart->setVisible(false);
        lvmPart->setEnabled(false);
    }

    m_SCreenWidth=SCreenWidth;
    m_SCreenHeight=SCreenHeight;
    //setFixedSize(m_SCreenWidth, m_SCreenHeight);
    setMinimumSize(m_SCreenWidth, m_SCreenHeight);
    update();
}

void CreatePartitionFrame::initUI()
{
    QGridLayout* gLayout = new QGridLayout;
    gLayout->setMargin(24);
    this->setLayout(gLayout);
    gLayout->setColumnStretch(0, 3);
    gLayout->setColumnStretch(1, 1);
    gLayout->setColumnStretch(2, 1);
    gLayout->setColumnStretch(3, 3);
    gLayout->setVerticalSpacing(5);

    QHBoxLayout* hlay = new QHBoxLayout;
    hlay->setMargin(0);
    m_close = new QPushButton;
    m_close->setObjectName("close");
    m_close->installEventFilter(this);
    hlay->addStretch();
    hlay->addWidget(m_close);
    gLayout->addLayout(hlay, 0, 3, 1, 1, Qt::AlignCenter | Qt::AlignTop);


    m_mainTitle = new QLabel;
    m_mainTitle->setObjectName("mainTitle");
    gLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred), 1, 1, 1, 1);
    gLayout->addWidget(m_mainTitle, 2, 1, 1, 2, Qt::AlignCenter | Qt::AlignTop);
    gLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred), 3, 1, 1, 1);

    typeLabel= new QLabel;
    typeLabel->setObjectName("label");
    primaryPart = new QRadioButton;
    primaryPart->setObjectName("primaryPart");
    primaryPart->installEventFilter(this);
    lvmPart = new QRadioButton;
//    partGroup = new QButtonGroup;
//    partGroup->addButton(primaryPart, 0);
//    partGroup->addButton(lvmPart, 1);

    gLayout->addWidget(typeLabel, 4, 1, 1, 1, Qt::AlignLeft);
    gLayout->addWidget(primaryPart, 5, 1, 1, 1, Qt::AlignLeft);
    gLayout->addWidget(lvmPart, 5, 2, 1, 1, Qt::AlignLeft);
    gLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Preferred), 6, 1);

    positionLabel= new QLabel;
    positionLabel->setObjectName("label");
    posGroup = new QButtonGroup;
    startPosition = new QRadioButton;
    endPosition = new QRadioButton;
    posGroup->addButton(startPosition, 0);
    posGroup->addButton(endPosition, 1);


    gLayout->addWidget(positionLabel, 7, 1, 1, 1, Qt::AlignLeft);
    gLayout->addWidget(startPosition, 8, 1, 1, 1, Qt::AlignLeft);
    gLayout->addWidget(endPosition, 8, 2, 1, 1, Qt::AlignLeft);
    gLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Preferred), 9, 1);

    fsLabel= new QLabel;
    fsLabel->setObjectName("label");
    fileFormat = new QComboBox;
    fileFormat->setView(new QListView);
    fileFormat->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    fileFormat->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    fileFormat->view()->verticalScrollBar()->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    fileFormat->setObjectName("combox");
//    filesystemList = m_delegate->getFsTypeList();
    //2021-5-24根据内核支持的文件系统对文件系统进行加载
    filesystemList = getKernelFs();
    if(!isEFIEnabled())
    {
        filesystemList.removeAll("efi");
    }

//    fileFormat->addItems(filesystemList);
    fileFormat->setMaxVisibleItems(5);
    fileFormat->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    fileFormat->view()->verticalScrollBar()->setStyleSheet("QScrollBar{width:15px;}");
   // fileFormat->setStyleSheet("QComboBox::drop-down{ width: 48px; border: 4px solid transparent; margin-right: 8px; margin-left: 8x; border-radius: 8px;}"
     //                        "QComboBox::down-arrow{ right: 1px; width: 12px; height: 9px; image: url(:/data/png/pan-down-symbolic.svg);}"
      //                       "QComboBox::down-arrow:on{ width: 12px; height: 9px; border: 0px solid transparent ; image: url(:/data/png/pan-up-symbolic.svg);}"
      //                      );

    gLayout->addWidget(fsLabel, 10, 1, 1, 2, Qt::AlignLeft);
    gLayout->addWidget(fileFormat, 11, 1, 1, 2);
    gLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Preferred), 12, 1);

    mountLabel= new QLabel;
    mountLabel->setObjectName("label");

    mountFile = new QComboBox;
    mountFile->setView(new QListView);
    mountFile->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    mountFile->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    mountFile->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    mountFile->view()->verticalScrollBar()->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    mountFile->view()->verticalScrollBar()->setStyleSheet("QScrollBar{width:15px;}");
    mountFile->setObjectName("combox");
    mountedList = m_delegate->getMountPoints();
    mountFile->addItems(mountedList);
    mountFile->setEditable(true);
    mountFile->setMaxVisibleItems(5);
    mountFile->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mountFile->setCompleter(nullptr);

    //mountFile->setStyleSheet("QComboBox::drop-down{ width: 48px; border: 4px solid transparent; margin-right: 8px; margin-left: 8x; border-radius: 8px;}"
    //			     "QComboBox::down-arrow{ right: 1px; width: 12px; height: 9px; image: url(:/data/png/pan-down-symbolic.svg);}"
    //			     "QComboBox::down-arrow:on{ width: 12px; height: 9px; border: 0px solid transparent ; image: url(:/data/png/pan-up-symbolic.svg);}"
    //            	    );

    gLayout->addWidget(mountLabel, 13, 1, 1, 2, Qt::AlignLeft);
    gLayout->addWidget(mountFile, 14, 1, 1, 2);
    gLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Preferred), 15, 1);

    sizeLabel = new QLabel;
    sizeLabel->setObjectName("label");

    sizeEdit = new LineEditAddReduce();
//    sizeEdit->setFocusPolicy(Qt::StrongFocus);

    gLayout->addWidget(sizeLabel, 16, 1, 1, 1, Qt::AlignLeft);
    gLayout->addWidget(sizeEdit, 17, 1, 1, 2);
    gLayout->addItem(new QSpacerItem(30, 20, QSizePolicy::Expanding, QSizePolicy::Preferred), 18, 1);


    tipLabel = new QLabel;
    gLayout->addWidget(tipLabel, 19, 1, 1, 3, Qt::AlignLeft | Qt::AlignTop);
    gLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred), 20, 1, 1, 1);
    gLayout->setRowStretch(19, 1);

    m_OKBtn = new QPushButton;
    m_OKBtn->setObjectName("OKBtn");
    gLayout->addWidget(m_OKBtn, 21, 1, 1, 2, Qt::AlignCenter);
    gLayout->setRowStretch(22, 1);
    translateStr();

}


void CreatePartitionFrame::initAllConnect()
{
    connect(primaryPart,SIGNAL(clicked()),this,SLOT(clickPrimaryPart()));
    connect(lvmPart,SIGNAL(clicked()),this,SLOT(clickLvmPart()));
    connect(startPosition,SIGNAL(clicked()),this,SLOT(clickStartPos()));
    connect(endPosition,SIGNAL(clicked()),this,SLOT(clickEndPos()));
    connect(m_close, &QPushButton::clicked, [=]{this->close();});
    connect(m_OKBtn, &QPushButton::clicked, this, &CreatePartitionFrame::slotOKBtn);

    connect(fileFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(changeFileFormat(int)));
    connect(mountFile, &QComboBox::currentTextChanged, this, &CreatePartitionFrame::changeMountFile);
    connect(this, &CreatePartitionFrame::destroyed, this, &CreatePartitionFrame::deleteLater);
}

void CreatePartitionFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}


void CreatePartitionFrame::setPartitionValue(Partition::Ptr partition)
{
    sizevalue = sizeEdit->getValue();
    partition->m_devPath = m_dev->m_path;
    if(priTypebl) {
        //primary
        partition->m_type = PartitionType::Normal;
    } else {
        //logical
        partition->m_type = PartitionType::Logical;
    }

    if(startPosbl) {
        //start
        partition->m_sectorStart = m_partition->m_sectorStart;
        partition->m_sectorSize = m_partition->m_sectorSize;
        partition->m_sectorEnd = m_partition->m_sectorStart + (sizevalue * kMebiByte / m_partition->m_sectorSize);
    } else {
        //end
        partition->m_sectorEnd = m_partition->m_sectorEnd;
        partition->m_sectorStart = m_partition->m_sectorEnd - (sizevalue * kMebiByte / m_partition->m_sectorSize);
        partition->m_sectorSize = m_partition->m_sectorSize;
    }
    if(mountFileStr.isEmpty()) {
        partition->m_isMounted = false;
    } else {
        partition->m_isMounted = true;
        partition->m_mountPoint = mountFile->currentText();
//        m_delegate->updateMountPoint(partition, mountFile->currentText());
    }
    if(fileFormat->itemText(fileFormat->currentIndex()).contains("efi", Qt::CaseInsensitive)) {
        partition->fs = FSType::EFI;
        partition->m_isMounted= true;
        partition->m_mountPoint = "/boot/efi";
    } else
        partition->fs = findFSTypeByName(fileFormat->itemText(fileFormat->currentIndex()));
//    m_delegate->formatPartition(partition, partition->fs, mountpoint);


}

void CreatePartitionFrame::slotOKBtn()
{
    fileFormatStr = fileFormat->itemData(fileFormat->currentIndex()).toString();//防止初始值为空

    if(mountFile->currentText().compare("/")==0 )
//            && mountFileStr.compare("/")!=0 )//去掉此行，否则会由于mountFileStr与mountFile->currentText()值一直相同而造成以下逻辑永远进不去
        {
                Device::Ptr p;
                Partition::Ptr q;
                //遍历所有挂在磁盘，是否已存在root挂载点的分区
                for(int k=0;k<devlst->count();k++)
                {

                    p=(Device::Ptr)(devlst->at(k));
                    for(int i=0;i<p->partitions.count();i++)
                    {
                        q=(Partition::Ptr)(p->partitions.at(i));
                        if( q->m_mountPoint.compare("/")==0)
                        {
                            message=new MessageBox(this);
                            message->setMessageInfo(tr("The disk can only create one root partition!"));

                            message->setCancleHidden();
                            message->exec();
                            mountFile->setCurrentIndex(0);
                            QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers|QEventLoop::ExcludeUserInputEvents);
                            this->activateWindow();
                            return;
                        }
                    }
                }
		if( (sizeEdit->getValue()*kMebiByte) < 40*kGibiByte )
                {
                    message=new MessageBox(this);
                	message->setMessageInfo(tr("Root partition size is greater than 40GB."));//根分区大小大于40GB
                	message->setCancleHidden();
                	message->exec();
                    QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers|QEventLoop::ExcludeUserInputEvents);
                    this->activateWindow();
                	return;
                }

        }

    if(fileFormatStr.contains("efi", Qt::CaseInsensitive ))
        {
                Device::Ptr p;
                Partition::Ptr q;
                for(int k=0;k<devlst->count();k++)
                {
                    p=(Device::Ptr)(devlst->at(k));
                    for(int i=0;i<p->partitions.count();i++)
                    {
                        q=(Partition::Ptr)(p->partitions.at(i));
                        if( q->m_mountPoint.compare("/boot/efi")==0)
                        {
                          q->m_mountPoint="";
                          //  return;
                        }
                    }
                }
		
		if( (sizeEdit->getValue()*kMebiByte) < 256*kMebiByte || (sizeEdit->getValue()*kMebiByte) > 2*kGibiByte )
                {
		    message=new MessageBox(this);
                    message->setMessageInfo(tr("the EFI partition size should be set between 256MB and 2GB\n"));
                    message->setCancleHidden();
                    message->exec();
                    QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers|QEventLoop::ExcludeUserInputEvents);
                    this->activateWindow();
                    return;
                }

        }


    if(mountFile->currentText().compare("/boot")==0 &&
           mountFileStr.compare("boot")!=0 )//所有架构都不可有重复的boot挂载点
        {
                Device::Ptr p;
                Partition::Ptr q;
                //遍历所有挂在磁盘，是否已存在boot挂载点的分区
                for(int k=0;k<devlst->count();k++)
                {

                    p=(Device::Ptr)(devlst->at(k));
                    for(int i=0;i<p->partitions.count();i++)
                    {
                        q=(Partition::Ptr)(p->partitions.at(i));
                        if( q->m_mountPoint.compare("/boot")==0)
                        {
                            message=new MessageBox(this);
                            message->setMessageInfo(tr("The disk can only create one boot partition!"));

                            message->setCancleHidden();
                            message->exec();
                            mountFile->setCurrentIndex(0);
                            QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers|QEventLoop::ExcludeUserInputEvents);
                            this->activateWindow();
                            return;
                        }
                    }
                }
        }

    if(mountFile->currentText().compare("/boot")==0)//分区/boot的文件系统格式只能为ext4或ext3
    {
              if(!fileFormatStr.contains("ext4", Qt::CaseInsensitive) &&
                 !fileFormatStr.contains("ext3", Qt::CaseInsensitive))
              {
                  message=new MessageBox(this);
                  message->setMessageInfo(tr("The filesystem type of boot partition should be ext4 or ext3!"));
                  message->setCancleHidden();
                  message->exec();
                  mountFile->setCurrentIndex(0);
                  QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers|QEventLoop::ExcludeUserInputEvents);
                  this->activateWindow();
                  return;
              }

              if( (sizeEdit->getValue()*kMebiByte) < 2*kGibiByte )
              {
                  message=new MessageBox(this);
                  message->setMessageInfo(tr("Boot partition size should not be less than 2GB."));
                  message->setCancleHidden();
                  message->exec();
                  QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers|QEventLoop::ExcludeUserInputEvents);
                  this->activateWindow();
                  return;
              }
    }

    //手动输入挂载点判断 2021-3-22
    if(!mountFileStr.isEmpty()) {
        if(!mountFileStr.startsWith("/") && !mountFileStr.contains("unused", Qt::CaseInsensitive)) {
            mountFile->setItemText(0, "");
            message = new MessageBox();
            message->setMessageInfo(tr("Mount point starts with '/'"));
            message->setCancleHidden();
            message->exec();
            this->activateWindow();
//            if(message->exec() == QDialog::Accepted )//去掉此句，以防止点击右上角关闭按钮时跳过此段逻辑
            return;
        }
    }

    //遍历所有挂在磁盘，是否存在重复的挂载点
    if(!mountFile->currentText().isEmpty())
    {
        for(int k=0;k<devlst->count();k++)
        {
            Device::Ptr p=(Device::Ptr)(devlst->at(k));
            for(int i=0;i<p->partitions.count();i++)
            {
                Partition::Ptr q=(Partition::Ptr)(p->partitions.at(i));
                if( !q->m_mountPoint.isEmpty())
                {
                    if(mountFile->currentText().compare(q->m_mountPoint) == 0)
                    {
                        message=new MessageBox(this);
                        QString strM = tr("Mount point repeat:");
                        strM += " [";
                        strM += q->m_mountPoint;
                        strM += "]";
                        message->setMessageInfo(strM);
                        message->setCancleHidden();
                        message->exec();
                        mountFile->setCurrentIndex(0);
                        QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers|QEventLoop::ExcludeUserInputEvents);
                        this->activateWindow();
                        return;
                    }
                }
            }
        }
    }
    
    if(searchEfi)
    {
    	qDebug() << "Start search efi";	
	searchEfi=false;
        bool enEfi=false;
	Partition::Ptr qTest;
        for(int i=0;i<m_dev->partitions.count();i++)
	{
		qTest=(Partition::Ptr)(m_dev->partitions.at(i));
                if( qTest->m_mountPoint.compare("/boot/efi")==0)
                {
                	enEfi=true;
                }
	}
	
	Device::Ptr p;
        Partition::Ptr q;
        for(int k=0;k<devlst->count();k++)
        {
        	p=(Device::Ptr)(devlst->at(k));
                for(int i=0;i<p->partitions.count();i++)
                {
                	q=(Partition::Ptr)(p->partitions.at(i));
                        if( q->m_mountPoint.compare("/boot/efi")==0)
                        {
                        	if((enEfi) && (p->m_path != m_dev->m_path)){
					q->m_mountPoint="";
				}
                        }
                }
         }
    }

    Partition::Ptr partition(new Partition);
    setPartitionValue(partition);
    if(fileFormat->itemData(fileFormat->currentIndex()).toString().contains("unused", Qt::CaseInsensitive)) {
        partition->blFormatPartition = false;
        m_delegate->createPartition(m_partition,
                                    partition->m_type,
                                    startPosbl,
                                    FSType::Empty,
                                    partition->m_mountPoint,
                                    sizevalue * kMebiByte / m_partition->m_sectorSize,
                                    0);
    }
    if(fileFormatStr.contains("kylin-data", Qt::CaseInsensitive)) {
        partition->fs = FSType::KylinData;
    }
    if(fileFormatStr.contains("lvm-pv", Qt::CaseInsensitive)) {
        partition->fs = FSType::Pv;
    }
    if(fileFormatStr.contains("efi", Qt::CaseInsensitive)) {
        partition->m_mountPoint="/boot/efi";
    }

    m_delegate->createPartition(m_partition,
                                partition->m_type,
                                startPosbl,
                                partition->fs,
                                partition->m_mountPoint,
                                sizevalue * kMebiByte / m_partition->m_sectorSize);
    if(partition->fs == FSType::Pv){
        emit createLvm();
    }

    emit finished();
    //this->releaseKeyboard();
}

void CreatePartitionFrame::keyPressEvent(QKeyEvent *event)
{
    qDebug() << Q_FUNC_INFO << "create partition";
    if(event->key() == Qt::Key_Escape || event->key() == Qt::Key_Control)
    {
//        this->setFocus();
    }
    else if(event->key() == Qt::Key_Tab)
    {
        QWidget * tempwidget=QApplication::focusWidget();
        QString ss= tempwidget->objectName();
        int i=0;
        int b=i++;
    }
    else
        QWidget::keyPressEvent(event);
}

void CreatePartitionFrame::showEvent(QShowEvent *event)
{
    primaryPart->setFocus();//因为界面属性原因show后会重新设置焦点，加上此句话的焦点设置一共触发三次所以后续如果要在显示时一开始不显示焦点事件过滤器得将前三次视为第一次触发一次处理

}

void CreatePartitionFrame::addStyleSheet()
{
    QFile file(":/res/qss/CreatePartitionFrame.css");
    qDebug() << Q_FUNC_INFO << file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

void CreatePartitionFrame::translateStr()
{
    m_OKBtn->setText(tr("OK"));//确定
    fsLabel->setText(tr("File system type:"));//文件系统类型
    //fsLabel->setText(tr("Used to:"));//
    m_mainTitle->setText(tr("Create Partition"));//添加分区


    mountLabel->setText(tr("Mount point"));//挂载点

    positionLabel->setText(tr("Location for the new partition"));//新分区的位置
    endPosition->setText(tr("End of this space"));//剩余空间尾部
    startPosition->setText(tr("Beginning of this space"));//剩余空间头部

    typeLabel->setText(tr("Type for the new partition:"));//新分区的类型
    lvmPart->setText(tr("Logical"));//逻辑分区
    primaryPart->setText(tr("Primary"));//主分区
//    tipLabel->setText(tr("*Please ensure that the root partition size is at least 15G.\n"
//                         "*Please make sure the root partition is /dev/sda2 on FT1500A."));//*请保证分区大小至少为8G。   *FT1500A上请保证分区为/dev/sda2。

    sizeLabel->setText(tr("Size(MB)"));//新建分区大小
    fileFormat->clear();
    for(int i = 0; i < filesystemList.length(); i++ ) {
        QString str = filesystemList.at(i);
        if(str == "unused") {
            str = QObject::tr("unused");
        } else if(str == "kylin-data") {
            str = QObject::tr("kylin-data");
        }
        fileFormat->addItem(str);//显示的是国际化语言
        fileFormat->setItemData(i, filesystemList.at(i));//附加信息是纯英文，方便了以后的查询对比
    }
}

void CreatePartitionFrame::clickPrimaryPart()
{
    priTypebl = true;
    primaryPart->setChecked(priTypebl);
    lvmPart->setChecked(!priTypebl);
}
void CreatePartitionFrame::clickLvmPart()
{
    priTypebl = false;
    primaryPart->setChecked(priTypebl);
    lvmPart->setChecked(!priTypebl);
}
void CreatePartitionFrame::clickStartPos()
{
    startPosbl = true;
    startPosition->setChecked(startPosbl);
    endPosition->setChecked(!startPosbl);
}
void CreatePartitionFrame::clickEndPos()
{
    startPosbl = false;
    startPosition->setChecked(startPosbl);
    endPosition->setChecked(!startPosbl);
}
void CreatePartitionFrame::changeFileFormat(int index)
{
    fileFormatStr = fileFormat->itemData(index).toString();
    fileFormat->setCurrentText(fileFormatStr);
    sizeEdit->setValue(sizeStr);
    
    
    if(fileFormatStr.contains("swap", Qt::CaseInsensitive) || fileFormatStr.contains("efi", Qt::CaseInsensitive)
            || fileFormatStr.contains("unused", Qt::CaseInsensitive) ) {
        mountFile->setEnabled(false);
        mountFile->setCurrentText("");
    }
    else if(fileFormatStr.contains("kylin-data", Qt::CaseInsensitive)) {
	if(mountFile->currentText().compare("/data")!=0 ) {
		mountFile->setCurrentText("/data");
        	mountFile->setEnabled(true);
        	fileFormat->setEnabled(true);
	}

    }
    else if(fileFormatStr.contains("lvm-pv", Qt::CaseInsensitive)) {
        mountFile->setCurrentText("");
        mountFile->setEnabled(false);
    }
    else {
        mountFile->setEnabled(true);
    }
    if(fileFormatStr.contains("efi", Qt::CaseInsensitive)) {
	if(sizeStr.toInt()>=512){
		sizeEdit->m_ledit->setText("512");
	}
	mountFile->setCurrentText("/boot/efi");
	tipLabel->setText(tr("Recommended efi partition size is between 256MB and 2GB."));//EFI分区大小256MB到2GB之间
    }
    else if(fileFormatStr.contains("swap", Qt::CaseInsensitive)) {
	if(sizeStr.toInt()>=swap_default_size){
		sizeEdit->m_ledit->setText(QString::number(swap_default_size));
	}
    }
    else if(fileFormatStr.contains("kylin-data", Qt::CaseInsensitive)) {
	if(sizeStr.toInt()>=data_default_size){
		sizeEdit->m_ledit->setText(QString::number(data_default_size));
	}
        tipLabel->setText("");
    } else {
        tipLabel->setText("");
    }

    if((mountFile->currentText().compare("/data")==0)) {
    	if((fileFormatStr.contains("kylin-data", Qt::CaseInsensitive))) {
		 qDebug() << "change is ok";
	} else {
		mountFile->setCurrentText("");
	}
    }
    
    if((mountFile->currentText().compare("/boot/efi")==0)) {
    	if((fileFormatStr.contains("efi", Qt::CaseInsensitive))) {
		 qDebug() << "change is ok";
	} else {
		mountFile->setCurrentText("");
	}
    }
    
    if((fileFormat->currentText().isEmpty()==false)) {
        if((mountFile->currentText().isEmpty()==false) || mountFile->isEnabled()==false ) {
                if((mountFileStr == "/data") || (mountFileStr == "/boot") ||(mountFileStr == "/boot/efi")) {
                        sizeEdit->m_ledit->setFocus();
		} else if((mountFileStr == "/backup") ||(mountFileStr == "/tmp")) {
                        sizeEdit->m_ledit->setFocus();
                } else if(fileFormatStr.contains("swap", Qt::CaseInsensitive)){
                        sizeEdit->m_ledit->setFocus();
                }
        }
    }
   

}
void CreatePartitionFrame::changeMountFile(const QString &text)
{
    mountFileStr = text;
    fileFormat->setEnabled(true);	
    mountFile->setCurrentText(mountFileStr);
    sizeEdit->setValue(sizeStr);


    if(text.contains("/boot", Qt::CaseInsensitive)) {
    	if(text.contains("/efi", Qt::CaseInsensitive)) {
		qDebug() << "This is efi";
		if(sizeStr.toInt()>=512){
			sizeEdit->m_ledit->setText("512");
		}
	}else{
		if(sizeStr.toInt()>=2048){
			sizeEdit->m_ledit->setText("2048");
		}
        	tipLabel->setText(tr("Boot partition size should not be less than 2GB."));//boot分区不可小于2GB
	}
    } else if(text == "/") {
	if(sizeStr.toInt()>=root_default_size){
		sizeEdit->m_ledit->setText(QString::number(root_default_size));
	}
        tipLabel->setText(tr("Root partition size is greater than 40GB."));//根分区大小大于40GB
    } else if(text == "/data") {
	fileFormat->setCurrentText(QObject::tr("kylin-data"));
	fileFormat->setEnabled(true);
        tipLabel->setText("");
    } else if(text == "/backup") {
	if(sizeStr.toInt()>=backup_default_size){
		sizeEdit->m_ledit->setText(QString::number(backup_default_size));
	}
    } else {
        tipLabel->setText("");
    }
    if((text != "/data") && (fileFormatStr.contains("kylin-data", Qt::CaseInsensitive))) {
	fileFormat->setCurrentText("ext4");
    }
    
    if((fileFormat->currentText().isEmpty()==false)) {
        if((mountFile->currentText().isEmpty()==false) || mountFile->isEnabled()==false ) {
                if((mountFileStr == "/data") || (mountFileStr == "/boot") ||(mountFileStr == "/boot/efi")) {
                        sizeEdit->m_ledit->setFocus();
		} else if((mountFileStr == "/backup") ||(mountFileStr == "/tmp")) {
                        sizeEdit->m_ledit->setFocus();
                } else if(fileFormatStr.contains("swap", Qt::CaseInsensitive)){
                        sizeEdit->m_ledit->setFocus();
                }
        }
    }

}


bool CreatePartitionFrame::eventFilter(QObject *watched, QEvent *event)
{
    if(watched->objectName() == "close") {
        if(event->type() == QEvent::Enter) {
            if(m_tTip != nullptr) {
                m_tTip->close();
            }
            m_tTip = new ArrowWidget();
            m_tTip->setObjectName("ArrowWidget");
            m_tTip->setDerection(ArrowWidget::Derection::down);
            m_tTip->setText(tr("close"));
            m_tTip->move(m_close->x() + (m_close->width() - m_tTip->width()) / 2, m_close->y() + m_close->height() + 4);
//            w->move(pos.x() - 20, pos.y() - w->height());
            m_tTip->show();
            connect(m_close, &QPushButton::clicked, [=]{m_tTip->close();});

        } else if(event->type() == QEvent::Leave){
            if(m_tTip != nullptr) {
                m_tTip->close();
            }
        }
    }

    if(watched==primaryPart&&event->type()==QEvent::FocusIn)
    {
        m_focusgetnum++;
        if(m_focusgetnum==1)//第一次进入设置焦点隐藏
        {
                                primaryPart->setStyleSheet(QString(
                                                         " QRadioButton#%1::indicator::checked:focus {"
                                                         "image: url(:/res/pic/radiochecked.svg);"
                                                         "}"
                                                         ).arg(primaryPart->objectName()));
        }
        else if(m_focusgetnum>3)//第三次后证明是人为触发设置焦点显示
        {
            primaryPart->setStyleSheet(QString(
                                           " QRadioButton#%1::indicator::checked:focus {"
                                           "image: url(:/res/pic/radio-checked-focus.svg);"
                                           "}"
                                           ).arg(primaryPart->objectName()));
            m_focusgetnum=4;//防止计数越界
        }

    }
    return QWidget::eventFilter(watched, event);
}


}
