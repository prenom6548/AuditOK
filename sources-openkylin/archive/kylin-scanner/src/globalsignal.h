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
#ifndef GLOBALSIGNAL_H
#define GLOBALSIGNAL_H

#include <QObject>

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <sane/sane.h>
#include <QList>
#include "utils/rotatechangeinfo.h"
#include <QGSettings>

/**
 * @brief The GlobalUserSignal class
 * Global user signal
 *      -
 */
class GlobalUserSignal : public QObject
{
    Q_OBJECT

public:
    static GlobalUserSignal* getInstance();
    bool getCurrentMode();
    void setCurrentMode(bool isPCMode);

    bool exitWindowWithSaveFlag = false;

    void setDeviceList(const SANE_Device **list);
    QList<SANE_Device> getDeviceList();
    void setDeviceInUse(SANE_Device device);
    SANE_Device  getDeviceInUse();
    void openDeviceSignalFun(int);
    void updateSetting();
    void warnMsg(QString msg);
    void setIsFailPage(bool res);
    bool getIsFailPage();

    void exitApplication();
    void minimumWindow();
    void maximumWindow();
    void showHelpDialog();

    void themeChangedWhite();
    void themeChangedBlack();
    void iconThemeChanged(QString iconThemeName);
    void fontSizeChanged(int fontSize);

    void detectPageWaitTimerStart();
    void detectPageWaitTimerStop();

    void openDeviceStatus(bool openSucceed);

    void startScanOperation();

    void stopOrFinishedScan();

    void scanThreadFinished(int saneStatus);
    void scanThreadFinishedImageLoad(QStringList loadPath, QStringList savePath);

    void showImageAfterClickedThumbnail(QString loadPath);
    void updateSaveNameTextAfterScanSuccess();

    void closeWindowSaveAs(bool exitApp = false);
    void saveAsButtonClicked(QString filepath);
    void sendMailButtonClicked();

    void toolbarBeautyOperationStart();
    void doBeautyOperation();
    void doBeautyOperationFinished();
    void toolbarBeautyOperationStop();

    void toolbarRectifyOperationStart();
    void doRectifyOperation();
    void doRectifyOperationFinished(bool isTrue);
    void toolbarRectifyOperationStop();

    void toolbarOcrOperationStart();
    void doOcrOperation();
    void toolbarOcrOperationFinished();
    void stopOcrTimer();

    void toolbarCropOperation();
    void toolbarRotateOperation();
    void toolbarMirrorOperation();
    void toolbarWatermarkOperation();
    void toolbarZoomoutOperation();
    void toolbarZoominOperation();
    void toolbarPercentageChanged();

    void closeScanDialog();
    void switchToDetectPage();

signals:
    void openDeviceSignal(int);
    void updateSettingSignal();
    void warnMsgSignal(QString);
    void scanStart();

    void toolbarPercentageChangedSignel();
    void toolbarZoominOperationSignal();
    void toolbarZoomoutOperationSignal();
    void toolbarWatermarkOperationSignal();
    void toolbarMirrorOperationSignal();
    void toolbarRotateOperationSignal();
    void toolbarCropOperationSignal();

    void switchPageSig();
    void stopOcrTimerSignal();
    void toolbarOcrOperationStartSignal();
    void doOcrOperationSignal();
    void toolbarOcrOperationFinishedSignal();
    void toolbarOcrOperationStopSignal();

    void toolbarBeautyOperationStartSignal();
    void doBeautyOperationSignal();
    void doBeautyOperationFinishedSignal();
    void toolbarBeautyOperationStopSignal();

    void toolbarRectifyOperationStartSignal();
    void doRectifyOperationSignal();
    void doRectifyOperationFinishedSignal(bool isTrue);
    void toolbarRectifyOperationStopSignal();

    void sendMailButtonClickedSignal();
    void closeWindowSaveAsSignal(bool exitApp = false);
    void saveAsButtonClickedSignal(QString filepath);

    void updateSaveNameTextAfterScanSuccessSignal();
    void showImageAfterClickedThumbnailSignal(QString loadPath);
    void exitOCR();

    void scanThreadFinishedImageLoadSignal(QStringList, QStringList);
    void scanThreadFinishedSignal(int saneStatus);

    void stopOrFinishedScanSignal();

    void startScanOperationSignal();

    void openDeviceStatusSignal(bool openSucceed);
    void detectPageWaitTimerStopSignal();
    void detectPageWaitTimerStartSignal();

    void showHelpDialogSignal();
    void maximumWindowSignal();
    void minimumWindowSignal();
    void exitApplicationSignal();

    void saneCancelSignal();
    void saneRestartSignal();

    void dbusInhabitSignal();
    void dbusUnInhabitSignal();

    void cancelScanning();
    void cancelOprationOKSignal();
    void closeScanDialogSignal();
    void switchToDetectPageSignal();
    void setDeviceBoxDisableSignal();
    void changeToConnectuccessPageSignal();
    void startUsbHotPlugThreadSignal();
    void closeUsbHotPlugThreadSignal();
    void switchToSuccessPageSignal();
    void updateConnectSuccessTextSignal(bool hasdevice);
    void setExitFlagTrueSignal();
    void setExitFlagFalseSignal();
    void startHotPlugSignal();
    void refreshListInFilePage();
    void refreshListSignal();
    void setScanIconDisableSignal();
    void hotPlugScanCompleteSignal();
    void posChange(QPoint pos);
    void resetNavigatorsignal();
    void showScanWidgetSignal(QString path);
    void findNoDriverDeviceSignal();
    void cancleCorpSignal();
    void exitOcrWhenScanSignal();
	void rotationChangedSig(bool);
	void setAddDevStatusSignal(bool status);
    void initHotPlugSignal();
    void updateSettingBtnSignal();
    // 配置恢复上一次扫描的信号
    void setPageNumberSignal(QString);
    void setScannerTypeSignal(QString);
    void setScannerColorSignal(QString);
    void setScannerResolutionSignal(QString);
    void setScannerPageSizeSignal(QString);
    void setScannerFormatSignal(QString);
    void setScannerSavePathSignal(QString);

    void windowFocousSignal(bool);
private:
    static GlobalUserSignal* instance;
    bool m_isPCMode = true;
    QList<SANE_Device> deviceList;
    SANE_Device inUseDevice;
    bool isFailPage = false;

    explicit GlobalUserSignal(QObject *parent = nullptr);
    ~GlobalUserSignal();
    GlobalUserSignal(const GlobalUserSignal &) = delete;
    GlobalUserSignal &operator = (const GlobalUserSignal &) = delete;
};


class GlobalCoreSignal : public QObject
{
    Q_OBJECT

public:
    static GlobalCoreSignal* getInstance();


private:
    static GlobalCoreSignal *instance;

    explicit GlobalCoreSignal(QObject *parent = nullptr);
    ~GlobalCoreSignal();
    GlobalCoreSignal(const GlobalCoreSignal &) = delete;
    GlobalCoreSignal &operator = (const GlobalCoreSignal &) = delete;
signals:

};

class GlobalConfigSignal : public QObject
{
    Q_OBJECT

public:
    static GlobalConfigSignal* getInstance();

    QString m_configPath;
    QString m_scannerPath;
    QString m_scannerPnmImagePath;
    QString m_scannerImagePath;
    QString m_scannerTempPath;

    QString m_openFileName;
    bool m_kylinScannerImageDebug;
    bool m_hideScanSettingsWidget;

    double m_transparency = 0.60;
    QGSettings *m_data = nullptr;

private:
    static GlobalConfigSignal *instance;

    explicit GlobalConfigSignal(QObject *parent = nullptr);
    ~GlobalConfigSignal();
    GlobalConfigSignal(const GlobalConfigSignal &) = delete;
    GlobalConfigSignal &operator = (const GlobalConfigSignal &) = delete;

signals:

};

#define g_user_signal GlobalUserSignal::getInstance()
#define g_core_signal GlobalCoreSignal::getInstance()
#define g_config_signal GlobalConfigSignal::getInstance()

#endif // GLOBALSIGNAL_H

