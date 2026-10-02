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

#include "levelscrolldiskview.h"
#include <QScroller>
#include <QDebug>
#include "partman/datastruct.h"
#include "../PluginService/kcommand.h"
#include "../PluginService/kdir.h"

const int disknamelen = 20;
const int disklength = 4;

LevelScrollDiskView::LevelScrollDiskView(QWidget *parent) : QWidget(parent),
    blViewPic(false),
    numDisk(0),
    m_curDevPath(""),
    movesize(0)
{
    initUI();
    addStyleSheet();
    initAllConnect();
    emit signalWidgetSelected("");
}

void LevelScrollDiskView::initUI()
{
    layout = new QHBoxLayout();
    this->setLayout(layout);

    //2021-3-6将以前的水平布局形式更换为listwidget形式展示
    listWidget = new QListWidget();
    listWidget->installEventFilter(this);
    listWidget->viewport()->installEventFilter(this);
    listWidget->setObjectName("ScrollList");
    //设置为列表显示模式
    listWidget->setViewMode(QListView::ListMode);
    //设置列表从左往右排列
    listWidget->setFlow(QListView::LeftToRight);
    listWidget->setItemAlignment(Qt::AlignCenter);
    listWidget->setSpacing(10);
    listWidget->setFocusPolicy(Qt::TabFocus);
    //屏蔽水平与垂直的滑动条
    listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //设置为像素滚动
    listWidget->setHorizontalScrollMode(QListWidget::ScrollPerItem);

    //设置鼠标左键拖动
    QScroller::grabGesture(listWidget,QScroller::LeftMouseButtonGesture);
    connect(listWidget,&QListWidget::itemPressed,this,[=](QListWidgetItem *item){
        listWidget->setStyleSheet("QListWidget#ScrollList::Item:focus{border:2px solid transparent;background-color:transparent;border-radius:6px;}");
    });
    hlistlayout = new QHBoxLayout(this);
    hlistlayout->setSpacing(10);


    leftBtn = new QPushButton();
    leftBtn->setMinimumWidth(50);
    leftBtn->setMaximumHeight(50);
    leftBtn->setFlat(true);
    leftBtn->hide();
    leftBtn->setObjectName("ScrollListbtn");
    leftBtn->setIcon(QIcon(":/data/png/slidershow/prev.svg"));
//    m_prev->setIconSize(QSize(16, 16));
    leftBtn->setStyleSheet(QLatin1String("QPushButton{border: none;outline: none;color: white;padding:16px 16px 16px 16px;background:transparent;border:none;}"
                                        "QPushButton:hover{"
                                        "width: 48px;height: 48px;background: rgba(255, 255, 255, 0.35);border-radius: 6px;}"
                                        " QPushButton:checked{ "
                                        " height: 48px;background: rgba(255, 255, 255, 0.25); border-radius: 6px;}"
                                        "QPushButton:pressed{height: 48px;background: rgba(255, 255, 255, 0.25); border-radius: 6px;}"
                                        "QPushButton:foucs{height: 48px;background: rgba(255, 255, 255, 0.25); border-radius: 6px;}"));

    rightBtn = new QPushButton();
    rightBtn->setMinimumWidth(50);
    rightBtn->setMaximumHeight(50);
    rightBtn->setFlat(true);
    rightBtn->hide();
    rightBtn->setObjectName("ScrollListbtn");
    rightBtn->setIcon(QIcon(":/data/png/slidershow/next.svg"));
//    m_next->setIconSize(QSize(16, 16));
    rightBtn->setStyleSheet(QLatin1String("QPushButton{border: none;outline: none;color: white;padding:16px 16px 16px 16px;background:transparent;border:none;}"
                                        "QPushButton:hover{"
                                        "width: 48px;height: 48px;background: rgba(255, 255, 255, 0.35);border-radius: 6px;}"
                                        " QPushButton:checked{ "
                                        " height: 48px;background: rgba(255, 255, 255, 0.25); border-radius: 6px;}"
                                        "QPushButton:pressed{height: 48px;background: rgba(255, 255, 255, 0.25); border-radius: 6px;}"
                                        "QPushButton:foucs{height: 48px;background: rgba(255, 255, 255, 0.25); border-radius: 6px;}"));
    layout->addWidget(leftBtn, Qt::AlignLeft);
    layout->addWidget(listWidget, Qt::AlignHCenter);

//    layout->addLayout(hlistlayout, Qt::AlignCenter);
    layout->addWidget(rightBtn, Qt::AlignRight);
    layout->setSpacing(5);
}

void LevelScrollDiskView::initAllConnect()
{
    connect(leftBtn, &QPushButton::clicked, this, &LevelScrollDiskView::checkDiskLeft);
    connect(rightBtn, &QPushButton::clicked, this, &LevelScrollDiskView::checkDiskRight);

}

void LevelScrollDiskView::showBtn()
{
    if(m_disklist.length() > disklength)
    {
        leftBtn->setVisible(true);
        rightBtn->setVisible(true);
    } else {
        leftBtn->setVisible(false);
        rightBtn->setVisible(false);
    }
}

QListWidget* LevelScrollDiskView::getListWidget()
{
    if(listWidget != nullptr) {
        return listWidget;
    } else {
        return nullptr;
    }
}



void LevelScrollDiskView::addItemsToList(DeviceList devlist)
{
    for (QLayoutItem* item = hlistlayout->takeAt(0);
          item != nullptr;
          item = hlistlayout->takeAt(0)) {
       delete item->widget();
       delete item;
       item = nullptr;
     }
    QSize size;
    numDisk = devlist.length();
    for(int i = 0; i < numDisk; i++) {
        disk = new DiskInfoView();
        disk->setDevPath(devlist.at(i)->m_path);
        m_disklist.append(disk);
        disk->mIndex=i;
        connect(disk,&DiskInfoView::signalClickView,this,&LevelScrollDiskView::selectView);
        QListWidgetItem *item = new QListWidgetItem();
        size = disk->size();
        item->setTextAlignment(Qt::AlignCenter);
        listWidget->addItem(item);
        item->setSizeHint(size);
        listWidget->setItemWidget(item, disk);

        disk->setDiskPath(devlist.at(i)->m_path);
        QString model = devlist.at(i)->m_model;
        disk->setDiskName(model);

        qint64 sizeDataCurrent = 0;
        qint64 sizeSysCurrent = 0;
        qint64 nDataMiniSize = 0;
        qint64 nDataMaxSize = 0;
        getDiskSizeLimits(devlist.at(i)->m_path, sizeDataCurrent, sizeSysCurrent, nDataMiniSize, nDataMaxSize);
        disk->getDiskSizeInfo(devlist.at(i)->m_diskTotal,sizeDataCurrent*1024*1024,nDataMiniSize*1024*1024,nDataMaxSize*1024*1024);

        QString total = "";
        total = GetBiByateValueStartingG(QString("%1").arg(devlist.at(i)->m_diskTotal));
        qint64 usedspace;
        if(devlist.at(i)->m_partTableType == PartTableType::Empty) {
            usedspace = 0;
         } else {
            usedspace = (devlist.at(i)->m_diskTotal - devlist.at(i)->m_diskFreeSpace);
        }
        QString fspace = "";
        fspace = GetBiByateValueStartingG(QString("%1").arg(usedspace));
        qDebug() << "fspace/total:" << fspace << total;

        //此磁盘中是否含有被加密分区
        bool isEncrypted = false;
        Device::Ptr  p=(Device::Ptr)(devlist.at(i));
        for(int i=0;i<p->partitions.count();i++)
        {
            Partition::Ptr partitionptr=(Partition::Ptr)(p->partitions.at(i));
            if(( FSType::Empty == partitionptr->fs)//如果此分区容量信息无法读取，表示已经被加密了
                &&(-1 == partitionptr->m_freespace)
              ){
                isEncrypted = true;
                break;
            }
        }
        if(!isEncrypted){
            disk->m_isEncrypted = 0;
            disk->setDiskSize(QString("%1/%2").arg(fspace).arg(total));
        }else{
            if(lsLVMDisk(disk->devPath)){//bug220086
                disk->m_isEncrypted = 2;
                disk->setDiskSize(QString("LVM/%1").arg(total));
            }else{
                disk->m_isEncrypted = 1;
                disk->translateStr();
            }
        }

        double d =100.0 * usedspace / devlist.at(i)->m_diskTotal;
        disk->setProgressValue(d);
        qint64 t = devlist.at(i)->m_diskTotal/1024/1024/1024;
        int r = t - 50;//50G
        if(r < 0) {
            m_disklist.at(i)->setWidgetDisabled();
            m_disklist.at(i)->m_checkable = false;
            m_disklist.at(i)->setProgressStyle();
        }

        if (getDiskFormat(devlist.at(i)->m_path) == "SSD" && !m_SSDBoolValue) {
            m_SSDBoolValue = true;
            m_SSDIndex = i;
        }
        //如果只有1个移动盘，数据盘选项不可用
        if (numDisk == 1) {
            m_disklist.at(i)->dataSel->setDisabled(true);
            m_disklist.at(i)->dataSel->setVisible(false);
        } else if (numDisk == 2) {
            if (getDiskFormat(devlist.at(i)->m_path) == "HDD") {
                m_HDDBoolValue = true;
                m_HDDIndex = i;
            }
        }
        connect(disk, &DiskInfoView::signalClicked, this, &LevelScrollDiskView::showWidgetChecked);
        connect(disk, &DiskInfoView::destroyed, [=]{disk->deleteLater();});
    }

    showBtn();
    update();
    if(numDisk > disklength) {
        setListwidgetSize(QSize(((size.width() + 20)* disklength), size.height()));
    } else
        setListwidgetSize(QSize(((size.width() + 20) * numDisk), size.height()));
}

QString LevelScrollDiskView::getDiskFormat(QString diskPath)
{
    if (diskPath.length() == 0) {
        qDebug() << "传入磁盘路经为空";
        return nullptr;
    }

    QStringList diskPathList = diskPath.split("/");
    QString filePath = "/sys/block/" + diskPathList.at(diskPathList.length()-1) + "/queue/rotational";
    QString cmd = "cat";
    QStringList cmdArgvList;
    cmdArgvList.clear();
    cmdArgvList.append(filePath);
    QProcess diskFormatProcess(0);
    diskFormatProcess.start(cmd, cmdArgvList);
    diskFormatProcess.waitForFinished(-1);
    QString outputString = diskFormatProcess.readAllStandardOutput();
    outputString.remove("\n");
    if (outputString == "1") {
        return "HDD";
    } else if (outputString == "0") {
        return "SSD";
    }
    return nullptr;
}

void LevelScrollDiskView::showWidgetChecked(QString devpath)
{
    //保证在键盘焦点到listWidget时鼠标点击选中项外的任意一项不会出现白框先出现在点击项上后消失问题
    listWidget->setStyleSheet("QListWidget#ScrollList::Item:focus{border:2px solid transparent;background-color:transparent;border-radius:6px;}");
    listWidget->update();


    bool lsDataChecked = false;
    for(int i = 0; i < m_disklist.length(); i++) {
        if(m_disklist.at(i)->devPath == devpath)//保证在点击ListWidget选项时将焦点切换到选项位置
        {
            listWidget->setCurrentRow(i);
        }
        if (!m_disklist.at(i)->m_checkable) {
            m_disklist.at(i)->checkpic->setVisible(false);
            m_disklist.at(i)->dataSel->setVisible(false);
            continue;
        }

        if (m_disklist.at(i)->devPath != devpath) {
            m_disklist.at(i)->checkpic->setVisible(false);
            m_disklist.at(i)->dataSel->setVisible(true);
            m_disklist.at(i)->dataSel->setCheckable(true);//bug163945
            m_disklist.at(i)->setPartiSizeBtVisible(false);
        } else if (m_disklist.at(i)->checkpic->isVisible()) {
            m_disklist.at(i)->checkpic->setVisible(false);
            m_disklist.at(i)->dataSel->setVisible(true);
            m_disklist.at(i)->dataSel->setCheckable(true);//bug163945
            m_disklist.at(i)->dataSel->setChecked(false);
            m_disklist.at(i)->setPartiSizeBtVisible(false);
            emit signalWidgetSelected("");
        } else {
            m_disklist.at(i)->checkpic->setVisible(true);
            m_disklist.at(i)->dataSel->setChecked(false);
            m_disklist.at(i)->dataSel->setVisible(false);

            emit signalWidgetSelected(devpath);
        }

        if(m_disklist.at(i)->dataSel->isChecked() && m_disklist.at(i)->dataSel->isVisible())
        {
            lsDataChecked=true;
        }
    }

    foreach (DiskInfoView* p,  m_disklist) {
        if(!lsDataChecked)
            p->setPartiSizeBtVisible(true);
        else
            p->setPartiSizeBtVisible(false);
    }

    if (m_disklist.length()==1) {
        m_disklist.at(0)->dataSel->setDisabled(true);
        m_disklist.at(0)->dataSel->setVisible(false);
    }
    
    emit signalCheckSavehistory(devpath);
}

void LevelScrollDiskView::selectView(int k)
{
    DiskInfoView* p;
    int nCheckData=-1;
    for (int i = 0; i < m_disklist.size(); ++i) {
        p = m_disklist.at(i);
        if(i==k)
        {
            if(p->dataSel->isChecked())
            {
                nCheckData=i;
            }
        }
        else
        {
            p->dataSel->setChecked(false);
        }
    }
    DataDeviceIndex = nCheckData;

    if(DataDeviceIndex>-1)
       emit signalDataDiskSelected(true);
    else
       emit signalDataDiskSelected(false);
}

int LevelScrollDiskView::checkDisk()
{
    int checknum = 0;

    for(int i = 0; i < m_disklist.length(); i++)
    {
        if(m_disklist.at(i)->checkpic->isVisible() == true)
        {
            checknum += 1;
        }
        else
            continue;
    }

    qDebug() << "Unchecked disk and disk number: " << checknum << m_disklist.length();
    return checknum;
}

void LevelScrollDiskView::checkDiskLeft()
{
    int check = checkDisk();
    QString strdevPath;
    qDebug() << "disk quantity: " << m_disklist.length();
    qDebug() << check << "disk checked!";
    if(m_disklist.length() == 0)
        return;
    if (m_disklist.length() <= 1 || check == 0 ) {
        if(check == 0)
        {
            setListItem(0);
            showWidgetChecked(m_disklist.at(0)->devPath);//bug167256
        }
    } else {
        qDebug() << m_disklist.at(0)->devPath << m_disklist.at(m_disklist.length()-1)->devPath;
        for (int i = 0; i < m_disklist.length(); i++) {
            if (m_disklist.at(i)->checkpic->isVisible() == true) {
                qDebug() << m_disklist.at(i)->devPath;
                showWidgetChecked(m_disklist.at(i)->devPath);//取消选中
                if (i == 0) {
                    strdevPath = m_disklist.at(m_disklist.length()-1)->devPath;
                    setListItem(m_disklist.length() - 1);
                } else {
                    strdevPath = m_disklist.at(i-1)->devPath;
                    setListItem(i - 1);
                }
                break;
            }
        }
        if(!strdevPath.isEmpty())
        {
            qDebug() << "switch to " << strdevPath;
            showWidgetChecked(strdevPath);//选择下一个设备 bug167256
        }
        else
        {
            qDebug() << "switch to NULL!";
        }
    }
}

void LevelScrollDiskView::checkDiskRight()
{
    int check = checkDisk();
    QString strdevPath;
    qDebug() << "disk quantity: " << m_disklist.length();
    qDebug() << check << "disk checked!";
    if(m_disklist.length() == 0)
        return;
    if(m_disklist.length() <= 1 || check == 0)
    {
        if(check == 0)
        {
            setListItem(0);
            showWidgetChecked(m_disklist.at(0)->devPath);//bug167256
        }
    } else {
        qDebug() << m_disklist.at(0)->devPath << m_disklist.at(m_disklist.length()-1)->devPath;
        for (int i = 0; i < m_disklist.length(); i++) {
            if (m_disklist.at(i)->checkpic->isVisible() == true) {
                qDebug() << m_disklist.at(i)->devPath;
                showWidgetChecked(m_disklist.at(i)->devPath);//取消选中
                if (i == m_disklist.length() - 1) {
                    strdevPath = m_disklist.at(0)->devPath;
                    setListItem(0);
                } else {
                    strdevPath = m_disklist.at(i+1)->devPath;
                    setListItem(i + 1);
                }
                break;
            }
        }
        if(!strdevPath.isEmpty())
        {
            qDebug() << "switch to " << strdevPath;
            showWidgetChecked(strdevPath);//选择下一个设备 bug167256
        }
        else
        {
            qDebug() << "switch to NULL!";
        }
    }
}

bool LevelScrollDiskView::eventFilter(QObject *watched, QEvent *event)
{
    if(watched==listWidget&&event->type()==QEvent::KeyPress)
    {

        QKeyEvent *ev=(QKeyEvent*)event;
        if(ev->key()==Qt::Key_Space)
        {
            int index=listWidget->currentRow();
            if(index<0)
            {
                return false;
            }
            setListItem(index);
            showWidgetChecked(m_disklist.at(index)->devPath);

        }
        else if(ev->key()==Qt::Key_Up)
        {
            int index=listWidget->currentRow()-1;
            if(index<0)
                index=0;
            listWidget->setCurrentRow(index);

        }
        else if(ev->key()==Qt::Key_Down)
        {
            int index=listWidget->currentRow()+1;
            if(index>=listWidget->count())
                index=listWidget->count()-1;
            listWidget->setCurrentRow(index);
        }
        else if(ev->key()==Qt::Key_PageUp)
        {
            listWidget->setCurrentRow(0);
        }
        else if(ev->key()==Qt::Key_PageDown)
        {
            listWidget->setCurrentRow(listWidget->count()-1);
        }
        else if(ev->key()==Qt::Key_Home)
        {
            listWidget->setCurrentRow(0);
        }
        else if(ev->key()==Qt::Key_End)
        {
            listWidget->setCurrentRow(listWidget->count()-1);
        }
        listWidget->setStyleSheet("QListWidget#ScrollList::Item:focus{border:2px solid white;background-color:transparent;border-radius:6px;}");
    }
    if((watched==listWidget||watched==listWidget->viewport())&&event->type()==QEvent::MouseButtonPress)
    {
        listWidget->setStyleSheet("QListWidget#ScrollList::Item:focus{border:2px solid transparent;background-color:transparent;border-radius:6px;}");
        listWidget->update();
    }
    if(watched==listWidget&&event->type()==QEvent::FocusIn)
    {
        listWidget->setStyleSheet("QListWidget#ScrollList::Item:focus{border:2px solid white;background-color:transparent;border-radius:6px;}");
    }
    if(watched==listWidget&&event->type()==QEvent::FocusOut)
    {
        listWidget->setStyleSheet("QListWidget#ScrollList::Item:focus{border:2px solid transparent;background-color:transparent;border-radius:6px;}");
    }
    return QWidget::eventFilter(watched, event);
}

void LevelScrollDiskView::addStyleSheet()
{
    QFile file(":/res/qss/LevelScrollDiskView.css");
    qDebug() << Q_FUNC_INFO << file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = QString("");
    stylesheet = filetext.readAll();
    file.close();
    this->setStyleSheet(stylesheet);
}

void LevelScrollDiskView::setListwidgetSize(QSize size)
{
    listWidget->setMaximumSize(size);
    listWidget->setMinimumSize(size);
}
QSize LevelScrollDiskView::getListwidgetSize()
{
    return listWidget->size();
}

void LevelScrollDiskView::setListItem(int id)
{
    if(id <0)
    {
        return;
    }
    listWidget->setCurrentRow(id);
    m_disklist.at(id)->widget->setStyleSheet("#widget{background-color: rgba(255, 255, 255, 0.25);}");
    for (int i = 0; i < listWidget->count(); ++i) {
        if(i == id) {
            continue;
        } else {
            m_disklist.at(i)->widget->setStyleSheet("#widget{background-color: rgba(0, 0, 0, 0.15);}");
        }
    }
}

void LevelScrollDiskView::getDiskSizeLimits(QString devpath, qint64& sizeDataCurrent, qint64& sizeSysCurrent, qint64& nDataMiniSize, qint64& nDataMaxSize)
{
    if(devpath.isEmpty())
    {
        qDebug()<< Q_FUNC_INFO <<" parameters error!";
        return;
    }

    QString output = "";
    QString ScriptPath  = "";
    QString error = "";
    int exitcode = 0;
    if(!KServer::KCommand::getInstance()->RunScripCommand("/usr/share/kylin-os-installer/data/systemdatasize.sh",
                     {devpath}, ScriptPath, output, error, exitcode)) {
        qDebug()<< Q_FUNC_INFO <<"Run systemdatasize.sh error!";
        return;
    }

    //单位MiB
    for(const QString& line : output.split('\n')) {
        if(line.startsWith("data_default_size:")) {
            sizeDataCurrent = line.split(':').last().trimmed().toLong();
        }
        if(line.startsWith("system_default_size:")) {
            sizeSysCurrent = line.split(':').last().trimmed().toLong();
        }
        if(line.startsWith("data_min_size:")) {
            nDataMiniSize = line.split(':').last().trimmed().toLong();
        }
        if(line.startsWith("data_max_size:")) {
            nDataMaxSize = line.split(':').last().trimmed().toLong();
        }
    }
}

bool LevelScrollDiskView::lsLVMDisk(QString devPath)
{
//    QStringList paths(part->m_partitionPath);
    QString output="", path, error="";
    int excode =-1;
    KServer::KCommand::getInstance()->RunScripCommand("pvdisplay", path, output, error, excode);
    if(output.contains(devPath)){
        return true;
    }
    else{
        return false;
    }
}
