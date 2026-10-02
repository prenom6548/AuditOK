/*
 * Copyright (c) KylinSoft Co., Ltd. 2025.All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <QGuiApplication>
#include <QLineF>
#include <QTimer>

#include "log.h"
#include "screenwatcher.h"

ScreenWatcher::ScreenWatcher(QObject *parent) : QObject(parent) {
    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    lastPrimaryScreenName_ = primaryScreen ? primaryScreen->name() : "Unknown";
    updateScreenList();
    connectScreenSignals();
    KVKBD_INFO("ScreenWatcher initialized, primaryScreen:{}",
               lastPrimaryScreenName_.toStdString());
}

ScreenWatcher &ScreenWatcher::getInstance() {
    static ScreenWatcher screenWatcher(qApp);
    return screenWatcher;
}

QRect ScreenWatcher::getOptimalScreenGeometry(const QPoint &position) const {
    if (const auto *screen = QGuiApplication::screenAt(position)) {
        KVKBD_DEBUG("screen name:{}", screen->name().toStdString());
        return screen->geometry();
    }

    for (auto it = screenList_.begin(); it != screenList_.end(); ++it) {
        const ScreenInfo &info = it.value();
        if (info.hasWindowMark()) {
            KVKBD_DEBUG("marked screen name:{}",
                        info.screen->name().toStdString());
            return info.screen->geometry();
        }
    }
    KVKBD_DEBUG("primaryScreen name:{}",
                QGuiApplication::primaryScreen()->name().toStdString());

    return QGuiApplication::primaryScreen()->geometry();
}

bool ScreenWatcher::isScreenValid(const QScreen *screen) const {
    if (screen == nullptr) {
        return false;
    }

    if (screen->name().isEmpty()) {
        return false;
    }

    auto screenGeo = screen->geometry();
    if (screenGeo.width() <= 0 || screenGeo.height() <= 0) {
        return false;
    }

    return true;
}

bool ScreenWatcher::isAnyScreenMarked() const {
    for (auto it = screenList_.begin(); it != screenList_.end(); ++it) {
        if (it.value().hasWindowMark()) {
            return true;
        }
    }
    return false;
}

void ScreenWatcher::markScreen(const QPoint &position, bool needNotify) {
    QScreen *windowScreen = QGuiApplication::screenAt(position);
    if (windowScreen == nullptr) {
        return;
    }

    if (isScreenMarked(windowScreen)) {
        return;
    }

    for (auto it = screenList_.begin(); it != screenList_.end(); ++it) {
        it.value().hasWindow = false;
    }

    ScreenInfo *info = getScreenInfo(windowScreen);
    if (info != nullptr) {
        info->hasWindow = true;
        if (needNotify) {
            isScreenMarkChanged_ = true;
        }
        KVKBD_DEBUG("screen name:{}", info->screen->name().toStdString());
    }
}

void ScreenWatcher::notifyScreenMarkChanged() {
    if (!isScreenMarkChanged_) {
        return;
    }
    isScreenMarkChanged_ = false;
    emit screenMarkChanged();
}

bool ScreenWatcher::isScreenMarked(const QScreen *screen) const {
    if (screen == nullptr) {
        return false;
    }

    const ScreenInfo *info = getScreenInfoConst(const_cast<QScreen *>(screen));
    return info != nullptr && info->hasWindowMark();
}

void ScreenWatcher::updateScreenList() {
    screenList_.clear();

    for (QScreen *screen : QGuiApplication::screens()) {
        if (!isScreenValid(screen)) {
            continue;
        }

        QString name = screen->name();
        QRect geometry = screen->geometry();
        screenList_[screen] = ScreenInfo(screen, name, geometry);
    }
}

ScreenInfo *ScreenWatcher::getScreenInfo(QScreen *screen) {
    auto it = screenList_.find(screen);
    return (it != screenList_.end()) ? &it.value() : nullptr;
}

const ScreenInfo *ScreenWatcher::getScreenInfoConst(QScreen *screen) const {
    auto it = screenList_.constFind(screen);
    return (it != screenList_.constEnd()) ? &it.value() : nullptr;
}

void ScreenWatcher::connectScreenSignals() {
    // 连接屏幕添加信号
    connect(qApp, &QGuiApplication::screenAdded, this,
            &ScreenWatcher::onScreenAdded);

    // 连接屏幕移除信号
    connect(qApp, &QGuiApplication::screenRemoved, this,
            &ScreenWatcher::onScreenRemoved);

    // 连接所有现有屏幕的几何变化信号
    for (QScreen *screen : QGuiApplication::screens()) {
        connectScreenGeometryChanged(screen);
    }

    // 连接主屏幕改变信号
    connect(qApp, &QGuiApplication::primaryScreenChanged, this,
            &ScreenWatcher::onPrimaryScreenChanged);
}

void ScreenWatcher::connectScreenGeometryChanged(QScreen *screen) {
    if (screen == nullptr) {
        return;
    }

    connect(screen, &QScreen::geometryChanged, this,
            &ScreenWatcher::onScreenGeometryChanged, Qt::UniqueConnection);
}

void ScreenWatcher::onScreenAdded(QScreen *screen) {
    if (screen == nullptr) {
        return;
    }

    if (!isScreenValid(screen)) {
        KVKBD_WARN("Screen added but is invalid, maybe a fake screen, ignoring "
                   "screen added signal");
        return;
    }

    KVKBD_DEBUG("Screen added:{}", screen->name().toStdString());

    QString name = screen->name();
    QRect geometry = screen->geometry();
    screenList_[screen] = ScreenInfo(screen, name, geometry);

    connectScreenGeometryChanged(screen);
    emit screensChanged();
}

void ScreenWatcher::onScreenRemoved(QScreen *screen) {
    if (screen == nullptr) {
        return;
    }

    QString screenName = screen->name();
    KVKBD_INFO("Screen name: {}", screenName.toStdString());

    if (!isScreenValid(QGuiApplication::primaryScreen())) {
        KVKBD_WARN("Screen removed but no valid primary screen (only fake "
                   "screens?), ignoring screen removed signal"
                   ", Removed screen: {}",
                   screenName.toStdString());
        screenList_.remove(screen);
        return;
    }

    screenList_.remove(screen);

    if (!isAnyScreenMarked()) {
        KVKBD_INFO("Screen removed: {}", screenName.toStdString());
        emit screensChanged();
    }
}

void ScreenWatcher::onScreenGeometryChanged(const QRect &geometry) {
    QScreen *screen = qobject_cast<QScreen *>(sender());
    if (screen == nullptr) {
        KVKBD_WARN("Screen geometry changed screen is nullptr!");
        return;
    }

    ScreenInfo *info = getScreenInfo(screen);
    if (info == nullptr) {
        KVKBD_WARN("Screen geometry changed but screen not in list: {}",
                   screen->name().toStdString());
        return;
    }

    bool screenMarked = info->hasWindowMark();
    info->geometry = geometry;

    if (screenMarked) {
        KVKBD_WARN("Screen geometry changed: {} -> {}x{}",
                   screen->name().toStdString(), geometry.width(),
                   geometry.height());
        emit screensChanged();
    }
}

void ScreenWatcher::onPrimaryScreenChanged(QScreen *screen) {
    if (!isScreenValid(screen)) {
        KVKBD_WARN("Primary screen changed but this screen is invalid (fake "
                   "screen?), ignoring primary screen changed signal");
        return;
    }

    QString newScreenName = screen->name();
    QString oldScreenName =
        lastPrimaryScreenName_.isEmpty() ? "None" : lastPrimaryScreenName_;
    KVKBD_INFO("Primary screen changed: {} -> {}", oldScreenName.toStdString(),
               newScreenName.toStdString());

    lastPrimaryScreenName_ = newScreenName;
    connectScreenGeometryChanged(screen);
    emit screensChanged();
}
