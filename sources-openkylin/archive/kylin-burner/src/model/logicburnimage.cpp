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

#include "logicburnimage.h"

#include "src/common/dbuscaller.h"
#include "src/frame/logger.h"
#include "src/frame/configuration.h"
#include "src/po/translation.h"
#include "src/model/logicmain.h"
#include "src/data/datacdrom.h"
#include "src/process/processlsof.h"
#include "src/view/modaldialog.h"
#include "src/view/tip_burnresult.h"

#include "burnjob.h"
#include "burnjobmanager.h"
#include "src/view/progress.h"

#include <QFileInfo>
#include <QDebug>
#include <QEventLoop>

using namespace ky_plugin_burn;
using namespace dfmburn;
LogicBurnImage *LogicBurnImage::m_oInstance = nullptr;
QMutex          LogicBurnImage::m_oMutex;

LogicBurnImage::LogicBurnImage(QObject *parent) :
    QObject(parent),
    m_iBurner(-1)
{
    LOGGER->registerName(this, _("BURNIMAGE"));

    m_oISOPath.clear();
    m_oISOPaths.clear();
    m_oBurners.clear();
    connect(LogicMain::instance(),SIGNAL(appendableMedium(void*)),this,SLOT(slotsNewBurner(void*)));
    connect(LogicMain::instance(), SIGNAL(emptyMedium(void*)), this, SLOT(slotsNewBurner(void*)));
    connect(LogicMain::instance(), &LogicMain::mediumRmoved, this, &LogicBurnImage::slotsDeleteBurner);
    connect(LogicMain::instance(),SIGNAL(burnImage()), this, SLOT(slotsBurnImage()));
    connect(LogicMain::instance(), SIGNAL(burnImageSetting()), this, SLOT(slotsSetting()));
    connect(this, SIGNAL(setting(void*)), BUSNESSSETTING, SLOT(load(void *)));
}

LogicBurnImage *LogicBurnImage::instance()
{
    if (nullptr == m_oInstance)
    {
        QMutexLocker locker(&m_oMutex);
        if (nullptr == m_oInstance)
        {
            m_oInstance = new LogicBurnImage();
        }
    }
    return m_oInstance;
}

void LogicBurnImage::init()
{
    if (0 <= m_oBurners.size())
    {
        emit enableBurner(m_iBurner >= 0);
        emit enableBusness(check());
    }
}

QIcon LogicBurnImage::mediumIcon(int idx)
{
    DataCDROM *cdrom;
    if (idx < 0 || idx >= m_oBurners.size()) return QIcon();
    cdrom = (DataCDROM *)(m_oBurners.at(idx));
    return cdrom->mediumIcon();
}

QString LogicBurnImage::display(int idx)
{
    DataCDROM *cdrom;
    if (idx < 0 || idx >= m_oBurners.size()) return QString();
    cdrom = (DataCDROM *)(m_oBurners.at(idx));
    return cdrom->mDisplay();
}

QString LogicBurnImage::tooltip(int idx)
{
    DataCDROM *cdrom;
    if (idx < 0 || idx >= m_oBurners.size()) return QString();
    cdrom = (DataCDROM *)(m_oBurners.at(idx));
    return cdrom->display();
}

QString LogicBurnImage::currentDeviceUuid()
{
    if((m_iBurner >= 0) && (m_oBurners.size() > m_iBurner)){
        DataCDROM* indexRom = (DataCDROM *)(m_oBurners[m_iBurner]);
        return indexRom->getUnique_uuid();
    }
    return QString("");
}

void LogicBurnImage::onNewISOPath(QString path)
{
    m_oISOPath = path;
    if (-1 == m_oISOPaths.indexOf(path))
    {
        m_oISOPaths << path;
        LOGGER->debug(_("Record new iso file path <%1>").arg(path));
    }
    init();
    LOGGER->debug(_("Ready to burn : %1").arg(m_oISOPath));
}

void LogicBurnImage::setCurrentBurner(QString uuid)
{
    m_iBurner = -1;
    int device_count = m_oBurners.size();
    for(int index = 0; index < device_count; ++index)
    {
        DataCDROM * cdrom = (DataCDROM *)(m_oBurners.at(index));
        if(cdrom->getUnique_uuid() == uuid){
            m_iBurner = index;
            break;
        }
    }
    init();
    LOGGER->debug(_("Current index for burning is %1").arg(m_iBurner));
}

void LogicBurnImage::slotShowResultMessageBox(bool isSuccess)
{
    if(isSuccess){
        Tip_BurnResult::instance()->slotsSuccess(_("Burn Image"));
    }else{
        Tip_BurnResult::instance()->slotsFailed(_("Burn Image"));
    }
}

BurnImageMediaStatus LogicBurnImage::checkBurner()
{
    if (m_iBurner < 0 || 0 == m_oBurners.size())
    {
        emit burnerError(_("No valid burner,need burner which with empty meduim."));
        return BIMAGE_NOTFOUND;
    }
    if ((m_oBurners.size() > 0) && (m_iBurner < m_oBurners.size())){
        DataCDROM *cdrom = (DataCDROM *)(m_oBurners.at(m_iBurner));
        if (cdrom->mediumType().contains("RW") && DataMedium::MEDIUM_EMPTY != cdrom->mediumStatus()){
            emit burnerError(_("This disc is a non-empty disc and needs to be erased!"));
            return BIMAGE_NonEmpty;
        } else if (DataMedium::MEDIUM_EMPTY != cdrom->mediumStatus()){
            emit burnerError(_("No valid burner,need burner which with empty meduim."));
            return BIMAGE_NonEmpty;
        }
    }
    return BIMAGE_Empty_SUPPORT;
}

bool LogicBurnImage::isHasBurner()
{
    if (m_oBurners.size() <= 0) {
        return false;
    } else {
        return true;
    }
}

bool LogicBurnImage::isDLDisc()
{
    if (m_iBurner < 0 || m_iBurner >= m_oBurners.size()) {
        return false;
    }

    DataCDROM *cdrom = (DataCDROM *)(m_oBurners.at(m_iBurner));
    if (cdrom->mediumType().contains("DL")) {
        return true;
    }

    return false;
}

bool LogicBurnImage::checkCapacity()
{
    if (checkBurner() < BIMAGE_Empty_SUPPORT) {
        return false;
    }

    if (m_oISOPath.isEmpty()) {
        return false;
    }

    QFileInfo f;
    f.setFile(m_oISOPath);

    DataCDROM *cdrom = (DataCDROM *)(m_oBurners.at(m_iBurner));
    if ((unsigned long)f.size() > cdrom->mediumFreeSize()) {
        return false;
    }
    return true;
}

bool LogicBurnImage::check()
{
    if (checkBurner() < BIMAGE_Empty_SUPPORT) {
        return false;
    }

    if (m_oISOPath.isEmpty()) {
        return false;
    }

    if (!checkCapacity()) {
        emit isoError(_("Too long to burn to current medium."));
        emit burnerError(_("Current medium has not enough space for burning current iso file."));
        return false;
    }

    emit ready(_("Ready to burn,please click [BurnImage] button."));
    return true;
}

void LogicBurnImage::slotsNewBurner(void *d)
{
    int idx;
    DataCDROM *cdrom;

    cdrom = (DataCDROM *)d;

    idx = m_oBurners.indexOf(d);
    if (-1 == idx)
    {
        m_oBurners << d;
        if(cdrom->mediumStatus() == DataMedium::MEDIUM_EMPTY){
            emit newDeviceArrieved(cdrom->mediumIcon(), cdrom->mDisplay(), cdrom->display(), cdrom->getUnique_uuid());
        }

        LOGGER->debug(_("New empty medium burner %1, medium %2")
              .arg(cdrom->display()).arg(cdrom->mDisplay()));
    }
}

void LogicBurnImage::slotsDeleteBurner(void *d)
{
    int idx = m_oBurners.indexOf(d);
    if (-1 != idx)
    {
        DataCDROM * cdrom = (DataCDROM *)(d);
        LOGGER->debug(_("Remove burner."));
        m_oBurners.takeAt(idx);

        emit deleteBurner(cdrom->getUnique_uuid());
    }
}

void LogicBurnImage::slotsBurnImage()
{
    DataCDROM *cdrom;
    cdrom = (DataCDROM *)m_oBurners.at(m_iBurner);
    LOGGER->debug(_("Begin to burn image, burn image<%1> to medium in <%2>")
          .arg(m_oISOPath).arg(cdrom->display()));

    if (cdrom->checkUsingOfOtherApp()) {
        return;
    }

    if (cdrom->isOpen()) {
        cdrom->close();
    }

    //umount device
    bool umountSucceed = DataCDROM::umountMedium(cdrom->device());
    if (!umountSucceed) {
        LOGGER->info(QString("(ProcessBurnData::working) cdrom run umount:%1").arg(umountSucceed));
    }

    QIcon progressIcon(QIcon::fromTheme("ukui-loading-0-symbolic"));
    Progress* progressDlg = static_cast<Progress*>(ModalDialog::instance()->showPage(ModalDialog::Progress));
    progressDlg->title(_("Burning Image"));
    progressDlg->setsSetRange(0,100);
    progressDlg->slotsSetValue(0, progressIcon, _("Burn Image"));
    ModalDialog::instance()->cancelEnable(false);
    ModalDialog::instance()->show();

    /* 检测源文件是否包含敏感信息*/
    DBusCaller caller;
    QStringList fileListPath;
    fileListPath << m_oISOPath;
    int result = caller.callSensitiveInfoCheck("", fileListPath);
    if (!caller.handleSensitiveInfoCheckResult(result)) {
        ModalDialog::instance()->hide();
        Tip_BurnResult::instance()->slotsFailed(_("Sensitive Info Check "));
        return;
    }

    BurnJobManager::instance()->disconnect();
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobProgressChanged, this, [&](int value){
        qDebug() << "ProgressValue is:" << value;
        if(value >= 100){
            progressDlg->slotsSetValueWithoutClosing(value, _("Fixating(Will wait minutes)"));
        }else{
            progressDlg->slotsSetValueWithoutClosing(value, _("Burn Image"));
        }
    },Qt::UniqueConnection);

    //Qt::UniqueConnection doesn't work
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, this, [&](int jobType, bool result){

        emit g_user_signal->noMedium(this);
        cdrom->eject();

        ModalDialog::instance()->cancelEnable(false);
        ModalDialog::instance()->accept();
        slotShowResultMessageBox(result);

    });

    connect(BurnJobManager::instance(), &BurnJobManager::logMsgGenerated, this, [&](int level, const QStringList msgList){
        for(QString msg : msgList){
            if(level == 0){
                LOGGER->error(msg);
            }else if(level == 1){
                LOGGER->info(msg);
            }
        }
    });


    BurnJobManager::Config config;
    config.speeds = cdrom->settingSpeed();

    //slow burn speed to confirm successful iso burn
    if(cdrom->mediumType() == "DVD+R DL"){
        config.speeds = 4;
    }

    QEventLoop loop;
    connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, &loop, &QEventLoop::quit);
    //start burn job
    BurnJobManager::instance()->startBurnISOImage(cdrom->device(),QUrl::fromLocalFile(m_oISOPath),config);
    loop.exec();
}

void LogicBurnImage::slotsSetting()
{
    ModalDialog::instance()->showPage(ModalDialog::BusnessSetting);
    ModalDialog::instance()->show();
    setting(m_oBurners.at(m_iBurner));
}

LogicBurnImage::~LogicBurnImage()
{
    QVariantList vars;

    vars.clear();
    while (m_oISOPaths.size())
        vars << m_oISOPaths.takeFirst();
    CFG->sets("BurnImage", "Paths", vars);
}
