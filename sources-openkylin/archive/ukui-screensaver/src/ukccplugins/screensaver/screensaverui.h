/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#ifndef SCREENSAVERUI_H
#define SCREENSAVERUI_H

#include <widgets/settinggroup.h>
#include <widgets/switchwidget.h>
#include <widgets/pushbuttonwidget.h>
#include <widgets/comboxwidget.h>
#include <widgets/radiobuttonwidget.h>
#include <widgets/flowlayout.h>
#include "screensaverpluginso.h"
#include "previewwidget.h"
#include "photocorver.h"
#include "../../screensaver/albumthumbnailwidget.h"
#include "../../screensaver/saverthumbnailwidget.h"
#include "../../screensaver/weatherthumbnailwidget.h"
#include "../../screensaver/musicthumbnailwidget.h"
#include <QHBoxLayout>
#include <QWidget>
#include <QRadioButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QButtonGroup>
#include <QFileSystemWatcher>
#include <kysdk/applications/accessinfohelper.h>
#include <kysdk/applications/klabel.h>
#include <kysdk/applications/kbackgroundgroup.h>
#include <QMessageBox>

using namespace kdk;

class ScreensaverUi : public QWidget
{
    Q_OBJECT
public:
    ScreensaverUi();
    ~ScreensaverUi();
    void setIdleTimeOptions(const QStringList &textList, const QList<int> &dataList);
    void setScreensaverOptions(const QStringList &textList, const QStringList &dataList);
    void setSwitchTimeOptions(const QStringList &textList, const QList<int> &dataList);
    void setPhotoSwitchTimeOptions(const QStringList &textList, const QList<int> &dataList);
    void updatePreview(QWidget *w);
    void closePreview();
    void setSourcePath(const QString &text);
    void setSwitchRandom(const bool &b);
    void setSwitchTime(const int &time);
    void setPhotoSwitchTime(const int &time);
    void setCustomText(const QString &text);
    void setTextCentered(const bool &b);
    void setShowUkuiTime(const bool &b);
    void setShowCustomTime(const bool &b);
    void setLockscreen(const bool &b);
    void setThumbnailBackground(QString background);
    void setSubMode(int mode);
    void setCurrentPosition(QString position);
    void setMusicFollowSystem(const bool &b);
    void setMusicPath(QString path);
    void setPhotoCycleMode(int mode);
    void setPhotoPath(QStringList paths);
    void setPhotoSelectPath(QString path);
    void setSaverDatetimeEnable(const bool &b);
    void customizeNeedHide(bool hide);
//    void setPhotoSource();

public:
    void setIdleTime(int time);
    void setScreensaver(QString type);

private:
    void initUi();
    void initThumbnailSaver();
    void initPreview();
    void initBasisSettings();
    void initWeatherSettings();
    void initMusicSettings();
    void initPhotoSettings();
    void initPhotoSource();
    void initSetting();
    void initSourceFrame();
    void initSwitchModeFrame();
    void initSwitchTimeFrame();
    void initCustomTextFrame();
    void initIdleTimeFrame();
    void initTextPositionFrame();
    void initBreakTimeFrame();
    void initScreenlockFrame();
    void initSaverDatetimeFrame();
    void sendSignals();
    void setPhotoSelectSource();
    void setMusicSelectSource();

private:
    QVBoxLayout *uiLayout         = nullptr;

    bool m_bCustomSaver = false;
    bool m_bHideCustom = false;
    int m_subMode = 0;

    QStringList m_currentPhotoSource;

private:
    KLabel    *screensaverTitle = nullptr;
    UkccFrame     *previewFrame     = nullptr;
    QHBoxLayout   *previewLayout    = nullptr;
    PreviewWidget *previewWidget    = nullptr;
    SettingGroup  *screensaverGroup = nullptr;
    QComboBox  *m_idleTimeBox   = nullptr;
    ComboxWidget  *programWidget    = nullptr;
    SettingGroup  *customizeGroup   = nullptr;
    UkccFrame   *m_showTextFrame = nullptr;
    UkccFrame   *m_timeSetFrame = nullptr;
    UkccFrame    *m_pictureSwitchFrame = nullptr;
    UkccFrame    *m_showTextSetFrame = nullptr;

    KBackgroundGroup *m_basisGroup = nullptr;

    SettingGroup *m_thumbnailSaverGroup = nullptr;
    UkccFrame *m_thumbnailFrame = nullptr;
    FlowLayout *m_thumbnailLayout = nullptr;
    MusicThumbnailWidget *m_musicWidget = nullptr;
    AlbumThumbnailWidget *m_albunWidget = nullptr;
    SaverThumbnailWidget *m_defaulSaverWidget = nullptr;
    WeatherThumbnailWidget *m_weatherThumbnailWidget = nullptr;

    UkccFrame *m_weatherFrame = nullptr;
    SettingGroup *m_musicSaverGroup = nullptr;
    SettingGroup *m_PhotoSettingGroup = nullptr;
    RadioButtonWidget *m_photoSwitchWidget = nullptr;
    ComboxWidget *m_photoFrequencyWidget = nullptr;
    SettingGroup *m_photoSourceGroup = nullptr;
    UkccFrame *m_photoSourceFrame = nullptr;

private:
    QLineEdit     *sourcePathLine     = nullptr;
    QPushButton   *sourceButton       = nullptr;
    QButtonGroup  *switchModeGroup    = nullptr;
    QComboBox     *switchTimeComboBox = nullptr;
    QLineEdit     *textLineEdit       = nullptr;
    QButtonGroup  *textPositionGroup  = nullptr;
    KSwitchButton *showCustomTimeBtn  = nullptr;
    KSwitchButton *showUkuiTimeBtn    = nullptr;
    KSwitchButton *lockButton         = nullptr;
    KSwitchButton *m_saverDatetimeBtn = nullptr;

    QPushButton *m_weatherSettingBtn = nullptr;
    KLabel *m_localLabel = nullptr;

    QRadioButton *m_followSystemBtn = nullptr;
    QRadioButton *m_customMusicBtn = nullptr;
    QLineEdit *m_musicPathLine = nullptr;
    QPushButton *m_musicSourceBtn = nullptr;

    QRadioButton *m_orderBtn = nullptr;
    QRadioButton *m_randomBtn = nullptr;

    QPushButton *m_photoSourceBtn = nullptr;
    FlowLayout *m_photoSourceLayout =nullptr;
    QString m_sSelectPhotoSource;

Q_SIGNALS:
    void idleTimeChanged(int time);
    void screensaverChanged(QString type);
    void customPathChanged(QString path);
    void switchRandomChanged(bool b);
    void switchTimeChanged(int time);
    void photoSwitchTimeChanged(int time);
    void customTextChanged(QString text);
    void textCenteredChanged(bool b);
    void showUkuiTimeChanged(bool b);
    void showCustomChanged(bool b);
    void lockscreenChanged(bool b);
    void saverDatetimeEnableChanged(bool b);
    void subModeChanged(int mode);
    void musicPathChanged(QString path);
    void musicFollowSystemChanged(bool b);
    void photoCycleModeChanged(int mode);
    void photoCycleInterval(int time);
    void photoSourceChanged(QString path);
    void photoSelectPathChanged(QString path);
    void removePhotoPathClicked(QString path);
    void openFileFolderClicked(QString path);
};

#endif // SCREENSAVERUI_H
