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

#include "logiccopydisc.h"
#include "src/frame/logger.h"
#include "src/data/datacdrom.h"
#include "src/model/logicmain.h"
#include "src/view/modaldialog.h"
#include "src/view/mainwindow.h"
#include "src/process/processlsof.h"
#include "src/view/tip_burnresult.h"
#include "src/view/execdialog.h"
#include "src/po/translation.h"
#include "burnjob.h"
#include "burnjobmanager.h"
#include "src/view/progress.h"
#include "src/frame/auditlog.h"
#include "src/common/utils.h"

#include <QEventLoop>
#include <QStandardPaths>

#include <kmessagebox.h>

using namespace ky_plugin_burn;
using namespace dfmburn;
LogicCopyDisc *LogicCopyDisc::m_oInstance = nullptr;
QMutex     LogicCopyDisc::m_oMutex;

LogicCopyDisc::LogicCopyDisc(QObject *parent) :
    QObject(parent),
    m_iSource(-1),
    m_iDestination(-1)
{
    m_oSources.clear();
    m_oDestinations.clear();
    connect(LogicMain::instance(), &LogicMain::createCopyImage, this, &LogicCopyDisc::slotsAddDestination);
    connect(LogicMain::instance(), &LogicMain::emptyMedium, this, &LogicCopyDisc::slotsAddDestination);
    connect(LogicMain::instance(), &LogicMain::appendableMedium, this, &LogicCopyDisc::slotsAddSource);
    connect(LogicMain::instance(), &LogicMain::finishedMedium, this, &LogicCopyDisc::slotsAddSource);
    connect(LogicMain::instance(), &LogicMain::mediumRmoved, this, &LogicCopyDisc::slotsNoMedium);
    connect(LogicMain::instance(), &LogicMain::burnCopySetting, this, &LogicCopyDisc::slotsSetting);
    connect(LogicMain::instance(), &LogicMain::getISO, this, &LogicCopyDisc::slotsGetISO);
    connect(LogicMain::instance(), &LogicMain::copyData, this, &LogicCopyDisc::slotsCopyData);
    connect(this, SIGNAL(setting(void*)), BUSNESSSETTING, SLOT(load(void *)));
}

LogicCopyDisc *LogicCopyDisc::instance()
{
    if (nullptr == m_oInstance)
    {
        QMutexLocker locker(&m_oMutex);

        if (nullptr == m_oInstance)
        {
            m_oInstance = new LogicCopyDisc();
        }
    }
    return m_oInstance;
}

bool LogicCopyDisc::isDestinationDiscDL()
{
    if ( m_iDestination <= 0 || m_oDestinations.size() < 0)
        return false;

    DataCDROM *des = static_cast<DataCDROM *>(m_oDestinations.at(m_iDestination));
    if (des->mediumType().contains("DL")) {
        return true;
    } else {
        return false;
    }

}


bool LogicCopyDisc::capacity()
{
    DataCDROM *src;
    DataCDROM *des;

    if (m_iDestination == 0) return true;
    if (m_iSource < 0 || m_iDestination < 0) return false;


    src = static_cast<DataCDROM *>(m_oSources.at(m_iSource));
    des = static_cast<DataCDROM *>(m_oDestinations.at(m_iDestination));

    return des->mediumFreeSize() >= src->mediumUsedSize();
}

void LogicCopyDisc::updateISOPath(QString path)
{
    DataCDROM *cdrom;

    cdrom = static_cast<DataCDROM *>(m_oDestinations.at(0));
    cdrom->updateDisplay(path);
}

void LogicCopyDisc::slotsAddDestination(void *d)
{
    if (-1 != m_oDestinations.indexOf(d)){
        //update information
        emit sigAddDestination(d);
        return;
    }

    m_oDestinations << d;
    emit sigAddDestination(d);
}

void LogicCopyDisc::slotsAddSource(void *d)
{
    if (-1 != m_oSources.indexOf(d))
        return;

    m_oSources << d;
    emit sigAddSource(d);
}

void LogicCopyDisc::slotsNoMedium(void *d)
{
    int idx = m_oDestinations.indexOf(d);
    if (-1 != idx)
    {
        m_oDestinations.takeAt(idx);
        emit sigDelDestination(idx);
    }

    idx = m_oSources.indexOf(d);
    if (-1 != idx)
    {
        m_oSources.takeAt(idx);
        emit sigDelSource(idx);
    }
}

void LogicCopyDisc::slotsSetting()
{
    DataCDROM *src;
    DataCDROM *des;
    ModalDialog::instance()->showPage(ModalDialog::BusnessSetting);
    ModalDialog::instance()->show();
    src = static_cast<DataCDROM *>(m_oSources.at(m_iSource));
    des = static_cast<DataCDROM *>(m_oDestinations.at(m_iDestination));
    des->setSettingVolumnID(src->settingVolumnID());
    des->setSettingPreparer(src->settingPreparer());
    des->setSettingPublisher(src->settingPublisher());
    des->setSettingCopyright(src->settingCopyright());
    if (0 == m_iDestination)
    {
        des->setSettingClean(false);
        des->setSettingSetOpen(false);
    }
    else
    {
        des->setSettingCloseSession(src->mediumStatus() == DataMedium::MEDIUM_FINALIZED);
        des->setSettingCreateISO(false);
    }
    setting(des);
}

void LogicCopyDisc::slotsGetISO()
{
#ifndef UNIT_TEST
    QIcon progressIcon(QIcon::fromTheme("ukui-loading-0-symbolic"));
    Progress* progressDlg = static_cast<Progress*>(ModalDialog::instance()->showPage(ModalDialog::Progress));
    progressDlg->title( _("Getting ISO"));
    progressDlg->setsSetRange(0,100);
    progressDlg->slotsSetValue(0, progressIcon,_("Getting ISO"));
    ModalDialog::instance()->cancelEnable(false);
    ModalDialog::instance()->show();

    progressWidgetDelayResponeTimer(50);

    if((m_iSource < 0) || (m_iSource >= m_oSources.size())){
        return;
    }

    DataCDROM *srcCdrom = static_cast<DataCDROM *>(m_oSources.at(m_iSource));
    if (srcCdrom->checkUsingOfOtherApp()) {
        return;
    }

    //burn option
    BurnOptions opts;
    BurnJobManager::Config config;
    config.opts = opts;
    config.speeds = srcCdrom->settingSpeed();

    if (srcCdrom->mediumMountPoint().isEmpty())
    {
        LOGGER->error(_("Medium in CDROM %1 donot been mounted.").arg(srcCdrom->display()));
        ModalDialog::instance()->hide();
        Tip_BurnResult::instance()->slotsFailed( _("Getting ISO"));
        return;
    }

    QString output_path = static_cast<DataCDROM *>(m_oDestinations.at(0))->device();
    output_path.append(srcCdrom->settingVolumnID());
    output_path.append(".iso");

    QFileInfo file_info(output_path);
    file_info.setFile(output_path);
    if (file_info.exists()) {
        bool result = MyMessageBox::showIsoReplaceMessageBox(file_info.fileName());
        if (!result) {
            ModalDialog::instance()->hide();
            return;
        }

        if (file_info.isFile()) {
            QFile::remove(file_info.absoluteFilePath());
        } else {
            QDir dir(file_info.absoluteFilePath());
            dir.removeRecursively();
        }
    }

    //umount device
    if (!srcCdrom->mediumType().contains("RW") && (srcCdrom->numDataTracks() == 1)) {
        bool umountSucceed = DataCDROM::umountMedium(srcCdrom->device());
        if (!umountSucceed) {
            LOGGER->error(QString("(ProcessBurnData::working) cdrom run umount:%1").arg(umountSucceed));
        }
    }

    // auditlog
    QGSettings auditLogGSettings("org.ukui.kylinburner");
    QString auditLogPath="/var/log/kylin-burner/os_audit.log";
    bool isOpenAuditLog = auditLogGSettings.get("burneraudit-isrunning").toBool();
    AuditLog burnDataLog;

    BurnJobManager::instance()->disconnect();
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, this, [&](int jobType, bool result){
        if (isOpenAuditLog) {
            auditLogGSettings.set("burner-os-auditlog-name",auditLogPath);
            burnDataLog.outputCopyResult(srcCdrom->mediumMountPoint(), srcCdrom->display(), srcCdrom->mediumType(), _("Getting ISO"), true==result);;
        }

        if(!srcCdrom->mediumType().contains("RW") && (srcCdrom->numDataTracks() == 1)) {
            bool mountSucceed = DataCDROM::mountMedium(srcCdrom->device());
            if (!mountSucceed) {
                LOGGER->error(QString("(copy image) cdrom run mount:%1").arg(mountSucceed));
            }
        }

        ModalDialog::instance()->cancelEnable(false);
        ModalDialog::instance()->accept();

        if(result){
            Tip_BurnResult::instance()->slotsSuccess( _("Get ISO"));
        }else{
            Tip_BurnResult::instance()->slotsFailed( _("Get ISO"));
        }
    });

    connect(BurnJobManager::instance(), &BurnJobManager::currentJobProgressChanged, this, [&](int value){
        if(value >= 100){
            progressDlg->slotsSetValueWithoutClosing(value, _("Getting ISO"));
        }else{
            progressDlg->slotsSetValueWithoutClosing(value,  _("Getting ISO"));
        }
    },Qt::UniqueConnection);

    connect(BurnJobManager::instance(), &BurnJobManager::logMsgGenerated, this, [&](int level, const QStringList msgList){
        for(QString msg : msgList){
            if(level == 0){
                LOGGER->error(msg);
            }else if(level == 1){
                LOGGER->info(msg);
            }
        }
    });

    QEventLoop loop;
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, &loop, &QEventLoop::quit);
    if(srcCdrom->mediumType().contains("RW") || (srcCdrom->numDataTracks() > 1)){
        config.speeds = 8;
        opts |= BurnOption::kJolietSupport;
        opts |= BurnOption::kKeepAppendable;
        config.opts = opts;

        QString medium_mount_point = srcCdrom->mediumMountPoint();
        medium_mount_point.append("/");

        BurnJobManager::instance()->startBurnISOFiles(output_path,QUrl::fromLocalFile(medium_mount_point),config);
    }else{
        //start burn job
        BurnJobManager::instance()->startDumpISOImage(srcCdrom->device(), QUrl::fromLocalFile(output_path));
    }
    loop.exec();
#endif
}

void LogicCopyDisc::slotsCopyData()
{
#ifndef UNIT_TEST
    if((m_iSource < 0) || (m_iSource >= m_oSources.size()) || (m_iDestination < 0) || (m_iDestination >= m_oDestinations.size())){
        return;
    }

    DataCDROM *srcCdrom = static_cast<DataCDROM *>(m_oSources.at(m_iSource));
    DataCDROM *desCdrom = static_cast<DataCDROM *>(m_oDestinations.at(m_iDestination));
    
    if (srcCdrom->mediumType().contains("BD-R") || desCdrom->mediumType().contains("BD-R")){
        EXEC->information("",_("The Blu-ray disc does not support the operation of copying discs"),true);
        return;
    } else if (srcCdrom->mediumType().contains("CD") && desCdrom->mediumType().contains("DVD")){
        EXEC->information("",_("This disc is a CD, and copying to DVD is not supported"),true);
        return;

    } else if (srcCdrom->mediumType().contains("DVD") && desCdrom->mediumType().contains("CD")){
        EXEC->information("",_("This disc is a DVD, and copying to CD is not supported"),true);
        return;
    }

    QIcon progressIcon(QIcon::fromTheme("ukui-loading-0-symbolic"));
    Progress* progressDlg = static_cast<Progress*>(ModalDialog::instance()->showPage(ModalDialog::Progress));
    progressDlg->title( _("Copy Disk"));
    progressDlg->setsSetRange(0,100);
    progressDlg->slotsSetValue(0, progressIcon,_("Copy Disk"));
    ModalDialog::instance()->cancelEnable(false);
    ModalDialog::instance()->show();

    if (srcCdrom->checkUsingOfOtherApp()) {
        return;
    }

    QString mediaType = desCdrom->mediumType();

    bool cannotapppended = !mediaType.contains("DVD+RW")
                         && !mediaType.contains("DVD-RW")
                         && !mediaType.contains("DVD+R DL");

    if (cannotapppended) {
        int ret = QMessageBox::warning(MainWindow::Instance(), "",  _("The copied target disc cannot be appended. Do you want to continue with the copy operation?"), _("Cancel"), _("OK"), "", 1);
        if (ret <= 0) {
            ModalDialog::instance()->hide();
            return;
        }
    }
    //burn option
    BurnOptions opts;
    BurnJobManager::Config config;

    QString output_path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    output_path.append("/.kylin-burner/").append(srcCdrom->settingVolumnID()).append(".iso");

    QFileInfo fileInfo;
    fileInfo.setFile(output_path);
    if (fileInfo.exists())
    {
        if (fileInfo.isFile()) {
            QFile::remove(fileInfo.absoluteFilePath());
        }else{
            QDir dir(fileInfo.absoluteFilePath());
            dir.removeRecursively();
        }
    }

    QString dirPath = fileInfo.absolutePath();
    QDir dir;
    if(!dir.exists(dirPath)) {
        if(!dir.mkpath(dirPath)){
             LOGGER->error(QString("(copy image) create %1 error").arg(dirPath));
        }
    }

    LOGGER->error(QString("(copy image) src->numDataTracks() = %1").arg(srcCdrom->numDataTracks()));

    //umount device
    if(!srcCdrom->mediumType().contains("RW") && (srcCdrom->numDataTracks() == 1)){
        bool umountSucceed = DataCDROM::umountMedium(srcCdrom->device());
        if (!umountSucceed) {
            LOGGER->error(QString("(copy image) cdrom run umount:%1").arg(umountSucceed));
        }
    }

    // auditlog
    QGSettings auditLogGSettings("org.ukui.kylinburner");
    QString auditLogPath="/var/log/kylin-burner/os_audit.log";
    bool isOpenAuditLog = auditLogGSettings.get("burneraudit-isrunning").toBool();
    AuditLog burnDataLog;

    BurnJobManager::instance()->disconnect();
    bool isDumpSucceed = true;
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, this, [&](int jobType, bool result){
        if (isOpenAuditLog) {
            auditLogGSettings.set("burner-os-auditlog-name",auditLogPath);
            burnDataLog.outputCopyResult(srcCdrom->mediumMountPoint(), desCdrom->display(), desCdrom->mediumType(), _("Copy Disk"), true==result);;
        }

        if(!srcCdrom->mediumType().contains("RW") && (srcCdrom->numDataTracks() == 1)){
            bool mountSucceed = DataCDROM::mountMedium(srcCdrom->device());
            if (!mountSucceed) {
                LOGGER->error(QString("(copy image) cdrom run mount:%1").arg(mountSucceed));
            }
        }

        if(!result){
            isDumpSucceed = false;
            ModalDialog::instance()->cancelEnable(false);
            ModalDialog::instance()->accept();
            Tip_BurnResult::instance()->slotsFailed(_("Copy Disk"));
        }
    });

    connect(BurnJobManager::instance(), &BurnJobManager::currentJobProgressChanged, this, [&](int value){
        value = value*0.5;
        progressDlg->slotsSetValueWithoutClosing(value,_("Copy Disk"));
    },Qt::UniqueConnection);

    connect(BurnJobManager::instance(), &BurnJobManager::logMsgGenerated, this, [&](int level, const QStringList msgList){
        for(QString msg : msgList){
            if(level == 0){
                LOGGER->error(msg);
            }else if(level == 1){
                LOGGER->info(msg);
            }
        }
    });

    QEventLoop loop;
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, &loop, &QEventLoop::quit);
    if(srcCdrom->mediumType().contains("RW") || (srcCdrom->numDataTracks() > 1) ){
        config.speeds = 8;
        opts |= BurnOption::kJolietSupport;
        opts |= BurnOption::kKeepAppendable;
        config.opts = opts;
        config.volName = srcCdrom->settingVolumnID();

        QString medium_mount_point = srcCdrom->mediumMountPoint();
        medium_mount_point.append("/");

        BurnJobManager::instance()->startBurnISOFiles(output_path,QUrl::fromLocalFile(medium_mount_point),config);
    }else{
        //start burn job
        BurnJobManager::instance()->startDumpISOImage(srcCdrom->device(), QUrl::fromLocalFile(output_path));
    }
    loop.exec();

    if(!isDumpSucceed){
        return;
    }

    //start copy disk
    BurnJobManager::instance()->disconnect();
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, this, [&](int jobType, bool result){
        if (isOpenAuditLog) {
            auditLogGSettings.set("burner-os-auditlog-name",auditLogPath);
            burnDataLog.outputCopyResult(srcCdrom->mediumMountPoint(), desCdrom->display(), desCdrom->mediumType(), _("Copy Disk"), true==result);;
        }

        ModalDialog::instance()->cancelEnable(false);
        ModalDialog::instance()->accept();

        emit g_user_signal->noMedium(desCdrom);
        desCdrom->eject();

        if(result){
            Tip_BurnResult::instance()->slotsSuccess(_("Copy Disk"));
        }else{
            Tip_BurnResult::instance()->slotsFailed(_("Copy Disk"));
        }

    });

    connect(BurnJobManager::instance(), &BurnJobManager::currentJobProgressChanged, this, [&](int value){
        value = 50 + value*0.5;
        progressDlg->slotsSetValueWithoutClosing(value,_("Copy Disk"));
    },Qt::UniqueConnection);

    connect(BurnJobManager::instance(), &BurnJobManager::logMsgGenerated, this, [&](int level, const QStringList msgList){
        for(QString msg : msgList){
            if(level == 0){
                LOGGER->error(msg);
            }else if(level == 1){
                LOGGER->info(msg);
            }
        }
    });

    QEventLoop copy_loop;
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, &copy_loop, &QEventLoop::quit);

    if (desCdrom->mediumType().contains("DVD+R DL")) {
        config.speeds = 4;
    } else {
        config.speeds = 8;
    }
    opts |= BurnOption::kJolietSupport;
    opts |= BurnOption::kKeepAppendable;
    config.opts = opts;

    BurnJobManager::instance()->startBurnISOImage(desCdrom->device(),QUrl::fromLocalFile(output_path),config);

    copy_loop.exec();

    //remove tmp file after copy
    if (fileInfo.exists())
    {
        if (fileInfo.isFile()) {
            QFile::remove(fileInfo.absoluteFilePath());
        }else{
            QDir dir(fileInfo.absoluteFilePath());
            dir.removeRecursively();
        }
    }
#endif
}


LogicCopyDisc::~LogicCopyDisc()
{
}

void LogicCopyDisc::progressWidgetDelayResponeTimer(int ms)
{
    QEventLoop eventloop;
    QTimer *dataDelayCopyTimer = new QTimer(this);
    connect(dataDelayCopyTimer, &QTimer::timeout, this, [&](){
        dataDelayCopyTimer->stop();
        eventloop.quit();
    });
    dataDelayCopyTimer->start(ms);
    eventloop.exec();
    dataDelayCopyTimer->deleteLater();
}
