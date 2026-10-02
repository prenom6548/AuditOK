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
#include "globalsignal.h"

#include <QDebug>
#define KYLINSCANNER "org.kylin-scanner-data.settings"

GlobalUserSignal* GlobalUserSignal::instance = new GlobalUserSignal;

GlobalUserSignal *GlobalUserSignal::getInstance()
{
    return instance;

}
bool GlobalUserSignal::getCurrentMode(){
    return m_isPCMode;
}
void GlobalUserSignal::setCurrentMode(bool isPCMode){
    m_isPCMode = isPCMode;
}
void GlobalUserSignal::setDeviceList(const SANE_Device **list){
    deviceList.clear();
    for (int i = 0; list[i]; ++i) {
        SANE_Device tmp;
        tmp.model = list[i]->model;
        tmp.name = list[i]->name;
        tmp.type = list[i]->type;
        tmp.vendor = list[i]->vendor;
        deviceList.insert(i,tmp);
    }
    qDebug() << "Device Count:" << deviceList.length();
}
QList<SANE_Device> GlobalUserSignal::getDeviceList(){
    return deviceList;
}
void GlobalUserSignal::setDeviceInUse(SANE_Device device){
    inUseDevice = device;
}
SANE_Device  GlobalUserSignal::getDeviceInUse(){
    return inUseDevice;
}
void GlobalUserSignal::openDeviceSignalFun(int num){
    emit openDeviceSignal(num);
}
void GlobalUserSignal::updateSetting(){
    emit updateSettingSignal();
}
void GlobalUserSignal::warnMsg(QString msg){
    emit warnMsgSignal(msg);
}
void GlobalUserSignal::setIsFailPage(bool res){
    isFailPage = res;
}
bool GlobalUserSignal::getIsFailPage(){
    return isFailPage;
}
void GlobalUserSignal::exitApplication()
{
    emit exitApplicationSignal();
}

void GlobalUserSignal::minimumWindow()
{
    emit minimumWindowSignal();
}

void GlobalUserSignal::maximumWindow()
{
    emit maximumWindowSignal();
}

void GlobalUserSignal::showHelpDialog()
{
    emit showHelpDialogSignal();
}


void GlobalUserSignal::detectPageWaitTimerStart()
{
    emit detectPageWaitTimerStartSignal();
}

void GlobalUserSignal::detectPageWaitTimerStop()
{
    emit detectPageWaitTimerStopSignal();
}

void GlobalUserSignal::openDeviceStatus(bool openSucceed)
{
    emit openDeviceStatusSignal(openSucceed);
}

void GlobalUserSignal::startScanOperation()
{
    emit startScanOperationSignal();
}

void GlobalUserSignal::stopOrFinishedScan()
{
    emit stopOrFinishedScanSignal();
}

void GlobalUserSignal::scanThreadFinished(int saneStatus)
{
    emit scanThreadFinishedSignal(saneStatus);
}

void GlobalUserSignal::scanThreadFinishedImageLoad(QStringList loadPath,QStringList savePath)
{
    emit scanThreadFinishedImageLoadSignal(loadPath,savePath);
}

void GlobalUserSignal::showImageAfterClickedThumbnail(QString loadPath)
{
    emit showImageAfterClickedThumbnailSignal(loadPath);
}

void GlobalUserSignal::updateSaveNameTextAfterScanSuccess()
{
    emit updateSaveNameTextAfterScanSuccessSignal();
}

void GlobalUserSignal::closeWindowSaveAs(bool exitApp)
{
    emit closeWindowSaveAsSignal(exitApp);
}

void GlobalUserSignal::saveAsButtonClicked(QString filepath)
{
    emit saveAsButtonClickedSignal(filepath);
}

void GlobalUserSignal::sendMailButtonClicked()
{
    emit sendMailButtonClickedSignal();
}

void GlobalUserSignal::toolbarBeautyOperationStart()
{
    qDebug() << "emit beauty signal!";
    emit toolbarBeautyOperationStartSignal();
    qDebug() << "emit beauty signal complete!";
}

void GlobalUserSignal::doBeautyOperation()
{
    emit doBeautyOperationSignal();
}

void GlobalUserSignal::doBeautyOperationFinished()
{
    emit doBeautyOperationFinishedSignal();
}

void GlobalUserSignal::toolbarBeautyOperationStop()
{
    emit toolbarBeautyOperationStopSignal();
}

void GlobalUserSignal::toolbarRectifyOperationStart()
{
    emit toolbarRectifyOperationStartSignal();
}

void GlobalUserSignal::doRectifyOperation()
{
    emit doRectifyOperationSignal();
}

void GlobalUserSignal::doRectifyOperationFinished(bool isTrue)
{
    emit doRectifyOperationFinishedSignal(isTrue);
}

void GlobalUserSignal::toolbarRectifyOperationStop()
{
    emit toolbarRectifyOperationStopSignal();
}

void GlobalUserSignal::toolbarOcrOperationStart()
{
    emit toolbarOcrOperationStartSignal();
}

void GlobalUserSignal::doOcrOperation()
{
    emit doOcrOperationSignal();
}

void GlobalUserSignal::toolbarOcrOperationFinished()
{
    emit toolbarOcrOperationFinishedSignal();
}

void GlobalUserSignal::toolbarCropOperation()
{
    emit toolbarCropOperationSignal();
}

void GlobalUserSignal::toolbarRotateOperation()
{
    emit toolbarRotateOperationSignal();
}

void GlobalUserSignal::toolbarMirrorOperation()
{
    emit toolbarMirrorOperationSignal();
}

void GlobalUserSignal::toolbarWatermarkOperation()
{
    emit toolbarWatermarkOperationSignal();
}

void GlobalUserSignal::toolbarZoomoutOperation()
{
    emit toolbarZoomoutOperationSignal();
}
void GlobalUserSignal::toolbarZoominOperation()
{
    emit toolbarZoominOperationSignal();
}

void GlobalUserSignal::toolbarPercentageChanged()
{
    emit toolbarPercentageChangedSignel();
}

void GlobalUserSignal::closeScanDialog()
{
    emit closeScanDialogSignal();
}

void GlobalUserSignal::switchToDetectPage()
{
    emit switchToDetectPageSignal();
}

void GlobalUserSignal::stopOcrTimer()
{
    emit stopOcrTimerSignal();
}
GlobalUserSignal::GlobalUserSignal(QObject *parent) : QObject(parent)
{

}

GlobalUserSignal::~GlobalUserSignal()
{

}


GlobalCoreSignal* GlobalCoreSignal::instance = new GlobalCoreSignal;

GlobalCoreSignal *GlobalCoreSignal::getInstance()
{
    return instance;
}


GlobalCoreSignal::GlobalCoreSignal(QObject *parent) : QObject(parent)
{

}

GlobalCoreSignal::~GlobalCoreSignal()
{

}


GlobalConfigSignal* GlobalConfigSignal::instance = new GlobalConfigSignal;
GlobalConfigSignal *GlobalConfigSignal::getInstance()
{
    return instance;
}

GlobalConfigSignal::GlobalConfigSignal(QObject *parent)
{
    if(QGSettings::isSchemaInstalled(KYLINSCANNER)){
        m_data = new QGSettings(KYLINSCANNER);
    }
    m_configPath = QDir::homePath() + "/.config";

    m_scannerPath = m_configPath + "/kylin-scanner";

    m_scannerPnmImagePath = QStandardPaths::standardLocations(QStandardPaths::TempLocation).at(0)+ "/kylin-scanner/images";

    QLocale locale = QLocale::system().name();
    if(locale.language() == QLocale::Chinese){
        m_scannerImagePath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0) + "/扫描图像";
    }else if(locale.language() == QLocale::English){
        m_scannerImagePath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0) + "/kylin-scanner-images";
    }else if(locale.language() == QLocale::Tibetan){
        m_scannerImagePath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0) + "/བཤར་འབེབས་འབུར་ཐོན་";
    }else{
        m_scannerImagePath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0) + "/kylin-scanner-images";
    }

    m_scannerTempPath = QDir::tempPath() + "/kylin-scanner";

    m_kylinScannerImageDebug = false;
    m_hideScanSettingsWidget = false;
}


GlobalConfigSignal::~GlobalConfigSignal()
{

}
