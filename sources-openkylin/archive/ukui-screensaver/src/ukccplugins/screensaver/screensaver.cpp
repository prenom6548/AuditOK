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
#include "screensaver.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QTranslator>
#include <QLocale>
#include <QApplication>

#define PREENTRY_TIME "preentryTime"
#define SCREENSAVER_TYPE "screensaverType"
#define SHOWTIME_UKUI "showBreakTimeUkui"
#define SHOWTIME_CUSTOM "showBreakTimeCustom"
#define SCREENLOCK_ENABLED "screenLockEnabled"
#define CUSTOM_PATH "customPath"
#define SWITCH_RANDOW "switchRandom"
#define CYCLE_TIME "cycleTime"
#define CUSTOM_TEXT "customText"
#define CUSTOM_TEXT_CENTERED "customTextCentered"
#define SUB_MODE "subMode"
#define FOLLOW_SYSTEM "followSystem"
#define MUSIC_PATH "musicPath"
#define PHOTO_PATH "photoPath"
#define PHOTO_SELECT_PATH "photoSelectPath"
#define PHOTO_REMOVE_PATH "photoRemovePath"
#define PHOTO_CYCLE_MODE "photoCycleMode"
#define PHOTO_CYCLE_INTERVAL "photoCycleInterval"
#define WEATHER_DATA "weatherData"
#define BACKGROUND "background"
#define DEFAULT_PHOTO_PATH "/usr/share/backgrounds"
#define LOCAL_WALLPAPER_PATH (QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/ukui-control-center/wallpaperData/"))
#define SAVER_DATETIME_ENADLE "saverDatetimeEnable"
#define VIDEO_FORMAT "videoFormat"

Screensaver::Screensaver() : mFirstLoad(true)
{
    QTranslator *translator = new QTranslator(this);
    QLocale locale;
    translator->load("/usr/share/ukui-screensaver/ukccplugins/i18n_qm/" + locale.name() + ".qm");
    QApplication::installTranslator(translator);

    pluginName = tr("Screensaver");
    pluginType = PERSONALIZED;
    // QtConcurrent::run([=] { preInitialize(); });
    screensaverInterface = new QDBusInterface(
        "org.ukui.ScreenSaver",
        "/Screensaver",
        "org.ukui.ScreenSaver.session.Screensaver",
        QDBusConnection::sessionBus(),
        this);
    if (!screensaverInterface->isValid()) {
        qCritical() << "org.ukui.ScreenSaver.session.ScreenSaver DBus error:" << screensaverInterface->lastError();
        return;
    }
    QDBusMessage message = screensaverInterface->call("ping"); // 判断dbus路径是否存在
    if (message.type() == QDBusMessage::ErrorMessage
        && message.errorMessage().contains("No such object path", Qt::CaseInsensitive)) {
        qCritical() << screensaverInterface << ":" << message.errorMessage();
    }
}

Screensaver::~Screensaver()
{
    if (!mFirstLoad) {
        screensaverUi->deleteLater();
        screensaverUi = nullptr;
    }
}

QString Screensaver::plugini18nName()
{
    return pluginName;
}

int Screensaver::pluginTypes()
{
    return pluginType;
}

void Screensaver::preInitialize()
{
    QPluginLoader pluginLoader("/usr/lib/ukui-screensaver/libscreensaver-default.so");
    pluginLoader.load();
    QObject *plugin = pluginLoader.instance();
    screensaverPtr = nullptr;
    if (plugin) {
        screensaverPtr = std::unique_ptr<ScreensaverPlugin>(qobject_cast<ScreensaverPlugin *>(plugin));
    } else {
        qWarning() << "pluginLoader '/usr/lib/ukui-screensaver/libscreensaver-default.so' failed";
    }
    m_previewPictures = screensaverInterface->property("previewWallpapers").toStringList();
    m_sourcePictures  = screensaverInterface->property("sourceWallpapers").toStringList();
    m_picInfoList.clear();
}

QWidget *Screensaver::pluginUi()
{
    if (mFirstLoad) {
        preInitialize();
        mFirstLoad = false;
        loadPictureInfo();
        screensaverUi = new ScreensaverUi;
        initOptions();
        initContent();
        connectUiSignals();
        connectToServer();
    } else {
        QTimer::singleShot(0, [&]{
            initPreview();
        });
    }
    return screensaverUi;
}

const QString Screensaver::name() const
{
    return QStringLiteral("Screensaver");
}

void Screensaver::plugin_leave()
{
    if (screensaverUi) {
        screensaverUi->closePreview();
    }
    return;
}

bool Screensaver::isShowOnHomePage() const
{
    return true;
}

QIcon Screensaver::icon() const
{
    return QIcon::fromTheme("ukui-screensaver-symbolic");
}

bool Screensaver::isEnable() const
{
    return true;
}

void Screensaver::dataChanged(const QString &key)
{
    if (key != toChangeKey) {
        if (key == PREENTRY_TIME) {
            initIdleTime();
        } else if (key == SCREENSAVER_TYPE) {
            initScreensaver();
            screensaverUi->customizeNeedHide(checkSourcePathNeedHide());
        } else if (key == CUSTOM_PATH) {
            initCustomSource();
            screensaverUi->customizeNeedHide(checkSourcePathNeedHide());
        } else if (key == SWITCH_RANDOW) {
            initSwicthMode();
        } else if (key == CYCLE_TIME) {
            initSwitchTime();
        } else if (key == CUSTOM_TEXT) {
            initCustomText();
        } else if (key == CUSTOM_TEXT_CENTERED) {
            initTextPosition();
        } else if (key == SHOWTIME_UKUI) {
            initShowBreakTimeUkui();
        } else if (key == SHOWTIME_CUSTOM) {
            initShowBreakTimeCustom();
        } else if (key == SCREENLOCK_ENABLED) {
            initLockscreen();
        } else if (key == BACKGROUND) {
            initBackground();
        } else if (key == WEATHER_DATA) {
            initWeatherLocal();
        } else if (key == SUB_MODE) {
            initSubMode();
        } else if (key == FOLLOW_SYSTEM) {
            initPlaySettings();
        } else if (key == MUSIC_PATH) {
            initMusicPath();
        } else if (key == PHOTO_PATH) {
            initPhothPath();
        } else if (key == PHOTO_SELECT_PATH) {
            initPhothSelectPath();
        } else if (key == PHOTO_CYCLE_MODE) {
            initPhotoCycleMode();
        } else if (key == PHOTO_CYCLE_INTERVAL) {
            initPhotoSwitchTime();
        } else if (key == SAVER_DATETIME_ENADLE) {
            initSaverDatetimeEnable();
        }
    }
    toChangeKey = "";
}

void Screensaver::initOptions()
{
    QStringList textList = {};
    QList<int> dataList = {};
    QStringList dataList2 = {};

    textList.clear();
    textList << tr("5min") << tr("10min") << tr("15min") << tr("30min") << tr("1hour") << tr("Never");

    dataList.clear();
    dataList << 5 << 10 << 15 << 30 << 60 << -1;

#ifdef OPENKYLIN
    textList.insert(0, tr("1min"));
    dataList.insert(0, 1);
#endif
    screensaverUi->setIdleTimeOptions(textList, dataList);

    textList.clear();
    textList << tr("UKUI") << tr("Customize");
    dataList2.clear();
    dataList2 << "ukui"
              << "custom";
    screensaverUi->setScreensaverOptions(textList, dataList2);

    textList.clear();
    textList << tr("1min") << tr("5min") << tr("10min") << tr("30min");
    dataList.clear();
    dataList << 60 << 300 << 600 << 1800;
    screensaverUi->setSwitchTimeOptions(textList, dataList);

    textList.clear();
    textList << tr("1min") << tr("5min") << tr("10min") << tr("30min");
    dataList.clear();
    dataList << 60 << 300 << 600 << 1800;
    screensaverUi->setPhotoSwitchTimeOptions(textList, dataList);
}

void Screensaver::initContent()
{
    initIdleTime();
    initScreensaver();
    initCustomSource();
    initSwicthMode();
    initSwitchTime();
    initPhotoSwitchTime();
    initCustomText();
    initTextPosition();
    initShowBreakTimeUkui();
    initShowBreakTimeCustom();
    initLockscreen();
    initSubMode();
    initPlaySettings();
    initMusicPath();
    initPhothPath();
    initPhotoCycleMode();
    initWeatherLocal();
    initBackground();
    initPhothSelectPath();
    initSaverDatetimeEnable();
    screensaverUi->customizeNeedHide(checkSourcePathNeedHide());
}

void Screensaver::convertPixmapSlot(const QString &filename, int num) {
    QPixmap qPixmap = QPixmap(filename);
    if (qPixmap.isNull()){
        QFile file(filename);
        qint64 fileSize = file.size();
        if (fileSize <= 100000000 && file.open(QIODevice::ReadOnly)){
            qPixmap.loadFromData(file.readAll());
            file.close();
        }
    }
    QSize IMAGE_SIZE(166, 110);
    QPixmap pixmap = qPixmap.scaled(IMAGE_SIZE);
    QString tmpFile = filename;
    tmpFile.replace("/", "-");
    QFile wallpaperFile;
    wallpaperFile.setFileName(LOCAL_WALLPAPER_PATH + QString::number(num) + tmpFile);
    QFileInfo fileinfo = QFileInfo(tmpFile);
    pixmap.save(&wallpaperFile, fileinfo.suffix().toUtf8().data());
}

void Screensaver::loadPictureInfo() {
    if (m_previewPictures.size() != m_sourcePictures.size()) {
        qWarning() << "picturePathList.size():" << m_previewPictures.size() << " != sourcePathList.size():" << m_sourcePictures.size();
        return;
    }

    QDir dir(LOCAL_WALLPAPER_PATH);
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs);
    qDebug() << Q_FUNC_INFO << "fileInfoList size" << fileInfoList.size() << "m_sourcePictures size" << m_sourcePictures.size();
    if ((fileInfoList.size() - 1) == m_sourcePictures.size()) {
        qDebug() << Q_FUNC_INFO << "equal file num， skip load";
        m_picInfoList.clear();
        for (int i = 0; i < m_previewPictures.size(); ++i) {
            m_picInfoList.append(m_previewPictures.at(i));
        }
        return;
    }
    int num = 0;
    for (QString filename : m_sourcePictures) {
        m_watcher.setFuture(QtConcurrent::run([=, filename, num]() {
            convertPixmapSlot(filename, num);
        }));
        num++;
    }
}

void Screensaver::initPreview()
{
    if (screensaverPtr) {
        qInfo() << "screensaverPtr can be used";
        QWidget *widget = screensaverPtr->createWidget(false, nullptr, screensaverUi);
        widget->show();
        screensaverUi->updatePreview(widget);
    } else {
        qInfo() << "screensaverPtr can not be used";
    }
}

void Screensaver::initIdleTime()
{
    int time = screensaverInterface->property(PREENTRY_TIME).toInt();
    screensaverUi->setIdleTime(time);
}

void Screensaver::initScreensaver()
{
    QString type = screensaverInterface->property(SCREENSAVER_TYPE).toString();
    screensaverUi->setScreensaver(type);
    initPreview();
}

void Screensaver::initCustomSource()
{
    QString path = screensaverInterface->property(CUSTOM_PATH).toString();
    screensaverUi->setSourcePath(path);
}

void Screensaver::initSwicthMode()
{
    bool random = screensaverInterface->property(SWITCH_RANDOW).toBool();
    screensaverUi->setSwitchRandom(random);
}

void Screensaver::initSwitchTime()
{
    int time = screensaverInterface->property(CYCLE_TIME).toInt();
    screensaverUi->setSwitchTime(time);
}
void Screensaver::initPhotoSwitchTime()
{
    int time = screensaverInterface->property(PHOTO_CYCLE_INTERVAL).toInt();
    screensaverUi->setPhotoSwitchTime(time);
}


void Screensaver::initCustomText()
{
    QString text = screensaverInterface->property(CUSTOM_TEXT).toString();
    screensaverUi->setCustomText(text);
}

void Screensaver::initTextPosition()
{
    bool centered = screensaverInterface->property(CUSTOM_TEXT_CENTERED).toBool();
    screensaverUi->setTextCentered(centered);
}

void Screensaver::initShowBreakTimeUkui()
{
    bool b = screensaverInterface->property(SHOWTIME_UKUI).toBool();
    screensaverUi->setShowUkuiTime(b);
}

void Screensaver::initShowBreakTimeCustom()
{
    bool b = screensaverInterface->property(SHOWTIME_CUSTOM).toBool();
    screensaverUi->setShowCustomTime(b);
}

void Screensaver::initLockscreen()
{
    bool b = screensaverInterface->property(SCREENLOCK_ENABLED).toBool();
    screensaverUi->setLockscreen(b);
}

void Screensaver::initSubMode()
{
    int b = screensaverInterface->property(SUB_MODE).toInt();
    screensaverUi->setSubMode(b);
}

void Screensaver::initPlaySettings()
{
    bool b = screensaverInterface->property(FOLLOW_SYSTEM).toBool();
    screensaverUi->setMusicFollowSystem(b);
}

void Screensaver::initMusicPath()
{
    QString path = screensaverInterface->property(MUSIC_PATH).toString();
    screensaverUi->setMusicPath(path);
}

void Screensaver::initPhothPath()
{
    QStringList b = screensaverInterface->property(PHOTO_PATH).toStringList();
    screensaverUi->setPhotoPath(b);
}

void Screensaver::initPhothSelectPath()
{
    QString b = screensaverInterface->property(PHOTO_SELECT_PATH).toString();
    if (b.isEmpty()) {
        b = DEFAULT_PHOTO_PATH;
    }
    screensaverUi->setPhotoSelectPath(b);
}

void Screensaver::initPhotoCycleMode()
{
    int b = screensaverInterface->property(PHOTO_CYCLE_MODE).toInt();
    screensaverUi->setPhotoCycleMode(b);
}

void Screensaver::initWeatherLocal()
{
    QString weather = screensaverInterface->property(WEATHER_DATA).toString();
    QStringList weatherInfoList = weather.split(",");
    if (weatherInfoList.count() > 3) {
        screensaverUi->setCurrentPosition(weatherInfoList.at(2));
    } else {
        qInfo() << "weather error!!!!" << weatherInfoList;
    }
}

void Screensaver::initBackground()
{
    QString background = screensaverInterface->property(BACKGROUND).toString();
    screensaverUi->setThumbnailBackground(background);
}

void Screensaver::initSaverDatetimeEnable()
{
    bool saverDateTimeEnable = screensaverInterface->property(SAVER_DATETIME_ENADLE).toBool();
    screensaverUi->setSaverDatetimeEnable(saverDateTimeEnable);
}

bool Screensaver::checkSourcePathNeedHide() {
    bool needHide = false;

    QStringList videoFormat;
    QString videoFormatStr = screensaverInterface->property(VIDEO_FORMAT).toString();
    videoFormat = videoFormatStr.split(",");

    QString sourcePathText = screensaverInterface->property(CUSTOM_PATH).toString();
    QFileInfo fileInfo(sourcePathText);
    //视频文件是否存在
    if (fileInfo.isFile()) {
        QStringList sourcePathList = sourcePathText.split(".");
        if (sourcePathList.size() >= 2) {
            QString suffix = sourcePathList.at(sourcePathList.size() - 1);
            qDebug() << Q_FUNC_INFO << "videoFormat" << videoFormat << suffix;
            if (videoFormat.contains(suffix)) {
                needHide = true;
                qDebug() << Q_FUNC_INFO << "sourcePathText hit videoFormat" << sourcePathText;
            }
        }
    }

    if (!needHide) {
        QUrl url(sourcePathText);
        qDebug() << Q_FUNC_INFO << "sourcePathText" << sourcePathText;
        if (url.isValid() && (url.scheme() == "http" || url.scheme() == "https")) {
            needHide = true;
        }
    }

    if (!needHide) {
        qDebug() << Q_FUNC_INFO << "no need hide, sourcePathText" << sourcePathText;
        return false;
    }

    return true;
}

void Screensaver::connectUiSignals()
{
    QDBusConnection::sessionBus().connect(
        "org.ukui.ScreenSaver",
        "/Screensaver",
        "org.ukui.ScreenSaver.session.Screensaver",
        "changed",
        this,
        SLOT(dataChanged(QString)));
    connect(screensaverUi, &ScreensaverUi::idleTimeChanged, this, [=](int time) {
        toChangeKey = PREENTRY_TIME;
        screensaverInterface->call("setPreentryTime", time);
        UkccCommon::buriedSettings(name(), QString("Idle time"), QString("select"), QString::number(time));
    });
    connect(screensaverUi, &ScreensaverUi::screensaverChanged, this, [=](QString type) {
        toChangeKey = SCREENSAVER_TYPE;
        screensaverInterface->call("setScreensaverType", type);
        UkccCommon::buriedSettings(name(), QString("Screensaver program"), QString("select"), type);
    });
    connect(screensaverUi, &ScreensaverUi::customPathChanged, this, [=](QString path) {
        toChangeKey = CUSTOM_PATH;
        screensaverInterface->call("setCustomPath", path);
        UkccCommon::buriedSettings(name(), QString("Screensaver source Select"), QString("select"), path);
    });
    connect(screensaverUi, &ScreensaverUi::switchRandomChanged, this, [=](bool b) {
        toChangeKey = SWITCH_RANDOW;
        screensaverInterface->call("setSwitchRandom", b);
        UkccCommon::buriedSettings(name(), QString("Random switching"), QString("settings"), b ? "Random" : "Ordinal");
    });
    connect(screensaverUi, &ScreensaverUi::switchTimeChanged, this, [=](int time) {
        toChangeKey = CYCLE_TIME;
        screensaverInterface->call("setCycleTime", time);
        UkccCommon::buriedSettings(name(), QString("Switching time"), QString("select"), QString::number(time));
    });
    connect(screensaverUi, &ScreensaverUi::photoSwitchTimeChanged, this, [=](int time) {
        toChangeKey = PHOTO_CYCLE_INTERVAL;
        screensaverInterface->call("setPhotoCycleInterval", time);
        UkccCommon::buriedSettings(name(), QString("Photo switching time"), QString("select"), QString::number(time));
    });
    connect(screensaverUi, &ScreensaverUi::customTextChanged, this, [=](QString text) {
        toChangeKey = CUSTOM_TEXT;
        screensaverInterface->call("setCustomText", text);
        UkccCommon::buriedSettings(name(), QString("Text(up to 30 characters):"), QString("settings"), text);
    });
    connect(screensaverUi, &ScreensaverUi::textCenteredChanged, this, [=](bool b) {
        toChangeKey = CUSTOM_TEXT_CENTERED;
        screensaverInterface->call("setCustomTextCentered", b);
        UkccCommon::buriedSettings(
            name(), QString("Text position"), QString("settings"), b ? "Centered" : "Randow(Bubble text)");
    });
    connect(screensaverUi, &ScreensaverUi::showUkuiTimeChanged, this, [=](bool b) {
        toChangeKey = SHOWTIME_UKUI;
        screensaverInterface->call("setShowBreakTimeUkui", b);
        UkccCommon::buriedSettings(
            name(), QString("custom Show rest time"), QString("settings"), UkccCommon::boolToString(b));
    });
    connect(screensaverUi, &ScreensaverUi::showCustomChanged, this, [=](bool b) {
        toChangeKey = SHOWTIME_CUSTOM;
        screensaverInterface->call("setShowBreakTimeCustom", b);
        UkccCommon::buriedSettings(
            name(), QString("ukui Show rest time"), QString("settings"), UkccCommon::boolToString(b));
    });
    connect(screensaverUi, &ScreensaverUi::lockscreenChanged, this, [=](bool b) {
        toChangeKey = SCREENLOCK_ENABLED;
        screensaverInterface->call("setScreenLockEnabled", b);
        UkccCommon::buriedSettings(
            name(), QString("Lock screen when screensaver boot"), QString("settings"), UkccCommon::boolToString(b));
    });
    connect(screensaverUi, &ScreensaverUi::saverDatetimeEnableChanged, this, [=](bool b) {
        toChangeKey = SAVER_DATETIME_ENADLE;
        screensaverInterface->call("setSaverDatetimeEnable", b);
        UkccCommon::buriedSettings(
            name(), QString("The date and time are displayedt"), QString("settings"), UkccCommon::boolToString(b));
    });

    // 如果在dbus端监听gsetting的改变信号，再发送changed信号，此时再去initPreview可能会异常
    // 因为界面端程序此时去读取gsetting的值可能还是修改前的值，该问题应该是qgsetting的设计逻辑问题
    // 因此这里去监听gsetting的改变而不是dbus的changed
    QGSettings *mScreensaverGsettings = nullptr;
    if (!mScreensaverGsettings) {
        const QByteArray id("org.ukui.screensaver");
        if (QGSettings::isSchemaInstalled(id)) {
            mScreensaverGsettings = new QGSettings(id, QByteArray(), this);
        }
        connect(mScreensaverGsettings, &QGSettings::changed, this, [=](QString key) {
            if (key == "mode") {
                initPreview();
            }
        });
    }
    connect(screensaverUi, &ScreensaverUi::subModeChanged, this, [=](int mode) {
        toChangeKey = SUB_MODE;
        screensaverInterface->call("setSubMode", mode);
        UkccCommon::buriedSettings(
            name(), QString("ScreenSaver subMode changed"), QString("settings"), UkccCommon::boolToString(mode));
    });
    connect(screensaverUi, &ScreensaverUi::musicFollowSystemChanged, this, [=](bool b) {
        toChangeKey = FOLLOW_SYSTEM;
        screensaverInterface->call("setMusicFollowSystem", b);
        UkccCommon::buriedSettings(name(), QString("Screensaver music follow system changed"), QString("settings"), UkccCommon::boolToString(b));
    });
    connect(screensaverUi, &ScreensaverUi::musicPathChanged, this, [=](QString path) {
        toChangeKey = MUSIC_PATH;
        screensaverInterface->call("setMusicPath", path);
        UkccCommon::buriedSettings(name(), QString("Screensaver music path Select"), QString("select"), path);
    });
    connect(screensaverUi, &ScreensaverUi::photoCycleModeChanged, this, [=](int mode) {
        toChangeKey = PHOTO_CYCLE_MODE;
        screensaverInterface->call("setPhotoCycleMode", mode);
        UkccCommon::buriedSettings(
            name(), QString("ScreenSaver photo cucle mode changed"), QString("settings"), UkccCommon::boolToString(mode));
    });
    connect(screensaverUi, &ScreensaverUi::photoSourceChanged, this, [=](QString path) {
        toChangeKey = PHOTO_PATH;
        screensaverInterface->call("setPhotoCustomPath", path);
        UkccCommon::buriedSettings(name(), QString("Screensaver photo path changed"), QString("select"), path);
        initPhothPath();
    });
    connect(screensaverUi, &ScreensaverUi::photoSelectPathChanged, this, [=](QString path) {
        toChangeKey = PHOTO_SELECT_PATH;
        screensaverInterface->call("setSelectPhotoPath", path);
        UkccCommon::buriedSettings(name(), QString("Screensaver select photo path changed"), QString("select"), path);
        initPhothSelectPath();
    });
    connect(screensaverUi, &ScreensaverUi::removePhotoPathClicked, this, [=](QString path) {
        toChangeKey = PHOTO_REMOVE_PATH;
        screensaverInterface->call("removePhotoPath", path);
        UkccCommon::buriedSettings(name(), QString("Screensaver remove photo path"), QString("remove"), path);
        initPhothPath();
        initPhothSelectPath();
    });
    connect(screensaverUi, &ScreensaverUi::openFileFolderClicked, this, [=](QString path) {
        screensaverInterface->call("openFileFolder", path);
        UkccCommon::buriedSettings(name(), QString("Screensaver open photo path folder"), QString("open"), path);
    });
}

void Screensaver::connectToServer()
{
    QThread *NetThread = new QThread;
    MThread *NetWorker = new MThread;
    NetWorker->moveToThread(NetThread);
    connect(NetThread, &QThread::started, NetWorker, &MThread::run);
    connect(NetWorker, &MThread::keychangedsignal, this, &Screensaver::keyChangedSlot);
    connect(NetThread, &QThread::finished, NetWorker, &MThread::deleteLater);
    NetThread->start();
}

void Screensaver::keyChangedSlot(const QString &key)
{
    if (key == "ukui-screensaver") {
        initContent();
    }
}
