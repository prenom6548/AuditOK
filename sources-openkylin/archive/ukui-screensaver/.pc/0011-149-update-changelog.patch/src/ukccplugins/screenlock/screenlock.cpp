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

Screenlock::Screenlock() : mFirstLoad(true)
{
    QTranslator *translator = new QTranslator(this);
    translator->load("/usr/share/ukui-screensaver/ukccplugins/i18n_qm/" + QLocale::system().name());
    QApplication::installTranslator(translator);

    pluginName = tr("Screenlock");
    pluginType = PERSONALIZED;
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
        screenlockInterface = new QDBusInterface(
            "org.ukui.ScreenSaver",
            "/Screenlock",
            "org.ukui.ScreenSaver.session.Screenlock",
            QDBusConnection::sessionBus(),
            this);
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

void Screenlock::initContent()
{
    QStringList previewPictures = screenlockInterface->property("previewWallpapers").toStringList();
    QStringList sourcePictures = screenlockInterface->property("sourceWallpapers").toStringList();
    screenlockui->setPictures(previewPictures, sourcePictures);
    initWallpaper();
    initShowOnLogin();
}

void Screenlock::initShowOnLogin()
{
    screenlockui->setShowOnLogin(screenlockInterface->property("showOnLogin").toBool());
}

void Screenlock::initWallpaper()
{
    screenlockui->setPicture(screenlockInterface->property("wallpaper").toString());
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
    connect(screenlockui, &ScreenlockUi::pictureChanged, this, [=](QString file) {
        toChangeKey = "wallpaper";
        screenlockInterface->call("setWallpaper", file);
    });
    connect(screenlockui, &ScreenlockUi::localButtonClicked, this, [=]() { showLocalWpDialog(); });
    connect(screenlockui, &ScreenlockUi::onlineButtonClicked, this, [=]() {
        QDesktopServices::openUrl(QUrl(QLatin1String("https://www.ubuntukylin.com/wallpaper.html")));
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

    QStringList fileRes = selectedfile.split("/");

    QProcess *process = new QProcess(this);
    QString program("cp");
    QStringList arguments;
    arguments << selectedfile;
    arguments << "/tmp";
    process->start(program, arguments);

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
