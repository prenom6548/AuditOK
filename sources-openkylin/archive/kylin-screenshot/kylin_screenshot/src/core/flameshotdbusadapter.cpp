/* Copyright (C) 2020, KylinSoft Co., Ltd.

*
* This file is part of Kylin-Screenshot.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "flameshotdbusadapter.h"
#include "src/utils/confighandler.h"
#include "src/utils/screengrabber.h"
#include "src/core/controller.h"
#include "src/utils/screenshotsaver.h"
#include "src/utils/systemnotification.h"
#include <QBuffer>
#include <QGSettings/qgsettings.h>
#include <QDebug>
#include <QTime>
#include <QThread>

FlameshotDBusAdapter::FlameshotDBusAdapter(QObject *parent) :
    QDBusAbstractAdaptor(parent)
{
    auto controller = Controller::getInstance();
    connect(controller, &Controller::captureFailed,
            this, &FlameshotDBusAdapter::captureFailed);
    connect(controller, &Controller::captureTaken,
            this, &FlameshotDBusAdapter::handleCaptureTaken);
    connect(controller, &Controller::captureExit,
            this, &FlameshotDBusAdapter::captureExit);
    connect(controller, &Controller::captureCopy,
            this, &FlameshotDBusAdapter::captureCopy);
}

FlameshotDBusAdapter::~FlameshotDBusAdapter()
{
}

void FlameshotDBusAdapter::graphicCapture(QString path, int delay, uint id)
{
#ifndef UNUI
    CaptureRequest req(CaptureRequest::GRAPHICAL_MODE, delay, path);
// if (toClipboard) {
// req.addTask(CaptureRequest::CLIPBOARD_SAVE_TASK);
// }
    req.setStaticID(id);
    Controller::getInstance()->requestCapture(req);
#endif
}

void FlameshotDBusAdapter::osManagerCapture(QString path, int delay, uint id)
{
    CaptureRequest req(CaptureRequest::OSMANAGER_MODE, delay, path);

    req.setStaticID(id);
    Controller::getInstance()->requestCapture(req);
}

void FlameshotDBusAdapter::fullScreen(
    QString path, QString type, bool toClipboard, int delay, uint id)
{
#ifndef UNUI
    CaptureRequest req(CaptureRequest::FULLSCREEN_MODE, delay, path, type);
    if (toClipboard) {
        req.addTask(CaptureRequest::CLIPBOARD_SAVE_TASK);
    }
    if (!path.isEmpty()) {
        req.addTask(CaptureRequest::FILESYSTEM_SAVE_TASK);
    }
    req.setStaticID(id);
    Controller::getInstance()->requestCapture(req);
#endif
}

void FlameshotDBusAdapter::openLauncher()
{
#ifndef UNUI
    Controller::getInstance()->openLauncherWindow();
#endif
}

void FlameshotDBusAdapter::captureScreen(int number, QString path, QString type, bool toClipboard,
                                         int delay, uint id)
{
#ifndef UNUI
    CaptureRequest req(CaptureRequest::SCREEN_MODE, delay, path, type, number);
    if (toClipboard) {
        req.addTask(CaptureRequest::CLIPBOARD_SAVE_TASK);
    }
    if (!path.isEmpty()) {
        req.addTask(CaptureRequest::FILESYSTEM_SAVE_TASK);
    }
    req.setStaticID(id);
    Controller::getInstance()->requestCapture(req);
#endif
}

void FlameshotDBusAdapter::openConfig()
{
#ifndef UNUI
    Controller::getInstance()->openConfigWindow();
#endif
}

void FlameshotDBusAdapter::trayIconEnabled(bool enabled)
{
#ifndef UNUI
    auto controller = Controller::getInstance();
    if (enabled) {
        controller->enableTrayIcon();
    } else {
        controller->disableTrayIcon();
    }
#endif
}

void FlameshotDBusAdapter::autostartEnabled(bool enabled)
{
#ifndef UNUI
    ConfigHandler().setStartupLaunch(enabled);
    auto controller = Controller::getInstance();
    // Autostart is not saved in a .ini file, requires manual update
    controller->updateConfigComponents();
#endif
}

void FlameshotDBusAdapter::handleCaptureTaken(uint id, const QPixmap &p)
{
    // 使用 QThread 在后台线程中执行图片转换
    QThread *convertThread = new QThread();
    QObject *worker = new QObject();
    worker->moveToThread(convertThread);

    connect(convertThread, &QThread::started, worker, [=]() {
        QThread::msleep(100);
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        p.save(&buffer, "PNG");
        emit captureTaken(id, byteArray);
        convertThread->quit();
        worker->deleteLater();
    });

    connect(convertThread, &QThread::finished, convertThread, &QThread::deleteLater);

    convertThread->start();
}

QVariantMap FlameshotDBusAdapter::getSaveInfo()
{
    QVariantMap map;
#ifndef UNUI
    QGSettings *ScreenshotGsettings = new QGSettings("org.ukui.screenshot");
    map.insert("screenshot-path",
               QVariant::fromValue(ScreenshotGsettings->get("screenshot-path").toString()));
    map.insert("screenshot-name",
               QVariant::fromValue(ScreenshotGsettings->get("screenshot-name").toString()));
    map.insert("screenshot-type",
               QVariant::fromValue(ScreenshotGsettings->get("screenshot-type").toString()));
#endif
    return map;
}
