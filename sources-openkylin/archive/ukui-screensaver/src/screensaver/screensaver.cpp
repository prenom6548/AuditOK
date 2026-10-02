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

#include <QGuiApplication>
#include <QScreen>
#include <QWidget>
#include <QPalette>
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QDate>
#include <QDate>
#include <QApplication>
#include <QTextCodec>
#include <QProcess>
#include <QDateTime>
#include <QKeyEvent>
#include <QSplitterHandle>
#include <QCursor>
#include <QColor>
#include <QPalette>
#include <QDateTime>
#include <QLayout>
#include <QStringList>
#include <QVBoxLayout>
#include <QDir>
#include <QMovie>
#include <QByteArray>
#include <QImageReader>
#include <QHBoxLayout>
#include <QX11Info>
#include <QDBusInterface>
#include <QDBusReply>
#include <QApplication>
#include "screensaver.h"
#include "weathermanager.h"
#include "commonfunc.h"
#include <QSet>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "utils.h"

extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

#define WORKING_DIRECTORY "/usr/share/ukui-screensaver"
#define TIME_FONT_SIZE 72
#define TIME_DATE_SIZE 26
#define CENTER1_LABEL_SIZE 36
#define CENTER2_LABEL_SIZE 28

QTime Screensaver::m_currentTime = QTime::currentTime();

Screensaver::Screensaver(bool isscreensaver, QPixmap *originalBackground, QWidget *parent)
    : isScreensaver(isscreensaver)
    , QWidget(parent)
    , switchTimer(nullptr)
    , backgroundPath("")
    , cycleTime(300)
    , isCustom(false)
    , isCShowRestTime(true)
    , isUShowRestTime(true)
    , myTextLabel(nullptr)
    , myPreviewLabel(nullptr)
    , // 预览label标签
    configuration(SCConfiguration::instance())
    , myTextWidget(nullptr)
    , m_centerWidget(nullptr)
    , sleepTime(nullptr)
    , timer(nullptr)
    , background("")
    , isAutoSwitch(false)
    , flag(0)
    , hasChanged(false)
    , process(nullptr)
    , screenLabel(nullptr)
    , respondClick(false)
//    , m_weatherManager(new WeatherManager)
{
    // 加载翻译文件
    QString locale = QLocale::system().name();
    if (locale.contains(":")) {
        locale = locale.split(":").at(0);
    }
    QTranslator translator;
    QString qmFile = QString(WORKING_DIRECTORY "/i18n_qm/%1.qm").arg(locale);
    translator.load(qmFile);
    qApp->installTranslator(&translator);
    qDebug() << "load translation file " << qmFile;

    installEventFilter(this);
    //  setWindowFlags(Qt::X11BypassWindowManagerHint);
    setUpdateCenterWidget();
    setMouseTracking(true);
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));

    getConfiguration();

    m_background = new MBackground();
    QString backgroundFile;
    backgroundFile
        = (isScreensaver ? configuration->getDefaultBackground()
                         : mapToThumbnailPath(configuration->getDefaultBackground()));
    if (m_nSubMode == 0 || m_nSubMode < 0) {
        if (isScreensaver && originalBackground && !originalBackground->isNull()) {
            background = *originalBackground;
            m_scaleBackground = *originalBackground;
        } else {
            background = isScreensaver ? scaleWallpaperFillFromFile(width(), height(), backgroundFile)
                                       : loadFromFile(backgroundFile);
        }
    } else {
        /* 子模式 1/2/3 须高斯模糊；originalBackground 为锁屏清晰壁纸，不可直接用作背景 */
        background = scaleBlurPixmap(width(), height(), backgroundFile);
        m_scaleBackground = scaleBlurPixmap(width(), height(), backgroundFile);
    }

    initUI();

    if (m_nSubMode == 3 && m_customAlbumViewer) {
        m_customAlbumViewer->setDefaultPixmap(loadFromFile(backgroundFile));
    }

    QList<QLabel *> labelList = this->findChildren<QLabel *>();
    for (int i = 0; i < labelList.count(); i++) {
        labelList.at(i)->setAlignment(Qt::AlignCenter);
    }
#ifndef USE_INTEL
    if (m_nSubMode == 0 || m_nSubMode < 0 && isCustom) {
        updateBackgroundPath();
        startSwitchImages();
    }

    connectSingles();
#endif

    screenLabel = new QLabel(this);
    screenLabel->setObjectName("screenLabel");
    switch (m_saverState) {
    case SAVERERRORTYPE::VIDEOFORMATEERROR :
        screenLabel->setText(tr("The dynamic screensavers video file is corrupted"));
        break;
    case SAVERERRORTYPE::VIDEOTOOLARGE :
        screenLabel->setText(tr("The dynamic screensavers video file is too large"));
        break;
    case SAVERERRORTYPE::RESOLUTIONERROR :
        screenLabel->setText(tr("The dynamic screensavers video file resolution is too high"));
        break;
    case SAVERERRORTYPE::URLADDRESSERROR :
        screenLabel->setText(tr("Invalid URL"));
        break;
    default:
        screenLabel->setText(tr("Picture does not exist"));
        break;
    }
    screenLabel->adjustSize();
    screenLabel->hide();
    m_strPreViewTrans = tr("View");
    m_isInitUICompleted = true;
    qDebug() << "preview translation:" << m_strPreViewTrans;
}

Screensaver::~Screensaver()
{
    playController::getInstance().stop();
}

void Screensaver::getConfiguration()
{
    isCustom = configuration->getIsCustom();
    if (isCustom) {
        cycleTime = configuration->getCycleTime();
        isAutoSwitch = configuration->getAutoSwitch();
        backgroundPath = configuration->getBackgroundPath();
        isCShowRestTime = configuration->getCShowRestTime();
        textIsCenter = configuration->getTextIsCenter();
        myText = configuration->getMyText();
    } else
        isUShowRestTime = configuration->getUShowRestTime();
    if (isScreensaver) {
        curFontSize = configuration->getFontSize();
    } else {
        curFontSize = 0.0f;
    }
    m_ptToPx = configuration->getPtToPx();
    m_isShowDateTime = configuration->getSaverDatetimeEnable();

    m_videoPath = configuration->getVideoPath();
    m_videoFormat = configuration->getVideoFormat();
    m_videoSize = configuration->getVideoSize();
    m_videoWidth = configuration->getVideoWidth();
    m_videoHeight = configuration->getVideoHeight();

    m_nSubMode = configuration->getSubMode();
    m_nWeatherPosition = configuration->getWeatherPosition();
    m_bFollowSystem = configuration->getMusicFollowSystem();
    m_sMusicPath = configuration->getMusicPath();
    m_nPhotoCycleMode = configuration->getPhotoCycleMode();
    m_nPhotoCycleInterval = configuration->getPhotoCycleInterval();
    m_sDefaultPhotoPath = configuration->getDefaultPhotoPath();
    m_sCustomPhptoPath = configuration->getCustomPhotoPath();
    QSet<QString> set;
    for (const QString &str : m_sCustomPhptoPath) {
        set.insert(str);
    }
    for (const QString &str : m_sDefaultPhotoPath) {
        set.insert(str);
    }
    m_sAllPhotoPath = set.values();
    qDebug() << "allList = " << m_sAllPhotoPath;
    m_iPhotoSwitchMode = configuration->getPhotoCycleMode();
    m_iPhotoSwitchTime = configuration->getPhotoCycleInterval() * 1000;
    m_sSelectPhotoPath = configuration->getSelectPhotoPath();
}

void Screensaver::connectSingles()
{
    connect(configuration, &SCConfiguration::isCustomModeChanged, this, &Screensaver::onIsCustomModeChanged);
    connect(configuration, &SCConfiguration::autoSwitchChanged, this, &Screensaver::autoSwitchChanged);
    connect(configuration, &SCConfiguration::backgroundPathChanged, this, &Screensaver::backgroundPathChanged);
    connect(configuration, &SCConfiguration::backgroundChanged, this, &Screensaver::backgroundChanged);
    connect(configuration, &SCConfiguration::cycleTimeChanged, this, &Screensaver::cycleTimeChanged);
    connect(configuration, &SCConfiguration::myTextChanged, this, &Screensaver::myTextChanged);
    connect(configuration, &SCConfiguration::showCRestTimeChanged, this, &Screensaver::showCRestTimeChanged);
    connect(configuration, &SCConfiguration::showURestTimeChanged, this, &Screensaver::showURestTimeChanged);
    connect(configuration, &SCConfiguration::textIsCenterChanged, this, &Screensaver::textIsCenterChanged);
    connect(configuration, &SCConfiguration::timeTypeChanged, this, &Screensaver::onTimeFormatChanged);
    connect(configuration, &SCConfiguration::dateTypeChanged, this, &Screensaver::onDateFormatChanged);
    connect(configuration, &SCConfiguration::blurChanged, this, &Screensaver::onBlurNumChanged);
    connect(configuration, &SCConfiguration::styleChanged, this, &Screensaver::onStyleChanged);
    connect(configuration, &SCConfiguration::subModeChanged, this, &Screensaver::onSubModeChanged);
    connect(configuration, &SCConfiguration::photoSelectPathChanged, this, &Screensaver::onPhotoSelectPathChanged);
    connect(configuration, &SCConfiguration::musicFollowSystemChanged, this, &Screensaver::onMusicFollowSystemChanged);
    connect(configuration, &SCConfiguration::musicPathChanged, this, &Screensaver::onMusicPathChanged);
    connect(
        configuration, &SCConfiguration::photoCycleIntervalChanged, this, &Screensaver::onPhotoCycleIntervalChanged);
    connect(configuration, &SCConfiguration::photoCycleModeChanged, this, &Screensaver::onPhotoCycleModeChanged);
    connect(
        configuration,
        &SCConfiguration::saverDatetimeEnableChanged,
        this,
        &Screensaver::onScreensaverDatetimeEnableChanged);
}

void Screensaver::onBlurNumChanged(int num)
{
    if (m_isDynamicSaver || m_isUrlSaver)
        return;
    blur_Num = num;
    if (curStyle == "ukui-dark" || curStyle == "ukui-black") {
        myTextLabel->setStyleSheet(
            QString("QLabel{background: rgba(0, 0, 0, %1); color:#FFFFFF; border-radius:16px}").arg(blur_Num * 0.01));
        cycleLabel->setStyleSheet(
            QString("QLabel{background: rgba(0, 0, 0, %1); color:#FFFFFF; border-radius:8px}").arg(blur_Num * 0.01));
    } else {
        myTextLabel->setStyleSheet(
            QString("QLabel{background: rgba(255, 255, 255, %1); border-radius:16px}").arg(blur_Num * 0.01));
        cycleLabel->setStyleSheet(
            QString("QLabel{background: rgba(255, 255, 255, %1); border-radius:8px}").arg(blur_Num * 0.01));
    }
}

void Screensaver::onStyleChanged(QString style)
{
    if (m_isDynamicSaver || m_isUrlSaver)
        return;

    curStyle = style;
    if (curStyle == "ukui-dark" || curStyle == "ukui-black") {
        myTextLabel->setStyleSheet(
            QString("QLabel{background: rgba(0, 0, 0, %1); color:#FFFFFF; border-radius:16px}").arg(blur_Num * 0.01));
        cycleLabel->setStyleSheet(
            QString("QLabel{background: rgba(0, 0, 0, %1); color:#FFFFFF; border-radius:8px}").arg(blur_Num * 0.01));
    } else {
        myTextLabel->setStyleSheet(
            QString("QLabel{background: rgba(255, 255, 255, %1); border-radius:16px}").arg(blur_Num * 0.01));
        cycleLabel->setStyleSheet(
            QString("QLabel{background: rgba(255, 255, 255, %1); border-radius:8px}").arg(blur_Num * 0.01));
    }
}

void Screensaver::onSubModeChanged(int mode)
{
    if (m_nSubMode == mode) {
        return;
    }
    m_nSubMode = mode;

    QFileInfo fileInfo(backgroundPath);
    bool isUrlValid = false;
    isUrlValid = getIsValidUrl(backgroundPath);
    if (isCustom) {
        if (fileInfo.isFile()) {
            initDynamicSaver(backgroundPath);
            if (m_isDynamicSaver) {
                m_isUrlSaver = false;
            }
        } else if (isUrlValid) {
            initUrlWidget();
            m_isUrlSaver = true;
            m_isDynamicSaver = false;
        } else {
            m_isDynamicSaver = false;
            m_isUrlSaver = false;
        }
    }
    initOtherWidget();

    if (m_isDynamicSaver && m_nSubMode == 0 && m_playerView) {
        if (m_smartSaverLabel) {
            m_smartSaverLabel->hide();
        }
        if (m_thumBnalListWidget) {
            m_thumBnalListWidget->hide();
        }
        if (m_sidebarBtn) {
            m_sidebarBtn->hide();
        }
        m_widgetDatetime->hide();
        qDebug() << "player raise show";
        if (m_playerView) {
            qDebug() << "player raise show111";
            m_playerView->raise();
            m_playerView->show();
        }
        QCoreApplication::postEvent(this, new QEvent(QEvent::User));
        update();
        return;
    } else if (m_isUrlSaver && m_nSubMode == 0) {
        if (m_pWebView) {
            if (m_smartSaverLabel) {
                m_smartSaverLabel->hide();
            }
            if (m_thumBnalListWidget) {
                m_thumBnalListWidget->hide();
            }
            if (m_sidebarBtn) {
                m_sidebarBtn->hide();
            }
            m_widgetDatetime->hide();
            qDebug() << "player raise show";
            if (m_pWebView) {
                qDebug() << "player raise show111";
                m_pWebView->raise();
                m_pWebView->show();
            }
            QCoreApplication::postEvent(this, new QEvent(QEvent::User));
            update();
            return;
        }
    }

    QString backgroundFile;
    backgroundFile
        = (isScreensaver ? configuration->getDefaultBackground()
                         : mapToThumbnailPath(configuration->getDefaultBackground()));
    if (m_nSubMode == 0 || m_nSubMode < 0) {
        if (isCustom) {
            updateBackgroundPath();
            startSwitchImages();
        } else {
            background = isScreensaver ? scaleWallpaperFillFromFile(width(), height(), backgroundFile)
                                       : loadFromFile(backgroundFile);
        }
        showCRestTimeChanged(isCShowRestTime);
        showURestTimeChanged(isUShowRestTime);
        if (!isCustom) {
            if (m_centerWidget) {
                m_centerWidget->show();
            }
        } else {
            textIsCenterChanged(textIsCenter);
        }
    } else {
        if (m_centerWidget) {
            m_centerWidget->hide();
        }
        background = scaleBlurPixmap(width(), height(), backgroundFile);
    }
    if (m_nSubMode == 1) {
        initWeatherWidget();
        if (myTextWidget)
            myTextWidget->hide();
        if (m_playerView)
            m_playerView->hide();
        if (m_centerWidget)
            m_centerWidget->hide();
    }
    if (m_nSubMode == 2) {
        initMediaControlWidget();
        if (myTextWidget)
            myTextWidget->hide();
        if (m_playerView)
            m_playerView->hide();
        if (m_centerWidget)
            m_centerWidget->hide();
    } else if (m_nSubMode == 3) {
        initCustomAlbumViewer();
        m_customAlbumViewer->setDefaultPixmap(loadFromFile(backgroundFile));
        if (myTextWidget)
            myTextWidget->hide();
        if (m_playerView)
            m_playerView->hide();
        if (m_centerWidget)
            m_centerWidget->hide();
    }
    QCoreApplication::postEvent(this, new QEvent(QEvent::User));
    update();
}

void Screensaver::onMusicFollowSystemChanged(bool b)
{
    if (m_bFollowSystem != b && m_widgetMediaControl) {
        if (b) {
            m_mediaControlWidget->setSaverMediaType(!b);
        } else {
            QStringList musicPath = getAudioFiles(m_sMusicPath);
            if (musicPath.isEmpty()) {
                m_mediaControlWidget->setSaverMediaType(b);
            } else {
                m_mediaControlWidget->setSaverMediaType(!b);
                playController::getInstance().addMusicToDatebase(musicPath);
                playController::getInstance().setCurPlaylist(musicPath);
            }
        }
    }
    m_bFollowSystem = b;
}

void Screensaver::onWeatherPositionChanged(int position) {}

void Screensaver::onMusicPathChanged(QString path)
{
    m_sMusicPath = path;
    if (m_bFollowSystem) {
        m_mediaControlWidget->setSaverMediaType(!m_bFollowSystem);
    } else {
        QStringList musicPath = getAudioFiles(m_sMusicPath);
        if (musicPath.isEmpty()) {
            m_mediaControlWidget->setSaverMediaType(m_bFollowSystem);
        } else {
            m_mediaControlWidget->setSaverMediaType(!m_bFollowSystem);
            playController::getInstance().addMusicToDatebase(musicPath);
            playController::getInstance().setCurPlaylist(musicPath);
        }
    }
}

void Screensaver::onPhotoCycleIntervalChanged(int interval)
{
    if (m_iPhotoSwitchTime != (interval * 1000)) {
        m_iPhotoSwitchTime = interval * 1000;
        m_customAlbumViewer->setUpdateInterval(m_iPhotoSwitchTime);
    }
}

void Screensaver::onPhotoCycleModeChanged(int mode)
{
    qDebug() << "mode = " << mode;
    if (m_iPhotoSwitchMode != mode) {
        m_iPhotoSwitchMode = mode;
        m_customAlbumViewer->setSequentialMode((m_iPhotoSwitchMode == 0) ? true : false);
    }
}

void Screensaver::onPhotoPathChanged(QString path) {}

void Screensaver::onPhotoSelectPathChanged(QString path)
{
    m_sSelectPhotoPath = path;
    if (m_albunWidget && m_sSelectPhotoPath == DEFAULT_PHOTO_PATH) {
        m_albunWidget->setDefaultPath(DEFAULT_PHOTO_PATH);
    } else if (m_albunWidget) {
        m_albunWidget->setDefaultPath(m_sSelectPhotoPath);
    }
    if (!isScreensaver && m_sSelectPhotoPath == DEFAULT_PHOTO_PATH) {
        m_sSelectPhotoPath = LOCAL_WALLPAPER_PATH;
    }
    m_customAlbumViewer->setImagesPath(m_sSelectPhotoPath);
    m_customAlbumViewer->updateViewerSize();
}

void Screensaver::onTimeFormatChanged(int type)
{
    timeType = type;
}

void Screensaver::onDateFormatChanged(QString type)
{
    dateType = type;
}

void Screensaver::onFontSizeChanged(int fontSize)
{
    curFontSize = fontSize;
}

void Screensaver::onIsCustomModeChanged(bool custom)
{
    m_bImageLoadFinished = false;
    isCustom = custom;
    if (isCustom) {
        cycleTime = configuration->getCycleTime();
        isAutoSwitch = configuration->getAutoSwitch();
        backgroundPath = configuration->getBackgroundPath();
        isCShowRestTime = configuration->getCShowRestTime();
        textIsCenter = configuration->getTextIsCenter();
        myText = configuration->getMyText();
    } else
        isUShowRestTime = configuration->getUShowRestTime();
}

void Screensaver::autoSwitchChanged(bool isSwitch)
{
    if (!isCustom || m_isDynamicSaver || m_isUrlSaver)
        return;
    isAutoSwitch = isSwitch;
    if (!isSwitch) {
        stopSwitchImages();
    }
    startSwitchImages();
}

/*
 * 图片路径改变
 */
void Screensaver::backgroundPathChanged(QString path)
{
    backgroundPath  = path;
    if(!isCustom)
        return ;

    QFileInfo fileInfo(backgroundPath);
    bool isUrlValid = false;
    isUrlValid = getIsValidUrl(backgroundPath);
    if (fileInfo.isFile()) {
        initDynamicSaver(backgroundPath);
        switch (m_saverState) {
        case SAVERERRORTYPE::VIDEOFORMATEERROR :
            screenLabel->setText(tr("The dynamic screensavers video file is corrupted"));
            break;
        case SAVERERRORTYPE::VIDEOTOOLARGE :
            screenLabel->setText(tr("The dynamic screensavers video file is too large"));
            break;
        case SAVERERRORTYPE::RESOLUTIONERROR :
            screenLabel->setText(tr("The dynamic screensavers video file resolution is too high"));
            break;
        default:
            screenLabel->setText(tr("Picture does not exist"));
            break;
        }

        if (m_isDynamicSaver) {
            m_isUrlSaver = false;
        }
    } else if (isUrlValid) {
        m_saverState = SAVERERRORTYPE::NORMALSAVER;
        initUrlWidget();
        m_isUrlSaver = true;
        m_isDynamicSaver = false;
    } else if (m_saverState == SAVERERRORTYPE::URLADDRESSERROR) {
        screenLabel->setText(tr("Invalid URL"));
    } else {
        m_saverState = SAVERERRORTYPE::NORMALSAVER;
        m_isDynamicSaver = false;
        m_isUrlSaver = false;
    }
    if  (!m_isDynamicSaver && !m_isUrlSaver) {
        updateBackgroundPath();//更新图片路径
        stopSwitchImages();
        startSwitchImages();
    }
    if (isCustom && imagePaths.count() == 0 && m_saverState == SAVERERRORTYPE::NORMALSAVER) {
        screenLabel->setText(tr("Picture does not exist"));
    }
    screenLabel->adjustSize();
    if (isCustom)
        initOtherWidget();
    QCoreApplication::postEvent(this, new QEvent(QEvent::User));
    update();
}

void Screensaver::backgroundChanged(QString path)
{
    if (!isScreensaver) {
        path = mapToThumbnailPath(path);
    }
    if (m_nSubMode == 0 || m_nSubMode < 0) {
        background = isScreensaver ? scaleWallpaperFillFromFile(width(), height(), path) : loadFromFile(path);
    } else {
        background = scaleBlurPixmap(width(), height(), path);
    }
    if (m_nSubMode == 3 && m_customAlbumViewer) {
        m_customAlbumViewer->setDefaultPixmap(loadFromFile(path));
    }
}

void Screensaver::cycleTimeChanged(int cTime)
{
    cycleTime = cTime;
    if (!isCustom || m_isDynamicSaver || m_isUrlSaver)
        return;
    stopSwitchImages();
    startSwitchImages();
}

void Screensaver::myTextChanged(QString text)
{
    if (!isCustom || m_isDynamicSaver || m_isUrlSaver)
        return;

    myText = text;

    if (textIsCenter && m_centerWidget) {
        if (centerlabel1)
            centerlabel1->setText(myText);
        if (centerlabel2) {
            centerlabel2->setText("");
            centerlabel2->hide();
        }
        if (authorlabel) {
            authorlabel->setText("");
            authorlabel->hide();
        }
        m_centerWidget->adjustSize();

        m_centerWidget->setGeometry(
            (width() - m_centerWidget->width()) / 2,
            (height() - m_centerWidget->height()) / 2,
            m_centerWidget->width(),
            m_centerWidget->height());

        if ((height() - m_centerWidget->height()) / 2 < timeLayout->y() + timeLayout->height())
            m_centerWidget->setGeometry(
                (width() - m_centerWidget->width()) / 2,
                timeLayout->y() + timeLayout->height(),
                m_centerWidget->width(),
                m_centerWidget->height());
    } else {
        setRandomText();
        setRandomPos();
        if (m_centerWidget)
            m_centerWidget->hide();
    }
    if (m_defaulSaverWidget) {
        m_defaulSaverWidget->setDictumText(centerlabel1->text(), centerlabel2->text(), authorlabel->text());
    }
}

void Screensaver::showCRestTimeChanged(bool isShow)
{
    isCShowRestTime = isShow;
    if (!isCustom || m_isDynamicSaver || m_isUrlSaver)
        return;

    setSleeptime(isCShowRestTime);
}

void Screensaver::showURestTimeChanged(bool isShow)
{
    isUShowRestTime = isShow;
    if (isCustom || m_isDynamicSaver || m_isUrlSaver)
        return;

    setSleeptime(isUShowRestTime);
}

void Screensaver::textIsCenterChanged(bool isCenter)
{
    textIsCenter = isCenter;
    if (!isCustom || m_isDynamicSaver || m_isUrlSaver)
        return;

    if (isCenter) {
        if (myTextWidget)
            myTextWidget->hide();
        if (!m_centerWidget) {
            setCenterWidget();
            resize(width(), height());
        } else {
            m_centerWidget->show();
            myTextChanged(myText);
        }
    } else {
        if (m_centerWidget)
            m_centerWidget->hide();
        setRandomText();
        setRandomPos();
    }
}

bool Screensaver::eventFilter(QObject *obj, QEvent *event)
{
#ifndef USE_INTEL
    if (obj == this) {
        if (event->type() == QEvent::MouseButtonPress) {
            if (!isScreensaver) {
                if (!process) {
                    process = new QProcess(this);
                }
                process->start("ukui-screensaver-command -s");
            }
            if (obj == m_thumBnalListWidget) {
                return true;
            } else {
                return QWidget::eventFilter(obj, event);
            }
        } else if (event->type() == QEvent::User) {
            QResizeEvent fakeResizeEvent(size(), size());
            resizeEvent(&fakeResizeEvent);
        }
    }
    if (obj == m_thumBnalListWidget) {
        if (event->type() == QEvent::Leave) {
            m_hideThumBnalWidgetTimer->start(2000);
        } else if (event->type() == QEvent::Enter) {
            m_hideThumBnalWidgetTimer->stop();
        }
    }
    if (obj == m_sidebarBtn && event->type() == QEvent::ToolTip) {
        if (m_thumBnalListWidget && m_thumBnalListWidget->isHidden()) {
            m_thumBnalListWidget->show();
            m_smartSaverLabel->show();
            m_sidebarBtn->move(
                m_thumBnalListWidget->x() + m_thumBnalListWidget->width(), (height() - m_sidebarBtn->height()) / 2);
            m_sidebarBtn->setIcon(
                drawSymbolicColoredPixmap(QIcon::fromTheme("pan-start-symbolic").pixmap(44, 44), "white"));
        }
    }
#endif
    return QWidget::eventFilter(obj, event);
}

void Screensaver::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    /*时间未同步的时候重新刷新一次,主要是避免睡眠唤醒时的时间跳变*/
    /*    if (m_currentTime.hour() != QTime::currentTime().hour() ||
                m_currentTime.minute() != QTime::currentTime().minute()) {
            updateTime();
        }
    */
    if (m_isDynamicSaver || m_isUrlSaver) {
        return ;
    } else if (m_bImageLoadFinished && m_nSubMode == 0 && isCustom && imagePaths.count() == 0 || m_saverState != SAVERERRORTYPE::NORMALSAVER) {
        painter.setBrush(Qt::gray);
        painter.drawRect(-1, -1, this->width() + 1, this->height() + 1);
        if(screenLabel->isHidden()) {
            screenLabel->show();
        }
    } else {
        if (background.isNull()) {
            QColor cor = "#035290";
            painter.setBrush(cor);
            painter.drawRect(-1, -1, this->width() + 1, this->height() + 1);
        } else {
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            if (!isScreensaver) {
                QPainterPath backgroundPath;
                backgroundPath.addRoundedRect(this->rect(), 10, 10);
                painter.setClipPath(backgroundPath);
            }
            if (m_nSubMode == 0 || m_nSubMode < 0) {
                if (isScreensaver) {
                    drawPixmapWallpaperFill(painter, background, rect());
                } else {
                    painter.drawPixmap(this->rect(), background, background.rect());
                }
            } else {
                painter.drawPixmap(this->rect(), background, background.rect());
            }
            QPainterPath path;
            QPainter painter1(this);
            painter1.setOpacity(0.25);
            painter1.setRenderHint(QPainter::Antialiasing); // 反锯齿;
            painter1.setClipping(true);
            painter1.setPen(Qt::transparent);
            if (isScreensaver) {
                path.addRect(this->rect());
            } else {
                path.addRoundedRect(this->rect(), 10, 10);
            }
            path.setFillRule(Qt::WindingFill);
            painter1.setBrush(QColor("#000000"));
            painter1.setPen(Qt::transparent);
            painter1.drawPath(path);
            if (m_nSubMode != 0) {
                screenLabel->hide();
            }
        }
    }
    /*这里是为了不显示笔的线条*/
    // painter.drawRect(-1,-1,this->width()+1,this->height()+1);
}

void Screensaver::isMovie()
{
    if (!currentPixmap.isEmpty()) {
        currentPixmap.clear();
        currentCount = 0;
        if (movieTimer) {
            movieTimer->stop();
            movieTimer->deleteLater();
            movieTimer = nullptr;
        }
    }
    if (!is_gif) {
        repaint();
        return;
    }
    QMovie *movie = new QMovie(currentPath);
    movie->setScaledSize(QSize(this->width(), this->height()));
    for (int i = 0; i < movie->frameCount(); ++i) {
        movie->jumpToFrame(i);
        currentPixmap.append(movie->currentPixmap());
        if (i == 0)
            delayTime = movie->nextFrameDelay();
    }
    if (!movieTimer) {
        movieTimer = new QTimer(this);
        connect(movieTimer, &QTimer::timeout, this, [&] {
            if (currentCount == currentPixmap.count() - 1)
                currentCount = 0;
            else
                currentCount += 1;
            background = currentPixmap.at(currentCount);
            repaint();
        });
    }
    if (delayTime < 50)
        movieTimer->start(50);
    else
        movieTimer->start(delayTime);
}

void Screensaver::getVideoExistence()
{
    qDebug() << "videoPath = " << m_videoPath << "videoSize = " << m_videoSize << "videoFormat = " << m_videoFormat;
    if (m_nSubMode == 0) {
        QFileInfo fileInfo(m_videoPath);
        // 视频文件是否存在
        if (fileInfo.isFile()) {
            // 视频文件大小限制
            if (fileInfo.size() <= m_videoSize * 1024 * 1024) {
                getVideoFormat(m_videoPath);
                QStringList formatList = m_videoFormat.split(',');
                // 视频格式限制
                bool m_rightFormat = false;
                for (int formatAt = 0; formatAt < formatList.count(); formatAt++) {
                    if (cur_video_format.contains(formatList.at(formatAt))) {
                        m_rightFormat = true;
                        break;
                    }
                }
                QString suffix;
                QStringList sourcePathList = m_videoPath.split(".");
                if (sourcePathList.size() >= 2) {
                    suffix = sourcePathList.at(sourcePathList.size() - 1);
                    if (!cur_video_format.contains(suffix) && m_rightFormat) { //视频尾缀不包含在可用动态屏保格式内，但是解析出来的编码格式为可用，将该尾缀设为可用，bug#492999
                        cur_video_format.append(",").append(suffix);
                        configuration->setVideoFormat(cur_video_format);
                    }
                }
                if (m_rightFormat) {
                    // 分辨率限制
                    if (cur_video_Width <= m_videoWidth && cur_video_Height <= m_videoHeight) {
                        m_isDynamicSaver = true;
                    } else {
                        qInfo() << "High video resolution!!! resolution = " << cur_video_Width << "x"
                                << cur_video_Height;
                    }
                } else {
                    qInfo() << "The video format is not supported!!! format = " << cur_video_format
                            << "video_Codetype = " << m_videoFormat;
                }
            } else {
                qInfo() << "Video file too large!!!";
            }
        } else {
            qInfo() << "The video file does not exist!!!";
        }
    } else {
        m_isDynamicSaver = false;
        qDebug() << "The nSubMode is not 0!!!";
    }
}

QPixmap Screensaver::getPaddingPixmap()
{
    if (background.width() == 0 || background.height() == 0) {
        return QPixmap();
    }

    bool useHeight;
    float scaled = 0.0;
    QPixmap scaledPixmap;
    QPixmap paddingPixmap;
    qint64 rw = qint64(this->height()) * qint64(background.width()) / qint64(background.height());

    useHeight = (rw >= this->width());
    if (useHeight) {
        scaled = float(this->height()) / float(background.height());
        scaledPixmap = background.scaled(background.width() * scaled, this->height());
        paddingPixmap
            = scaledPixmap.copy((background.width() * scaled - this->width()) / 2, 0, this->width(), this->height());
    } else {
        scaled = float(this->width()) / float(background.width());
        scaledPixmap = background.scaled(this->width(), background.height() * scaled);
        paddingPixmap
            = scaledPixmap.copy(0, (background.height() * scaled - this->height()) / 2, this->width(), this->height());
    }

    return paddingPixmap;
}

void Screensaver::addClickedEvent()
{
    respondClick = true;
    isScreensaver = false;
}

void Screensaver::resizeEvent(QResizeEvent * /*event*/)
{
    if (m_isDynamicSaver) {
        if (m_playerView) {
            m_playerView->setFixedSize(this->size());
            m_playerItem->setSize(this->size());
        }
        return;
    } else if (m_isUrlSaver) {
        m_pCenterUrlWidget->setFixedSize(this->size());
        m_pWebView->setFixedSize(this->size());
        if (m_pErrorUrlLabel) {
            m_pErrorUrlLabel->setFixedSize(this->size());
            m_pErrorUrlLabel->setPixmap(QPixmap(":/assets/error-resolved.png").scaled(this->size()));
        }
        return;
    }
    if ((m_nSubMode == 1 || m_nSubMode == 2 || m_nSubMode == 3) && width() > 0 && height() > 0) {
        const QString backgroundFile
            = (isScreensaver ? configuration->getDefaultBackground()
                             : mapToThumbnailPath(configuration->getDefaultBackground()));
        if (background.isNull() || background.width() != width() || background.height() != height()) {
            background = scaleBlurPixmap(width(), height(), backgroundFile);
            m_scaleBackground = scaleBlurPixmap(width(), height(), backgroundFile);
        }
    }
    float scale = 1.0;
    scale = (float)width() / 1920;
    if ((width() < 600 || height() < 400) && !isScreensaver) { // 当显示在控制面板上时，字体缩小三倍。
        if (flag == 0) {
            QList<QLabel *> labelList = this->findChildren<QLabel *>();
            for (int i = 0; i < labelList.count(); i++) {
                if (labelList.at(i) != myPreviewLabel && labelList.at(i)->parentWidget() != m_weatherWidget) {
                    int fontsize = labelList.at(i)->font().pixelSize();
                    int fontpt = labelList.at(i)->font().pointSize();
                    if (fontsize > 0) {
#ifdef USE_INTEL
                        const QString SheetStyle = QString("font-size:%1px;").arg(fontsize / 3);
#else
                        const QString SheetStyle = QString("font-size:%1px;").arg(fontsize / 4);
#endif
                        labelList.at(i)->setStyleSheet(SheetStyle);
                    } else {
                        QFont font = labelList.at(i)->font();
#ifdef USE_INTEL
                        font.setPointSize(fontpt / 3);
#else
                        font.setPointSize(fontpt / 4);
#endif
                        labelList.at(i)->setFont(font);
                    }
                }
            }
            QList<QWidget *> childList = timeLayout->findChildren<QWidget *>();
            for (int i = 0; i < childList.count(); ++i) {
                childList.at(i)->adjustSize();
            }
            timeLayout->adjustSize();
            if (m_widgetDatetime) {
                m_widgetDatetime->adjustSize();
            }
            if (m_centerWidget)
                m_centerWidget->adjustSize();
        }
        flag = 1;
#ifndef USE_INTEL
        if (myTextWidget) {
            if (curStyle == "ukui-dark" || curStyle == "ukui-black") {
                myTextLabel->setStyleSheet(
                    QString("QLabel{font-size: 5px; background: rgba(0, 0, 0, %1); color:#FFFFFF; border-radius: 2px; "
                            "padding: 4px 8px 4px 8px; border-width: 1px;}")
                        .arg(blur_Num * 0.01));
                cycleLabel->setStyleSheet(
                    QString("QLabel{background: rgba(0, 0, 0, %1); color:#FFFFFF; border-radius:3px}")
                        .arg(blur_Num * 0.01));
            } else {
                myTextLabel->setStyleSheet(QString("QLabel{font-size: 5px; background: rgba(255, 255, 255, %1); "
                                                   "border-radius:2px; padding: 4px 8px 4px 8px; border-width: 1px;}")
                                               .arg(blur_Num * 0.01));
                cycleLabel->setStyleSheet(
                    QString("QLabel{background: rgba(255, 255, 255, %1); border-radius:3px}").arg(blur_Num * 0.01));
            }
            cycleLabel->setFixedSize(6, 6);
        }
        if (screenLabel)
            screenLabel->adjustSize();
        if (sleepTime)
            sleepTime->setSmallMode();
#endif
        scale = 0.1;
    }

#ifdef USE_INTEL
    int x = 840 * scale;
    int y = 96 * scale;

    m_weatherLaout->setGeometry(
        (this->width() - m_weatherLaout->width()) / 2,
        y,
        m_weatherLaout->geometry().width(),
        m_weatherLaout->geometry().height());

    timeLayout->setGeometry(
        (this->width() - timeLayout->width()) / 2,
        m_weatherLaout->geometry().bottom() + 33,
        timeLayout->geometry().width(),
        timeLayout->geometry().height());

    if (m_centerWidget) {
        m_centerWidget->adjustSize();
        m_centerWidget->setGeometry(
            (width() - m_centerWidget->width()) / 2,
            (height() * 85) / 100 - (m_centerWidget->height()) / 2,
            m_centerWidget->width(),
            m_centerWidget->height());

        if ((height() - m_centerWidget->height()) / 2 < timeLayout->y() + timeLayout->height())
            m_centerWidget->setGeometry(
                (width() - m_centerWidget->width()) / 2,
                timeLayout->y() + timeLayout->height(),
                m_centerWidget->width(),
                m_centerWidget->height());
    }

    if (m_widgetNotice) {
        m_widgetNotice->setGeometry(
            (this->width() - m_widgetNotice->width()) / 2,
            (m_centerWidget->y() + m_centerWidget->height() + 20),
            m_widgetNotice->geometry().width(),
            m_widgetNotice->geometry().height());
    }
#else
    int x, y;
    if (m_nSubMode == 0 || m_nSubMode < 0) {
        x = (this->width() - m_widgetDatetime->geometry().width()) / 2;
        y = 80 * scale;
        if (m_widgetDatetime) {
            m_widgetDatetime->setGeometry(
                x, y, m_widgetDatetime->geometry().width(), m_widgetDatetime->geometry().height());
        }
        if (m_weatherWidget) {
            m_weatherWidget->hide();
        }
        if (m_widgetMediaControl) {
            m_widgetMediaControl->hide();
        }
        if (m_customAlbumViewer) {
            m_customAlbumViewer->hide();
        }
    } else {
        x = 284 * scale;
        y = 416 * scale;
        m_widgetDatetime->setGeometry(
            x,
            (height() - m_widgetDatetime->height()) / 2,
            m_widgetDatetime->geometry().width(),
            m_widgetDatetime->geometry().height());
        if (m_centerWidget) {
            m_centerWidget->hide();
        }
        if (myTextWidget)
            myTextWidget->hide();
        if ((m_nSubMode == 1 || m_nSubMode == 2 || m_nSubMode == 3) && !m_isDynamicSaver) {
            onScreensaverDatetimeEnableChanged(m_isShowDateTime, getenv("USER"));
        }
    }

    if (m_thumBnalListWidget) {
        m_thumBnalListWidget->setGeometry(0, m_smartSaverLabel->height() + 70 * scale, 350 + 20 * scale,
                                          this->height() - (m_smartSaverLabel->height() + 70 * scale) - (sleepTime->height() + 26 * scale));
    }

    if (m_smartSaverLabel) {
        m_smartSaverLabel->setGeometry(0, 35 * scale, m_thumBnalListWidget->width() - 16, 50);
    }

    if (m_sidebarBtn) {
        m_sidebarBtn->setFixedSize(44 * scale, 136 * scale);
        m_sidebarBtn->setIconSize(QSize(44 * scale, 44 * scale));
        if (m_thumBnalListWidget && m_thumBnalListWidget->isVisible()) {
            m_sidebarBtn->move(m_thumBnalListWidget->width(), (height() - m_sidebarBtn->height()) / 2);
            m_sidebarBtn->setIcon(
                drawSymbolicColoredPixmap(QIcon::fromTheme("pan-start-symbolic").pixmap(44, 44), "white"));
        } else if (m_thumBnalListWidget && m_thumBnalListWidget->isHidden()) {
            m_sidebarBtn->move(0, (height() - m_sidebarBtn->height()) / 2);
            m_sidebarBtn->setIcon(
                drawSymbolicColoredPixmap(QIcon::fromTheme("pan-end-symbolic").pixmap(44, 44), "white"));
        }
    }

    if (m_nSubMode == 1 && m_weatherWidget) {
        if ((width() < 600 || height() < 400) && !isScreensaver) {
            m_weatherWidget->setFixedSize(534 * 0.15, 770 * 0.15);
            m_weatherWidget->move(
                width() - m_weatherWidget->width() - 280 * 0.15, (this->height() - m_weatherWidget->height()) / 2);
            m_weatherWidget->show();
            m_weatherWidget->updateWeatherSize(0.15);
        } else {
            m_weatherWidget->setFixedSize(534 * scale, 770 * scale);
            m_weatherWidget->move(
                width() - m_weatherWidget->width() - 280 * scale, (this->height() - m_weatherWidget->height()) / 2);
            m_weatherWidget->show();
            m_weatherWidget->updateWeatherSize(scale);
        }
        m_weatherManager->initNetworkStatus();
        if (m_widgetMediaControl) {
            m_widgetMediaControl->hide();
        }
        if (m_customAlbumViewer) {
            m_customAlbumViewer->hide();
        }
        if (m_weatherWidget->x() - m_widgetDatetime->width() > 0) {
            m_widgetDatetime->move((m_weatherWidget->x() - m_widgetDatetime->width()) / 2,
                                   (height() - m_widgetDatetime->height()) / 2);
        } else {
            m_widgetDatetime->move(0, (height() - m_widgetDatetime->height()) / 2);
        }
    }

    if (m_nSubMode == 2 && m_widgetMediaControl) {
        if ((width() < 600 || height() < 400) && !isScreensaver) {
            m_widgetMediaControl->setFixedSize(521 * 0.15, 620 * 0.15);
            m_widgetMediaControl->move(
                width() - m_widgetMediaControl->width() - 280 * 0.15,
                (this->height() - m_widgetMediaControl->height()) / 2);
            m_widgetMediaControl->show();
            m_mediaControlWidget->updateMediaSize(0.15);
        } else {
            m_widgetMediaControl->setFixedSize(521 * scale, 620 * scale);
            m_widgetMediaControl->move(
                width() - m_widgetMediaControl->width() - 280 * scale,
                (this->height() - m_widgetMediaControl->height()) / 2);
            m_widgetMediaControl->show();
            m_mediaControlWidget->updateMediaSize(scale);
        }
        if (m_weatherWidget) {
            m_weatherWidget->hide();
        }
        if (m_customAlbumViewer) {
            m_customAlbumViewer->hide();
        }
        if (m_widgetMediaControl->x() - m_widgetDatetime->width() > 0) {
            m_widgetDatetime->move((m_widgetMediaControl->x() - m_widgetDatetime->width()) / 2,
                                   (height() - m_widgetDatetime->height()) / 2);
        } else {
            m_widgetDatetime->move(0, (height() - m_widgetDatetime->height()) / 2);
        }
    }

    if (m_nSubMode == 3 && m_customAlbumViewer) {
        if ((width() < 600 || height() < 400) && !isScreensaver) {
            m_customAlbumViewer->show();
            m_customAlbumViewer->setFixedSize(847 * 0.15, 790 * 0.15);
            m_customAlbumViewer->updateViewerSize(0.15);
            m_customAlbumViewer->move(
                width() - m_customAlbumViewer->width() - 100 * 0.15,
                (this->height() - m_customAlbumViewer->height()) / 2);
        } else {
            m_customAlbumViewer->show();
            m_customAlbumViewer->setFixedSize(847 * scale, 790 * scale);
            m_customAlbumViewer->updateViewerSize(scale);
            m_customAlbumViewer->move(
                width() - m_customAlbumViewer->width() - 100 * scale,
                (this->height() - m_customAlbumViewer->height()) / 2);
        }
        m_customAlbumViewer->setUpdateInterval(m_iPhotoSwitchTime);
        if (m_weatherWidget) {
            m_weatherWidget->hide();
        }
        if (m_widgetMediaControl) {
            m_widgetMediaControl->hide();
        }
        if (m_widgetDatetime) {
            if (m_customAlbumViewer->x() - m_widgetDatetime->width() > 0) {
                m_widgetDatetime->move((m_customAlbumViewer->x() - m_widgetDatetime->width()) / 2,
                                       (height() - m_widgetDatetime->height()) / 2);
            } else {
                m_widgetDatetime->move(0, (height() - m_widgetDatetime->height()) / 2);
            }
        }
    }

    if (sleepTime) {
        x = 26 * scale;
        y = this->height() - sleepTime->geometry().height() - 26 * scale;
        sleepTime->setGeometry(x, y, sleepTime->geometry().width(), sleepTime->geometry().height());
    }

    if (screenLabel) {
        screenLabel->setGeometry((width() - screenLabel->width()) / 2, y, screenLabel->width(), screenLabel->height());
    }

    if (m_centerWidget) {
        m_centerWidget->adjustSize();
        m_centerWidget->setGeometry(
            (width() - m_centerWidget->width()) / 2,
            (height() - m_centerWidget->height()) / 2,
            m_centerWidget->width(),
            m_centerWidget->height());

        if ((height() - m_centerWidget->height()) / 2 < m_widgetDatetime->y() + m_widgetDatetime->height())
            m_centerWidget->setGeometry(
                (width() - m_centerWidget->width()) / 2,
                m_widgetDatetime->y() + m_widgetDatetime->height(),
                m_centerWidget->width(),
                m_centerWidget->height());
    }

    if (myTextWidget)
        setRandomPos();
#endif
}

void Screensaver::setRandomPos()
{
    myTextWidget->adjustSize();
    int x1 = 10;
    int x2 = width() - 10 - myTextWidget->width();
    int y1 = timeLayout->geometry().bottom() + 10;
    int y2;
    if (sleepTime)
        y2 = sleepTime->geometry().top() - myTextWidget->height() - 10;
    else
        y2 = geometry().bottom() - myTextWidget->height() - 10;

    int x = 0;
    int y = 0;
    if (x2 > x1)
        x = qrand() % (x2 - x1) + x1;
    if (y2 > y1)
        y = qrand() % (y2 - y1) + y1;

    myTextWidget->move(x, y);
}

void Screensaver::setUpdateCenterWidget()
{
    // QString lang = qgetenv("LANG");
    QString lang = QLocale::system().name();
    QString homePath = qgetenv("HOME");
    QString cwdPath = "/usr/share/ukui-screensaver/";
    QString languageDirPath = cwdPath + "language/";
    QString defaultLanguageFilePath = languageDirPath + "screensaver-en_US.ini";
    qDebug() << "homePath=" << homePath;
    if (!lang.isEmpty()) {
        qDebug() << "lang = " << lang;
        if (lang.contains(":")) {
            lang = lang.split(":").at(0);
        }
        if (lang.contains('.')) {
            lang = lang.split('.')[0];
            qDebug() << "langStr = " << lang;
        }

        // 不管系统语言，默认显示中文
        QString languageFilePath = languageDirPath + "screensaver-zh_CN.ini";
        if (isOpenkylin()) {
            if (QStringList({ "zh_CN", "bo_CN", "zh_HK", "kk_KZ", "ky_KG", "mn_MN", "ug_CN" }).contains(lang)) {
                languageFilePath = languageDirPath + "screensaver-zh_CN-ok.ini";
            } else {
		languageFilePath = languageDirPath + "screensaver-" + lang + ".ini";
            }
        }
        // QString languageFilePath=languageDirPath+"screensaver-"+lang+".ini";
        QString homeLanguageFilePath = homePath + "/.config/ukui/screensaver-" + lang + ".ini";
        QString jdLanguageFilePath = languageDirPath + "screensaver-jd" + ".ini";
        qDebug() << "langnguageFile = " << languageFilePath;
        qDebug() << "homeLanguageFilePath = " << homeLanguageFilePath;
        QFileInfo fileInfo(languageFilePath);
        QFileInfo homeConfigFileInfo(homeLanguageFilePath);
        QFileInfo jdConfigFileInfo(jdLanguageFilePath);
        bool useJd = false;

        QDate date1(2021, 6, 20);
        QDate date2(2021, 7, 31);
        if (QDate::currentDate() >= date1 && QDate::currentDate() <= date2) {
            useJd = true;
        }

        if (useJd && jdConfigFileInfo.isFile()) {
            qsettings = new QSettings(jdLanguageFilePath, QSettings::IniFormat);
        } else if (homeConfigFileInfo.isFile()) {
            qsettings = new QSettings(homeLanguageFilePath, QSettings::IniFormat);
        } else if (fileInfo.isFile()) {
            qsettings = new QSettings(languageFilePath, QSettings::IniFormat);
        } else {
            qsettings = new QSettings(defaultLanguageFilePath, QSettings::IniFormat);
        }
    } else {
        qsettings = new QSettings(defaultLanguageFilePath, QSettings::IniFormat);
    }

    qsettings->setIniCodec(QTextCodec::codecForName("UTF-8"));
}

void Screensaver::updateBackgroundPath()
{
    qDebug() << "ScreenSaver::startSwitchImages";
    QFileInfo fileInfo(backgroundPath);
    imagePaths.clear();
    if (fileInfo.isFile())
        return;
    QList<QByteArray> formats = QImageReader::supportedImageFormats();
    if (fileInfo.isDir()) {
        QDir dir(backgroundPath);
        QStringList files = dir.entryList(QDir::Files | QDir::Readable, QDir::Name);
        for (QString file : files) {
            fileInfo.setFile(file);
            QString suffix = fileInfo.suffix();
            if (formats.contains(suffix.toUtf8()) && file.right(4) != ".ico" && file.right(4) != ".tga") {
                // qDebug() << "file:" <<file;
                imagePaths.push_back(backgroundPath + "/" + file);
            }
        }
    }
    m_bImageLoadFinished = true;
}
void Screensaver::enterEvent(QEvent *)
{
    // qDebug() << "enter ScreenSaver::enterEvent";
    // 当前是否是控制面板窗口
    if (!isScreensaver) {
        setPreviewText(true);
    }
}

void Screensaver::leaveEvent(QEvent *)
{
    // qDebug() << "enter ScreenSaver::leaveEvent";
    setPreviewText(false);
    // setToolTip("离开");
}

void Screensaver::startSwitchImages()
{
    if (!imagePaths.empty()) {
        background = loadFromFile(imagePaths.at(0));
        currentPath = imagePaths.at(0);
        qDebug() << currentPath;
        is_gif = currentPath.endsWith(".gif");
        if (!switchTimer) {
            switchTimer = new QTimer(this);
            connect(switchTimer, &QTimer::timeout, this, [&] {
                if (isAutoSwitch) {
                    int index = qrand() % imagePaths.count();
                    background = loadFromFile(imagePaths.at(index));
                    currentPath = imagePaths.at(index);
                    is_gif = currentPath.endsWith(".gif");
                } else {
                    if (currentIndex >= imagePaths.count() - 1 || currentIndex < 0) {
                        currentIndex = 0;
                    } else {
                        currentIndex++;
                    }
                    background = loadFromFile(imagePaths.at(currentIndex));
                    currentPath = imagePaths.at(currentIndex);
                    is_gif = currentPath.endsWith(".gif");
                }
                isMovie();
            });
        }

        switchTimer->start(cycleTime * 1000);

        isMovie();
    }
}

void Screensaver::stopSwitchImages()
{
    if (switchTimer && switchTimer->isActive())
        switchTimer->stop();
}

void Screensaver::initDynamic()
{
    if (m_isDynamicSaver) {
        if (m_playerView)
            return;
        m_pVideoPlayer = new KyVideoPlayer();
        m_playerScene = new QGraphicsScene(this);
        m_playerView = new GraphicsView(this);
        m_playerView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_playerView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_playerView->setStyleSheet("border: none; border-radius: 0px;");
        m_playerView->setMouseTracking(true);
        m_playerItem = new QGraphicsVideoItem;
        m_playerScene->addItem(m_playerItem);
        m_pVideoPlayer->setMediaFile(m_videoPath);
        m_playerView->setScene(m_playerScene);
        m_pVideoPlayer->setOutput(m_playerItem, video_Duration);
        m_playerView->show();
    } else {
        if (isCustom)
            setSleeptime(isCShowRestTime);
        else
            setSleeptime(isUShowRestTime);
        if (!m_widgetDatetime) {
            setDatelayout();
        }
        if (!m_centerWidget) {
            setCenterWidget();
        }
        setRandomText();
        if (m_centerWidget) {
            if (textIsCenter || myText == "") {
                myTextWidget->hide();
                m_centerWidget->show();
            } else {
                m_centerWidget->hide();
                myTextWidget->show();
            }
        }
    }
}

void Screensaver::initUI()
{
    QFile qssFile;

#ifdef USE_INTEL
    qssFile.setFileName(":/qss/assets/default-intel.qss");
#else
    qssFile.setFileName(":/qss/assets/default.qss");
#endif

    if (qssFile.open(QIODevice::ReadOnly)) {
        setStyleSheet(qssFile.readAll());
    }
    qssFile.close();

#ifdef USE_INTEL
    setWeatherLayout();
    setDatelayout();
    setCenterWidget();
    //    setNoticeLaout();
    m_weatherManager->getWeather();
    connect(m_weatherManager, &WeatherManager::onWeatherUpdate, this, &Screensaver::getWeatherFinish);
#else

    if (isCustom)
        setSleeptime(isCShowRestTime);
    else
        setSleeptime(isUShowRestTime);
    setDatelayout();
    setCenterWidget();
    setRandomText();
    if(m_centerWidget) {
        if (textIsCenter || myText == "") {
            myTextWidget->hide();
            m_centerWidget->show();
        }else{
            m_centerWidget->hide();
            myTextWidget->show();
        }
    }

    QFileInfo fileInfo(backgroundPath);
    bool isUrlValid = false;
    isUrlValid = getIsValidUrl(backgroundPath);
    if (isCustom) {
        if (fileInfo.isFile()) {
            initDynamicSaver(backgroundPath);
            if (m_isDynamicSaver) {
                m_isUrlSaver = false;
            }
        } else if (isUrlValid) {
            initUrlWidget();
            m_isUrlSaver = true;
            m_isDynamicSaver = false;
        } else {
            m_isDynamicSaver = false;
            m_isUrlSaver = false;
        }
    }

    initOtherWidget();

    if (m_nSubMode == 1) {
        initWeatherWidget();
        if (myTextWidget)
            myTextWidget->hide();
    }
    if (m_nSubMode == 2) {
        initMediaControlWidget();
        if (myTextWidget)
            myTextWidget->hide();
    }
    if (m_nSubMode == 3) {
        initCustomAlbumViewer();
        if (myTextWidget)
            myTextWidget->hide();
    }

    initThumBnalListWidget();

    if (m_isDynamicSaver || m_isUrlSaver) {
        if (m_smartSaverLabel) {
            m_smartSaverLabel->hide();
        }
        if (m_thumBnalListWidget) {
            m_thumBnalListWidget->hide();
        }
        if (m_sidebarBtn) {
            m_sidebarBtn->hide();
        }
    }

    // V11上在窗口关闭时会收到一个leaveEvent(),控制面板关闭的时候new这个label，会导致控制面板关闭卡顿。
    if (!myPreviewLabel) {
        myPreviewLabel = new QLabel(this);
        myPreviewLabel->setFixedHeight(30);
        myPreviewLabel->setMinimumWidth(54);

        // 设置样式
        myPreviewLabel->setStyleSheet("background-color: rgb(38,38,38); border-radius: 0px; color:white;");
        myPreviewLabel->setAlignment(Qt::AlignCenter);
        myPreviewLabel->hide();
    }

#endif
}

void Screensaver::initDynamicSaver(QString path)
{
    qDebug() << "videoPath = " << path << "videoSize = " << m_videoSize << "videoFormat = " << m_videoFormat;
    QFileInfo fileInfo(path);
    //视频文件是否存在
    if (fileInfo.isFile()) {
        //视频文件大小限制
        if (fileInfo.size() <= m_videoSize * 1024 * 1024) {
            getVideoFormat(path);
            QStringList formatList = m_videoFormat.split(',');
            //视频格式限制
            bool m_rightFormat = false;
            for (int formatAt = 0; formatAt < formatList.count(); formatAt++) {
                if (cur_video_format.contains(formatList.at(formatAt))) {
                    m_rightFormat = true;
                    break;
                }
            }
            if (m_rightFormat) {
                //分辨率限制
                if (cur_video_Width <= m_videoWidth && cur_video_Height <= m_videoHeight) {
                    m_isDynamicSaver = true;
                    m_saverState = SAVERERRORTYPE::NORMALSAVER;
                    if (!m_pVideoPlayer) {
                        m_pVideoPlayer = new KyVideoPlayer();
                        connect(m_pVideoPlayer, &KyVideoPlayer::playerShowError, this, [=](QMediaPlayer::Error error) {
                            switch (error) {
                            case QMediaPlayer::ResourceError:
                                qDebug() << "媒体资源无法被解析！";
                                m_sPlayerShowErrorTip = tr("The property cannot be parsed!");
                                break;
                            case QMediaPlayer::FormatError:
                                qDebug() << "不支持该媒体格式！";
                                m_sPlayerShowErrorTip = tr("This media format is not supported!");
                                break;
                            case QMediaPlayer::NetworkError:
                                qDebug() << "发生了一个网络错误！";
                                m_sPlayerShowErrorTip = tr("A network error has occurred!");
                                break;
                            case QMediaPlayer::AccessDeniedError:
                                qDebug() << "没有播放权限！";
                                m_sPlayerShowErrorTip = tr("No playback privileges!");
                                break;
                            case QMediaPlayer::ServiceMissingError:
                                qDebug() << "没有发现有效的播放服务！";
                                m_sPlayerShowErrorTip = tr("No effective playback service found!");
                                break;
                            default:
                                break;
                            }
                            m_isDynamicSaver = false;
                            m_saverState = SAVERERRORTYPE::PLAYERSHOWERROR;
                            m_playerView->hide();
                            screenLabel->setText(m_sPlayerShowErrorTip);
                            screenLabel->adjustSize();
                            initOtherWidget();
                        });
                    }
                    if (!m_playerScene)
                        m_playerScene = new QGraphicsScene(this);
                    if (!m_playerView) {
                        m_playerView = new GraphicsView(this);
                        m_playerView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                        m_playerView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                        m_playerView->setStyleSheet("border: none; border-radius: 0px;");
                    }
                    if (!m_playerItem)
                        m_playerItem = new QGraphicsVideoItem;
                    m_playerScene->addItem(m_playerItem);
                    m_pVideoPlayer->setMediaFile(backgroundPath);
                    m_playerView->setScene(m_playerScene);
                    m_pVideoPlayer->setOutput(m_playerItem, video_Duration);
                    m_playerView->show();
                } else {
                    if (formatList.contains(fileInfo.suffix())) {
                        m_saverState = SAVERERRORTYPE::RESOLUTIONERROR;
                    }
                    m_isDynamicSaver = false;
                    qInfo() << "High video resolution!!! resolution = " << cur_video_Width << "x" << cur_video_Height;
                }
            } else {
                if (formatList.contains(fileInfo.suffix())) {
                    m_saverState = SAVERERRORTYPE::VIDEOFORMATEERROR;
                }
                m_isDynamicSaver = false;
                qInfo() << "The video format is not supported!!! format = " << cur_video_format
                        << "video_Codetype = " << video_Codetype;
            }
        } else {
            QStringList formatList = m_videoFormat.split(',');
            if (formatList.contains(fileInfo.suffix())) {
                m_saverState = SAVERERRORTYPE::VIDEOTOOLARGE;
            }
            m_isDynamicSaver = false;
            qInfo() << "Video file too large!!!" ;
        }
    } else {
        m_isDynamicSaver = false;
        qInfo() << "The video file does not exist!!!";
    }
}

void Screensaver::initUrlWidget()
{
    if (!m_pCenterUrlWidget) {
        m_pCenterUrlWidget = new QWidget(this);
    }
    if (!m_pWebView) {
        m_pWebView = new QWebView(m_pCenterUrlWidget);
        m_pWebView->setEnabled(false);    //禁用网页鼠标交互

        // WebKit 设置
        QWebSettings *settings = m_pWebView->settings();
        settings->setAttribute(QWebSettings::ScrollAnimatorEnabled, false);
        settings->setAttribute(QWebSettings::JavascriptEnabled, true);
        settings->setAttribute(QWebSettings::PluginsEnabled, false);
        // 禁用滚动条
        m_pWebView->page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
        m_pWebView->page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
    }
    qDebug() << "load url = " << backgroundPath;
    m_pWebView->load(QUrl(backgroundPath));
    m_pWebView->setFocusPolicy(Qt::NoFocus);
    m_pWebView->setContextMenuPolicy(Qt::NoContextMenu);
    connect(m_pWebView, &QWebView::loadFinished, this, [=](bool ok) { qDebug() << "loadFinished: " << ok; });

    connect(m_pWebView, &QWebView::urlChanged, this, [=](const QUrl &url) {
        qDebug() << "urlChanged: " << url;
        if (url == QUrl("about:blank")) {
            qDebug() << "backgroundPath = " << backgroundPath;
            m_pErrorUrlLabel = new QLabel(m_pCenterUrlWidget);
            m_pErrorUrlLabel->setPixmap(QPixmap(":/assets/error-resolved.png"));
            m_pWebView->hide();
            m_pErrorUrlLabel->show();
        }
    });

    m_pWebView->show();
    m_pCenterUrlWidget->show();
}

void Screensaver::initOtherWidget()
{
    if (isCustom && (m_isDynamicSaver || m_isUrlSaver)) {
        if (m_isDynamicSaver && m_pWebView) {
            m_pWebView->stop();
            m_pWebView->hide();
            m_pCenterUrlWidget->hide();
        }
        if (m_isUrlSaver && m_pVideoPlayer) {
            m_pVideoPlayer->stop();
            m_pVideoPlayer->hide();
            m_playerView->hide();
        }
        dateOfDay->hide();
        dateOfLocaltime->hide();
        setSleeptime(false);
        if (myTextWidget) {
            myTextWidget->hide();
        }
        if (m_centerWidget) {
            m_centerWidget->hide();
        }
        if (screenLabel) {
            screenLabel->hide();
        }
    } else {
        if (m_pWebView) {
            m_pWebView->stop();
            m_pWebView->hide();
            m_pCenterUrlWidget->hide();
        }
        if (m_pVideoPlayer) {
            m_pVideoPlayer->stop();
            m_pVideoPlayer->hide();
            m_playerView->hide();
        }
        if (isCustom)
            setSleeptime(isCShowRestTime);
        else
            setSleeptime(isUShowRestTime);
        dateOfDay->show();
        dateOfLocaltime->show();
        if (textIsCenter && !m_centerWidget) {
            setCenterWidget();
        } else if (textIsCenter && m_centerWidget) {
            m_centerWidget->show();
        }
        if (isCustom && !textIsCenter) {
            setRandomText();
        }
        if (screenLabel)
            screenLabel->hide();
    }
    QCoreApplication::postEvent(this, new QEvent(QEvent::User));
}

void Screensaver::setDatelayout()
{
    timeType = configuration->getTimeType();
    dateType = configuration->getDateType();
#ifdef USE_INTEL
    timeLayout = new QWidget(this);
    timeLayout->setFixedSize(((timeType == 12) ? 397 : 326), 117);

    QHBoxLayout *htimeLayout = new QHBoxLayout(timeLayout);
    htimeLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *timeWidget = new QWidget(this);
    timeWidget->setFixedSize(235, 117);
    QGridLayout *gtimeLayout = new QGridLayout(timeWidget);
    gtimeLayout->setContentsMargins(0, 0, 0, 12);

    this->dateOfLocaltimeHour = new QLabel(this);
    this->dateOfLocaltimeHour->setObjectName("dateOfLocaltime");
    this->dateOfLocaltimeHour->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    this->dateOfLocaltimeHour->setFixedSize(107, 96);

    this->dateofLocaltimeColon = new QLabel(this);
    this->dateofLocaltimeColon->setObjectName("dateOfLocaltime");
    this->dateofLocaltimeColon->setAlignment(Qt::AlignCenter);
    this->dateofLocaltimeColon->setFixedSize(21, 96);
    this->dateofLocaltimeColon->setText(":");

    this->dateOfLocaltimeMinute = new QLabel(this);
    this->dateOfLocaltimeMinute->setObjectName("dateOfLocaltime");
    this->dateOfLocaltimeMinute->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    this->dateOfLocaltimeMinute->setFixedSize(107, 96);

    gtimeLayout->setSpacing(0);
    gtimeLayout->setRowMinimumHeight(0, 9);
    gtimeLayout->setRowMinimumHeight(1, 87);
    gtimeLayout->setRowMinimumHeight(2, 9);

    gtimeLayout->setColumnMinimumWidth(0, 107);
    gtimeLayout->setColumnMinimumWidth(1, 21);
    gtimeLayout->setColumnMinimumWidth(2, 107);

    gtimeLayout->addWidget(dateOfLocaltimeHour, 1, 0, 2, 1);
    gtimeLayout->addWidget(dateofLocaltimeColon, 0, 1, 2, 1);
    gtimeLayout->addWidget(dateOfLocaltimeMinute, 1, 2, 2, 1);

    QWidget *dateWidget = new QWidget(this);
    dateWidget->setFixedSize(((timeType == 12) ? 154 : 83), 117);
    QVBoxLayout *vDateLaout = new QVBoxLayout(dateWidget);
    vDateLaout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    vDateLaout->setContentsMargins(0, 18, 0, 12);

    this->dateOfWeek = new QLabel(this);
    this->dateOfWeek->setObjectName("dateOfWeek");
    this->dateOfWeek->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    this->dateOfWeek->setFixedWidth(83);

    this->dateOfDay = new QLabel(this);
    this->dateOfDay->setObjectName("dateOfDay");
    this->dateOfDay->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    this->dateOfDay->setFixedSize(((timeType == 12) ? 154 : 83), 32);
    updateTime();
    updateDate();

    this->dateOfWeek->adjustSize();
    vDateLaout->addWidget(dateOfWeek);
    vDateLaout->setSpacing(4);
    vDateLaout->addWidget(dateOfDay);

    htimeLayout->addWidget(timeWidget);
    htimeLayout->addSpacing(8);
    htimeLayout->addWidget(dateWidget);
#else
    timeLayout = new QWidget(this);
    QVBoxLayout *vtimeLayout = new QVBoxLayout(timeLayout);
    QSettings settings("org.ukui.style", QSettings::NativeFormat);
    QString defaultFont = settings.value("system-font", "Noto Sans CJK SC").toString();
    this->dateOfLocaltime = new QLabel(this);
    sysFont = qApp->font();
    sysFont.setPointSize(TIME_FONT_SIZE * m_ptToPx);
    sysFont.setFamily(defaultFont);
    sysFont.setWeight(700);
    this->dateOfLocaltime->setFont(sysFont);

    //    if (timeType == 12)
    //        this->dateOfLocaltime->setText(getLongFormatDate(TIME));
    //    else
    this->dateOfLocaltime->setText(getLongFormatDate(TIME));

    this->dateOfLocaltime->setObjectName("dateOfLocaltime");
    this->dateOfLocaltime->setAlignment(Qt::AlignCenter);
    this->dateOfLocaltime->adjustSize();
    vtimeLayout->addWidget(dateOfLocaltime);

    this->dateOfDay = new QLabel(this);
    sysFont.setPointSize(TIME_DATE_SIZE * m_ptToPx);
    sysFont.setFamily(defaultFont);
    sysFont.setWeight(50);
    this->dateOfDay->setFont(sysFont);
    //    if(dateType == "cn")
    //        this->dateOfDay->setText(QDate::currentDate().toString("yyyy/MM/dd ddd").replace("周","星期"));
    //    else
    //        this->dateOfDay->setText(QDate::currentDate().toString("yyyy-MM-dd ddd").replace("周","星期"));
    this->dateOfDay->setText(getLongFormatDate(DATE));
    this->dateOfDay->setObjectName("dateOfDay");
    this->dateOfDay->setAlignment(Qt::AlignCenter);
    this->dateOfDay->adjustSize();

    vtimeLayout->addWidget(this->dateOfDay);
    timeLayout->adjustSize();
    updateDate();
    m_widgetDatetime = new QWidget(this);
    QHBoxLayout *layoutDatetime = new QHBoxLayout(m_widgetDatetime);
    layoutDatetime->setContentsMargins(0, 0, 0, 0);
    layoutDatetime->setSpacing(0);
    layoutDatetime->addWidget(timeLayout);
    m_widgetDatetime->adjustSize();
    onScreensaverDatetimeEnableChanged(m_isShowDateTime, getenv("USER"));
#endif
}

void Screensaver::setWeatherLayout()
{
    m_weatherLaout = new QWidget(this);
    QHBoxLayout *hWeatherLayout = new QHBoxLayout(m_weatherLaout);
    hWeatherLayout->setContentsMargins(0, 0, 0, 0);

    this->m_weatherIcon = new QLabel(this);
    this->m_weatherArea = new QLabel(this);
    this->m_weatherCond = new QLabel(this);
    this->m_weatherTemperature = new QLabel(this);

    m_weatherIcon->setPixmap(m_weatherManager->getWeatherIcon());
    m_weatherArea->setText(m_weatherManager->getCityName());
    if (!m_weatherManager->getCond().isEmpty())
        m_weatherCond->setText("·" + m_weatherManager->getCond());
    m_weatherTemperature->setText(m_weatherManager->getTemperature());

    m_weatherArea->setStyleSheet("font-size:26px;color:#ffffff");
    m_weatherCond->setStyleSheet("font-size:26px;color:#ffffff");
    m_weatherTemperature->setStyleSheet("font-size:26px;color:#ffffff");

    hWeatherLayout->addWidget(m_weatherIcon);
    hWeatherLayout->addSpacing(8);
    hWeatherLayout->addWidget(m_weatherArea);
    hWeatherLayout->addWidget(m_weatherCond);
    hWeatherLayout->addSpacing(8);
    hWeatherLayout->addWidget(m_weatherTemperature);
    m_weatherLaout->adjustSize();
}

void Screensaver::setSleeptime(bool Isshow)
{
    if (!sleepTime)
        sleepTime = new SleepTime(this);

    sleepTime->adjustSize();
    if (Isshow) {
        sleepTime->show();
    } else {
        sleepTime->hide();
    }
}

void Screensaver::updateDate()
{
    if (!timer) {
        timer = new QTimer(this);
        timer->setTimerType(Qt::PreciseTimer);
        connect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));
    }
    timer->start(500);
    updateTime();
}

void Screensaver::updateTime()
{
#ifdef USE_INTEL
    QLocale locale(QLocale::system().name());

    QTimeZone timeZone(QString::fromLatin1(QTimeZone::systemTimeZoneId()).toLatin1());
    QDateTime tzNow = QDateTime::currentDateTime().toTimeZone(timeZone);

    QString time;

    if (timeType == 12) {
        time = tzNow.toString("hh:mm AP").split(" ").at(0);
        this->dateOfWeek->setText(tzNow.toString("AP"));
        if (dateType == "en") {
            this->dateOfDay->setText(tzNow.toString("ddd MM-dd"));
        } else {
            this->dateOfDay->setText(tzNow.toString("ddd MM/dd"));
        }
    } else {
        time = tzNow.toString("hh:mm");
        this->dateOfWeek->setText(tzNow.toString("ddd"));
        if (dateType == "en") {
            this->dateOfDay->setText(tzNow.toString("MM-dd"));
        } else {
            this->dateOfDay->setText(tzNow.toString("MM/dd"));
        }
    }

    this->dateOfLocaltimeHour->setText(time.split(":").at(0));
    this->dateOfLocaltimeMinute->setText(time.split(":").at(1));

    m_currentTime = QTime::currentTime();
#else
    QDateTime curDateTime = QDateTime::currentDateTime();
    if (m_lastDateTime.isNull() || qAbs(curDateTime.secsTo(m_lastDateTime)) >= 1) {
        if (timeType == 12)
            this->dateOfLocaltime->setText(getLongFormatDate(TIME));
        else
            this->dateOfLocaltime->setText(getLongFormatDate(TIME));

        //        if(dateType == "cn")
        //            this->dateOfDay->setText(curDateTime.date().toString("yyyy/MM/dd ddd").replace("周","星期"));
        //        else
        //            this->dateOfDay->setText(curDateTime.date().toString("yyyy-MM-dd ddd").replace("周","星期"));
        this->dateOfDay->setText(getLongFormatDate(DATE));
        m_lastDateTime = curDateTime;
    }

    if (sleepTime) {
        if (!sleepTime->setTime(QDateTime::currentDateTime())) {
            sleepTime->hide();
            delete sleepTime;
            sleepTime = NULL;
        }
    }
#endif
}

void Screensaver::setRandomText()
{
    if (!myTextWidget) {
        myTextWidget = new QWidget(this);
        QHBoxLayout *layout = new QHBoxLayout(myTextWidget);
        cycleLabel = new QLabel(this);
        cycleLabel->setFixedSize(16, 16);
        layout->addWidget(cycleLabel);
        layout->setSpacing(8);
        myTextLabel = new QLabel(myTextWidget);
        myTextLabel->setObjectName("myText");
        blur_Num = configuration->getBlurNumber();
        curStyle = configuration->getcurStyle();
        onBlurNumChanged(blur_Num);
        onStyleChanged(curStyle);
        layout->addWidget(myTextLabel);
    }
    qDebug() << " text = " << myText;
    sysFont.setPointSize((18 + curFontSize) * m_ptToPx);
    myTextLabel->setFont(sysFont);
    myTextLabel->setText(myText);
    myTextWidget->adjustSize();
    if (myText != "")
        myTextWidget->setVisible(true);
    else
        myTextWidget->setVisible(false);
}

void Screensaver::setPreviewText(bool bVisible)
{
    myPreviewLabel->setText(m_strPreViewTrans);
    myPreviewLabel->adjustSize();

    myPreviewLabel->setVisible(bVisible);
    // 先采取固定大小方式
    myPreviewLabel->move((width() - myPreviewLabel->width()) / 2, 120);
    myPreviewLabel->raise();
}

void Screensaver::setCenterWidget()
{
    QStringList qlist = qsettings->childGroups();
    if (qlist.count() < 1)
        return;
    QDate date = QDate::currentDate();
    QDate targetDate(2100, 1, 1);
    // 2100是否在未来
    int days = (date < targetDate) ? date.daysTo(targetDate) : targetDate.daysTo(date);
    int index = days % qlist.count() + 1;

    QString configPath = QDir::homePath() + "/.ukui-screensaver-default.conf";
    QSettings settings1(configPath, QSettings::IniFormat);
    if (settings1.value("FIRST").toString().isEmpty()) {
        settings1.setValue("FIRST", QDate::currentDate().toString("yy/MM/dd"));
        index = 1;
    }
    if (settings1.value("FIRST").toString() == QDate::currentDate().toString("yy/MM/dd"))
        index = 1;

    qsettings->beginGroup(QString::number(index));
    if (isCustom) {
        centerlabel1 = new QLabel(myText);
        centerlabel2 = new QLabel("");
        centerlabel2->hide();
        authorlabel = new QLabel("");
    } else if (qsettings->contains("OL")) {
        if (qsettings->value("OL").typeName() == "QString")
            centerlabel1 = new QLabel(qsettings->value("OL").toString());
        else
            centerlabel1 = new QLabel(qsettings->value("OL").toStringList().join(' '));
        centerlabel2 = new QLabel("");
        centerlabel2->hide();
#ifndef USE_INTEL
        authorlabel = new QLabel(qsettings->value("author").toString());
#endif
    } else if (qsettings->contains("FL")) {
        if (qsettings->value("FL").typeName() == "QString")
            centerlabel1 = new QLabel(qsettings->value("FL").toString());
        else
            centerlabel1 = new QLabel(qsettings->value("FL").toStringList().join(' '));
        if (qsettings->value("SL").typeName() == "QString")
            centerlabel2 = new QLabel(qsettings->value("SL").toString());
        else
            centerlabel2 = new QLabel(qsettings->value("SL").toStringList().join(' '));
        centerlabel2->show();
#ifndef USE_INTEL
        authorlabel = new QLabel(qsettings->value("author").toString());
#endif
    }

#ifdef USE_INTEL
    if (qsettings->contains("author") && !qsettings->value("author").toString().isEmpty()) {
        authorlabel = new QLabel(qsettings->value("author").toString());
        authorlabel->show();
    } else {
        authorlabel = new QLabel("");
        authorlabel->hide();
    }
#endif
    QSettings settings("org.ukui.style", QSettings::NativeFormat);
    QString defaultFont = settings.value("system-font", "Noto Sans CJK SC").toString();
    centerlabel1->setObjectName("centerLabel");
    centerlabel2->setObjectName("centerLabel");
    authorlabel->setObjectName("authorLabel");
    sysFont.setPointSize(CENTER1_LABEL_SIZE * m_ptToPx);
    sysFont.setFamily(defaultFont);
    centerlabel1->setFont(sysFont);
    sysFont.setPointSize(CENTER2_LABEL_SIZE * m_ptToPx);
    sysFont.setFamily(defaultFont);
    sysFont.setWeight(50);
    centerlabel2->setFont(sysFont);
    authorlabel->setFont(sysFont);
    qsettings->endGroup();

#ifdef USE_INTEL
    // 设置背景透明，qss中更改为透明不生效
    centerlabel1->setStyleSheet("QLabel{background-color: transparent;}");
    centerlabel2->setStyleSheet("QLabel{background-color: transparent;}");
    authorlabel->setStyleSheet("QLabel{background-color: transparent;}");

    m_centerWidget = new QWidget(this);
    m_centerWidget->setStyleSheet("QWidget{background:rgb(0,0,0,64);border-radius:16px}");
    QVBoxLayout *layout = new QVBoxLayout(m_centerWidget);

    //    QPushButton *line =new QPushButton(this);
    //    line->setWindowOpacity(0.08);
    //    line->setFocusPolicy(Qt::NoFocus);
    //    line->setMaximumHeight(1);

    layout->addWidget(centerlabel1);
    layout->addWidget(centerlabel2);
    //    layout->addWidget(line);
    layout->addWidget(authorlabel);

    adjustSize();
    m_centerWidget->setVisible(true);
#else
    m_centerWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(m_centerWidget);
    layout->addWidget(centerlabel1);
    layout->addWidget(centerlabel2);
    layout->addWidget(authorlabel);

    m_centerWidget->adjustSize();
    m_centerWidget->setGeometry(
        (width() - m_centerWidget->width()) / 2,
        (height() - m_centerWidget->height()) / 2,
        m_centerWidget->width(),
        m_centerWidget->height());
    m_centerWidget->setVisible(true);
#endif
}

void Screensaver::getWeatherFinish(QString city, QString cond, QString tmp)
{
    qDebug() << "getWeatherFinish";
    qDebug() << city << "," << cond << "," << tmp;

    this->m_weatherIcon->setPixmap(m_weatherManager->getWeatherIcon(cond));
    this->m_weatherArea->setText(city);

    if (!cond.isEmpty()) {
        this->m_weatherCond->show();
        this->m_weatherCond->setText("·" + cond);
    } else
        this->m_weatherCond->hide();

    if (!tmp.isEmpty()) {
        this->m_weatherTemperature->show();
        this->m_weatherTemperature->setText(tmp);
    } else
        this->m_weatherTemperature->hide();

    m_weatherLaout->adjustSize();
    m_weatherLaout->setGeometry(
        (this->width() - m_weatherLaout->width()) / 2,
        96 * (float)width() / 1920,
        m_weatherLaout->geometry().width(),
        m_weatherLaout->geometry().height());
}

void Screensaver::getVideoFormat(QString fileName)
{
    cur_video_format.clear();
    video_Duration = 0;
    video_Codetype.clear();
    cur_video_Width = 0;
    cur_video_Height = 0;

    QByteArray ba = fileName.toUtf8();
    const char *ch = ba.constData();

    int ret;
    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *avctx = NULL;

    ret = avformat_open_input(&fmt_ctx, ch, NULL, NULL);
    if (ret < 0) {
        qWarning() << "打开文件失败：" << fileName << "，错误码：" << ret;
        return;
    }

    ret = avformat_find_stream_info(fmt_ctx, NULL);
    if (ret < 0) {
        avformat_close_input(&fmt_ctx);
        return;
    }

    int video_stream_index = -1;
    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
        AVStream *stream = fmt_ctx->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }

    if (video_stream_index == -1) {
        qWarning() << "文件不是视频：" << fileName;
        avformat_close_input(&fmt_ctx);
        return;
    }

    // 获取封装格式
    cur_video_format = fmt_ctx->iformat->name;

    //通过上下文获取视频时长
    video_Duration = fmt_ctx->duration / 1000;

    // 获取视频流上下文
    AVStream *video_stream = fmt_ctx->streams[video_stream_index];

    // 获取视频编码类型（h264/h265等）
    avctx = avcodec_alloc_context3(NULL);
    ret = avcodec_parameters_to_context(avctx, video_stream->codecpar);
    if (ret >= 0) {
        video_Codetype = avcodec_get_name(avctx->codec_id);
        cur_video_Width = avctx->width;
        cur_video_Height = avctx->height;
        avcodec_free_context(&avctx);
    } else {
        qWarning() << "解析编码信息失败：" << fileName;
    }

    // 关闭格式上下文
    avformat_close_input(&fmt_ctx);
    qInfo() << "视频文件解析成功：";
}

/*
 *   检测网址是否合法，如"http://10.42.5.777777"这种非法网址就会出现一直加载的情况，导致显示空白
*/
bool Screensaver::getIsValidUrl(const QString &urlStr)
{
    QUrl url(urlStr);
    if (url.isValid()) {
        if (url.scheme() == "http" || url.scheme() == "https") {
            QString host = url.host();
            int dotCount = std::count(host.begin(), host.end(), '.');
            if (dotCount == 3) {
                QStringList parts = host.split('.');
                if (parts.size() == 4) {
                    for (const QString &part : parts) {
                        bool ok;
                        int num = part.toInt(&ok);
                        if (!ok || num < 0 || num > 255) {
                            m_saverState = SAVERERRORTYPE::URLADDRESSERROR;
                            return false;
                        }
                    }
                    return true;
                }
            } else {
                return true;
            }
        }
    }
    return false;
}

QString Screensaver::getLongFormatDate(int type)
{
    kdk_logn_dateinfo *dateInfo = kdk_system_login_lock_dateinfo(getenv("USER"));
    if (type == DATE) {
        QString date = dateInfo->date;
        QString week = dateInfo->week;
        if (dateInfo) {
            kdk_free_logn_dateinfo(dateInfo);
        }
        QLocale local;
        QString systemLang = local.name();
        if (systemLang == "vi_VN" || systemLang == "ms_MY" || systemLang == "th_TH") {
            return week + " " + date;
        } else {
            return date + " " + week;
        }
    } else if (type == TIME) {
        QString time = dateInfo->time;
        if (dateInfo) {
            kdk_free_logn_dateinfo(dateInfo);
        }
        return time;
    }
}

void Screensaver::onScreensaverDatetimeEnableChanged(bool isEnable, const QString &strUserName)
{
    if (strUserName != QString(getenv("USER"))) {
        return;
    }
    if (!m_isInitUICompleted || isEnable != m_isShowDateTime) {
        m_isShowDateTime = isEnable;
        if (isEnable) {
            if (m_widgetDatetime) {
                m_widgetDatetime->show();
            }
        } else {
            if (m_widgetDatetime) {
                m_widgetDatetime->hide();
            }
        }
    }
}

void Screensaver::initWeatherWidget()
{
    if (!m_weatherWidget) {
        m_weatherWidget = new WeatherWidget(this);
        m_weatherManager = new WeatherManager(this);
        connect(
            m_weatherManager,
            &WeatherManager::onWeatherUpdate,
            this,
            [=](QString city, QString cond, QString temperature) {
                m_weatherWidget->setCurrentWeather(temperature + "C", city + " " + cond);
                if (m_weatherThumbnailWidget) {
                    m_weatherThumbnailWidget->setCurrentWeather(temperature + "C", city + " " + cond);
                }
            });
        connect(m_weatherManager, &WeatherManager::onNetworkError, this, [=]() {
            m_weatherWidget->setErrorTip(tr("Not connected to a network"));
            if (m_weatherThumbnailWidget) {
                m_weatherThumbnailWidget->setErrorTip(tr("Not connected to a network"));
            }
        });
        connect(m_weatherManager, &WeatherManager::onGetWeatherFailed, this, [=]() {
            m_weatherWidget->setErrorTip(tr("The weather update failed"));
            if (m_weatherThumbnailWidget) {
                m_weatherThumbnailWidget->setErrorTip(tr("The weather update failed"));
            }
        });
        connect(m_weatherManager, &WeatherManager::onGetWeatherIconFinished, this, [=]() {
            QString weatherIcon = m_weatherManager->getWeatherIcon();
            QPixmap weatherStateIcon = m_weatherManager->getWeatherStateIcon();

            m_weatherWidget->setWeatherIcon(weatherIcon, weatherStateIcon);
            if (m_weatherThumbnailWidget) {
                m_weatherThumbnailWidget->setWeatherIcon(weatherIcon, weatherStateIcon);
            }
        });
    }
}

void Screensaver::initMediaControlWidget()
{
    if (isGreeterMode()) {
        return;
    }
    if (!m_widgetMediaControl) {
        QStringList musicPath = getAudioFiles(m_sMusicPath);
        m_widgetMediaControl = new QWidget(this);
        QVBoxLayout *layout = new QVBoxLayout();
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);
        if (!m_mediaControlWidget) {
            if (!m_bFollowSystem) {
                if (musicPath.isEmpty()) {
                    m_mediaControlWidget = new MediaSaverWidget(m_widgetMediaControl, false);
                    m_mediaControlWidget->initMultimediaControl();
                    m_bSaverMusic = false;
                } else {
                    m_mediaControlWidget = new MediaSaverWidget(m_widgetMediaControl, true);
                    playController::getInstance().addMusicToDatebase(musicPath);
                    playController::getInstance().setCurPlaylist(musicPath);
                    m_bSaverMusic = true;
                }
            } else {
                m_mediaControlWidget = new MediaSaverWidget(m_widgetMediaControl, false);
                m_bSaverMusic = false;
                m_mediaControlWidget->initMultimediaControl();
                connect(m_mediaControlWidget, &MediaSaverWidget::loadDbusListNameFinished, this, [=](bool success) {
                    if (!success) {
                        m_mediaControlWidget->setSaverMediaType(true);
                        m_bSaverMusic = true;
                        playController::getInstance().addMusicToDatebase(musicPath);
                        playController::getInstance().setCurPlaylist(musicPath);
                    }
                });
            }
            layout->addWidget(m_mediaControlWidget);
        }
        m_widgetMediaControl->setLayout(layout);
    }
}

void Screensaver::initCustomAlbumViewer()
{
    if (m_customAlbumViewer)
        return;
    m_customAlbumViewer = new CustomAlbumViewer(this);
    connect(m_customAlbumViewer, &CustomAlbumViewer::pixmapChanged, this, [=]() { update(); });
    if (!isScreensaver && m_sSelectPhotoPath == DEFAULT_PHOTO_PATH) {
        m_sSelectPhotoPath = LOCAL_WALLPAPER_PATH;
    }
    m_customAlbumViewer->loadImages(m_sSelectPhotoPath);
    m_customAlbumViewer->setSequentialMode((m_iPhotoSwitchMode == 0) ? true : false);
    m_customAlbumViewer->hide();
}

void Screensaver::initThumBnalListWidget()
{
    if (!isScreensaver) {
        return;
    }
    m_sidebarBtn = new StatusButton(this, SIDEBARBTN);
    m_sidebarBtn->setFixedSize(44, 136);
    m_sidebarBtn->installEventFilter(this);
    m_sidebarBtn->setIconSize(QSize(44, 44));

    m_sidebarBtn->setIcon(drawSymbolicColoredPixmap(QIcon::fromTheme("pan-end-symbolic").pixmap(44, 44), "white"));

    m_smartSaverLabel = new QLabel(this);
    sysFont.setPointSize((24 + curFontSize) * m_ptToPx);
    m_smartSaverLabel->setFont(sysFont);
    m_smartSaverLabel->setText(tr("Smart Saver"));
    m_smartSaverLabel->setStyleSheet("color: white");
    m_smartSaverLabel->setAlignment(Qt::AlignHCenter);

    m_thumBnalListWidget = new QListWidget(this);
    m_thumBnalListWidget->setFlow(QListWidget::TopToBottom);
    m_thumBnalListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_thumBnalListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QString buttonListStyle = "QListWidget{ background-color: rgba(255,255,255,0%);}"
                              "QListWidget::item{background:rgba(255,255,255,0%);}"
                              "QListWidget::item:hover{background-color:rgba(255,255,255,0%)}"
                              "QListWidget::item::selected{background-color:rgba(255,255,255,0%);}";
    m_thumBnalListWidget->setStyleSheet(buttonListStyle);
    m_thumBnalListWidget->setContentsMargins(20, 0, 0, 0);

    QWidget *defaultSaverWidget = new QWidget(m_thumBnalListWidget);
    defaultSaverWidget->setFixedSize(340, 218);
    QVBoxLayout *defaultSaverLayout = new QVBoxLayout(defaultSaverWidget);
    QListWidgetItem *defaultSaverItem = new QListWidgetItem();
    defaultSaverItem->setSizeHint(QSize(340, 218));

    QHBoxLayout *thumbnailSaverLayout = new QHBoxLayout();
    thumbnailSaverLayout->addStretch();
    m_defaulSaverWidget = new SaverThumbnailWidget(defaultSaverWidget);
    thumbnailSaverLayout->addWidget(m_defaulSaverWidget);
    thumbnailSaverLayout->addStretch();
    QString backgroundFile = configuration->getDefaultBackground();
    QString thumbnailPath = mapToThumbnailPath(backgroundFile);
    m_defaulSaverWidget->setBackground(background);
    if (centerlabel1 && centerlabel2 && authorlabel) {
        m_defaulSaverWidget->setDictumText(centerlabel1->text(), centerlabel2->text(), authorlabel->text());
    }
    connect(m_defaulSaverWidget, &SaverThumbnailWidget::defaultSaverModeClicked, this, [=]() {
        m_albunWidget->setSelectStatus(false);
        m_musicWidget->setSelectStatus(false);
        m_weatherThumbnailWidget->setSelectStatus(false);
        configuration->setSubMode(0);
        if (m_customAlbumViewer) {
            m_customAlbumViewer->stopDisplayImages();
            m_customAlbumViewer->hide();
        }
        if (m_widgetMediaControl && m_widgetMediaControl->isVisible())
            m_widgetMediaControl->hide();
        if (m_weatherWidget && m_weatherWidget->isVisible())
            m_weatherWidget->hide();
    });
    QLabel *defaultSaverLabel = new QLabel(defaultSaverWidget);
    defaultSaverLabel->setFixedSize(330, 30);
    defaultSaverLabel->setText("UKUI");
    defaultSaverLabel->setStyleSheet("color: white");
    defaultSaverLabel->setAlignment(Qt::AlignCenter);
    defaultSaverLayout->addLayout(thumbnailSaverLayout);
    defaultSaverLayout->addWidget(defaultSaverLabel, Qt::AlignHCenter);

    QWidget *weatherWidget = new QWidget(m_thumBnalListWidget);
    weatherWidget->setFixedSize(340, 218);
    QVBoxLayout *weatherLayout = new QVBoxLayout(weatherWidget);
    QListWidgetItem *weatherItem = new QListWidgetItem();
    weatherItem->setSizeHint(QSize(340, 218));
    QHBoxLayout *thumbnailWeatherLayout = new QHBoxLayout();
    thumbnailWeatherLayout->addStretch();
    m_weatherThumbnailWidget = new WeatherThumbnailWidget(weatherWidget);
    thumbnailWeatherLayout->addWidget(m_weatherThumbnailWidget);
    thumbnailWeatherLayout->addStretch();
    m_weatherThumbnailWidget->setBackground(scaleBlurPixmap(width(), height(), thumbnailPath));
    connect(m_weatherThumbnailWidget, &WeatherThumbnailWidget::weatherModeClicked, this, [=]() {
        m_albunWidget->setSelectStatus(false);
        m_defaulSaverWidget->setSelectStatus(false);
        m_musicWidget->setSelectStatus(false);
        configuration->setSubMode(1);
        if (m_customAlbumViewer) {
            m_customAlbumViewer->stopDisplayImages();
        }
    });
    QLabel *weatherLabel = new QLabel(weatherWidget);
    weatherLabel->setFixedSize(330, 30);
    weatherLabel->setText(tr("Weather"));
    weatherLabel->setStyleSheet("color: white");
    weatherLabel->setAlignment(Qt::AlignCenter);
    weatherLayout->addLayout(thumbnailWeatherLayout);
    weatherLayout->addWidget(weatherLabel, Qt::AlignHCenter);

    QWidget *musicWidget = new QWidget(m_thumBnalListWidget);
    musicWidget->setFixedSize(340, 218);
    QVBoxLayout *musicLayout = new QVBoxLayout(musicWidget);
    QListWidgetItem *musicItem = new QListWidgetItem();
    musicItem->setSizeHint(QSize(340, 218));
    QHBoxLayout *thumbnailMusicLayout = new QHBoxLayout();
    thumbnailMusicLayout->addStretch();
    m_musicWidget = new MusicThumbnailWidget(musicWidget);
    thumbnailMusicLayout->addWidget(m_musicWidget);
    thumbnailMusicLayout->addStretch();
    m_musicWidget->setBackground(scaleBlurPixmap(width(), height(), thumbnailPath));
    connect(m_musicWidget, &MusicThumbnailWidget::musicModeClicked, this, [=]() {
        m_albunWidget->setSelectStatus(false);
        m_defaulSaverWidget->setSelectStatus(false);
        m_weatherThumbnailWidget->setSelectStatus(false);
        configuration->setSubMode(2);
        if (m_customAlbumViewer) {
            m_customAlbumViewer->stopDisplayImages();
        }
    });
    QLabel *musicLabel = new QLabel(musicWidget);
    musicLabel->setFixedSize(330, 30);
    musicLabel->setText(tr("Music"));
    musicLabel->setStyleSheet("color: white");
    musicLabel->setAlignment(Qt::AlignCenter);
    musicLayout->addLayout(thumbnailMusicLayout);
    musicLayout->addWidget(musicLabel, Qt::AlignHCenter);

    QWidget *albunWidget = new QWidget(m_thumBnalListWidget);
    albunWidget->setFixedSize(340, 218);
    QVBoxLayout *albunLayout = new QVBoxLayout(albunWidget);
    QListWidgetItem *photoItem = new QListWidgetItem();
    photoItem->setSizeHint(QSize(340, 218));
    QHBoxLayout *thumbnailAlbunLayout = new QHBoxLayout();
    thumbnailAlbunLayout->addStretch();
    m_albunWidget = new AlbumThumbnailWidget(m_thumBnalListWidget);
    thumbnailAlbunLayout->addWidget(m_albunWidget);
    thumbnailAlbunLayout->addStretch();
    m_albunWidget->setBackground(scaleBlurPixmap(width(), height(), thumbnailPath));
    m_albunWidget->setDefaultPhoto(loadFromFile(thumbnailPath));
    if (m_sSelectPhotoPath == DEFAULT_PHOTO_PATH) {
        m_albunWidget->setDefaultPath(DEFAULT_PHOTO_PATH);
    } else {
        m_albunWidget->setDefaultPath(m_sSelectPhotoPath);
    }
    connect(m_albunWidget, &AlbumThumbnailWidget::albumModeClicked, this, [=]() {
        m_musicWidget->setSelectStatus(false);
        m_defaulSaverWidget->setSelectStatus(false);
        m_weatherThumbnailWidget->setSelectStatus(false);
        configuration->setSubMode(3);
    });
    QLabel *albunLabel = new QLabel(albunWidget);
    albunLabel->setFixedSize(330, 30);
    albunLabel->setText(tr("Photo Albun"));
    albunLabel->setStyleSheet("color: white");
    albunLabel->setAlignment(Qt::AlignCenter);
    albunLayout->addLayout(thumbnailAlbunLayout);
    albunLayout->addWidget(albunLabel, Qt::AlignHCenter);

    m_thumBnalListWidget->insertItem(0, defaultSaverItem);
    m_thumBnalListWidget->setItemWidget(defaultSaverItem, defaultSaverWidget);
    m_thumBnalListWidget->insertItem(1, weatherItem);
    m_thumBnalListWidget->setItemWidget(weatherItem, weatherWidget);
    m_thumBnalListWidget->insertItem(2, musicItem);
    m_thumBnalListWidget->setItemWidget(musicItem, musicWidget);
    m_thumBnalListWidget->insertItem(3, photoItem);
    m_thumBnalListWidget->setItemWidget(photoItem, albunWidget);
    m_thumBnalListWidget->hide();
    m_smartSaverLabel->hide();

    if (m_nSubMode == 0) {
        m_defaulSaverWidget->setSelectStatus(true);
    } else if (m_nSubMode == 1) {
        m_weatherThumbnailWidget->setSelectStatus(true);
    } else if (m_nSubMode == 2) {
        m_musicWidget->setSelectStatus(true);
    } else if (m_nSubMode == 3) {
        m_albunWidget->setSelectStatus(true);
    }

    m_thumBnalListWidget->installEventFilter(this);
    m_hideThumBnalWidgetTimer = new QTimer(this);
    m_hideThumBnalWidgetTimer->setSingleShot(true);
    connect(m_hideThumBnalWidgetTimer, &QTimer::timeout, this, [=]() {
        if (m_thumBnalListWidget && m_thumBnalListWidget->isVisible()) {
            m_thumBnalListWidget->hide();
            m_smartSaverLabel->hide();
            m_sidebarBtn->move(0, (height() - m_sidebarBtn->height()) / 2);
            m_sidebarBtn->setIcon(
                drawSymbolicColoredPixmap(QIcon::fromTheme("pan-end-symbolic").pixmap(44, 44), "white"));
        }
    });
}
