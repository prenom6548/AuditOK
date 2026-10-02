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
#include "screenlock.h"
#include <interface/ukcccommon.h>
using namespace ukcc;
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QDesktopServices>
#include <QProcess>
#include <QFileSystemWatcher>
#include <QTranslator>
#include <QLocale>
#include <QApplication>
#include <QTemporaryDir>
#include <QStandardPaths>
#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>

#define LOCAL_USR_PATH QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
#define LOCAL_APP_PATH QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)\
                       %QStringLiteral("/ukui-control-center/")
#define LOCAL_WALLPAPER_PATH QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)\
                             %QStringLiteral("/ukui-control-center/wallpaperData/")
#define LOCAL_WALLPAPER_HASH_PATH QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)\
                                  %QStringLiteral("/ukui-control-center/wallpaperData/HASH")

#define HTTPSTYPE "x-scheme-handler/https"
#define LOCAL_CONFIG_DIR "/.config/"
#define SYSTEM_CONFIG_DIR "/usr/share/applications/"

Screenlock::Screenlock() : mFirstLoad(true)
{
    QTranslator *translator = new QTranslator(this);
    QLocale locale;
    translator->load("/usr/share/ukui-screensaver/ukccplugins/i18n_qm/" + locale.name() + ".qm");
    QApplication::installTranslator(translator);

    pluginName = tr("Screenlock");
    pluginType = PERSONALIZED;
    screenlockInterface = new QDBusInterface(
            "org.ukui.ScreenSaver",
            "/Screenlock",
            "org.ukui.ScreenSaver.session.Screenlock",
            QDBusConnection::sessionBus(),
            this);
    m_previewPictures = screenlockInterface->property("previewWallpapers").toStringList();
    m_sourcePictures  = screenlockInterface->property("sourceWallpapers").toStringList();
    m_picInfoList.clear();
}

Screenlock::~Screenlock() {}

QString Screenlock::plugini18nName()
{
    return pluginName;
}

int Screenlock::pluginTypes()
{
    return pluginType;
}

QWidget *Screenlock::pluginUi()
{
    if (mFirstLoad) {
        mFirstLoad = false;
        screenlockui = new ScreenlockUi();
        if (screenlockInterface->isValid()) {                         // 判断服务是否存在
            QDBusMessage message = screenlockInterface->call("ping"); // 判断dbus路径是否存在
            if (message.type() == QDBusMessage::ErrorMessage
                && message.errorMessage().contains("No such object path", Qt::CaseInsensitive)) {
                qWarning() << screenlockInterface << ":" << message.errorMessage();
            } else {
                QDBusConnection::sessionBus().connect(
                    "org.ukui.ScreenSaver",
                    "/Screenlock",
                    "org.ukui.ScreenSaver.session.Screenlock",
                    "changed",
                    this,
                    SLOT(dataChanged(QString)));
                initContent();
                connectUiSignals();
                connectToServer();
            }
        } else {
            qCritical() << "org.ukui.ScreenSaver.session.Screenlock DBus error:" << screenlockInterface->lastError();
        }
    } else {
        screenlockui->resize(screenlockui->size() - QSize(1, 1));
        screenlockui->resize(screenlockui->size() + QSize(1, 1));
    }
    return screenlockui;
}

const QString Screenlock::name() const
{
    return QStringLiteral("Screenlock");
}

bool Screenlock::isShowOnHomePage() const
{
    return true;
}

QIcon Screenlock::icon() const
{
    return QIcon::fromTheme("ukui-lock-screen-symbolic");
}

bool Screenlock::isEnable() const
{
    return true;
}

void Screenlock::dataChanged(const QString &key)
{
    if (toChangeKey != key) {
        if (key == "showOnLogin") {
            initShowOnLogin();
        } else if (key == "wallpaper") {
            initWallpaper();
        }
    }
    toChangeKey = "";
}

void Screenlock::convertPixmapSlot(const QString &filename, int num) {
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

void Screenlock::loadPictureInfo() {
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

void Screenlock::loadPictureInfoSlot() {
    screenlockui->setPictures(m_previewPictures, m_sourcePictures);
}

void Screenlock::initContent()
{
    loadPictureInfo();
    if (!m_picInfoList.empty()) {
        loadPictureInfoSlot();
    } else {
        QObject::connect(&m_watcher, &QFutureWatcher<void>::finished, [=]() {
            QTimer::singleShot(300, [=]() {
                loadPictureInfoSlot();
            });
        });
    }
    initWallpaper();
    initShowOnLogin();
    initLockDatetimeEnable();
}

void Screenlock::initLockDatetimeEnable()
{
    screenlockui->setLockDatetimeEnable(screenlockInterface->property("lockDatetimeEnable").toBool());
}

void Screenlock::initShowOnLogin()
{
    screenlockui->setShowOnLogin(screenlockInterface->property("showOnLogin").toBool());
}

void Screenlock::initWallpaper()
{
    screenlockui->setPicture(screenlockInterface->property("wallpaper").toString());
}

bool Screenlock::getDefaultAppId(const char *contentType)
{
    GAppInfo* info = g_app_info_get_default_for_type(contentType, FALSE);
    if (!info) {
        qDebug() << Q_FUNC_INFO << "g_app_info_get_default_for_type is null" << contentType;
        return false;
    }

    const char* name = g_app_info_get_name(info);
    qDebug() << Q_FUNC_INFO << "getDefaultAppId" << name;

    return name == nullptr ? false : true;
}

void Screenlock::connectUiSignals()
{
    connect(screenlockui, &ScreenlockUi::showOnLoginChanged, this, [=](bool b) {
        toChangeKey = "showOnLogin";
        screenlockInterface->call("setShowOnLogin", b);
        UkccCommon::buriedSettings(
            name(),
            QString("Show picture of screenlock on screenlogin"),
            QString("settings"),
            UkccCommon::boolToString(b));
    });
    connect(screenlockui, &ScreenlockUi::lockDatetimeEnableChanged, this, [=](bool b) {
        toChangeKey = "lockDatetimeEnable";
        screenlockInterface->call("setLockDatetimeEnable", b);
        UkccCommon::buriedSettings(
            name(),
            QString("The date and time are displayed"),
            QString("settings"),
            UkccCommon::boolToString(b));
    });
    connect(screenlockui, &ScreenlockUi::pictureChanged, this, [=](QString file) {
        toChangeKey = "wallpaper";
        screenlockInterface->call("setWallpaper", file);
    });
    connect(screenlockui, &ScreenlockUi::localButtonClicked, this, [=]() { showLocalWpDialog(); });
    connect(screenlockui, &ScreenlockUi::onlineButtonClicked, this, [=]() {
        bool openUrl = getDefaultAppId(HTTPSTYPE);
        qDebug() << Q_FUNC_INFO << "openUrl" << openUrl;
        if(openUrl) {
            QDesktopServices::openUrl(QUrl(QLatin1String("https://www.ubuntukylin.com/wallpaper.html")));
        } else {
            QMessageBox *msg = new QMessageBox(screenlockui);
            msg->setIcon(QMessageBox::Icon::Critical);
            msg->setText(tr("Cannot access online images, please check the system's default browser settings."));
            msg->addButton(tr("Cancel"), QMessageBox::RejectRole);
            msg->addButton(tr("Set up the browser"), QMessageBox::AcceptRole);
            connect(msg, &QMessageBox::accepted, this, [=]() {
                QProcess::startDetached(
                    "ukui-control-center",
                    QStringList() << "-m"
                                  << "Defaultapp");
            });
            msg->exec();
        }
        UkccCommon::buriedSettings(name(), QString("onlineBtnonlineButton"), QString("clicked"));
    });
    connect(screenlockui, &ScreenlockUi::resetButtonClicked, this, [=]() {
        screenlockInterface->call("resetDefault");
        UkccCommon::buriedSettings(name(), QString("resetBtn"), QString("clicked"));
    });
    connect(screenlockui, &ScreenlockUi::toSetMonitor, this, [=]() {
        UkccCommon::buriedSettings(name(), "setBtn(Monitor Off)", QString("clicked"));
        QProcess::startDetached(
            "ukui-control-center",
            QStringList() << "-m"
                          << "Power");
    });
    connect(screenlockui, &ScreenlockUi::toSetScreensaver, this, [=]() {
        UkccCommon::buriedSettings(name(), "setBtn(Screensaver)", QString("clicked"));
        QProcess::startDetached(
            "ukui-control-center",
            QStringList() << "-m"
                          << "Screensaver");
    });
}

void Screenlock::showLocalWpDialog()
{
    QStringList filters;
    filters << tr("Wallpaper files(*.jpg *.jpeg *.bmp *.dib *.png *.jfif *.jpe *.gif *.tif *.tiff *.wdp)");
    QFileDialog fd;

    QList<QUrl> usb_list = fd.sidebarUrls();
    int sidebarNum = 8; // 最大添加U盘数，可以自己定义
    QString home_path = QDir::homePath().section("/", -1, -1);
    QString mnt = "/media/" + home_path + "/";
    QDir mntDir(mnt);
    mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList file_list = mntDir.entryInfoList();
    QList<QUrl> mntUrlList;
    for (int i = 0; i < sidebarNum && i < file_list.size(); ++i) {
        QFileInfo fi = file_list.at(i);
        mntUrlList << QUrl("file://" + fi.filePath());
    }

    QFileSystemWatcher m_fileSystemWatcher(&fd);
    m_fileSystemWatcher.addPath("/media/" + home_path + "/");
    connect(
        &m_fileSystemWatcher,
        &QFileSystemWatcher::directoryChanged,
        &fd,
        [=, &sidebarNum, &mntUrlList, &usb_list, &fd](const QString path) {
            QDir m_wmntDir(path);
            m_wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
            QFileInfoList m_wfilist = m_wmntDir.entryInfoList();
            mntUrlList.clear();
            for (int i = 0; i < sidebarNum && i < m_wfilist.size(); ++i) {
                QFileInfo m_fi = m_wfilist.at(i);
                mntUrlList << QUrl("file://" + m_fi.filePath());
            }
            fd.setSidebarUrls(usb_list + mntUrlList);
            fd.update();
        });

    connect(&fd, &QFileDialog::finished, &fd, [=, &usb_list, &fd]() { fd.setSidebarUrls(usb_list); });

    fd.setDirectory(QString(const_cast<char *>(g_get_user_special_dir(G_USER_DIRECTORY_PICTURES))));
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::List);
    fd.setNameFilters(filters);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setWindowTitle(tr("select custom wallpaper file"));
    fd.setLabelText(QFileDialog::Accept, tr("Select"));
    fd.setLabelText(QFileDialog::LookIn, tr("Position: "));
    fd.setLabelText(QFileDialog::FileName, tr("FileName: "));
    fd.setLabelText(QFileDialog::FileType, tr("FileType: "));
    fd.setLabelText(QFileDialog::Reject, tr("Cancel"));

    fd.setSidebarUrls(usb_list + mntUrlList);

    if (fd.exec() != QDialog::Accepted) {
        return;
    }
    QString selectedfile;
    selectedfile = fd.selectedFiles().first();
    screenlockInterface->call("setWallpaper", selectedfile);
    UkccCommon::buriedSettings(name(), QString("browserLocalwpBtn"), QString("clicked"));
}

void Screenlock::connectToServer()
{
    QThread *NetThread = new QThread;
    MThread *NetWorker = new MThread;
    NetWorker->moveToThread(NetThread);
    connect(NetThread, &QThread::started, NetWorker, &MThread::run);
    connect(NetWorker, &MThread::keychangedsignal, this, &Screenlock::keyChangedSlot);
    connect(NetThread, &QThread::finished, NetWorker, &MThread::deleteLater);
    NetThread->start();
}

void Screenlock::keyChangedSlot(const QString &key)
{
    if (key == "ukui-screensaver") {
        if (!bIsCloudService) {
            bIsCloudService = true;
        }
        initShowOnLogin();
        initWallpaper();
    }
}
