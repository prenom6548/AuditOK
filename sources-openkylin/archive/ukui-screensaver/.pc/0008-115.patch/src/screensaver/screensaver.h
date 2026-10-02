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

#include "sleeptime.h"
#include "chinesedate.h"
#include "mbackground.h"
#include "checkbutton.h"
#include "scconfiguration.h"
#include "cyclelabel.h"
#include "videoplayer.h"
#include "graphicsview.h"

class WeatherManager;

enum dateinfo {
    DATE,
    TIME,
};

class Screensaver : public QWidget
{
    Q_OBJECT

public:
    explicit Screensaver(bool isScreensaver, QWidget *parent = 0);
    ~Screensaver();
    void addClickedEvent();

private:
    void initUI();
    void setDatelayout();
    void setWeatherLayout();
    void setSleeptime(bool Isshow);
    void setCenterWidget();
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
    QPixmap loadFromFile(QString strPath);
    void getVideoFormat(QString fileName); //获取视频格式（视频格式、编码格式、分辨率）
    QString getLongFormatDate(int type);
    /**
     * @brief 获取动态屏保视频是否存在
     * @param
     * @return
     */
    void getVideoExistence();

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
    QWidget *centerWidget;

    QWidget *timeLayout;
    SleepTime *sleepTime;
    QTimer *timer;

    QPixmap background;

    QPushButton *settingsButton;
    QPushButton *WallpaperButton;
    QWidget *buttonWidget;
    QSettings *qsettings;

    MBackground *m_background;

    QLabel *centerlabel1;
    QLabel *centerlabel2;
    QLabel *authorlabel;

    checkButton *checkSwitch;
    QLabel *autoSwitchLabel;
    QFrame *autoSwitch;

    QFrame *vboxFrame;
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

    //动态屏保
    bool m_isDynamicSaver = false;
    VideoPlayer *m_pVideoPlayer;
    QGraphicsScene *m_playerScene;
    GraphicsView *m_playerView;
    QGraphicsVideoItem *m_playerItem;
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
    QString m_strPreViewTrans;

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

private Q_SLOTS:
    void updateTime();
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
};

#endif // MAINWINDOW_H
