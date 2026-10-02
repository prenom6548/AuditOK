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
#include <kysdk/applications/klabel.h>

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
    void setSourcePath(const QString &text);
    void setSwitchRandom(const bool &b);
    void setSwitchTime(const int &time);
    void setPhotoSwitchTime(const int &time);
    void setCustomText(const QString &text);
    void setTextCentered(const bool &b);
    void setShowUkuiTime(const bool &b);
    void setShowCustomTime(const bool &b);
    void setLockscreen(const bool &b);
    void setSubMode(int mode);
    void setCurrentPosition(QString position);
    void setMusicFollowSystem(const bool &b);
    void setMusicPath(QString path);
    void setPhotoCycleMode(int mode);
    void setPhotoPath(QStringList paths);
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
    void initTextPositionFrame();
    void initBreakTimeFrame();
    void initScreenlockFrame();
    void sendSignals();

private:
    QVBoxLayout *uiLayout         = nullptr;

    bool m_bCustomSaver = false;

    QStringList m_currentPhotoSource;

private:
    KLabel    *screensaverTitle = nullptr;
    UkccFrame     *previewFrame     = nullptr;
    QHBoxLayout   *previewLayout    = nullptr;
    PreviewWidget *previewWidget    = nullptr;
    SettingGroup  *screensaverGroup = nullptr;
    ComboxWidget  *idleTimeWidget   = nullptr;
    ComboxWidget  *programWidget    = nullptr;
    SettingGroup  *customizeGroup   = nullptr;

    SettingGroup *m_basisGroup = nullptr;

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
    void subModeChanged(int mode);
    void musicPathChanged(QString path);
    void musicFollowSystemChanged(bool b);
    void photoCycleModeChanged(int mode);
    void photoCycleInterval(int time);
    void photoSourceChanged(QString path);
};

#endif // SCREENSAVERUI_H
