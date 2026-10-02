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

#include "logicmain.h"

#include "src/frame/logger.h"
#include "src/frame/configuration.h"
#include "src/frame/signals.h"
#include "src/po/translation.h"
#include "src/model/menufactory.h"
#include "src/data/datacdrom.h"
#include "src/data/newdevicetype.h"
#include "src/view/modaldialog.h"
#include "src/view/mainwindow.h"
#include "src/frame/signals.h"
#include "src/status-fix/discmonitor.h"

#include <unistd.h>
#include <pwd.h>
#include <QDebug>
#include <QStandardPaths>
#include <QFileInfo>
#include <QMutex>
#include <QThread>
#include <QDebug>
#include <QEventLoop>
#include <windowmanager/windowmanager.h>

#define USE_NEW_DEVICE

LogicMain *LogicMain::m_oInstance = nullptr;
QMutex     LogicMain::m_oMutex;

LogicMain::LogicMain(QObject *parent) :
    QObject(parent),
    m_iBusness(-1),
    m_oTheme(nullptr)
{
    setObjectName("LogicMain");

    m_oCDROMS.clear();
    m_oDiscMonitor = new DiscMonitor();
    m_oDiscList    = DiscMonitor::allDiscDevices();   //静态获取已连接的光盘

    LOGGER->registerName(this, _("LogicMain"));

    if (QGSettings::isSchemaInstalled("org.ukui.style"))
    {
        m_oTheme = new QGSettings("org.ukui.style");
        m_oCurrentIcon = m_oTheme->get("iconThemeName").toString();
        m_oCurrnetThemeBackground = theme();

        connect(m_oTheme, &QGSettings::changed, [=](){
            if (m_oCurrentIcon != m_oTheme->get("iconThemeName").toString())
            {
                m_oCurrentIcon = m_oTheme->get("iconThemeName").toString();
                emit g_user_signal->iconChange(QIcon::fromTheme("burner"));
            }

            QString themeBg = theme();
            if (m_oCurrnetThemeBackground != themeBg) {
                m_oCurrnetThemeBackground = themeBg;
            }
        });

        auto handleFontSizeChange = [&](){
            int fontSizeKey = m_oTheme->get("systemFontSize").toString().toInt();
            //fix theme fontsize bug
            if(fontSizeKey == 0){
                fontSizeKey = 14;
            }
            if (fontSizeKey > 0) {
                emit systemFontSizeChanged(fontSizeKey);
            }
        };

        //monitor fontsize change
        connect(m_oTheme, &QGSettings::changed,this,[=] (const QString &key) {
            if(key == "systemFontSize") {
                handleFontSizeChange();
            }else if(key == "systemFont"){
                QString fontName = m_oTheme->get(key).toString();
                emit systemFontnameChanged(fontName);
            }

        });
    }

    connect(m_oDiscMonitor, &DiscMonitor::discDriveAdd, this, &LogicMain::driveAddedSlot);
    connect(m_oDiscMonitor, &DiscMonitor::discDriveRemove, this, &LogicMain::driveRemovedSlot);
    connect(m_oDiscMonitor, &DiscMonitor::discMediaAdd, this, &LogicMain::discAddedSlot);
    connect(m_oDiscMonitor, &DiscMonitor::discMediaRemove, this, &LogicMain::discRemovedSlot);

    connect(MenuFactory::instance(), SIGNAL(burnSetting(bool)), this, SLOT(slotsMenuBurnSetting()));
    connect(this, &LogicMain::newCDromArrived, MenuFactory::instance(), &MenuFactory::newCDROM);
    connect(this, &LogicMain::CDRomRemoved, MenuFactory::instance(), &MenuFactory::takeCDROM);
    connect(g_user_signal, &GlobalUserSignal::noMedium, this, &LogicMain::mediumRmoved);
}

LogicMain *LogicMain::instance()
{
    if (nullptr == m_oInstance)
    {
        QMutexLocker locker(&m_oMutex);

        if (nullptr == m_oInstance)
        {
            m_oInstance = new LogicMain();
            QThread * thread = LogicMain::getThread();
            m_oInstance->moveToThread(thread);
            thread->start();
        }
    }
    return m_oInstance;
}

QThread * LogicMain::getThread()
{
    static QThread *thread = nullptr;
    if (!thread) {
        thread = new QThread();
        // 设置线程名称，便于调试
        thread->setObjectName("LogicMainThread");
    }
    return thread;
}

void LogicMain::destroy()
{
    LOGGER->info(QString("( LogicMain::destroy ) Line:%1 准备析构LogicMain").arg(__LINE__));

    // 终止线程
    QThread *thread = LogicMain::getThread();
    if (thread && thread->isRunning()) {
        thread->quit();
        if (!thread->wait(1000)) {
            LOGGER->warn(QString("( LogicMain::destroy ) Line:%1 线程终止超时，强制终止").arg(__LINE__));
            thread->terminate();
            thread->wait(500);
        }
    }

    // 释放单例实例
    if (m_oInstance) {
        delete m_oInstance;
        m_oInstance = nullptr;
    }
}

void LogicMain::init()
{
     currentDiscs();
}

void LogicMain::initISO()
{
    DataCDROM *c;
    QString path;

    path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    createCDROM(CFG->get("CreateISO", "DefaultPath", path).toString());
    path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    path = CFG->get("CopyDisc", "DefaultPath", path).toString();
    if (!path.endsWith("/")) path.append("/");
#ifdef USE_NEW_DEVICE
    c = new ExtendDataCDRom(path,this);
#else
    c = new DataCDROM(path, this);
#endif
    createCopyImage(c);
}

QString LogicMain::theme()
{
    QString ret;

    ret = m_oTheme->get("styleName").toString();
    if (ret == "ukui-dark") ret = "ukui-black";
    return ret;
}

QString LogicMain::icon()
{
    return m_oTheme->get("iconThemeName").toString();
}

int LogicMain::GetSystemFontSize()
{
    if (QGSettings::isSchemaInstalled("org.ukui.style"))
    {
        int fontSizeKey = m_oTheme->get("systemFontSize").toString().toInt();

        //fix theme fontsize bug
        if(fontSizeKey == 0){
            fontSizeKey = 14;
        }

        if (fontSizeKey > 0) {
            return fontSizeKey;
        }
    }
    return 10;
}

void LogicMain::createCDROM(QString blockName)
{
    //当插入光驱后在打开刻录 不会接收到changeDevice信号；
    //当打开软件后 在插入光驱，会有一个creatDevice信号，2个changeDevice信号。
    //光驱门 打开后 发出两个changeDevice信号
    //光驱门 关闭   发出

    LOGGER->info(QString("( LogicMain::createCDROM ) Line:%1 进入函数，dev = %2").arg(__LINE__).arg(blockName));
    DataCDROM *cdrom;

    if (nullptr == sender())
    {
        blockName.append("/");
        blockName.append(CFG->get("CreateISO", "VolumnID", _("KylinBurner")).toString());
        blockName.append(".iso");
    }
    LOGGER->debug(_("Creating new cdrom named %1").arg(blockName));
    if (!MODAL->isHidden()) MODAL->accept();
    cdrom = static_cast<DataCDROM *>(m_oCDROMS.value(blockName));
    if (cdrom)
    {
        LOGGER->warn(QString("LogicMain::createCDROM Line:%1  dev=%2 已经被加载，退出").arg(__LINE__).arg(blockName));
        return;
    }
    else
    {
        qDebug()<<__LINE__<<__func__<<"Creating new cdrom named"<<blockName << "current threadid:" << QThread::currentThreadId();
        LOGGER->info(QString("LogicMain::createCDROM Line:%1  dev=%2 将被创建").arg(__LINE__).arg(blockName));
#ifdef USE_NEW_DEVICE
        cdrom = new ExtendDataCDRom(blockName, this);
#else
        cdrom = new DataCDROM(blockName, this);
#endif
        m_oCDROMS.insert(blockName, cdrom);
    }

    if (cdrom->isCDROM())
    {
#ifndef UNIT_TEST
        QThread::msleep(200);
        qDebug()<<__LINE__<<__func__<<"new cdrom "<<blockName << "is real CDROM" << "current threadid:" << QThread::currentThreadId();
        connect(cdrom, SIGNAL(empty()),this, SLOT(slotsEmptyMedium()),Qt::UniqueConnection);
        connect(cdrom, SIGNAL(appendale()), this, SLOT(slotsAppendableMedium()),Qt::UniqueConnection);
        connect(cdrom, SIGNAL(finished()), this, SLOT(slotsFinishedMedium()),Qt::UniqueConnection);

        if(!cdrom->m_bUpdateStart)
        {
            bool result = false;
            cdrom->m_bUpdateStart = true;
            qDebug()<<"createCDROM update() start " << blockName;
#ifdef USE_NEW_DEVICE
            result = dynamic_cast<ExtendDataCDRom*>(cdrom)->update();
#else
            result = cdrom->update();
#endif
            LOGGER->warn(QString("LogicMain::createCDROM Line:%1  dev=%2 update() 执行结果为 result=%3 ").arg(__LINE__).arg(blockName).arg(result));
            qDebug()<<"createCDROM update() end " << blockName;
            cdrom->m_bUpdateStart = false;
            if (!result) {
                LOGGER->warn(QString("LogicMain::createCDROM Line:%1  dev=%2 result=%3 update()获取光盘信息失败，光盘不可用").arg(__LINE__).arg(blockName).arg(result));
                return;
            }
         }
#endif
    }
    else
    {
        emit createISO(cdrom);
    }
    emit newCDromArrived(cdrom);
}

void LogicMain::slotsEmptyMedium()
{
#ifndef UNIT_TEST
    DataCDROM *cdrom;

    cdrom = qobject_cast<DataCDROM *>(sender());

    LOGGER->debug(_("Get empty medium in CDROM %1").arg(cdrom->display()));
    LOGGER->debug(_("BurnerName : %1 DiskName : %2").arg(cdrom->display()).arg(cdrom->mDisplay()));

    emit emptyMedium(sender());
#endif
}

void LogicMain::slotsAppendableMedium()
{
#ifndef UNIT_TEST
    DataCDROM *cdrom;

    cdrom = qobject_cast<DataCDROM *>(sender());
    LOGGER->debug(_("Get appendable medium in CDROM %1").arg(cdrom->display()));
    LOGGER->debug(_("BurnerName : %1 DiskName : %2").arg(cdrom->display()).arg(cdrom->mDisplay()));

    emit appendableMedium(sender());
#endif
}

void LogicMain::slotsFinishedMedium()
{
#ifndef UNIT_TEST
    DataCDROM *cdrom;

    cdrom = qobject_cast<DataCDROM *>(sender());

    LOGGER->debug(_("Get finished medium in CDROM %1").arg(cdrom->display()));
    LOGGER->debug(_("BurnerName : %1 DiskName : %2").arg(cdrom->display()).arg(cdrom->mDisplay()));

    emit finishedMedium(sender());
#endif
}

void LogicMain::slotsMenuBurnSetting()
{
    if (m_iBusness < CreateISO || m_iBusness > GetISO) return;
    if (m_iBusness < BurnImage) burnDataSetting();
    else if (m_iBusness == BurnImage) burnImageSetting();
    else burnCopySetting();
}


void LogicMain::slotsSetBusness(int bus)
{
    if (bus < CreateISO || bus > GetISO)
    {
        LOGGER->error(_("Undefined busness."));
        emit functionMenuChanged(MenuFactory::instance()->getMenu(bus));
        return;
    }
    m_iBusness = bus;
    emit functionMenuChanged(MenuFactory::instance()->getMenu(bus));
    LOGGER->debug(_("Change to do busness %1").arg(m_iBusness));
}

void LogicMain::slotsDoBusness()
{
    switch (m_iBusness)
    {
    case CreateISO:
        LOGGER->debug(_("Prepare to create iso file."));
        emit createISO();
        break;
    case BurnISO9660Data:
        LOGGER->debug(_("Prepare to burn data."));
        emit burnData();
        break;
    case BurnUDFData:
        LOGGER->debug(_("Prepare to burn UDF Format data."));
        emit burnData();
        break;
    case AddBurn:
        LOGGER->debug(_("Prepare to append burn."));
        emit addBurn();
        break;
    case BurnImage:
        LOGGER->debug(_("Prepare to burn image."));
        emit burnImage();
        break;
    case CopyData:
        LOGGER->debug(_("Prepare to copy disk data."));
        emit copyData();
        break;
    case GetISO:
        LOGGER->debug(_("Prepare to get iso from disk data."));
        emit getISO();
        break;
    default:
        LOGGER->error(_("Invalid busness [%1]").arg(m_iBusness));
        break;
    }
}

void LogicMain::discAddedSlot(QString blockName)
{

    qDebug()<<"检测到光盘"<<blockName<<"插入";
    LOGGER->info(QString("new media [%1] insert.").arg(blockName));

    DataCDROM *cdrom = nullptr;
    if (m_oCDROMS.contains(blockName))
    {
        LOGGER->warn(QString("LogicMain::discAddedSlot Line:%1  dev=%2 已经被加载，退出").arg(__LINE__).arg(blockName));
        return;
    } else {
        qDebug()<<__LINE__<<__func__<<"Creating new cdrom named"<<blockName << "current threadid:" << QThread::currentThreadId();
        LOGGER->info(QString("LogicMain::discAddedSlot Line:%1  dev=%2 将被创建").arg(__LINE__).arg(blockName));
#ifdef USE_NEW_DEVICE
        cdrom = new ExtendDataCDRom(blockName, this);
#else
        cdrom = new DataCDROM(blockName, this);
#endif
        m_oCDROMS.insert(blockName, cdrom);
    }

    //判断是否是光驱
    if (cdrom->isCDROM())
    {
#ifndef UNIT_TEST
        qDebug()<<__LINE__<<__func__<<"new cdrom "<<blockName << "is real CDROM" << "current threadid:" << QThread::currentThreadId();

        //判断是空盘 还是可追加 还是不可写入
        connect(cdrom, SIGNAL(empty()),this, SLOT(slotsEmptyMedium()),Qt::UniqueConnection);
        connect(cdrom, SIGNAL(appendale()), this, SLOT(slotsAppendableMedium()),Qt::UniqueConnection);
        connect(cdrom, SIGNAL(finished()), this, SLOT(slotsFinishedMedium()),Qt::UniqueConnection);

        if(!cdrom->m_bUpdateStart)
        {
            bool result = false;
            cdrom->m_bUpdateStart = true;
            qDebug()<<"LogicMain::discAddedSlot update() start " << blockName;
#ifdef USE_NEW_DEVICE
            result = dynamic_cast<ExtendDataCDRom*>(cdrom)->update();
#else
            result = cdrom->update();
#endif
            LOGGER->warn(QString("LogicMain::discAddedSlot Line:%1  dev=%2 update() 执行结果为 result=%3 ").arg(__LINE__).arg(blockName).arg(result));
            qDebug()<<"LogicMain::discAddedSlot update() end " << blockName;
            cdrom->m_bUpdateStart = false;
            if (!result) {
                LOGGER->warn(QString("LogicMain::discAddedSlot Line:%1  dev=%2 result=%3 update()获取光盘信息失败，光盘不可用").arg(__LINE__).arg(blockName).arg(result));
                return;
            }
         }
#endif
    }
    else
    {
        emit createISO(cdrom);
    }

    emit this->newCDromArrived(cdrom);

    //运行完update()可能光驱被拔出了，所以要判断一下，不要出现段错误
    if(cdrom->m_is_using > 0 && cdrom->isCDROM())//有光驱
    {
        cdrom->use();
        if(cdrom->mediumStatus() >=0)//有光盘
        {
            emit g_user_signal->haveMedium(cdrom);
        }
        else
        {
            emit mediumRmoved(cdrom);
        }
        cdrom->release();
    }
}

void LogicMain::discRemovedSlot(QString block)
{
    LOGGER->info(QString("(LogicMain::discRemovedSlot) Line:%1 进入函数， dev = %2").arg(__LINE__).arg(block));
    DataCDROM *cdrom = static_cast<DataCDROM *>(m_oCDROMS.value(block));

    if (!MODAL->isHidden())
        MODAL->accept();

    if (nullptr == cdrom)
        return;

#ifndef UNIT_TEST
    LOGGER->info(_("Remove CDROM %1").arg(block));
    qDebug()<<__LINE__<<__func__<<"Remove CDROM %1"<< block;
    m_oCDROMS.remove(block);

    emit g_user_signal->removeDisc(block);
    emit mediumRmoved(cdrom); // 首先移除数据
    emit CDRomRemoved(cdrom); // 移除界面展示中遗留的光驱信息。

    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(&timer,&QTimer::timeout,&loop,&QEventLoop::quit);
    timer.start(4000);
    loop.exec();

    delete cdrom;
    cdrom = nullptr;
#endif
}

void LogicMain::driveAddedSlot(QString block)
{
    qDebug()<<"检测到光驱"<<block<<"插入";
}

void LogicMain::currentDiscs()
{
    qDebug() <<"-----------光驱------------";
    for(int i=0;i<m_oDiscList.size();++i)
    {
        qDebug() << "(LogicMain::currentDiscs) 光驱设备【" << i <<"】"<< m_oDiscList.at(i);
        LOGGER->info(QString("(LogicMain::currentDiscs) 光驱设备[%1]:%2").arg(i).arg(m_oDiscList.at(i)));
        DataCDROM *cdrom;
        cdrom = static_cast<DataCDROM *>(m_oCDROMS.value(m_oDiscList.at(i)));
        if (cdrom){
            LOGGER->error(QString("(LogicMain::currentDiscs) CDROM %1 already loaded.").arg(cdrom->display()));
            break;
        } else {
            qDebug() << "(LogicMain::currentDiscs) currnetDiscs 创建DataCDROM";
            LOGGER->info(QString("(LogicMain::currentDiscs) currnetDiscs 创建DataCDROM"));
#ifdef USE_NEW_DEVICE
            cdrom = new ExtendDataCDRom(m_oDiscList.at(i), this);
#else
            cdrom = new DataCDROM(m_oDiscList.at(i), this);
#endif
        }

        m_oCDROMS.insert(m_oDiscList.at(i), cdrom);
        connect(cdrom, SIGNAL(empty()),this, SLOT(slotsEmptyMedium()),Qt::UniqueConnection);
        connect(cdrom, SIGNAL(appendale()), this, SLOT(slotsAppendableMedium()),Qt::UniqueConnection);
        connect(cdrom, SIGNAL(finished()), this, SLOT(slotsFinishedMedium()),Qt::UniqueConnection);

#ifdef USE_NEW_DEVICE
            dynamic_cast<ExtendDataCDRom*>(cdrom)->update();
#else
            cdrom->update();
#endif
        emit this->newCDromArrived(cdrom);
    }
}

void LogicMain::driveRemovedSlot(QString block)
{
    qDebug()<<"检测到光驱"<<block<<"移除";
}

LogicMain::~LogicMain()
{
    LOGGER->info(QString("[%1] [%2] 析构").arg(__func__).arg(__LINE__));
    // 确保线程正确终止
    if (thread() != nullptr && thread() != QThread::currentThread()) {
        // 先断开所有连接，避免线程中还有信号发送到正在析构的对象
        disconnect();

        // 终止线程
        thread()->quit();
        thread()->wait(500); // 等待最多500ms

        // 如果线程还在运行，强制终止
        if (thread()->isRunning()) {
            thread()->terminate();
            thread()->wait(100);
        }
    }

    if (m_oTheme) {
        delete m_oTheme;
        m_oTheme = nullptr;
    }

    if (m_oDiscMonitor) {
        delete m_oDiscMonitor;
        m_oDiscMonitor = nullptr;
    }
}
