/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "screencapture.h"
#include "kmrewindow.h"
#include "clipboard.h"
#include "kmreenv.h"
#include "preferences.h"
#include "utils.h"

#include <QDateTime>
#include <QGSettings>
#include <QFile>
#include <QMimeData>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>
#include <QProcess>
#include <syslog.h>

#define SCREEN_SHOT_GSETTING_PATH   "org.ukui.screenshot"

ScreenCapture::ScreenCapture(KmreWindow* window)
    : QObject(window)
    , mMainWindow(window)
    , mScreenCaptureDbus(ScreenCaptureDbus::getInstance())
    , mHideWindowDuringScreenShot(false)
    , mStartShot(false)
    , mHasImage(false)
    , mImageData(QVariant())
    , mIsCapturing(false)
{
    connect(mScreenCaptureDbus, SIGNAL(sigCaptureTaken(uint, QByteArray)), this, SLOT(onCaptureTaken(uint, QByteArray)), Qt::DirectConnection);
    connect(mScreenCaptureDbus, SIGNAL(sigCaptureExit(uint)), this, SLOT(onCaptureExit(uint)), Qt::QueuedConnection);
    connect(mScreenCaptureDbus, SIGNAL(sigCaptureCopy(uint)), this, SLOT(onCaptureCopy(uint)), Qt::DirectConnection);
    connect(mScreenCaptureDbus, SIGNAL(sigCaptureFailed(uint)), this, SLOT(onCaptureFailed(uint)), Qt::DirectConnection);

    if (QGSettings::isSchemaInstalled(SCREEN_SHOT_GSETTING_PATH)) {
        mScreenShotGSetting = std::make_unique<QGSettings>(SCREEN_SHOT_GSETTING_PATH);
        connect(mScreenShotGSetting.get(), &QGSettings::changed, this, [&] {
            if (mScreenShotGSetting->keys().contains("isrunning")) {
                syslog(LOG_DEBUG, "[%s] Screen shot gsettings changed...", __func__);
                if (mStartShot && (mScreenShotGSetting->get("isrunning").toString() == "false")) {
                    mStartShot = false;
                    getScreenShotImage("");
                }
            }
        });
    }
    else {
        syslog(LOG_WARNING, "[%s] GSetting schema: '%s' isn't installed!", __func__, SCREEN_SHOT_GSETTING_PATH);
    }
}

ScreenCapture::~ScreenCapture()
{
    
}

void ScreenCapture::onScreenShot()
{
    syslog(LOG_DEBUG, "[%s] Capture screen...", __func__);
    QString reply = mScreenCaptureDbus->introspect();

    if (reply.contains("captureCopy")) {
        Clipboard::getInstance().clear();
        if (mHideWindowDuringScreenShot) {
            KmreConfig::Preferences::getInstance()->m_toScreenHide = true;
            mMainWindow->showMinimized();
        }
        QThread::usleep(400000);//延时让菜单栏隐藏
        mIsCapturing = false;
        mScreenCaptureDbus->graphicCapture("", 0, 1);
        mIsCapturing = true;
    }
    else {
        QThread::usleep(400000);
        mHasImage = Clipboard::getInstance().getImage(mImageData);
        QString cmd = "/usr/bin/kylin-screenshot gui";
        QProcess::execute(cmd);
        mStartShot = true;
        mCopysig = true;
    }
}

void ScreenCapture::getScreenShotImage(const QString &filePath)
{
    syslog(LOG_DEBUG, "[%s] Get screen image and share to android...", __func__);
    if (mCopysig) {
        mCopysig = false;
        QVariant imageData;
        bool hasImage = Clipboard::getInstance().getImage(imageData);
        if (hasImage) {
            if (mHasImage && (mImageData == imageData)) {
                return;
            }
            QString fileName = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + QString(".png");
            QString filePath = QString("%1/%2").arg(utils::getDefaultScreenshotFolder()).arg(fileName);
            QImage img = qvariant_cast<QImage>(imageData);
            img.save(filePath);

            mMainWindow->shareFileToAndroid(filePath);
        }
    }
    else {
        mMainWindow->shareFileToAndroid(filePath);
    }
    mMainWindow->showMainWindow();
}

void ScreenCapture::onCaptureTaken(uint id, QByteArray rawImage)
{
    Q_UNUSED(id)
    Q_UNUSED(rawImage)
    syslog(LOG_DEBUG, "Screen capture taken(%d), mIsCapturing = %d", id, mIsCapturing);

    if (mIsCapturing) {
        QString path;
        QString name;
        QString suffix;

        QVariantMap reply = mScreenCaptureDbus->getSaveInfo();
        if (reply.keys().contains("screenshot-path")) {
            path = reply["screenshot-path"].toString();
        }
        if (reply.keys().contains("screenshot-name")) {
            name = reply["screenshot-name"].toString();
        }
        if (reply.keys().contains("screenshot-type")) {
            suffix = reply["screenshot-type"].toString();
        }

        QString picPath = QString("%1%2%3%4").arg(path).arg("/").arg(name).arg(suffix);
        getScreenShotImage(picPath);
        mIsCapturing = false;
    }
}

void ScreenCapture::onCaptureCopy(uint id)
{
    Q_UNUSED(id)
    if (mIsCapturing) {
        mCopysig = true;
        syslog(LOG_DEBUG, "Screen capture copy(%d).", id);
    }
}

void ScreenCapture::onCaptureExit(uint id)
{
    Q_UNUSED(id)
    if (mIsCapturing) {
        mMainWindow->showMainWindow();
        syslog(LOG_DEBUG, "Screen capture exit(%d).", id);
    }
}

void ScreenCapture::onCaptureFailed(uint id)
{
    Q_UNUSED(id)
    if (mIsCapturing) {
        syslog(LOG_ERR, "Screen capture failed(%d).", id);
    }
}
