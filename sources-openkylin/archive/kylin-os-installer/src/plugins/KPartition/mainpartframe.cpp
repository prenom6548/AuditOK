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

#include "mainpartframe.h"
#include <QMessageBox>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QKeyEvent>
#include <QDir>

#include "../PluginService/klinguist.h"
#include "../PluginService/ksystemenv.h"
#include "../PluginService/ksystemsetting_unit.h"
#include "../PluginService/kcommand.h"
#include "partman/partition_unit.h"
#include "../PluginService/kthread.h"
#include "delegate/customize_device_model.h"
#include "delegate/custom_partition_delegate.h"
#include "delegate/full_partition_delegate.h"
#include "../PluginService/ui_unit/arrowtooltip.h"
#include "../PluginService/base_unit/file_unit.h"

using namespace KServer;
namespace KInstaller
{
using namespace Partman;
MainPartFrame::MainPartFrame(QWidget *parent) : MiddleFrameManager(parent),
    m_prepareFrame(new PrepareInstallFrame),
    m_partDelegate(new CustomPartitiondelegate(this)),
    m_quickDelegate(new FullPartitionDelegate(this)),
    m_partModel(new PartitionModel(this)),
    enumCheckBtn(InstallBtn::QuikPart),
    message(nullptr),
    m_partModelThread(new QThread)
{
    this->initNextBnt();
    this->setObjectName("KPartition");

    //创建线程读取磁盘信息
    m_partModel->moveToThread(m_partModelThread);
    m_partModelThread->start();


    initUI();
    initAllConnect();
    addStyleSheet();
    translateStr();
    scanDevices();

    this->setFocus();
}

MainPartFrame::~MainPartFrame()
{
    quitThreadRun(m_partModelThread);
}

void MainPartFrame::initUI()
{    
    wid=new QWidget;
    QGridLayout* gLayout = new QGridLayout;
    gLayout->setContentsMargins(0, 0, 0, 0);
    wid->setLayout(gLayout);
    gLayout->setColumnStretch(0, 1);
    gLayout->setColumnStretch(1, 6);
    gLayout->setColumnStretch(2, 1);

    m_mainTitle = new QLabel(this);
    m_mainTitle->setObjectName("mainTitle");
    gLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred), 0, 1, 1, 1);
    gLayout->addWidget(m_mainTitle, 1, 1, 1, 1, Qt::AlignCenter | Qt::AlignTop);
    gLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Preferred), 2, 1, 1, 1);

    //添加快速安装,并存安装和自定义安装
    m_fullInstallBtn = new QPushButton();
    m_fullInstallBtn->setDefault(1);
    m_fullInstallBtn->setCheckable(true);
    m_fullInstallBtn->setChecked(true);
    m_fullInstallBtn->installEventFilter(this);
    m_customInstallBtn = new QPushButton();
    m_customInstallBtn->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Fixed);//bug166983
    m_customInstallBtn->installEventFilter(this);
    //添加按钮之间的分割线
    QHBoxLayout *hBtnLayout = new QHBoxLayout(this);
//    QFrame* installLine1 = new QFrame(this);
//    installLine1->setFrameShape(QFrame::VLine);
//    installLine1->setFrameShadow(QFrame::Sunken);
//    installLine1->setMaximumWidth(1);
//    installLine1->setMaximumHeight(15);
//    installLine1->setStyleSheet("background-color:#FFFFFF;");
    hBtnLayout->setSpacing(5);
    hBtnLayout->addWidget(m_fullInstallBtn);
    //hBtnLayout->addWidget(installLine1);
    hBtnLayout->addWidget(m_customInstallBtn);


    gLayout->addLayout(hBtnLayout, 3, 1, 1, 1, Qt::AlignCenter | Qt::AlignTop);
    gLayout->addItem(new QSpacerItem(10, 30, QSizePolicy::Expanding, QSizePolicy::Fixed), 4, 1, 1, 1);


    m_fullInstallBtn->setObjectName("quikInstallBtn");
    m_customInstallBtn->setObjectName("customInstallBtn");
    m_mainStackWidget = new QStackedWidget();

    m_fullPartFrame = new FullPartitionFrame(m_quickDelegate, this);
    m_customPartFrame = new CustomPartitionFrame(m_partDelegate, this);

    m_mainStackWidget->addWidget(m_fullPartFrame);
    m_mainStackWidget->addWidget(m_customPartFrame);

    m_mainStackWidget->setCurrentWidget(m_fullPartFrame);


    //准备安装提示窗口
    m_loadpic = new QLabel;
    m_loadpic->setObjectName("loadpic");
    m_loadpic->setPixmap(QPixmap(":/res/pic/loading/ukui-loading-0.svg").scaled(QSize(48, 48), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_loadpic->setScaledContents(true);
    gLayout->addWidget(m_loadpic, 5, 1, 2, 1, Qt::AlignCenter);
    timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, &QTimer::timeout, this, &MainPartFrame::finishLoadDisk);
    timer->start();

    this->setFocus();
    gLayout->addWidget(m_mainStackWidget, 5, 1, 1, 1);
    gLayout->setRowStretch(6, 1);

    stack=new QStackedWidget;
    QGridLayout* layout = new QGridLayout;
    m_Widget->setLayout(layout);
    layout->addWidget(stack);
    stack->addWidget(wid);
    stack->addWidget(m_prepareFrame);
    stack->setCurrentWidget(wid);

}

void MainPartFrame::showCustomPartFrame()
{
    //m_quikPartFrame->releaseKeyboard();
    if(m_customPartFrame) {
        m_mainStackWidget->setCurrentWidget(m_customPartFrame);
        enumCheckBtn = InstallBtn::CustomPart;
        m_customInstallBtn->setCheckable(true);
        m_customInstallBtn->setChecked(true);
        m_fullInstallBtn->setChecked(false);
        getChoiceDisk(true);
        //        m_coexistInstallBtn->setChecked(false);
    } else {
        qDebug() << "m_manualPartFrame:" << "not defined" ;
    }
    initCustomPartFrameFocusPolicy();
}

void MainPartFrame::scanDevices()
{
    m_partModel->scanDevices();
}

void MainPartFrame::showQuikPartFrame()
{
    if(m_fullPartFrame) {
        if(m_quickDelegate->selectedDevices().length() > 0) {
            getChoiceDisk(true);
        }
        else
        {
            getChoiceDisk(false);
        }
        m_mainStackWidget->setCurrentWidget(m_fullPartFrame);
        enumCheckBtn = InstallBtn::QuikPart;
        m_customInstallBtn->setChecked(false);
        m_fullInstallBtn->setCheckable(true);
        m_fullInstallBtn->setChecked(true);
        this->setFocus();
    }

    else {
        qDebug() << "m_quikPartFrame:not defined" ;
    }
}
void MainPartFrame::showCoexistFrame()
{
    if(m_coexitPartFrame) {
        m_mainStackWidget->setCurrentWidget(m_coexitPartFrame);
        enumCheckBtn = InstallBtn::CoexistPart;
        m_customInstallBtn->setChecked(false);
        m_fullInstallBtn->setChecked(false);
    } else {
        qDebug() << "m_coexitPartFrame:not defined" ;
    }
}



void MainPartFrame::finishLoadDisk()
{
    var++;
    if(var > 7) {
        var = 0;
    }
    QString s = QString(":/res/pic/loading/ukui-loading-%1.svg").arg(var);
    m_loadpic->setPixmap(QPixmap(s).scaled(QSize(48, 48), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_loadpic->setScaledContents(true);
    m_loadpic->update();
}


void MainPartFrame::initAllConnect()
{
    connect(m_fullPartFrame, &FullPartitionFrame::signalSeclectedListItem, this, &MainPartFrame::getChoiceDisk);
    connect(m_customPartFrame, &CustomPartitionFrame::signalNextBtn, this, &MainPartFrame::getChoiceDisk);
    connect(m_fullPartFrame, &FullPartitionFrame::enterpressed, this, &MainPartFrame::enterpressed);
    connect(m_customPartFrame, &CustomPartitionFrame::enterpressed, this, &MainPartFrame::enterpressed);
    connect(m_fullPartFrame, &FullPartitionFrame::backspacepressed, this, &MainPartFrame::backspacepressed);
    connect(m_customPartFrame, &CustomPartitionFrame::backspacepressed, this, &MainPartFrame::backspacepressed);
    connect(m_prepareFrame, &PrepareInstallFrame::backspacepressedpre, this, &MainPartFrame::clickPrevButton);
    connect(this, &MainPartFrame::leftKeyPressed, m_fullPartFrame, &FullPartitionFrame::leftCliked);
    connect(this, &MainPartFrame::rightKeyPressed, m_fullPartFrame, &FullPartitionFrame::rightClicked);
    connect(m_fullPartFrame, &FullPartitionFrame::signalFinishedLoadDisk,this,&MainPartFrame::DiskNumCheck);
    connect(m_fullPartFrame, &FullPartitionFrame::reSetFocusPolicy,this,&MainPartFrame::initfullPartFrameFocusPolicy);


    connect(m_partModel, &PartitionModel::deviceRefreshed, m_quickDelegate, &FullPartitionDelegate::onDeviceRefreshed);
    connect(m_partModel, &PartitionModel::deviceRefreshed, m_partDelegate, &CustomPartitiondelegate::onDeviceRefreshed);
    //    connect(m_prepareFrame, &PrepareInstallFrame::signalNextStartInstalling, [=]{emit signalStartInstall();});
    connect(m_fullInstallBtn, &QPushButton::clicked, this, &MainPartFrame::showQuikPartFrame);
    connect(m_customInstallBtn, &QPushButton::clicked, this, &MainPartFrame::showCustomPartFrame);

    connect(m_customPartFrame, &CustomPartitionFrame::signalCreatePartTable, m_partDelegate, &CustomPartitiondelegate::createDeviceTable);
    connect(m_customPartFrame, &CustomPartitionFrame::reSetFocusPolicy, this, &MainPartFrame::initCustomPartFrameFocusPolicy);
    connect(m_customPartFrame, &CustomPartitionFrame::modifyPartFinished, [=]{m_nextBtn->setFocus();});
    connect(m_prepareFrame, &PrepareInstallFrame::reSetFocusPolicy, this, &MainPartFrame::initPreframeFocusPolicy);
    connect(m_prepareFrame->m_tipCheck,&QCheckBox::clicked,[=](){
        if(m_prepareFrame->m_tipCheck->isChecked())
            m_nextBtn->setDisabled(false);
        else
            m_nextBtn->setDisabled(true);
    });
    //    qDebug() << "signalquickpart:" << connect(m_prepareFrame, &PrepareInstallFrame::signalQuickPart, m_partModel, &PartitionModel::quickPart);
    connect(m_prepareFrame, &PrepareInstallFrame::signalQuickPart, [=]{
        if(m_fullPartFrame->m_savehistory->isChecked()==true && enumCheckBtn == InstallBtn::QuikPart)
        {
            KServer::WriteSettingToIni("config","data-unformat","true");
            emit signalStackPageChanged(2);
        }
        else{
            KServer::WriteSettingToIni("config","data-unformat","false");
            signalStackPageChanged(1);

        }
    });
    connect(m_prepareFrame, &PrepareInstallFrame::signalCustomPart, [=] {emit signalStackPageChanged(1);});
    connect(m_prepareFrame, &PrepareInstallFrame::enterKeyPressed,  this, &MainPartFrame::enterpressed);
    qDebug() << "PartitionModel::customPartDone" << connect(m_partModel, &PartitionModel::customPartDone, [=](int execcode) {
        if(execcode == 0) {
            emit signalInstalScriptlFinished();
        } else
            emit signalInstallError(QString::number(execcode));
    });
    qDebug() << "PartitionModel::autoPartDone" << connect(m_partModel, &PartitionModel::fullPartDone, [=](int execcode) {
        if(execcode == 0) {
            emit signalInstalScriptlFinished();
        } else
            emit signalInstallError(QString::number(execcode));});

    connect(this, &MainPartFrame::testSignal, m_fullPartFrame, &FullPartitionFrame::defaultOption);
}

void MainPartFrame::clickNextButton()
{
    if(m_nextBtn->isEnabled()==false)
    {
        return;
    }
    if(stack->currentWidget()==m_prepareFrame)
    {
        if(m_fullPartFrame->m_savehistory->isChecked()==true && enumCheckBtn == InstallBtn::QuikPart)
        {
           bool isHistoryDisk=false;
           for(Device::Ptr device : m_fullPartFrame->devlist) {
                if((device->m_path == m_fullPartFrame->m_devPath) && (device->m_history==true)) {
                        isHistoryDisk=true;
                }
           }
           if(isHistoryDisk==true) {
                KServer::WriteSettingToIni("config","data-unformat","true");
                if(ReadSettingIni("config","first_mode").contains("oem1")==1)
                {
                        emit signalStackPageChanged(1);
                }
                else{
                        emit signalStackPageChanged(2);
                }
           }
           else{
                KServer::WriteSettingToIni("config","data-unformat","false");
                signalStackPageChanged(1);
           }
        }
        else{
            KServer::WriteSettingToIni("config","data-unformat","false");
            signalStackPageChanged(1);

        }

	if(KServer::ReadSettingIni("setting","ghost")=="true")
	{
		
		emit signalStackPageChanged(2);
	}

   //     emit signalStackPageChanged(1);
        return;
    }


    if(message != nullptr) {
        message->close();
    }
    message = new MessageBox(this);
    m_prepareFrame->setInstallBtn(enumCheckBtn);
    m_prepareFrame->m_tipCheck->setChecked(false);

    switch (enumCheckBtn) {
    case InstallBtn::QuikPart:{//全盘安装
        if(!m_quickDelegate->DiskSizeValidate(m_fullPartFrame->m_devPath)) {
            return;
        }

        m_prepareFrame->setMainTitleStr(tr("Confirm Full Installation"));
        if(m_fullPartFrame->m_savehistory->isChecked())
        {
            m_prepareFrame->m_tipCheck->setText(tr("Formatted the system disk"));
        }
        else
        {
            m_prepareFrame->m_tipCheck->setText(tr("Formatted the whole disk"));
        }

        QString lvm = ReadSettingIni("Encrypty", "lvm");

        QString tmp = getDeviceStrByType(getDeviceModel());
        tmp.remove(QRegExp("\\s"));//去除字符串中间空格
        if(tmp.contains("Kirin990") || tmp.contains("Kirin9006C")) {//如果是华为990或9006c机器
            if(!m_quickDelegate->isMBRPreferredDe()) {
                if(lvm.compare("true") == 0)
                {
                    partStr = QString(tr("EFI partition #1 of %1 as vfat;\n"
                                         "boot partition #2 of %1 as ext4;\n"
                                         "partition be left of %1 as lvm;\n"
                                         "/ partition of lvm as ext4;\n"
                                         "backup partition of lvm as ext4;\n"
                                         "data partition of lvm as ext4;\n"
                                         "Create swap_file file in / partition;\n")).arg(m_fullPartFrame->m_devPath);
                }
                else
                {
                    partStr = QString(tr("EFI partition #1 of %1 as vfat;\n"
                                         "boot partition #2 of  %1 as ext4;\n"
                                         "/ partition #3 of  %1 as ext4;\n"
                                         "backup partition #4 of  %1 as ext4;\n"
                                         "data partition #5 of  %1 as ext4;\n"
                                         "Create swap_file file in / partition;\n")).arg(m_fullPartFrame->m_devPath);
                }
            } else {
                if(lvm.compare("true") == 0)
                {
                    partStr = QString(tr("boot partition #1 of %1 as vfat;\n"
                                         "extend partition #2 of %1 ;\n"
                                         "partition be left of %1 as lvm;\n"
                                         "/ partition of lvm as ext4;\n"
                                         "backup partition of lvm as ext4;\n"
                                         "data partition of lvm as ext4;\n"
                                         "Create swap_file file in / partition;\n")).arg(m_fullPartFrame->m_devPath);

                }
                else
                {
                    partStr = QString(tr("boot partition #1 of %1 as vfat;\n"
                                         "extend partition #2 of  %1 ;\n"
                                         "/ partition #5 of  %1 as ext4;\n"
                                         "backup partition #6 of  %1 as ext4;\n"
                                         "data partition #7 of  %1 as ext4;\n"
                                         "Create swap_file file in / partition;\n")).arg(m_fullPartFrame->m_devPath);
                }
            }
        }
        else
        {
            if(!m_quickDelegate->isMBRPreferredDe()) {
                if(lvm.compare("true") == 0)
                {
                    partStr = QString(tr("EFI partition #1 of %1 as vfat;\n"
                                         "boot partition #2 of %1 as ext4;\n"
                                         "partition be left of %1 as lvm;\n"
                                         "/ partition of lvm as ext4;\n"
                                         "backup partition of lvm as ext4;\n"
                                         "data partition of lvm as ext4;\n"
                                         "swap partition of lvm as swap;\n")).arg(m_fullPartFrame->m_devPath);
                }
                else
                {
                    partStr = QString(tr("EFI partition #1 of %1 as vfat;\n"
                                         "boot partition #2 of  %1 as ext4;\n"
                                         "/ partition #3 of  %1 as ext4;\n"
                                         "backup partition #4 of  %1 as ext4;\n"
                                         "data partition #5 of  %1 as ext4;\n"
                                         "swap partition #6 of  %1 as swap;\n")).arg(m_fullPartFrame->m_devPath);
                }
            } else {
                if(lvm.compare("true") == 0)
                {
                    partStr = QString(tr("boot partition #1 of %1 as vfat;\n"
                                         "extend partition #2 of %1 ;\n"
                                         "partition be left of %1 as lvm;\n"
                                         "/ partition of lvm as ext4;\n"
                                         "backup partition of lvm as ext4;\n"
                                         "data partition of lvm as ext4;\n"
                                         "swap partition of lvm as swap;\n")).arg(m_fullPartFrame->m_devPath);

                }
                else
                {
                    partStr = QString(tr("boot partition #1 of %1 as vfat;\n"
                                         "extend partition #2 of  %1 ;\n"
                                         "/ partition #5 of  %1 as ext4;\n"
                                         "backup partition #6 of  %1 as ext4;\n"
                                         "data partition #7 of  %1 as ext4;\n"
                                         "swap partition #8 of  %1 as swap;\n")).arg(m_fullPartFrame->m_devPath);
                }
            }
        }

        //添加数据盘
        WriteSettingToIni("config", "data-device","");
	bool issetDataParti = false;
        bool issetDataDisk = false;
        qint64  DataPartiSize = 0;
        foreach (DiskInfoView* mb,m_fullPartFrame->slistDisk->m_disklist) {
            if(mb->dataSel->isChecked())
            {
                QString strT = "\"";
                strT += mb->devPath;
                strT += "\" ";
                strT += tr("will it be formatted, continue?");//将会被格式化，是否继续？
                strT.append("\n");
                strT += tr("The disk which was set as a data disk will be formatted");//设置为数据盘的磁盘数据会被格式化处理。

                message->setOKInfo(tr("Continue"));//继续
                message->setBelowLabelColor("color: #8d8d8d;");
                message->setMessageInfo(strT);
                if(message->exec() == QDialog::Rejected  && message->execid == 1){//如果点击了右上角的关闭按钮或取消按钮或键盘esc键，就不进入下个界面
                    //this->setFocus();
                    QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers|QEventLoop::ExcludeUserInputEvents);
                    m_mainStackWidget->currentWidget()->activateWindow();
                    return;
                }
                message->clearOKInfo();
                message->clearBelowLabelColor();

                WriteSettingToIni("config", "data-device", mb->devPath);
//                partStr.append("\n"+mb->devPath+" set to data device;\n");
                partStr.append("\n"+mb->devPath);
                partStr.append(tr(" set to data device;\n"));//bug118468设置独立磁盘为数据盘时，确认全盘安装界面未翻译

		issetDataDisk = true;
            }
	    if(mb->m_setSize)
               issetDataParti = true;

            if(mb->devPath == m_fullPartFrame->m_devPath)
                DataPartiSize = mb->m_sizeDataSetted;
        }

	if(issetDataDisk || !issetDataParti || (m_fullPartFrame->m_savehistory->isChecked() && m_fullPartFrame->m_savehistory->isVisible()))//如果勾选框“保留用户及其数据”或“设为数据盘”被勾选、或所有硬盘都没有单独设置数据分区大小
        {
            WriteSettingToIni("config", "data-customize-size", "false");
            qDebug()<< Q_FUNC_INFO <<"data-customize-size:false";
        }
        else
        {
            if(DataPartiSize > 0)
            {
                WriteSettingToIni("config", "data-customize-size", QString::number(DataPartiSize/1024/1024));
                qDebug()<< Q_FUNC_INFO <<" data size setted(MB):"<<DataPartiSize/1024/1024;
            }
            else
            {
                WriteSettingToIni("config", "data-customize-size", "false");
                qDebug()<< Q_FUNC_INFO <<"data-customize-size2:false";
            }
        }
        
	m_prepareFrame->setPartInfo(partStr);
        m_nextBtn->setDisabled(true);


	if(m_fullPartFrame->m_savehistory->isChecked()==true && m_fullPartFrame->m_savehistory->isVisible())
        {
        	QDir dir("/cdrom/apps-third");
        	if( !dir.exists() )
        	{
	       		m_nextBtn->setText(tr("Start Installation"));
        	}
        }

        writeSettingIni(InstallBtn::QuikPart);
    }
        break;
    case InstallBtn::CoexistPart:
        m_prepareFrame->setMainTitleStr(tr("Coexist Install"));//并存安装
        break;
    case InstallBtn::CustomPart://自定义安装
//        if (m_prepareFrame->m_tipCheck->isTristate()) {
//            m_nextBtn->setEnabled(true);
//        } else {
//            m_nextBtn->setEnabled(false);
//        }

        if(m_quickDelegate->isMBRPreferredDe()) //legacy启动,不可含有efi分区。对应bug126195
        {
            bool bEFI=false;
            //遍历所有挂在磁盘，是否已存在boot挂载点的分区
            Device::Ptr p;
            Partition::Ptr q;
            for(int k=0;k<m_customPartFrame->devlist.count();k++)
            {
                p=(Device::Ptr)(m_customPartFrame->devlist.at(k));
                for(int i=0;i<p->partitions.count();i++)
                {
                    q=(Partition::Ptr)(p->partitions.at(i));
                    if(FSType::EFI == q->fs)//efi分区
                    {
                        bEFI = true;
                        break;
                    }
                }
            }
            if(bEFI)
            {
                message->setMessageInfo(tr("bootloader type of Legacy should not contain EFI partition!"));
                message->setCancleHidden();
                message->show();
                //this->setFocus();
                QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers|QEventLoop::ExcludeUserInputEvents);
                m_mainStackWidget->currentWidget()->activateWindow();
                return;
            }
        }

        if(!validatePartition()) {//错误，不可进入下个界面
            message->setMessageInfo(validateMessage.left(validateMessage.size() - 1));//减1导致ts文件翻译内容后需要多加一个字符如空格，才能保证显示内容完整
            message->setCancleHidden();
            if (message->exec() == QDialog::Accepted ){
                //this->setFocus();
                QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers|QEventLoop::ExcludeUserInputEvents);
                m_mainStackWidget->currentWidget()->activateWindow();
                return;
            } else if(message->execid == 1){
                //this->setFocus();
                QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers|QEventLoop::ExcludeUserInputEvents);
                m_mainStackWidget->currentWidget()->activateWindow();
                return;
            }
        } else if(validatePartition() == -1) {//警告，点击“确定”后进入下个界面
            message->setMessageInfo(validateMessage.left(validateMessage.size() - 1));//减1导致ts文件翻译内容后需要多加一个字符如空格，才能保证显示内容完整
            if(message->exec() == QDialog::Rejected  && message->execid == 1){//如果点击了右上角的关闭按钮或取消按钮或键盘esc键，就不进入下个界面
                //this->setFocus();
                QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers|QEventLoop::ExcludeUserInputEvents);
                m_mainStackWidget->currentWidget()->activateWindow();
                return;
            }
        }

        //提示为解决磁盘当前的分区表类型和引导不一致导致不能 正常安装问题
        if(!checkLoaderAndPartitionTable(m_partDelegate->findDevice(m_customPartFrame->m_bootCombox->currentText().split(" ").at(0)))) {
            message->setMessageInfo(checkLoaderAndPartitionTableStr());
            if(message->exec() == QDialog::Rejected && message->execid == 1) {
                //this->setFocus();
                QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers|QEventLoop::ExcludeUserInputEvents);
                m_mainStackWidget->currentWidget()->activateWindow();
                return;
            }else {
                m_nextBtn->setDisabled(true);
            }
        } else {
                m_nextBtn->setDisabled(true);
        }

        m_prepareFrame->setMainTitleStr(tr("Confirm Custom Installation"));//确认自定义安装

        /*
         * 如果您继续，以下所列出的修改内容将会写入到磁盘中。或者，您也可以手动来进行其它修改。
         * 警告：任何已删除和将要格式化的分区上的数据都将被摧毁。
         */
        partStr = m_partDelegate->comboxOperatorPartitions();
        m_prepareFrame->setPartInfo(partStr);
        m_prepareFrame->m_tipCheck->setText(tr("Confirm the above operations"));
        m_partDelegate->writeMountOperate();
        writeSettingIni(InstallBtn::CustomPart);

        break;

    }

    if( (ReadSettingIni("config","first_mode").contains("oem1")) ||
        (KServer::ReadSettingIni("setting","ghost")=="true"))
       // (ReadSettingIni("config","first_mode").contains("normal") && (m_fullPartFrame->m_savehistory->isChecked()==true && enumCheckBtn == InstallBtn::QuikPart))
    {
        m_nextBtn->setText(tr("Start Installation"));
    }

    stack->setCurrentWidget(m_prepareFrame);
    //emit signalStackFrameManager((QWidget*)m_prepareFrame);
}

void MainPartFrame::slotStartInstall()
{
    if(enumCheckBtn == InstallBtn::QuikPart)
    {
       m_partModel->fullPart();
    }
    else
    {
        //m_partModel->customPart(m_partDelegate->operations());
	m_partModel->customPart(m_partDelegate->operations(),m_partDelegate->virtualDevices());
    }
}

void MainPartFrame::clickPrevButton(QWidget *p)
{
    if(p==this)
    {
        if(stack->currentWidget()==m_prepareFrame)
        {
            stack->setCurrentWidget(wid);
	    m_nextBtn->setText(tr("Next"));
            m_nextBtn->setDisabled(false);
        }
        else if(stack->currentWidget()==wid)
        {
            emit backspacepressed();
        }
    }
    else if(p==m_prepareFrame)//从确认安装子界面返回选择安装方式子界面
    {
         if(stack->currentWidget()==m_prepareFrame)
         {
             stack->setCurrentWidget(wid);
             m_nextBtn->setText(tr("Next"));
             m_nextBtn->setDisabled(false);
         }
    }
}

//bug 51780 add message
QString MainPartFrame::checkLoaderAndPartitionTableStr()
{
    bool table = checkLoaderAndPartitionTable(m_partDelegate->findDevice(m_customPartFrame->m_bootCombox->currentText().split(" ").at(0)));
    bool start = checkSpecialModel();

    QString info;
    if(start != table) {
        if(start == 1) {
            info = QString(tr("BootLoader method %1 inconsistent with the disk partition table \n"
                              "type %2.").arg("UEFI").arg("MSDos"));
        } else {
            info = QString(tr("BootLoader method %1 inconsistent with the disk partition table \n"
                              "type %2, cannot have efi partition.").arg("legacy").arg("GPT"));
        }
    } else {
        if(start == 0) {
            info = QString(tr("BootLoader method %1 inconsistent with the disk partition table \n"
                              "type %2, cannot have efi partition.").arg("legacy").arg("MSDos"));
        }
    }
    return info;

}

void MainPartFrame::initfullPartFrameFocusPolicy()
{
    if (m_fullPartFrame->slistDisk->m_disklist.isEmpty()) {
        return;
    }
    this->setTabOrder(m_customInstallBtn, m_fullPartFrame->slistDisk->m_disklist[0]);
    for (int i = 0; i < m_fullPartFrame->slistDisk->m_disklist.count() - 1; i++)
    {
        this->setTabOrder(m_fullPartFrame->slistDisk->m_disklist[i], m_fullPartFrame->slistDisk->m_disklist[i+1]);
    }
    this->setTabOrder(m_fullPartFrame->m_savehistory, m_nextBtn);
}

void MainPartFrame::initCustomPartFrameFocusPolicy()
{
    if( m_customPartFrame->m_tableViews.count()<=0)
        return;
    this->setTabOrder(m_customInstallBtn, m_customPartFrame->m_tableViews[0]->createPartTableBtn);
    for (int i = 0; i < m_customPartFrame->m_tableViews.count() - 1; i++)
    {
        this->setTabOrder(m_customPartFrame->m_tableViews[i]->createPartTableBtn, m_customPartFrame->m_tableViews[i+1]->createPartTableBtn);
    }
    this->setTabOrder(m_customPartFrame->m_tableViews[m_customPartFrame->m_tableViews.count()-1]->createPartTableBtn,m_customPartFrame->m_bootCombox);
    this->setTabOrder(m_customPartFrame->m_bootCombox,m_customPartFrame->m_revertBtn);
    this->setTabOrder(m_customPartFrame->m_revertBtn,m_nextBtn);
}

void MainPartFrame::initPreframeFocusPolicy()
{
    this->setTabOrder( m_prepareFrame->m_tipCheck,m_nextBtn);
}

//返回值为1：检测通过; 0：检测不通过; -1：分区不存在但可继续操作
int MainPartFrame::validatePartition()
{
    validateMessage.clear();
    m_validateStates = m_partDelegate->validate();
    if(m_validateStates.isEmpty()) {
        return 1;
    } else {
        //找出全部需合并的项
        ValidateStates errorStatesofNOParti;
        ValidateStates warningStatesofNOParti;
        for(ValidateState state : m_validateStates) {
            qDebug() <<"MainPartFrame::validatePartition:" << state->state();
            switch (state->state()) {
            //合并报错--缺少分区
            case ValidateState::RootMissing:
                  errorStatesofNOParti.append(ValidateState::RootMissing);
                  break;
            case ValidateState::BootMissing:
                  errorStatesofNOParti.append(ValidateState::BootMissing);
                  break;
            case ValidateState::EfiMissing:
                  errorStatesofNOParti.append(ValidateState::EfiMissing);
                  break;
            case ValidateState::NoDataPartition:
                  errorStatesofNOParti.append(ValidateState::NoDataPartition);
                  break;

            //合并警告--缺少分区
            case ValidateState::NoSwapPartition:
                  warningStatesofNOParti.append(ValidateState::NoSwapPartition);
                  break;
            case ValidateState::DataORBackupPartition:
                  warningStatesofNOParti.append(ValidateState::DataORBackupPartition);
                  break;
//            case ValidateState::NoDataPartition:
//                  warningStatesofNOParti.append(ValidateState::NoDataPartition);
//                  break;
            default:
                  break;
            }
         }

        //合并报错--缺少分区
        if(!errorStatesofNOParti.isEmpty()) {
            validateMessage.clear();
            validateMessage.append(tr("No created of "));
            int i = 0;
            for(ValidateState state : errorStatesofNOParti) {
                i++;
                if(1 != i)
                    validateMessage.append(("、"));
                switch (state->state()) {
                    case ValidateState::RootMissing:
                          validateMessage.append(("\"/\""));
                          break;
                    case ValidateState::BootMissing:
                          validateMessage.append(("\"boot\""));
                          break;
                    case ValidateState::EfiMissing:
                          validateMessage.append(("\"efi\""));
                          break;
                    case ValidateState::NoDataPartition:
                          validateMessage.append(("\"data\""));
                          //validateMessage.append(tr("The type is user data"));
                          break;
                    default:
                          break;
                }
             }
             validateMessage.append(tr(" partition\n"));
             return 0;
        }

        //单条报错
        for(ValidateState state : m_validateStates) {
            switch (state->state()) {
            case ValidateState::InvalidId:
                validateMessage.append(tr("InvalidId\n"));
                return 0;
            case ValidateState::PartitionTooSmall:
                validateMessage.append(tr("Partition too small\n"));
                return 0;
            case ValidateState::BootFsInvalid:
                validateMessage.append(tr("Boot filesystem invalid\n"));
                return 0;
            case ValidateState::BootPartNumberInvalid:
                validateMessage.append(tr("Boot partition not in the first partition\n"));
                return 0;
            case ValidateState::BootTooSmall:
                validateMessage.append(tr("Boot partition too small\n"));
                return 0;
            case ValidateState::EfiSizeError:
                validateMessage.append(tr("the EFI partition size should be set between 256MB and 2GB\n"));
                return 0;
            case ValidateState::EfiTooMany:
                validateMessage.append(tr("Only one EFI partition is allowed\n"));
                return 0;
            case ValidateState::EfiPartNumberInvalid:
                validateMessage.append(tr("Efi partition number invalid\n"));
                return 0;
            case ValidateState::RootTooSmall:
                validateMessage.append(tr("Root partition size is greater than 40GB\n"));//根分区大小大于40GB
                return 0;
            case ValidateState::BackUpPartitionTooSmall:
                validateMessage.append(tr("BackUp partition too small\n"));
                return 0;
            case ValidateState::RepeatedMountPoint:
                validateMessage.append(tr("Repeated mountpoint\n"));
                return 0;
                //           case ValidateState::HUAWEIKirin990:
                //               validateMessage.append(tr("HUAWEI Kirin 990 format EFI partition to fat16"));
                //               return 0;
                //           case ValidateState::MipsBootExt3:
                //               validateMessage.append(tr("Mips format Boot partition to Ext3\n"));
                //               return 0;
            case ValidateState::MipsNotSupportEFI:
                validateMessage.append(tr("This machine not support EFI partition\n"));
                return 0;
            case ValidateState::InvalidFS:
                validateMessage.append(tr("The filesystem type FAT16 or FAT32 is not fully-function filesystem,\n"
                                          "except EFI partition, other partition not proposed"));
                return 0;

//            //合并报错--缺少分区
//            case ValidateState::RootMissing:
//                validateMessage.append(tr("No root partition\n"));
//                return 0;
//            case ValidateState::BootMissing:
//                validateMessage.append(tr("No boot partition\n"));
//                return 0;
//            case ValidateState::EfiMissing:
//                validateMessage.append(tr("No Efi partition\n"));
//                return 0;

//            //警告
//            //合并警告--缺少分区
//            case ValidateState::DataORBackupPartition:
//                validateMessage.append(tr("No backup partition\n"));
//                return -1;
//            case ValidateState::NoDataPartition:
//                validateMessage.append(tr("No data partition\n"));
//                return -1;
//            case ValidateState::NoSwapPartition:
//                validateMessage.append(tr("No swap partition\n"));
//                return -1;

            default:
                break;
            }
        }

        //合并警告--缺少分区
        if(!warningStatesofNOParti.isEmpty()) {
            validateMessage.clear();
            validateMessage.append(tr("If the "));
            int i = 0;
            QString strWInfo;
            for(ValidateState state : warningStatesofNOParti) {
                i++;
                if(1 != i)
                    validateMessage.append(("、"));
                switch (state->state()) {
                    case ValidateState::NoSwapPartition:
                          validateMessage.append(("\"swap\""));
                          strWInfo.append(tr("If the swap partition is not created,computers cannot be woken up after the system goes to sleep. "));
                          break;
                    case ValidateState::DataORBackupPartition:
                          validateMessage.append(("\"backup\""));
//                          strWInfo.append(tr("If the /backup partition is not created,system data cannot be backed up. "));
                         break;
//                    case ValidateState::NoDataPartition:
//                          validateMessage.append(("\"/data\""));
//                          strWInfo.append(tr("If the /data partition is not created,user data cannot be backed up. "));
//                          break;
                    default:
                          break;
                }
             }
             validateMessage.append(tr(" partitions are not created,do you want to go to the next step?"));
             validateMessage.append("\n");
             if(!strWInfo.isEmpty())
             {
                 validateMessage.append(strWInfo);
                 validateMessage.append("\n");
             }
             return -1;
        }

        return true;
    }
}

void MainPartFrame::addStyleSheet()
{
    QFile file(":/res/qss/KPartition.css");
    qDebug() << file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

void MainPartFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}

void MainPartFrame::translateStr()
{
    m_mainTitle->setText(tr("Choose Installation Method"));
    m_fullInstallBtn->setText(tr("Full install"));
    m_customInstallBtn->setText(tr("Custom install"));
    //    m_coexistInstallBtn->setText(tr("Coexist install"));
    m_nextBtn->setText(tr("Next"));
}

void MainPartFrame::writeSettingIni(InstallBtn btn)
{
    if(btn == InstallBtn::QuikPart) {
        WriteSettingToIni("config", "devpath", m_fullPartFrame->m_devPath);
        WriteSettingToIni("config", "automatic-installation", QString::number(1));

        WriteSettingToIni("config", "factory-backup", QString::number(
                              m_fullPartFrame->m_firstback->isChecked()));

    } else if(btn == InstallBtn::CustomPart) {
        WriteSettingToIni("config", "automatic-installation", QString::number(0));
        WriteSettingToIni("custompartition", "partitions", m_partDelegate->iniPartitions);
        WriteSettingToIni("custompartition", "bootloader", m_partDelegate->getBootLoadPath());

    }

}


void MainPartFrame::getChoiceDisk(bool flag)
{
    if(flag) {
        m_nextBtn->setEnabled(true);
        qDebug()<<"getChoiceDisk set m_nextBtn true";
        //this->setFocus();
    } else {
        m_nextBtn->setEnabled(false);
        qDebug()<<"getChoiceDisk set m_nextBtn false";
    }
}

void MainPartFrame::keyPressEvent(QKeyEvent *event)
{
   // this->setFocus();
    if (event->key() == Qt::Key_Left) {
        if(stack->currentWidget()==wid)
        {
            // emit leftKeyPressed();
        }

    }
    else if (event->key() == Qt::Key_Right) {
        if(stack->currentWidget()==wid)
        {
            //  emit rightKeyPressed();
        }
    }
    else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        emit enterpressed();
    }
    else if(event->key() == Qt::Key_Backspace)
    {
//        emit backspacepressed();

        if(stack->currentWidget()==m_prepareFrame)
        {
             stack->setCurrentWidget(wid);
             m_nextBtn->setText(tr("Next"));
             m_nextBtn->setDisabled(false);
        }
        else if(stack->currentWidget()==wid)
        {
             emit backspacepressed();
        }
    }
    else if (event->key() == Qt::Key_Control || event->key() == Qt::Key_Escape) {
        this->setFocus();
    }
    else
        QWidget::keyPressEvent(event);
}

void MainPartFrame::DiskNumCheck(int num)
{
    m_loadpic->hide();
    m_fullPartFrame->m_encryptBox->show();
    m_fullPartFrame->m_lvm->show();
    if (m_fullPartFrame->firstbackShow) {
        m_fullPartFrame->m_firstback->show();
    }
    m_fullInstallBtn->setEnabled(true);
    m_customInstallBtn->setEnabled(true);
    getChoiceDisk(1);//使能往下一步的按钮
 qDebug()<<"NumCheck="<<num;
    if(m_fullInstallBtn->isChecked()==true)
    {
        QString tmp = getDeviceStrByType(getDeviceModel());
        tmp.remove(QRegExp("\\s"));//去除字符串中间空格
        if(tmp.contains("Kirin990") || tmp.contains("Kirin9006C")) {
            if(KServer::KReadFile("/proc/cmdline").contains("factory-backup")) {
               m_fullPartFrame->m_firstback->setChecked(true);//bug164736
            }
        }

        if(num==1 && !m_fullPartFrame->slistDisk->m_disklist.isEmpty())
        {
           qDebug()<<"paths="<<m_fullPartFrame->slistDisk->m_disklist.at(0)->devPath;
            m_fullPartFrame->m_devPath=m_fullPartFrame->slistDisk->m_disklist.at(0)->devPath;
        }
        else
        getChoiceDisk(0);
    }
    initCustomPartFrameFocusPolicy();

}

void MainPartFrame::showEvent(QShowEvent *event)
{
    showQuikPartFrame();//默认每次进去此界面时，显示全盘安装
    initfullPartFrameFocusPolicy();
    this->setFocus();
}

void MainPartFrame::setSizebyFrame(QWidget *pFrame, int SCreenWidth, int SCreenHeight)
{
    m_pFrame = pFrame;
    m_SCreenWidth = SCreenWidth;
    m_SCreenHeight= SCreenHeight;
    if(m_customPartFrame){
        m_customPartFrame->m_pFrame = m_pFrame;
        m_customPartFrame->m_SCreenWidth = m_SCreenWidth;
        m_customPartFrame->m_SCreenHeight = m_SCreenHeight;
    }
}
bool MainPartFrame::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == m_fullInstallBtn) {
        if(event->type()==QEvent::KeyPress)
        {
            QKeyEvent *ev=(QKeyEvent*)event;
            if(ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return)
            {
                m_fullInstallBtn->click();
                return true;
            }
        }
    }
    if(watched == m_customInstallBtn) {
         if(event->type()==QEvent::KeyPress)
        {
            QKeyEvent *ev=(QKeyEvent*)event;
            if(ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return)
            {
                m_customInstallBtn->click();
                return true;
            }
        }
    }
     return QWidget::eventFilter(watched, event);
}

}

