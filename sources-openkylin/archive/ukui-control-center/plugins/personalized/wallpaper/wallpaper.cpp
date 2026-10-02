/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "wallpaper.h"
#include <QDesktopServices>
#include <QFileInfo>
#include <QProcess>
#include <QFileSystemWatcher>
#include <QtConcurrent/QtConcurrent>
#include <QObject>
#include <QPixmap>
#include <QImageReader>

#include <QStandardPaths>
#include <QtXml>
#include <QThread>
#include <kysdk/applications/accessinfohelper.h>
#include <QMessageBox>
#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>

#define BROWSERTYPE "x-scheme-handler/http"
#define LOCAL_CONFIG_DIR "/.config/"
#define SYSTEM_CONFIG_DIR "/usr/share/applications/"

#define LOCAL_USR_PATH QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
#define LOCAL_APP_PATH QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)\
                       %QStringLiteral("/ukui-control-center/")
#define LOCAL_WALLPAPER_PATH QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)\
                             %QStringLiteral("/ukui-control-center/wallpaperData/")
#define LOCAL_WALLPAPER_HASH_PATH QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)\
                                  %QStringLiteral("/ukui-control-center/wallpaperData/HASH")

// 加载图片（支持 .dib 格式）
static QPixmap loadPixmapWithDibSupport(const QString &filename)
{
    QPixmap pixmap(filename);

    // 如果加载失败且是 .dib 格式，尝试用 BMP 格式读取
    if (pixmap.isNull() && filename.toLower().endsWith(".dib")) {
        QImageReader reader(filename, QByteArrayLiteral("bmp"));
        if (reader.canRead()) {
            QImage image = reader.read();
            if (!image.isNull()) {
                pixmap = QPixmap::fromImage(image);
            }
        }
    }

    // 如果仍然失败，尝试读取文件数据
    if (pixmap.isNull()) {
        QFile file(filename);
        if (file.size() <= 100000000 && file.open(QIODevice::ReadOnly)) {
            pixmap.loadFromData(file.readAll());
            file.close();
        }
    }

    return pixmap;
}

Wallpaper::Wallpaper()
    : mFirstLoad(true)
{
    pluginName = tr("Background");
    pluginType = PERSONALIZED;
    wallpaperInterface = new QDBusInterface("org.ukui.ukcc.session",
                                           "/Wallpaper",
                                           "org.ukui.ukcc.session.Wallpaper",
                                           QDBusConnection::sessionBus(), this);
    if (!wallpaperInterface->isValid()) {
        qCritical() << "org.ukui.ukcc.session.Wallpaper DBus error:" << wallpaperInterface->lastError();
        return;
    }
    m_previewPictures = wallpaperInterface->property("previewWallpapers").toStringList();
    m_sourcePictures  = wallpaperInterface->property("sourceWallpapers").toStringList();
    m_picInfoList.clear();
}

Wallpaper::~Wallpaper()
{
    if (wallpaperUi) {
        wallpaperUi->deleteLater();
        wallpaperUi = nullptr;
    }
}

QString Wallpaper::plugini18nName() {
    return pluginName;
}

int Wallpaper::pluginTypes() {
    return pluginType;
}

void Wallpaper::loadPictureInfo() {
    if (m_previewPictures.size() != m_sourcePictures.size()) {
        qWarning() << "picturePathList.size():" << m_previewPictures.size() << " != sourcePathList.size():" << m_sourcePictures.size();
        return;
    }
    QDir dir(LOCAL_WALLPAPER_PATH);
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs);
    if ((fileInfoList.size() - 1) == m_sourcePictures.size()) {
        qDebug() << Q_FUNC_INFO << "equal file num， skip load";
        m_picInfoList.clear();
        for (int i = 0; i < m_previewPictures.size(); ++i) {
            PictureInfo* pictureInfo = new PictureInfo();
            if (pictureInfo == nullptr)
                continue;
            pictureInfo->pixmap.load(m_previewPictures.at(i));
            pictureInfo->fileName = m_sourcePictures.at(i);
            m_picInfoList.append(pictureInfo);
        }
        return;
    }
    m_watcher.setFuture(QtConcurrent::filtered(m_sourcePictures, [this](QString filename){
        return this->convertPixmapSlot(filename);
    }));
}

bool Wallpaper::convertPixmapSlot(const QString &filename) {
    QPixmap qPixmap = loadPixmapWithDibSupport(filename);
    QSize IMAGE_SIZE(166, 110);
    QPixmap pixmap = qPixmap.scaled(IMAGE_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QString tmpFile = filename;
    tmpFile.replace("/", "-");
    QFile wallpaperFile;
    int index = m_sourcePictures.indexOf(filename);
    wallpaperFile.setFileName(LOCAL_WALLPAPER_PATH + QString::number(index) + tmpFile);
    qDebug() << "save filename" << index << filename;
    QFileInfo fileinfo = QFileInfo(tmpFile);
    pixmap.save(&wallpaperFile, fileinfo.suffix().toUtf8().data());
    return true;
}

QWidget *Wallpaper::pluginUi() {
    if (mFirstLoad) {
        mFirstLoad = false;
        wallpaperUi        = new WallpaperUi;
        initContent();
    } else {
        wallpaperUi->resize(wallpaperUi->size() - QSize(1, 1));
        wallpaperUi->resize(wallpaperUi->size() + QSize(1, 1));
    }
    return wallpaperUi;
}

const QString Wallpaper::name() const {

    return QStringLiteral("Wallpaper");
}

bool Wallpaper::isShowOnHomePage() const
{
    return true;
}

QIcon Wallpaper::icon() const
{
    return QIcon::fromTheme("folder-pictures-symbolic");
}

bool Wallpaper::isEnable() const
{
    return true;
}

void Wallpaper::dataChanged(const QString &key)
{
    if (toChangeKey != key) {
        if (key == "displayType") {
            initType();
        } else if (key == "wallpaper") {
            initMode();
        }
    }
    toChangeKey = "";
}

void Wallpaper::initContent()
{
    loadPictureInfo();
    initModes();
    initTypes();
    initMode();
    initType();
    initPictures();
    connectUiSignals();
}

void Wallpaper::initModes()
{
    QStringList modes = {tr("Picture"), tr("Color")};
    QStringList modeData = {"picture", "color"};
    wallpaperUi->setWallpaperModes(modes, modeData);
}

void Wallpaper::initTypes()
{
    QStringList types    = {tr("Scaled"), tr("Wallpaper"), tr("Centered"), tr("Stretched"), tr("Zoom"), tr("Spanned")};
    QStringList typeData = {"scaled", "wallpaper", "centered", "stretched", "zoom", "spanned"};
    wallpaperUi->setDisplayTypes(types, typeData);
}

void Wallpaper::initMode()
{
    QString mode = wallpaperInterface->property("wallpaperMode").toString();
    if (mode == "color") {
        wallpaperUi->setWallpaperMode(tr("Color"), wallpaperInterface->property("wallpaper").toString());
        wallpaperUi->showColors();
    } else if (mode == "picture") {
        wallpaperUi->setWallpaperMode(tr("Picture"), wallpaperInterface->property("wallpaper").toString());
        wallpaperUi->showPictures();
    }
}

void Wallpaper::initType()
{
    QString type = wallpaperInterface->property("displayType").toString();
    if (type == "scaled") {
        wallpaperUi->setDisplayType(tr("Scaled"));
    } else if (type == "wallpaper") {
        wallpaperUi->setDisplayType(tr("Wallpaper"));
    } else if (type == "centered") {
        wallpaperUi->setDisplayType(tr("Centered"));
    } else if (type == "stretched") {
        wallpaperUi->setDisplayType(tr("Stretched"));
    } else if (type == "zoom") {
        wallpaperUi->setDisplayType(tr("Zoom"));
    } else if (type == "spanned") {
        wallpaperUi->setDisplayType(tr("Spanned"));
    }
}

bool Wallpaper::getDefaultAppId(const char *contentType) {
    GAppInfo* info = g_app_info_get_default_for_type(contentType, FALSE);
    if (!info) {
        qDebug() << Q_FUNC_INFO << "g_app_info_get_default_for_type is null" << contentType;
        return false;
    }

    const char* name = g_app_info_get_name(info);
    qDebug() << Q_FUNC_INFO << "getDefaultAppId" << name;

    bool hasDefaultApp = name != nullptr;
    g_object_unref(info);
    return hasDefaultApp;
}

void Wallpaper::onlineButtonClickedSlot() {
    bool hasDefaultBrowser = getDefaultAppId(BROWSERTYPE);
    qDebug() << Q_FUNC_INFO << "hasDefaultBrowser" << hasDefaultBrowser;
    if (!hasDefaultBrowser) {
        QMessageBox *msg = new QMessageBox(wallpaperUi);
        msg->setIcon(QMessageBox::Icon::Critical);
        msg->setText(tr("Cannot access online images, please check the system's default browser settings."));
        msg->addButton(tr("Cancel"), QMessageBox::RejectRole);
        msg->addButton(tr("Set up the browser"), QMessageBox::AcceptRole);
        connect(msg, &QMessageBox::accepted, this, [=]() {
            QProcess::startDetached("ukui-control-center", QStringList() << "-m" << "Defaultapp");
        });
        msg->exec();
        return;
    }

    bool openUrl = QDesktopServices::openUrl(QUrl(QLatin1String("https://www.ubuntukylin.com/wallpaper.html")));
    qDebug() << Q_FUNC_INFO << "openUrl" << openUrl;
    UkccCommon::buriedSettings(name(), QString("Online Pictures"), QString("clicked"));
}

void Wallpaper::connectUiSignals()
{
    QDBusConnection::sessionBus().connect("org.ukui.ukcc.session",
                                          "/Wallpaper",
                                          "org.ukui.ukcc.session.Wallpaper",
                                          "changed",
                                          this,
                                          SLOT(dataChanged(QString)));
    connect(wallpaperUi, &WallpaperUi::wallpaperChanged, this, [=](QString mode, QString data){
        toChangeKey = "wallpaper";
        wallpaperInterface->call("setWallpaper", mode, data);
        UkccCommon::buriedSettings(name(), QString("Background"), QString("select"), mode + "/" + data);
    });
    connect(wallpaperUi, &WallpaperUi::typeChanged, this, [=](QString type){
        toChangeKey = "displayType";
        wallpaperInterface->call("setDisplayType", type);
        UkccCommon::buriedSettings(name(), QString("Type"), QString("select"), type);
    });
    connect(wallpaperUi, &WallpaperUi::localButtonClicked, this, [=](){
        showLocalWpDialog();
        UkccCommon::buriedSettings(name(), QString("Local Pictures"), QString("clicked"));
    });
    connect(wallpaperUi, &WallpaperUi::onlineButtonClicked, this, [=](){
        onlineButtonClickedSlot();
    });
    connect(wallpaperUi, &WallpaperUi::resetButtonClicked, this, [=](){
        toChangeKey = "wallpaper";
        wallpaperInterface->call("resetDefault");
        // 立即按新壁纸刷新选中态，供 UI 把焦点落到默认图
        initMode();
        UkccCommon::buriedSettings(name(), QString("Reset To Default"), QString("clicked"));
    });
    setVisibleBySecurity();
    QDBusConnection::sessionBus().connect(QString(), QString("/"), "org.ukui.ukcc.session.interface", "configChanged", this, SLOT(setVisibleBySecurity()));
}

void Wallpaper::setVisibleBySecurity()
{
    // 安全管控 免密登录及自动登录设置项的显示与隐藏
    QVariantMap ModuleMap = UkccCommon::getModuleHideStatus();
    QString moduleSettings = ModuleMap.value(name().toLower() + "Settings").toString();
    QStringList setItems = moduleSettings.split(",");

    qDebug() << Q_FUNC_INFO << moduleSettings <<  name().toLower();
    foreach (QString setItem, setItems) {
        QStringList item = setItem.split(":");
        if (item.size() != 2) {
            continue;
        }
        qDebug() << "set item Name: " << item.at(0);
        bool visible = (item.at(1) == "true");
        if (item.at(0) == "BackgroundFrame") {
            wallpaperUi->getModeWidget()->setVisible(visible);
        }
        if (item.at(0) == "ModeFrame") {
            wallpaperUi->getTypeWidget()->setVisible(visible);
        }
        if (item.at(0) == "LocalPicturesBtn") {
            wallpaperUi->getLocalBtn()->setVisible(visible);
        }
        if (item.at(0) == "OnlinePicturesBtn") {
            wallpaperUi->getOnlineBtn()->setVisible(visible);
        }
        if (item.at(0) == "ResetToDefaultBtn") {
            wallpaperUi->getResetBtn()->setVisible(visible);
        }
        if (item.at(0) == "PicFrame") {
            wallpaperUi->setPicVisibleBySec(visible);
            wallpaperUi->setModeWidgetVisible(visible);
        }
    }
}

void Wallpaper::loadPictureInfoSlot() {
    if (m_picInfoList.empty()) {
        for (int i = 0; i < m_previewPictures.size(); ++i) {
            PictureInfo* pictureInfo = new PictureInfo();
            if (pictureInfo == nullptr)
                continue;
            pictureInfo->pixmap.load(m_previewPictures.at(i));
            pictureInfo->fileName = m_sourcePictures.at(i);
            m_picInfoList.append(pictureInfo);
        }
    }
    wallpaperUi->setPicturesFile(m_picInfoList);
    wallpaperUi->update();
}

void Wallpaper::initPictures()
{
    if (!m_picInfoList.empty()) {
        loadPictureInfoSlot();
    } else {
        QObject::connect(&m_watcher, &QFutureWatcher<void>::finished, [=]() {
            loadPictureInfoSlot();
        });
    }
}

void Wallpaper::showLocalWpDialog()
{
    QStringList filters;
    filters<<tr("Wallpaper files(*.jpg *.jpeg *.bmp *.dib *.png *.jfif *.jpe *.gif *.tif *.tiff *.wdp)");
    QFileDialog fd(wallpaperUi);
    QList<QUrl> usb_list = fd.sidebarUrls();
    int sidebarNum = 8;// 最大添加U盘数，可以自己定义
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
    connect(&m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, &fd,
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

    connect(&fd, &QFileDialog::finished, &fd, [=, &usb_list, &fd]() {
        fd.setSidebarUrls(usb_list);
    });

    fd.setDirectory(QString(const_cast<char *>(g_get_user_special_dir(G_USER_DIRECTORY_PICTURES))));
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::List);
    fd.setNameFilters(filters);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setWindowTitle(tr("Select custom wallpaper file"));
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
    wallpaperInterface->call("setWallpaper", "picture", selectedfile);
}
