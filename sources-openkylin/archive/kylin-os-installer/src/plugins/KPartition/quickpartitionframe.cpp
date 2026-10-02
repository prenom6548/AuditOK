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

#include "quickpartitionframe.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QKeyEvent>
#include "../PluginService/kcommand.h"
//#include <kysdk/kysdk-system/libkysysinfo.h>
#include "delegate/customize_device_model.h"
#include "delegate/full_partition_delegate.h"
#include "partman/partition_server.h"
#include "../PluginService/ui_unit/messagebox.h"
#include "./frames/encryptsetframe.h"
#include "../PluginService/base_unit/file_unit.h"

namespace KInstaller {
using namespace Partman;
FullPartitionFrame::FullPartitionFrame(FullPartitionDelegate* quickDelegate, QWidget *parent) : QWidget(parent),
    m_quickDelegate(quickDelegate),
    m_devPath(""),
     var(0)
{
    
    initUI();
    addStyleSheet();
    initAllConnect();
    m_display = false;
    this->setFocus();
    m_savehistory->setVisible(true);
}
void FullPartitionFrame::initUI()
{
    QGridLayout* gLayout = new QGridLayout();
    gLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(gLayout);
    gLayout->setColumnStretch(0, 3);
    gLayout->setColumnStretch(1, 2);
    gLayout->setColumnStretch(2, 3);
    slistDisk = new LevelScrollDiskView();
    slistDisk->setObjectName("slistDisk");
    m_midStackLayout = new QStackedLayout();
    m_midStackLayout->setContentsMargins(0, 0, 0, 0);
    gLayout->addLayout(m_midStackLayout, 2, 1, 1, 1);
    m_midStackLayout->addWidget(slistDisk);

    //2021-4-16 当只有一个磁盘，而且不能选时（50G）提示信息
    m_tip = new QLabel;
    m_tip->hide();
    gLayout->addWidget(m_tip, 3, 1, 1, 1, Qt::AlignCenter);

    gLayout->addItem(new QSpacerItem(10, 190, QSizePolicy::Expanding, QSizePolicy::Expanding), 4, 1, 1, 1);
    gLayout->addItem(new QSpacerItem(10, 190, QSizePolicy::Expanding, QSizePolicy::Expanding), 5, 1, 1, 1);

     QHBoxLayout *midhlayout=new QHBoxLayout;
     midhlayout->setSpacing(40);

    //添加出厂备份
    m_firstback = new QCheckBox;
    m_firstback->setHidden(true);
    m_firstback->setObjectName("m_firstback");

    //全盘安装加密
    m_encryptBox = new QCheckBox;
    m_encryptBox->setHidden(true);
    m_encryptBox->setObjectName("encryptBox");

    //全盘lvm安装
    m_lvm = new QCheckBox;
    m_lvm->setHidden(true);
    m_lvm->setObjectName("lvm");

    //保留数据安装
    m_savehistory = new QCheckBox;
    //m_savehistory->setDisabled(true);
    m_savehistory->setVisible(false);
    m_savehistory->setObjectName("savehistoryBox");
   
    //虚拟机下不显示“出厂备份”勾选框
    if(KServer::KReadFile("/var/log/installer/os-prober.log").contains("virtual-machine=true")){
        //虚拟机不显示备份还原按钮
        firstbackShow = false;
        WriteSettingToIni("config", "virtual-machine", "true");
        midhlayout->addItem(new QSpacerItem(1000, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
        midhlayout->addWidget(m_encryptBox,Qt::AlignRight);
    } else {
        //物理机
        firstbackShow = true;
        midhlayout->addItem(new QSpacerItem(1000, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
        midhlayout->addWidget(m_firstback, Qt::AlignLeft);
        midhlayout->addWidget(m_encryptBox,Qt::AlignRight);

    }


    midhlayout->addWidget(m_lvm,Qt::AlignRight);
    midhlayout->addWidget(m_savehistory,Qt::AlignRight);
    midhlayout->addItem(new QSpacerItem(1000, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));

    gLayout->addLayout(midhlayout, 6, 1, 1, 1, Qt::AlignCenter);
    gLayout->setRowStretch(7, 1);

    if(slistDisk->m_disklist.length()==1)
    {
        currentDiskID(slistDisk->m_disklist.at(0)->devPath);
    }
    translateStr();

}

void FullPartitionFrame::showListDisk()
{
    if(slistDisk) {
        m_midStackLayout->setCurrentWidget(slistDisk);
        this->setFocus();
    }
}

void FullPartitionFrame::showTableDisk()
{
//    if(customPartitionFrame) {
//        m_midStackLayout->setCurrentWidget(customPartitionFrame);
//    }

}

void FullPartitionFrame::repaintDevices(DeviceList devs)//启动时执行一次，用来读取硬盘信息
{
    slistDisk->m_disklist.clear();
    slistDisk->numDisk = 0;
    slistDisk->addItemsToList(devs);
    emit signalFinishedLoadDisk(slistDisk->m_disklist.count());
    devlist = devs;
    if(devs.length() == 1 && (devs.at(0)->m_diskTotal/1024/1024/1024 - 50 < 0)) {
        m_tip->show();
    }
    if( m_display ) //bug165285，防止读取硬盘信息较快造成的过早(在首界面)显示提示信息
    {
	 defaultOption("KPartition");//bug159771，防止读取硬盘信息较慢造成的第一次进入本界面时不显示提示信息
    }
}


void FullPartitionFrame::initAllConnect()
{
    connect(m_quickDelegate, &FullPartitionDelegate::deviceRefreshed, this, &FullPartitionFrame::repaintDevices);
    connect(slistDisk, &LevelScrollDiskView::signalWidgetSelected, this, &FullPartitionFrame::currentDiskID);
    connect(slistDisk,&LevelScrollDiskView::signalDataDiskSelected,this,&FullPartitionFrame::DataDiskCheck);
    
    connect(slistDisk,&LevelScrollDiskView::signalCheckSavehistory,this, &FullPartitionFrame::slotCheckSavehistory);
    
    connect(this, &FullPartitionFrame::leftCliked, slistDisk, &LevelScrollDiskView::checkDiskLeft);
    connect(this, &FullPartitionFrame::rightClicked, slistDisk, &LevelScrollDiskView::checkDiskRight);
    connect(m_encryptBox, &QCheckBox::clicked, this, &FullPartitionFrame::setEncryptyToDisk);
    connect(m_lvm, &QCheckBox::clicked, this, &FullPartitionFrame::setLvmToDisk);
    connect(m_savehistory,&QCheckBox::clicked,[=]() {
        if (m_savehistory->isChecked()==true) {
	    message=new MessageBox(this);
            message->setMessageInfo(tr("This action will  use the original old account as the default account. Please make sure you remember the password of the original account!"));
            message->setCancleHidden();
            message->m_pOkButton->setStyleSheet("QPushButton { color: white; }");//解决bug158792
            int ret = message->exec();
	    QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers|QEventLoop::ExcludeUserInputEvents);
            this->activateWindow();//实现退出模态窗口后焦点仍然在原位置并可tab走焦
            if (ret == QDialog::Accepted) {
                qDebug() << "Accepted:" << ret;
		if(m_encryptBox->isChecked()==true){
			m_encryptBox->setChecked(false);
			WriteSettingToIni("Encrypty", "encrypty", "false");
        		WriteSettingToIni("Encrypty", "TPMEncrypt", "false");
		}
		if(m_lvm->isChecked()==true){
			m_lvm->setChecked(false);
            		WriteSettingToIni("Encrypty", "lvm", "false");
		}
            	foreach (DiskInfoView* p,  slistDisk->m_disklist) {
			if(p->dataSel->isChecked()==true){
				p->dataSel->setChecked(false);
			}
			p->setPartiSizeBtVisible(false);
            	}
            } else if (ret == QDialog::Rejected) {
                m_savehistory->setChecked(false);
            }
        } else {
            foreach (DiskInfoView* p,  slistDisk->m_disklist) {
		p->setPartiSizeBtVisible(true);
            }
        }
    });

    connect(m_encryptBox,&QCheckBox::clicked,[=](){
      if(m_encryptBox->isChecked()==true)
        {
     		if(m_lvm->isChecked()==true){
                        m_lvm->setChecked(false);
            		WriteSettingToIni("Encrypty", "lvm", "false");
                }
		if(m_savehistory->isChecked()==true){
                        m_savehistory->setChecked(false);
			bool isDataChecked=false;
                        foreach (DiskInfoView* p,  slistDisk->m_disklist) {
                                if(p->dataSel->isChecked()==true){
                                        isDataChecked=true;
                                }
                        }
                        if(isDataChecked==false){
                                foreach (DiskInfoView* p,  slistDisk->m_disklist) {
                                        for(Device::Ptr device : devlist) {
                                                if(device->m_path == m_devPath) {
                                                        p->setPartiSizeBtVisible(true);
                                                }
                                        }
                                }

                        }
                }
        }
    });

    connect(m_lvm,&QCheckBox::clicked,[=](){
      	if(m_lvm->isChecked()==true)
        {
     		if(m_savehistory->isChecked()==true){
                        m_savehistory->setChecked(false);
			bool isDataChecked=false;
			foreach (DiskInfoView* p,  slistDisk->m_disklist) {
				if(p->dataSel->isChecked()==true){
					isDataChecked=true;
				}
			}
			if(isDataChecked==false){
				foreach (DiskInfoView* p,  slistDisk->m_disklist) {
                			for(Device::Ptr device : devlist) {
                        			if(device->m_path == m_devPath) {
                                			p->setPartiSizeBtVisible(true);
                        			}
                			}
            			}

			}
                }
		if(m_encryptBox->isChecked()==true){
                        m_encryptBox->setChecked(false);
			WriteSettingToIni("Encrypty", "encrypty", "false");
        		WriteSettingToIni("Encrypty", "TPMEncrypt", "false");
                }
        }
    });

    connect(slistDisk,&LevelScrollDiskView::signalDataDiskSelected,[=](){
      
	foreach (DiskInfoView* p,  slistDisk->m_disklist) {
               
       		if(p->dataSel->isChecked())
		{
			num=1;
			break;
		}
		else
		{
			num=0;
		}
        }

	if(num==1)
        {
		 if(m_savehistory->isChecked()==true){
			m_savehistory->setChecked(false);
		 }
		 foreach (DiskInfoView* p,  slistDisk->m_disklist) {
			p->setPartiSizeBtVisible(false);
		 }
        }
        else
	{
		 foreach (DiskInfoView* p,  slistDisk->m_disklist) {
			p->setPartiSizeBtVisible(true);
		 }
        }
        });

    QString tmp = getDeviceStrByType(getDeviceModel());
    tmp.remove(QRegExp("\\s"));//去除字符串中间空格
    if(tmp.contains("Kirin990") || tmp.contains("Kirin9006C")) {
        if(KServer::KReadFile("/proc/cmdline").contains("factory-backup")) {
            connect(m_firstback,&QCheckBox::clicked,[=]() {
                if (m_firstback->isChecked()==false) {
                    m_firstback->setChecked(true);
		    message=new MessageBox(this);
                    //message->m_info->setWordWrap(true);
                    message->setMessageInfo(tr("The current installation mode enforces factory backup configuration and cannot be unchecked"));//当前安装模式强制配置出厂备份，无法取消勾选
                    message->setCancleHidden();
                    message->exec();
		    QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers|QEventLoop::ExcludeUserInputEvents);
                    this->activateWindow();//实现退出模态窗口后焦点仍然在原位置并可tab走焦
                }
            });
        }
    }
}

void FullPartitionFrame::addStyleSheet()
{
    QFile file(":/res/qss/KPartition.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = "";
    stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}


void FullPartitionFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}


void FullPartitionFrame::translateStr()
{
    m_tip->setText(tr("Please choose custom way to install, disk size less than 50G!"));//请选择自定义方式进行安装！
    m_encryptBox->setText(tr("Full disk encryption"));
    m_lvm->setText(tr("lvm"));
    m_firstback->setText(tr("Factory backup"));
    m_savehistory->setText(tr("save history date"));
}

void FullPartitionFrame::currentDiskID(QString devpath)
{
    //m_savehistory->setDisabled(true);
    m_savehistory->setVisible(false);
    if(devpath.isEmpty()) {
        m_quickDelegate->removeSelectDevice(devpath);
        emit signalSeclectedListItem(false);
        qDebug() << "devPath is error";
        return;
    }
    for(Device::Ptr device : devlist) {
        if(device->m_path == devpath) {
            m_devPath = devpath;
            m_quickDelegate->addSelectDevice(device);
            isGhost = ReadSettingIni("setting","ghost-path");
            qDebug() << "isGhost =" << isGhost;
    	    if(device->m_history==true &&  m_devPath.isEmpty()==false &&  isGhost.isEmpty()==true)
            {
                   m_savehistory->setVisible(true);
                   //m_savehistory->setDisabled(false);
		   foreach (DiskInfoView* p,  slistDisk->m_disklist) {
                	if(p->dataSel->isChecked()){
				if(m_savehistory->isChecked()==true){
                        		m_savehistory->setChecked(false);
                 		}
                   		//m_savehistory->setDisabled(true);
			}
            	   }

            }
        }
    }
    emit signalSeclectedListItem(true);
}

void FullPartitionFrame::slotCheckSavehistory(QString devpath)
{
	if(m_savehistory->isChecked()){
            foreach (DiskInfoView* p,  slistDisk->m_disklist) {
                for(Device::Ptr device : devlist) {
                        if((device->m_path == devpath) && (device->m_history==true)) {
                		p->setPartiSizeBtVisible(false);
            		}
            	}
            }
        }

}
void FullPartitionFrame::keyPressEvent(QKeyEvent *event)
{
    qDebug() << Q_FUNC_INFO << event->key() << "key pressed! quick part";
    if (event->key() == Qt::Key_Left) {
    }
    else if (event->key() == Qt::Key_Right){
    }
    else if(event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)
    {
    }
    else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
		QWidget::keyPressEvent(event);
    }
    else if(event->key() == Qt::Key_Backspace)
    {
        emit backspacepressed();
    }
    else if(event->key() == Qt::Key_Escape || event->key() == Qt::Key_Control)
    {
        this->setFocus();
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void FullPartitionFrame::setEncryptyToDisk()
{
    if(m_encryptBox->isChecked()) {
        if(!m_EncryptSetframe)
        {
            m_EncryptSetframe = new EncryptSetFrame(this);
        }

        int ret = m_EncryptSetframe->exec();
	QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers|QEventLoop::ExcludeUserInputEvents);
        this->activateWindow();
        if(ret == QDialog::Accepted) {
            if(m_EncryptSetframe->m_pSoftEncrypt->isChecked())
            {
                WriteSettingToIni("Encrypty", "encrypty", "true");
                WriteSettingToIni("Encrypty", "encryptyPWD", m_EncryptSetframe->m_pPasswordBox->text());
                WriteSettingToIni("Encrypty", "TPMEncrypt", "false");
            }
            else if(m_EncryptSetframe->m_pTPMEncrypt->isChecked())
            {
                WriteSettingToIni("Encrypty", "encrypty", "false");
                WriteSettingToIni("Encrypty", "TPMEncrypt", "true");
            }
        } else if (ret == QDialog::Rejected){
            m_encryptBox->setChecked(false);
            WriteSettingToIni("Encrypty", "encrypty", "false");
            WriteSettingToIni("Encrypty", "TPMEncrypt", "false");
        }
    } else {
        WriteSettingToIni("Encrypty", "encrypty", "false");
        WriteSettingToIni("Encrypty", "TPMEncrypt", "false");
    }
}

void FullPartitionFrame::setLvmToDisk()
{
    if(m_lvm->isChecked()) {
            WriteSettingToIni("Encrypty", "lvm", "true");
    } else {
            WriteSettingToIni("Encrypty", "lvm", "false");
    }

}
void FullPartitionFrame::DataDiskCheck(bool value)
{
        //m_encryptBox->setDisabled(value);
}

void FullPartitionFrame::defaultOption(QString objectName)
{
    qDebug() << "收到此信号--->>>>>>>>" << objectName;
    if (objectName != "KPartition") {
        return ;
    }
    m_display = true;

    isGhost = ReadSettingIni("setting","ghost-path");
    qDebug() << "isGhost =" << isGhost;
    if(isGhost.isEmpty()==false)
    {
   	if(m_savehistory->isChecked()==true){
        	m_savehistory->setChecked(false);
        }
	if(m_savehistory->isVisible()){
		m_savehistory->setVisible(false);
	}
    }

    if (slistDisk->m_SSDBoolValue && slistDisk->m_HDDBoolValue) {
        // 两个硬盘中任何一个小于50GB就不发出提示
        if( (devlist.at(slistDisk->m_SSDIndex)->m_diskTotal/1024/1024/1024 < 50) ||
            (devlist.at(slistDisk->m_HDDIndex)->m_diskTotal/1024/1024/1024 < 50))
            return;

        // 设置CheckBox不可见
        slistDisk->m_disklist.at(slistDisk->m_SSDIndex)->dataSel->setVisible(false);
        slistDisk->m_disklist.at(slistDisk->m_SSDIndex)->dataSel->setCheckable(true);//bug163962
        slistDisk->m_disklist.at(slistDisk->m_SSDIndex)->dataSel->setChecked(false);

        // 设置ssd勾选项磁盘默认勾选
        slistDisk->m_disklist.at(slistDisk->m_SSDIndex)->checkpic->setVisible(true);

        // 设置HDD作为数据盘勾选项自动勾选
        slistDisk->m_disklist.at(slistDisk->m_HDDIndex)->dataSel->setVisible(true);
        slistDisk->m_disklist.at(slistDisk->m_HDDIndex)->dataSel->setCheckable(true);
        slistDisk->m_disklist.at(slistDisk->m_HDDIndex)->dataSel->setChecked(true);
        slistDisk->m_disklist.at(slistDisk->m_HDDIndex)->checkpic->setVisible(false);

        if (slistDisk->m_disklist.at(slistDisk->m_SSDIndex)->devPath.length() != 0) {
            emit slistDisk->signalWidgetSelected(slistDisk->m_disklist.at(slistDisk->m_SSDIndex)->devPath);
        }
        MessageBox messageLable;
        messageLable.setMessageInfo(tr("By default, SSDS serve as system disks and HDDS serve as user data disks"));
        messageLable.setCancleHidden();
        messageLable.exec();
	QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers|QEventLoop::ExcludeUserInputEvents);
        this->activateWindow();
    }
    return ;
}

void FullPartitionFrame::showEvent(QShowEvent *event)
{
    if(slistDisk->m_disklist.count()==1)
    {
        if(!slistDisk->m_disklist.at(0)->checkpic->isVisible())
        {
             emit slistDisk->m_disklist.at(0)->signalClicked(slistDisk->m_disklist.at(0)->devPath);
        }

    }
    emit reSetFocusPolicy();
}

}
