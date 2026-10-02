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

QTime Screensaver::m_currentTime = QTime::currentTime();

Screensaver::Screensaver(bool isscreensaver, QWidget *parent)
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
    , centerWidget(nullptr)
    , sleepTime(nullptr)
    , timer(nullptr)
    , background("")
    , autoSwitch(nullptr)
    , vboxFrame(nullptr)
    , isAutoSwitch(false)
    , flag(0)
    , hasChanged(false)
    , process(nullptr)
    , screenLabel(nullptr)
    , respondClick(false)
//   , m_weatherManager(new WeatherManager(this))
{
    // 加载翻译文件
    QString locale = QString(getenv("LANGUAGE"));
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

    initUI();
    m_background = new MBackground();

    QString backgroundFile = configuration->getDefaultBackground();
    background = loadFromFile(backgroundFile);

    QList<QLabel *> labelList = this->findChildren<QLabel *>();
    for (int i = 0; i < labelList.count(); i++) {
        labelList.at(i)->setAlignment(Qt::AlignCenter);
    }
#ifndef USE_INTEL
    updateBackgroundPath();
    startSwitchImages();

    connectSingles();
#endif

    screenLabel = new QLabel(this);
    screenLabel->setObjectName("screenLabel");
    screenLabel->setText(tr("Picture does not exist"));
    screenLabel->adjustSize();
    screenLabel->hide();
    m_strPreViewTrans = tr("View");
    qDebug() << "preview translation:" << m_strPreViewTrans;
}

Screensaver::~Screensaver() {}

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

    m_videoPath = configuration->getVideoPath();
    m_videoFormat = configuration->getVideoFormat();
    m_videoSize = configuration->getVideoSize();
    m_videoWidth = configuration->getVideoWidth();
    m_videoHeight = configuration->getVideoHeight();
}

void Screensaver::connectSingles()
{
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
}

void Screensaver::onBlurNumChanged(int num)
{
    if (m_isDynamicSaver)
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
    if (m_isDynamicSaver)
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

void Screensaver::autoSwitchChanged(bool isSwitch)
{
    if (!isCustom || m_isDynamicSaver)
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
    backgroundPath = path;
    if (!isCustom || m_isDynamicSaver)
        return;
    updateBackgroundPath(); // 更新图片路径
    stopSwitchImages();
    startSwitchImages();
}

void Screensaver::backgroundChanged(QString path)
{
    background = loadFromFile(path);
}

void Screensaver::cycleTimeChanged(int cTime)
{
    cycleTime = cTime;
    if (!isCustom || !autoSwitch || m_isDynamicSaver)
        return;
    stopSwitchImages();
    startSwitchImages();
}

void Screensaver::myTextChanged(QString text)
{
    if (!isCustom || m_isDynamicSaver)
        return;

    myText = text;

    if (textIsCenter && centerWidget) {
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
        centerWidget->adjustSize();

        centerWidget->setGeometry(
            (width() - centerWidget->width()) / 2,
            (height() - centerWidget->height()) / 2,
            centerWidget->width(),
            centerWidget->height());

        if ((height() - centerWidget->height()) / 2 < timeLayout->y() + timeLayout->height())
            centerWidget->setGeometry(
                (width() - centerWidget->width()) / 2,
                timeLayout->y() + timeLayout->height(),
                centerWidget->width(),
                centerWidget->height());
    } else {
        setRandomText();
        setRandomPos();
        if (centerWidget)
            centerWidget->hide();
    }
}

void Screensaver::showCRestTimeChanged(bool isShow)
{
    isCShowRestTime = isShow;
    if (!isCustom || m_isDynamicSaver)
        return;

    setSleeptime(isCShowRestTime);
}

void Screensaver::showURestTimeChanged(bool isShow)
{
    isUShowRestTime = isShow;
    if (isCustom || m_isDynamicSaver)
        return;

    setSleeptime(isUShowRestTime);
}

void Screensaver::textIsCenterChanged(bool isCenter)
{
    textIsCenter = isCenter;
    if (!isCustom || m_isDynamicSaver)
        return;

    if (isCenter) {
        if (myTextWidget)
            myTextWidget->hide();
        if (!centerWidget) {
            setCenterWidget();
            resize(width(), height());
        } else {
            centerWidget->show();
            myTextChanged(myText);
        }
    } else {
        if (centerWidget)
            centerWidget->hide();
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
    if (isCustom && imagePaths.count() == 0) {
        painter.setBrush(QColor("#000000"));
        if (screenLabel->isHidden()) {
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
            painter.drawPixmap(this->rect(), background, background.rect());
            QPainterPath path;
            QPainter painter1(this);
            painter1.setOpacity(0.25);
            painter1.setRenderHint(QPainter::Antialiasing); // 反锯齿;
            painter1.setClipping(true);
            painter1.setPen(Qt::transparent);
            path.addRect(this->rect());
            path.setFillRule(Qt::WindingFill);
            painter1.setBrush(QColor("#000000"));
            painter1.setPen(Qt::transparent);
            painter1.drawPath(path);
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
            if (m_rightFormat) {
                // 分辨率限制
                if (cur_video_Width <= m_videoWidth && cur_video_Height <= m_videoHeight) {
                    m_isDynamicSaver = true;
                } else {
                    qInfo() << "High video resolution!!! resolution = " << cur_video_Width << "x" << cur_video_Height;
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
        m_playerView->setFixedSize(this->size());
        m_playerItem->setSize(this->size());
        return;
    }
    float scale = 1.0;
    scale = (float)width() / 1920;
    if ((width() < 600 || height() < 400) && !isScreensaver) { // 当显示在控制面板上时，字体缩小三倍。
        if (flag == 0) {
            QList<QLabel *> labelList = this->findChildren<QLabel *>();
            for (int i = 0; i < labelList.count(); i++) {
                if (labelList.at(i) != myPreviewLabel) {
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
            if (centerWidget)
                centerWidget->adjustSize();
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

    if (centerWidget) {
        centerWidget->adjustSize();
        centerWidget->setGeometry(
            (width() - centerWidget->width()) / 2,
            (height() * 85) / 100 - (centerWidget->height()) / 2,
            centerWidget->width(),
            centerWidget->height());

        if ((height() - centerWidget->height()) / 2 < timeLayout->y() + timeLayout->height())
            centerWidget->setGeometry(
                (width() - centerWidget->width()) / 2,
                timeLayout->y() + timeLayout->height(),
                centerWidget->width(),
                centerWidget->height());
    }

    if (m_widgetNotice) {
        m_widgetNotice->setGeometry(
            (this->width() - m_widgetNotice->width()) / 2,
            (centerWidget->y() + centerWidget->height() + 20),
            m_widgetNotice->geometry().width(),
            m_widgetNotice->geometry().height());
    }
#else
    int x = (this->width() - timeLayout->geometry().width()) / 2;
    int y = 59 * scale;

    timeLayout->setGeometry(x, y, timeLayout->geometry().width(), timeLayout->geometry().height());

    if (sleepTime) {
        x = 26 * scale;
        y = this->height() - sleepTime->geometry().height() - 26 * scale;
        sleepTime->setGeometry(x, y, sleepTime->geometry().width(), sleepTime->geometry().height());
    }

    if (screenLabel) {
        screenLabel->setGeometry((width() - screenLabel->width()) / 2, y, screenLabel->width(), screenLabel->height());
    }

    if (centerWidget) {
        centerWidget->adjustSize();
        centerWidget->setGeometry(
            (width() - centerWidget->width()) / 2,
            (height() - centerWidget->height()) / 2,
            centerWidget->width(),
            centerWidget->height());

        if ((height() - centerWidget->height()) / 2 < timeLayout->y() + timeLayout->height())
            centerWidget->setGeometry(
                (width() - centerWidget->width()) / 2,
                timeLayout->y() + timeLayout->height(),
                centerWidget->width(),
                centerWidget->height());
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
    QString lang = QString(getenv("LANGUAGE"));
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
            languageFilePath = languageDirPath + "screensaver-zh_CN-ok.ini";
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
        QStringList files = dir.entryList(QDir::Files | QDir::Readable);
        for (QString file : files) {
            fileInfo.setFile(file);
            QString suffix = fileInfo.suffix();
            if (formats.contains(suffix.toUtf8()) && file.right(4) != ".ico" && file.right(4) != ".tga") {
                // qDebug() << "file:" <<file;
                imagePaths.push_back(backgroundPath + "/" + file);
            }
        }
    }
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
    setNoticeLaout();
    m_weatherManager->getWeather();
    connect(m_weatherManager, &WeatherManager::onWeatherUpdate, this, &Screensaver::getWeatherFinish);
#else
    getVideoExistence();
    if (m_isDynamicSaver) {
        m_pVideoPlayer = new VideoPlayer();
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
        setDatelayout();
        setCenterWidget();
        setRandomText();
        if (centerWidget) {
            if (textIsCenter || myText == "") {
                myTextWidget->hide();
                centerWidget->show();
            } else {
                centerWidget->hide();
                myTextWidget->show();
            }
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

    this->dateOfLocaltime = new QLabel(this);
    sysFont = qApp->font();
    sysFont.setPointSize((58 + curFontSize) * m_ptToPx);
    this->dateOfLocaltime->setFont(sysFont);
    if (timeType == 12)
        this->dateOfLocaltime->setText(getLongFormatDate(TIME));
    else
        this->dateOfLocaltime->setText(getLongFormatDate(TIME));

    this->dateOfLocaltime->setObjectName("dateOfLocaltime");
    this->dateOfLocaltime->setAlignment(Qt::AlignCenter);
    this->dateOfLocaltime->adjustSize();
    vtimeLayout->addWidget(dateOfLocaltime);

    this->dateOfDay = new QLabel(this);
    sysFont.setPointSize((16 + curFontSize) * m_ptToPx);
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
        if (timer) {
            timer->stop();
        }
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
    myPreviewLabel->move((width() - myPreviewLabel->width()) / 2, 142);
}

void Screensaver::setCenterWidget()
{
    QStringList qlist = qsettings->childGroups();
    if (qlist.count() < 1)
        return;

    QDate date = QDate::currentDate();
    int days = date.daysTo(QDate(2100, 1, 1));
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
    centerlabel1->setObjectName("centerLabel");
    centerlabel2->setObjectName("centerLabel");
    authorlabel->setObjectName("authorLabel");
    sysFont.setPointSize((36 + curFontSize) * m_ptToPx);
    centerlabel1->setFont(sysFont);
    centerlabel2->setFont(sysFont);
    sysFont.setPointSize((28 + curFontSize) * m_ptToPx);
    authorlabel->setFont(sysFont);

    qsettings->endGroup();

#ifdef USE_INTEL
    // 设置背景透明，qss中更改为透明不生效
    centerlabel1->setStyleSheet("QLabel{background-color: transparent;}");
    centerlabel2->setStyleSheet("QLabel{background-color: transparent;}");
    authorlabel->setStyleSheet("QLabel{background-color: transparent;}");

    centerWidget = new QWidget(this);
    centerWidget->setStyleSheet("QWidget{background:rgb(0,0,0,64);border-radius:16px}");
    QVBoxLayout *layout = new QVBoxLayout(centerWidget);

    //    QPushButton *line =new QPushButton(this);
    //    line->setWindowOpacity(0.08);
    //    line->setFocusPolicy(Qt::NoFocus);
    //    line->setMaximumHeight(1);

    layout->addWidget(centerlabel1);
    layout->addWidget(centerlabel2);
    //    layout->addWidget(line);
    layout->addWidget(authorlabel);

    adjustSize();
    centerWidget->setVisible(true);
#else
    centerWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centerWidget);
    layout->addWidget(centerlabel1);
    layout->addWidget(centerlabel2);

    if (!isCustom) {
        QPushButton *line = new QPushButton(this);
        line->setWindowOpacity(0.08);
        line->setFocusPolicy(Qt::NoFocus);
        line->setMaximumHeight(1);
        layout->addWidget(line);
    }

    layout->addWidget(authorlabel);

    centerWidget->adjustSize();
    centerWidget->setGeometry(
        (width() - centerWidget->width()) / 2,
        (height() - centerWidget->height()) / 2,
        centerWidget->width(),
        centerWidget->height());
    centerWidget->setVisible(true);
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

QPixmap Screensaver::loadFromFile(QString strPath)
{
    QImageReader reader;
    reader.setFileName(strPath);
    reader.setAutoTransform(true);
    reader.setDecideFormatFromContent(true);
    return QPixmap::fromImageReader(&reader);
}

void Screensaver::getVideoFormat(QString fileName)
{
    char *ch;
    // 用toLiatin1()解析在有中文路径时回乱码
    QByteArray ba = fileName.toUtf8();
    ch = ba.data();

    int ret;
    int streams;
    char buf[256];
    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *avctx = NULL;
    AVInputFormat *fmt = NULL;
    ret = avformat_open_input(&fmt_ctx, ch, fmt, NULL);
    if (ret < 0) {
        return;
    }

    // 获取封装格式
    cur_video_format = fmt_ctx->iformat->name;

    // 通过上下文获取视频时长
    video_Duration = fmt_ctx->duration / 1000;

    // 获取视频帧率fps
    const AVStream *stream = fmt_ctx->streams[0];
    double fps = av_q2d(stream->avg_frame_rate);

    // 获取视频编码类型(h264/h265)
    avctx = avcodec_alloc_context3(NULL);
    ret = avcodec_parameters_to_context(avctx, stream->codecpar);
    video_Codetype = avcodec_get_name(avctx->codec_id);

    // 获取视频的分辨率
    cur_video_Width = (avctx->width >= avctx->height ? avctx->width : avctx->height);
    cur_video_Height = (avctx->width >= avctx->height ? avctx->height : avctx->width);
    qDebug() << "video_Codetype = " << video_Codetype;
    avcodec_free_context(&avctx);

    // 关闭上下文
    avformat_close_input(&fmt_ctx);
    return;
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
        return date + " " + week;
    } else if (type == TIME) {
        QString time = dateInfo->time;
        if (dateInfo) {
            kdk_free_logn_dateinfo(dateInfo);
        }
        return time;
    }
}
