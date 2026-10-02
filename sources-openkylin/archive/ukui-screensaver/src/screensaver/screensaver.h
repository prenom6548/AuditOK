/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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

#ifndef SCREENSAVER_H
#define SCREENSAVER_H
#include <QMainWindow>
#include <QProcess>
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QGSettings>
#include <QString>
#include <QPushButton>
#include <QSettings>
#include <QSplitterHandle>
#include <QSplitter>
#include <QTime>
#include <libkydate.h>
#include <QListWidget>
#include <QWebView>
#include <QWebFrame>
#include <QWebSettings>
#include "sleeptime.h"
#include "chinesedate.h"
#include "mbackground.h"
#include "checkbutton.h"
#include "scconfiguration.h"
#include "cyclelabel.h"
#include "videoplayer.h"
#include "graphicsview.h"
#include "mediasaverwidget.h"
#include "playcontroller.h"
#include "weatherwidget.h"
#include "customalbumviewer.h"
#include "musicthumbnailwidget.h"
#include "albumthumbnailwidget.h"
#include "saverthumbnailwidget.h"
#include "weatherthumbnailwidget.h"
#include "statusbutton.h"

class WeatherManager;

enum dateinfo {
    DATE,
    TIME,
};

enum SAVERERRORTYPE {
    VIDEOTOOLARGE,
    VIDEOFORMATEERROR,
    RESOLUTIONERROR,
    URLADDRESSERROR,
    PLAYERSHOWERROR,
    NORMALSAVER
};

class Screensaver : public QWidget
{
    Q_OBJECT

public:
    explicit Screensaver(bool isScreensaver, QPixmap *originalBackground = nullptr, QWidget *parent = 0);
    ~Screensaver();
    void addClickedEvent();

private:
    void initUI();
    void setDatelayout();
    void setWeatherLayout();
    void setSleeptime(bool Isshow);
    void setCenterWidget();
    void initDynamic();
    void updateDate();
    void setUpdateCenterWidget();
    void startSwitchImages();
    void stopSwitchImages();
    void setRandomText();
    void setPreviewText(bool bVisible); //设置预览样式
    void setRandomPos();
    void getConfiguration();
    void connectSingles();
    void updateBackgroundPath();
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void isMovie();
    void getVideoFormat(QString fileName); //获取视频格式（视频格式、编码格式、分辨率）
    QString getLongFormatDate(int type);
    /**
     * @brief 获取动态屏保视频是否存在
     * @param
     * @return
     */
    void getVideoExistence();
    bool getIsValidUrl(const QString &urlStr); //网页是否合法
    void initUrlWidget();
    void initDynamicSaver(QString path);
    void initOtherWidget();

    // 缩略选择窗口
    void initThumBnalListWidget();

    // 天气
    void initWeatherWidget();

    // 音乐播放器
    void initMediaControlWidget();

    // 相册
    void initCustomAlbumViewer();

    QTimer *switchTimer;
    QTimer *fadeTimer;
    QStringList imagePaths;
    QString backgroundPath;
    int cycleTime;
    float opacity;
    bool isCustom;
    bool isCShowRestTime;
    bool isUShowRestTime;
    bool textIsCenter;
    QString myText;
    QLabel *myTextLabel;
    QLabel *myPreviewLabel;
    QLabel *cycleLabel;

    SCConfiguration *configuration;

    QWidget *myTextWidget;
    QLabel *dateOfLocaltime;

    QLabel *dateOfWeek;
    QLabel *dateOfLocaltimeHour;
    QLabel *dateofLocaltimeColon;
    QLabel *dateOfLocaltimeMinute;
    QLabel *dateOfDay;
    QLabel *dateOfLunar;
    QWidget *m_centerWidget;

    QWidget *timeLayout = nullptr;
    QWidget *m_widgetDatetime = nullptr;
    SleepTime *sleepTime;
    QTimer *timer;

    QPixmap background;
    QPixmap m_scaleBackground;

    QPushButton *settingsButton;
    QPushButton *WallpaperButton;
    QWidget *buttonWidget;
    QSettings *qsettings;

    MBackground *m_background;

    QLabel *centerlabel1 = nullptr;
    QLabel *centerlabel2 = nullptr;
    QLabel *authorlabel = nullptr;

    bool isAutoSwitch;

    int flag;
    bool hasChanged;
    int timeType;
    QString dateType;
    QProcess *process;
    QLabel *screenLabel;
    bool respondClick;
    static QTime m_currentTime;
    int blur_Num;
    QString curStyle;
    double curFontSize;
    double m_ptToPx = 1.0;
    QFont sysFont;

    WeatherManager *m_weatherManager;
    QWidget *m_weatherLaout;
    QLabel *m_weatherIcon;
    QLabel *m_weatherArea;
    QLabel *m_weatherCond;
    QLabel *m_weatherTemperature;

    QWidget *m_widgetNotice;
    QLabel *m_labelNoticeIcon;
    QLabel *m_labelNoticeMessage;
    int currentIndex = 0;

    QString currentPath;
    bool is_gif = false;
    QList<QPixmap> currentPixmap;
    int delayTime;
    QTimer *movieTimer = nullptr;
    int currentCount = 0;
    QDateTime m_lastDateTime;
    bool isScreensaver = false;

    bool m_isShowDateTime = true; /** 是否显示日期时间 */
    bool m_isInitUICompleted = false;

    //动态屏保
    bool m_isDynamicSaver = false;
    KyVideoPlayer *m_pVideoPlayer = nullptr;
    QGraphicsScene *m_playerScene = nullptr;
    GraphicsView *m_playerView = nullptr;
    QGraphicsVideoItem *m_playerItem = nullptr;
    int video_Duration = 0;
    QString cur_video_format;
    QString video_Codetype;
    int cur_video_Width;
    int cur_video_Height;
    QString m_videoPath;
    QString m_videoFormat;
    int m_videoSize;
    int m_videoWidth;
    int m_videoHeight;
    int m_saverState = SAVERERRORTYPE::NORMALSAVER;
    QString m_sPlayerShowErrorTip;
    QString m_strPreViewTrans;

    //网页显示
    QWebView *m_pWebView = nullptr;
    QWidget *m_pCenterUrlWidget = nullptr;
    QLabel *m_pErrorUrlLabel = nullptr;
    bool m_isUrlSaver = false;

    // 智慧屏保
    int m_nSubMode = -1;
    int m_nWeatherPosition = 0;
    bool m_bFollowSystem = false;
    bool m_bSaverMusic = false;
    QString m_sMusicPath;
    int m_nPhotoCycleMode = -1;
    int m_nPhotoCycleInterval = -1;
    QStringList m_sDefaultPhotoPath;
    QStringList m_sCustomPhptoPath;
    QStringList m_sAllPhotoPath;
    QString m_sSelectPhotoPath;
    int m_iPhotoSwitchMode = 0;
    int m_iPhotoSwitchTime = 0;

    WeatherWidget *m_weatherWidget = nullptr;

    QWidget *m_widgetMediaControl = nullptr;
    MediaSaverWidget *m_mediaControlWidget = nullptr;

    CustomAlbumViewer *m_customAlbumViewer = nullptr;

    QLabel *m_smartSaverLabel = nullptr;
    QListWidget *m_thumBnalListWidget = nullptr;
    MusicThumbnailWidget *m_musicWidget = nullptr;
    AlbumThumbnailWidget *m_albunWidget = nullptr;
    SaverThumbnailWidget *m_defaulSaverWidget = nullptr;
    WeatherThumbnailWidget *m_weatherThumbnailWidget = nullptr;
    StatusButton *m_sidebarBtn = nullptr;
    QTimer *m_hideThumBnalWidgetTimer = nullptr;

    bool m_bImageLoadFinished = false;

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

private Q_SLOTS:
    void updateTime();
    void onIsCustomModeChanged(bool custom);
    void autoSwitchChanged(bool iswitch);
    void backgroundPathChanged(QString path);
    void backgroundChanged(QString path);
    void cycleTimeChanged(int cTime);
    void myTextChanged(QString text);
    void showCRestTimeChanged(bool isShow);
    void showURestTimeChanged(bool isShow);
    void textIsCenterChanged(bool isCenter);
    void getWeatherFinish(QString city, QString cond, QString tmp);
    void onTimeFormatChanged(int type);
    void onDateFormatChanged(QString type);
    void onBlurNumChanged(int num);
    void onStyleChanged(QString style);
    void onFontSizeChanged(int fontSize);
    QPixmap getPaddingPixmap();
    void onScreensaverDatetimeEnableChanged(bool isEnable, const QString &strUserName);
    void onSubModeChanged(int mode);
    void onMusicFollowSystemChanged(bool b);
    void onWeatherPositionChanged(int position);
    void onMusicPathChanged(QString path);
    void onPhotoCycleModeChanged(int mode);
    void onPhotoCycleIntervalChanged(int interval);
    void onPhotoPathChanged(QString path);
    void onPhotoSelectPathChanged(QString path);
};

#endif // MAINWINDOW_H
