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

#include "modifypartitionframe.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QEvent>
#include <QRect>
#include <QDebug>
#include <QTextStream>
#include <QListView>
#include <QStyledItemDelegate>
#include <QGridLayout>
#include <QScrollBar>
#include <QTimer>

#include "../PluginService/ksystemsetting_unit.h"
#include "../PluginService/ui_unit/xrandrobject.h"
#include "../PluginService/sysInfo/kernelfilesystemfs.h"
#include "../PluginService/sysInfo/machine.h"
#include "QMessageBox"
#include "partman/partition_unit.h"

namespace KInstaller {

ModifyPartitionFrame::ModifyPartitionFrame(CustomPartitiondelegate* delegate, QWidget *parent) : QDialog(parent),m_delegate(delegate),
    mountFileStr(""),
    closeTip(nullptr)
{
    this->setObjectName("ModifyPartitionFrame");
    initUI();
    addStyleSheet();
    initAllConnect();
    initBackground();
    this->setWindowFlags(Qt::Sheet|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);//保证界面显示在最上层且设置为窗口模式和应用模态，这样焦点就不会跑到下层界面上
    //setWindowModality(Qt::ApplicationModal);

}

ModifyPartitionFrame::~ModifyPartitionFrame()
{

}

void ModifyPartitionFrame::setModifyPartition( Partition::Ptr partition,int SCreenWidth,int SCreenHeight)//设置此窗口中以下控件的显示内容：文件类型下拉框，格式化此分区勾选框，挂载点下拉框。
{
    qDebug() << "setModifyPartition start";
    m_partition = partition;
    mountFileStr = partition->m_mountPoint;
    fileFormatStr = findNameByFSType(partition->fs);
    isPvFormat = fileFormatStr;
    realFileFormat = findNameByFSType(partition->realFs);
    qDebug() << "setModifyPartition realFileFormat=" << realFileFormat;
    qDebug() << "setModifyPartition fileFormatStr=" << fileFormatStr;
    qDebug() << "setModifyPartition mountFileStr=" << mountFileStr;

    if(partition->m_partitionPath.contains("lvm-lv")){
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

    if(fileFormatStr == "") {
        fileFormat->setCurrentText(tr("unused"));
    }
    else if (fileFormatStr.contains("efi")) {
        fileFormat->setCurrentText("efi");
        mountFile->setEnabled(false);
        mountFile->setCurrentText(mountFileStr);
    }
    else if (fileFormatStr.contains("swap")) {
        fileFormat->setCurrentText("linux-swap");
        mountFile->setEnabled(false);
        mountFile->setCurrentText(mountFileStr);
    }
    else if (fileFormatStr.contains("kylin-data")) {
        fileFormat->setCurrentText(QObject::tr("kylin-data"));//解决在中文环境下修改“用户数据分区”时初始值不对的问题
        mountFile->setEnabled(false);
       //mountFile->setCurrentText(mountFileStr);
	mountFile->setCurrentText("/data");
    }
    else if (fileFormatStr.contains("lvm-pv")) {
        fileFormat->setCurrentText("lvm-pv");
        mountFile->setEnabled(false);
        mountFile->setCurrentText(mountFileStr);
    }
    else
    {
        fileFormat->setCurrentText(fileFormatStr);
        mountFile->setEnabled(true);
        mountFile->setCurrentText(mountFileStr);
    }

    if(partition->blFormatPartition) {
	qDebug() << "setModifyPartition partition->blFormatPartition=true" ;
        checkFormat->setChecked(true);
        checkFormat->setDisabled(false);
    }
    else {
       	checkFormat->setChecked(false);
        checkFormat->setDisabled(false);
    }
    if((realFileFormat != fileFormatStr) && (!fileFormatStr.contains("unused", Qt::CaseInsensitive))) {
	qDebug() << "setModifyPartition realFileFormat != fileFormatStr)" ;
        checkFormat->setChecked(true);
        checkFormat->setEnabled(false);
    }
    m_SCreenWidth=SCreenWidth;
    m_SCreenHeight=SCreenHeight;
    //setFixedSize(m_SCreenWidth, m_SCreenHeight);
    setMinimumSize(m_SCreenWidth, m_SCreenHeight);

    update();

    qDebug() << "setModifyPartition realFileFormat=" << realFileFormat;
    qDebug() << "setModifyPartition fileFormatStr=" << fileFormatStr;
    qDebug() << "setModifyPartition end";
}

void ModifyPartitionFrame::initUI()
{
    QGridLayout* gLayout = new QGridLayout;
    gLayout->setMargin(24);
    this->setLayout(gLayout);
    gLayout->setColumnStretch(0, 3);
    gLayout->setColumnStretch(1, 2);
    gLayout->setColumnStretch(2, 3);
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
    gLayout->addWidget(m_mainTitle, 2, 1, 1, 1, Qt::AlignCenter | Qt::AlignTop);
    gLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred), 3, 1, 1, 1);

    fsLabel= new QLabel;
    fsLabel->setObjectName("label");
    fileFormat = new QComboBox;
    fileFormat->installEventFilter(this);
    fileFormat->setObjectName("fileFormat");
    fileFormat->setWindowFlags(Qt::FramelessWindowHint);
    fileFormat->setAttribute(Qt::WA_TranslucentBackground, true);

    QWidget* containerObj1 = static_cast<QWidget*>(fileFormat->view()->parent()); // QComboBoxPrivateContainer对象
    containerObj1->setAttribute(Qt::WA_TranslucentBackground, true);
    //    fileFormat->setView(new QListView());
    //    fileFormat->view()->parentWidget()->setWindowFlags(Qt::FramelessWindowHint);
    //    fileFormat->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground,true);
    fileFormat->setObjectName("combox");
    QStyledItemDelegate* itemDelegate = new QStyledItemDelegate();
    fileFormat->setItemDelegate(itemDelegate);
    //    filesystemList = m_delegate->getFsTypeList();
    //2021-5-24根据内核支持的文件系统对文件系统进行加载
    filesystemList = getKernelFs();
    if(!isEFIEnabled())
    {
        filesystemList.removeAll("efi");
    }

    fileFormat->setMaxVisibleItems(5);
    fileFormat->setView(new QListView());
    fileFormat->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    fileFormat->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    fileFormat->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    fileFormat->view()->verticalScrollBar()->setStyleSheet("QScrollBar{width:15px;}");
    fileFormat->view()->verticalScrollBar()->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    gLayout->addWidget(fsLabel, 4, 1, 1, 1, Qt::AlignLeft);
    gLayout->addWidget(fileFormat, 5, 1, 1, 1);


    checkFormat = new QCheckBox(this);
    //checkFormat->adjustSize();
    gLayout->addWidget(checkFormat, 6, 1, 1, 1, Qt::AlignLeft);
    // checkFormat->setStyleSheet("QCheckBox { font-size: 14px; font-family: Noto Sans CJK SC; font-weight: 400; line-height: 64px; color: #FFFFFF; background:transparent;opacity: 1;}"
    //                            "QCheckBox::indicator:enabled:unchecked:focus {image: url(:/res/pic/checkboxcheck-hover.svg);}"
    //                            "QCheckBox::indicator:disabled:unchecked { image: url(:/res/pic/discheckboxcheck.svg); }"
    //                            "QCheckBox::indicator:enabled:unchecked {image: url(:/res/pic/checkboxcheck.svg); }"
    //                            "QCheckBox::indicator:disabled:checked {image: url(:/res/pic/discheckboxchecked.svg);}"
    //                            "QCheckBox::indicator:enabled:checked {image: url(:/res/pic/checkboxchecked.svg);}"
    //            );
    gLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Preferred), 7, 1);

    mountLabel= new QLabel;
    mountLabel->setObjectName("label");

    mountFile = new QComboBox;
    mountFile->setObjectName("combox");
    mountFile->setView(new QListView());
    mountFile->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    mountFile->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    mountFile->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    
    mountFile->view()->verticalScrollBar()->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    mountFile->view()->verticalScrollBar()->setStyleSheet("QScrollBar{width:15px;}");
    mountFile->setItemDelegate(itemDelegate);
    mountFile->setWindowFlags(Qt::FramelessWindowHint);
    mountFile->setAttribute(Qt::WA_TranslucentBackground, true);
    QWidget* containerObj = static_cast<QWidget*>(mountFile->view()->parent()); // QComboBoxPrivateContainer对象
    containerObj->setAttribute(Qt::WA_TranslucentBackground, true);
    //    mountFile->setView(new QListView());
    //    mountFile->view()->parentWidget()->setWindowFlags(Qt::FramelessWindowHint);
    //    mountFile->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground,true);
    mountedList = m_delegate->getMountPoints();
    mountFile->addItems(mountedList);
    mountFile->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    mountFile->setMaxVisibleItems(5);
    mountFile->setEditable(true);
//    mountFile->setStyleSheet("QComboBox::drop-down{ width: 48px; border: 4px solid transparent; margin-right: 8px; margin-left: 8x; border-radius: 8px;}"
//                             "QComboBox::down-arrow{ right: 1px; width: 12px; height: 9px; image: url(:/data/png/pan-down-symbolic.svg);}"
//                             "QComboBox::down-arrow:on{ width: 12px; height: 9px; border: 0px solid transparent ; image: url(:/data/png/pan-up-symbolic.svg);}"
//                            );

    gLayout->addWidget(mountLabel, 8, 1, 1, 1, Qt::AlignLeft);
    gLayout->addWidget(mountFile, 9, 1, 1, 1);
    gLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Preferred), 10, 1);

    tipLabel = new QLabel;
    gLayout->addWidget(tipLabel, 11, 1, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    gLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred), 12, 1, 1, 1);
    gLayout->setRowStretch(13, 1);

    m_OKBtn = new QPushButton;
    m_OKBtn->setObjectName("OKBtn");
    gLayout->addWidget(m_OKBtn, 14, 1, 1, 1, Qt::AlignCenter);
    gLayout->setRowStretch(15, 1);
    translateStr();
}

void ModifyPartitionFrame::initAllConnect()
{
    connect(m_close, &QPushButton::clicked, [=]{this->close();});
    connect(m_OKBtn, &QPushButton::clicked, this, &ModifyPartitionFrame::slotOKBtn);

    connect(fileFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(changeFileFormat(int)));
    connect(mountFile, &QComboBox::currentTextChanged, this, &ModifyPartitionFrame::changeMountFile);
    connect(this, &ModifyPartitionFrame::destroyed, this, &ModifyPartitionFrame::deleteLater);
}

void ModifyPartitionFrame::translateStr()
{
    checkFormat->setText(tr("Format partition."));//格式化此分区
    m_OKBtn->setText(tr("OK"));//确定
    //    fsLabel->setText(tr("File system type:"));//文件系统类型
    fsLabel->setText(tr("Used to:"));//
    m_mainTitle->setText(tr("Modify Partition"));//添加分区
    //    tipLabel->setText(tr("*Please ensure that the root partition size is at least 15G.\n"
    //                         "*Please make sure the root partition is /dev/sda2 on FT1500A."));//*请保证分区大小至少为8G。   *FT1500A上请保证分区为/dev/sda2。


    mountLabel->setText(tr("Mount point"));//挂载点
    fileFormat->clear();
    for(int i = 0; i < filesystemList.length(); i++ ) {
        QString str = filesystemList.at(i);
        if(str == "unused") {
            str = tr("unused");
        } else if(str == "kylin-data") {
            str = QObject::tr("kylin-data");
        }
        fileFormat->addItem(str);//显示的是国际化语言
        fileFormat->setItemData(i, filesystemList.at(i));//附加信息是纯英文，方便了以后的查询对比
    }
}

void ModifyPartitionFrame::addStyleSheet()
{
    QFile file(":/res/qss/CreatePartitionFrame.css");
    qDebug() << Q_FUNC_INFO << file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

void ModifyPartitionFrame::initBackground()
{
    // 无边框
//        setWindowFlags(Qt::FramelessWindowHint |
//                       Qt::CustomizeWindowHint|
//    //                   Qt::WindowStaysOnTopHint |
//                       Qt::Tool |
//                       Qt::Dialog);
//    setWindowFlags(Qt::FramelessWindowHint);//bug293375

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

void ModifyPartitionFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}

void ModifyPartitionFrame::changeFileFormat(int index)
{
    m_filesystem = fileFormat->itemData(index).toString();
    qDebug() << "changeFileFormat start";
    qDebug() << "changeFileFormat realFileFormat=" << realFileFormat; 
    qDebug() << "changeFileFormat fileFormatStr=" << fileFormatStr; 
    qDebug() << "changeFileFormat m_filesystem=" << m_filesystem; 
    qDebug() << "changeFileFormat mountFileStr=" << mountFileStr; 
    //if(mountFileStr != m_filesystem) {
    if(realFileFormat != m_filesystem) {
        checkFormat->setChecked(true);
        checkFormat->setEnabled(false);
    } else {
        checkFormat->setChecked(true);
        checkFormat->setEnabled(true);
    }
    if(m_filesystem.contains("swap", Qt::CaseInsensitive) || m_filesystem.contains("unused", Qt::CaseInsensitive)) {
        mountFile->setDisabled(true);
        mountFile->setCurrentText("");
        mountFileStr = "";
        checkFormat->setChecked(false);

    } else if(m_filesystem.contains("efi", Qt::CaseInsensitive)) {
        mountFile->setCurrentText("/boot/efi");
        mountFileStr = "/boot/efi";
        mountFile->setDisabled(true);
    } else if(m_filesystem.contains("kylin-data", Qt::CaseInsensitive)) {
        if(mountFile->currentText().compare("/data")!=0 ) {
                mountFile->setCurrentText("/data");
                mountFile->setEnabled(false);
                fileFormat->setEnabled(true);
        }
    } else if(m_filesystem.contains("lvm-pv", Qt::CaseInsensitive)) {
         mountFile->setCurrentText("");
        mountFile->setEnabled(false);
    } else if(m_filesystem.contains("ext4", Qt::CaseInsensitive)) {
        mountFile->setDisabled(false);
    	if((mountFile->currentText().compare("/boot/efi")==0)) {
        	mountFile->setCurrentText("");
	}
    } else {
        mountFile->setDisabled(false);
        mountFile->setCurrentText("");
    }
    if((mountFile->currentText().compare("/data")==0)) {
        if((m_filesystem.contains("kylin-data", Qt::CaseInsensitive))) {
                 qDebug() << "change is ok";
        } else {
                mountFile->setCurrentText("");
        }
    }
    if((realFileFormat != m_filesystem) && (!m_filesystem.contains("unused", Qt::CaseInsensitive))) {
    	qDebug() << "changeFileFormat realFileFormat != m_filesystem "; 
        checkFormat->setChecked(true);
        checkFormat->setEnabled(false);
    }
    
    if(m_filesystem.contains("kylin-data", Qt::CaseInsensitive) && (fileFormatStr == "ext4")) {
        checkFormat->setChecked(true);
        checkFormat->setEnabled(true);
    }

    if(m_filesystem.contains("kylin-data", Qt::CaseInsensitive) && (realFileFormat.contains("kylin-data", Qt::CaseInsensitive))) {
        checkFormat->setChecked(false);
        checkFormat->setEnabled(true);
    }

    qDebug() << "changeFileFormat realFileFormat=" << realFileFormat; 
    qDebug() << "changeFileFormat fileFormatStr=" << fileFormatStr; 
    qDebug() << "changeFileFormat m_filesystem=" << m_filesystem; 
    qDebug() << "changeFileFormat mountFileStr=" << mountFileStr; 
    qDebug() << "changeFileFormat end";


}

void ModifyPartitionFrame::changeMountFile(const QString &text)
{

    qDebug() << "changeMountFile start";
    qDebug() << "changeMountFile realFileFormat=" << realFileFormat; 
    qDebug() << "changeMountFile fileFormatStr=" << fileFormatStr; 
    qDebug() << "changeMountFile text=" << text; 
    qDebug() << "changeMountFile m_filesystem=" << m_filesystem; 
    if(text == "/") {
        checkFormat->setChecked(true);
        checkFormat->setEnabled(false);
    }else if(text == "/data") {
        fileFormat->setCurrentText(QObject::tr("kylin-data"));
        //fileFormat->setEnabled(false);
    }
    else{

        if(fileFormatStr == m_filesystem )
        {
            checkFormat->setChecked(false);
            checkFormat->setEnabled(true);
        }
        else{//如果当前分区类型下拉框值与初始系统类型不同，就默认格式化此分区
            checkFormat->setChecked(true);
            checkFormat->setEnabled(true);
        }
    }
    if((text != "/data") && (fileFormat->currentText().contains(QObject::tr("kylin-data")))) {
    	fileFormat->setCurrentText("ext4");
        fileFormat->setEnabled(true);
    }
    
    if(m_filesystem.contains("kylin-data", Qt::CaseInsensitive) && (realFileFormat.contains("kylin-data", Qt::CaseInsensitive))) {
        checkFormat->setChecked(false);
        checkFormat->setEnabled(true);
    }
    
    qDebug() << "changeMountFile realFileFormat=" << realFileFormat; 
    qDebug() << "changeMountFile fileFormatStr=" << fileFormatStr; 
    qDebug() << "changeMountFile end";
    //  mountFileStr = text;

}

void ModifyPartitionFrame::slotOKBtn()
{
//    //龙芯只允许一个boot分区
//    if(getComputerArch()==MachineArch::LOONGSON && 0)
//    {

        m_filesystem = fileFormat->itemData(fileFormat->currentIndex()).toString();//防止初始值为空

        if(mountFile->currentText().compare("/boot")==0)//分区/boot的文件系统格式只能为ext4或ext3
        {
                if(!m_filesystem.contains("ext4", Qt::CaseInsensitive) &&
                   !m_filesystem.contains("ext3", Qt::CaseInsensitive))//
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

                if(2*kGibiByte > m_partition->m_total)
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

        if(mountFile->currentText().compare("/boot")==0 &&
               mountFileStr.compare("/boot")!=0 )//所有架构都不可有重复的boot挂载点
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
                        if( q->m_mountPoint.contains("boot"))

                        {
                            if(FSType::EFI == q->fs)//过滤掉efi分区
                            {
                                continue;
                            }

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
//    }

        if(mountFile->currentText().compare("/")==0 &&
               mountFileStr.compare("/")!=0 )//所有架构都不可有重复的/根挂载点
        {
                Device::Ptr p;
                Partition::Ptr q;
                //遍历所有挂在磁盘，是否已存在根挂载点的分区
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
        }

    mountFileStr = mountFile->currentText();
    //手动输入挂载点判断 2021-3-22
    if(!mountFileStr.isEmpty()) {
        if(!mountFileStr.startsWith("/") && !mountFileStr.contains("unused", Qt::CaseInsensitive)) {
            mountFile->setItemText(0, "");
            message = new MessageBox(this);
            message->setMessageInfo(tr("Mount point starts with '/'"));
            message->setCancleHidden();
            message->exec();
            QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers|QEventLoop::ExcludeUserInputEvents);
            this->activateWindow();
//            if(message->exec() == QDialog::Accepted )//去掉此句，以防止点击右上角关闭按钮时跳过此段逻辑
            return;
        }
    }
    
    if(searchEfi)
    {
        qDebug() << "1Start search efi";
        searchEfi=false;
        bool enEfi=false;
        
        Device::Ptr pTest;
	for(int j=0;j<devlst->count();j++)
	{
		pTest=(Device::Ptr)(devlst->at(j));
		if(m_partition->m_devPath == pTest->m_path)
		{
			Partition::Ptr qTest;
        		for(int i=0;i<pTest->partitions.count();i++)
        		{
                		qTest=(Partition::Ptr)(pTest->partitions.at(i));
                		if( qTest->m_mountPoint.compare("/boot/efi")==0)
                		{
                        		enEfi=true;
                		}
        	}


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
                                if((enEfi) && (p->m_path != m_partition->m_devPath)){
        qDebug() << "10Start search efi";
                                        q->m_mountPoint="";
                                }
                        }
                }
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
                if( !q->m_mountPoint.isEmpty() &&  m_partition != q)
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

    Partition::Ptr partition(m_partition);
    //qDebug() <<"222222 partition->fs=" <<findNameByFSType(partition->fs);
    //qDebug() <<"222222 partition->realFs=" <<findNameByFSType(partition->realFs);
    bool checked;
    if(m_filesystem == ("unused")) {
        partition->blFormatPartition = false;
        //        checkFormat->setCheckable(false);
        mountFileStr = "";
//        partition->m_mountPoint = "";
        partition->m_isMounted = false;

        if (fileFormatStr.contains("efi"))
        {
                if(filesystemList.contains("fat32", Qt::CaseInsensitive))
                {
                    partition->fs = FSType::Fat32;
                }
                else if(filesystemList.contains("fat16", Qt::CaseInsensitive))
                {
                    partition->fs = FSType::Fat16;
                }
                else
                {
                    partition->fs = FSType::VFat;
                }
        }
        else if (fileFormatStr.contains("swap")) {
                partition->fs = FSType::Empty;
        }
        else
        {
            partition->fs = findFSTypeByName(filesystemList.at(0));
        }

        m_delegate->resetOperationFormat(partition->m_partitionPath);
        m_delegate->updateMountPoint(partition, mountFileStr);
        //        m_delegate->formatPartition(partition, partition->fs, mountFileStr, false);

    } else {
        if(m_filesystem == "kylin-data") {
            partition->fs = FSType::KylinData;
            partition->blFormatPartition = checkFormat->isChecked();//bug204163
            if(checkFormat->isChecked())//需要格式化
            {
                m_delegate->formatPartition(partition, partition->fs, mountFileStr, true);
            }
            else//不需要格式化
                m_delegate->resetOperationFormat(partition->m_partitionPath);

        } else {
//            m_filesystem = fileFormat->itemData(fileFormat->currentIndex()).toString();
            FSType tmpfs = findFSTypeByName(m_filesystem);
            checked = checkFormat->isChecked();
            bool checkChanged = false;
            if(!checked && partition->blFormatPartition)
                checkChanged = true;

            partition->blFormatPartition = checked;
            qDebug()<<"currentFormat="<<(int)checked;

            if(m_partition->fs != tmpfs || checked ) { //如果改变了分区类型或者勾选了格式化勾选框，就格式化此分区
                partition->fs = tmpfs;
                qDebug()<<"currentfilesystemType="<<m_filesystem;
                m_delegate->formatPartition(partition, tmpfs, mountFileStr, true);
            }
            else if(checkChanged)//将此分区从格式化改为不格式化
            {
                m_delegate->resetOperationFormat(partition->m_partitionPath);
            }
        }
	if(m_filesystem == "lvm-pv") {
                partition->fs = FSType::Pv;
        }

        if(m_partition->m_mountPoint != mountFile->currentText()) {//如果改变了挂载点
            qDebug()<<"currentMountPoint="+mountFile->currentText();
//            partition->m_mountPoint =mountFile->currentText();
            m_delegate->updateMountPoint(partition, mountFile->currentText());
        }
    }
    qDebug()<<"isPvFormat="<<isPvFormat;
    if(partition->fs == FSType::Pv || isPvFormat.contains("lvm-pv", Qt::CaseInsensitive)){
        emit createLvm();
    }
    emit finished();
}

void ModifyPartitionFrame::showEvent(QShowEvent *event)
{
    fileFormat->setFocus();//放在showevent中会导致foucsin事件触发多次达不到第一次隐藏焦点边框要求固移到构造函数中
}

void ModifyPartitionFrame::keyPressEvent(QKeyEvent *event)
{
    if((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return))
        m_OKBtn->click();
    else
        QWidget::keyPressEvent(event);
}

bool ModifyPartitionFrame::eventFilter(QObject *watched, QEvent *event)
{
    if(watched->objectName() == "close") {
        if(event->type() == QEvent::Enter) {
            if(closeTip != nullptr) {
                closeTip->close();
            }
            closeTip = new ArrowWidget();
            closeTip->setObjectName("ArrowWidget");
            closeTip->setDerection(ArrowWidget::Derection::down);
            closeTip->setText(tr("close"));

            closeTip->move(m_close->x() + (m_close->width() - closeTip->width()) / 2, m_close->y() + m_close->height() + 4);
            //            w->move(pos.x() - 20, pos.y() - w->height());
            closeTip->show();
            connect(m_close, &QPushButton::clicked, [=]{closeTip->close();});
            return true;

        } else if(event->type() == QEvent::Leave){
            if(closeTip != nullptr) {
                closeTip->close();
                return false;
            }
        }
    }

    if(watched==fileFormat&&event->type()==QEvent::FocusIn)
    {
        m_focusgetnum++;
        if(m_focusgetnum==1)//第一次进入设置焦点隐藏
        {
            fileFormat->setStyleSheet(QString(
                                           " QComboBox#%1:focus {"
                                           "border: 2px solid transparent;"
                                           "}"
                                           ).arg(fileFormat->objectName()));
        }
        else if(m_focusgetnum>3)//第三次后证明是人为触发设置焦点显示
        {
            fileFormat->setStyleSheet(QString(
                                           " QComboBox#%1:focus {"
                                           "border: 2px solid white;"
                                           "}"
                                           ).arg(fileFormat->objectName()));
            m_focusgetnum=4;//防止计数越界
        }

    }
    return QWidget::eventFilter(watched, event);
}


}
