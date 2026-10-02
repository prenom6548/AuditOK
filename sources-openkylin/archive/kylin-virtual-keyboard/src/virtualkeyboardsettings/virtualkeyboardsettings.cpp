/*
 * Copyright (c) KylinSoft Co., Ltd. 2023.All rights reserved.
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

#include "virtualkeyboardsettings.h"
#include <QByteArray>
#include <QVariant>

VirtualKeyboardSettings::VirtualKeyboardSettings() { init(); }

void VirtualKeyboardSettings::init() {
    if (!QGSettings::isSchemaInstalled(gsettingsId_.toUtf8())) {
        KVKBD_WARN("WARNING : INCORRECT GSETTINGS ID :{}, IS NOT INSTALLED!",
                   gsettingsId_.toStdString());
        return;
    }

    gsettings_.reset(new QGSettings(gsettingsId_.toUtf8()));

    scaleFactorMergeTimer_.setSingleShot(true);
    scaleFactorMergeTimer_.setInterval(SCALE_FACTOR_MERGE_PERIOD);

    connect(gsettings_.get(), &QGSettings::changed, this,
            [this](const QString &key) {
                if (!gsettings_->keys().contains(key)) {
                    return;
                }

                if (key == floatButtonEnabledKey_) {
                    emitFloatButtonAvailabilityChanged();
                } else if (key == virtualKeyboardScaleFactorKey_) {
                    scaleFactorMergeTimer_.start();
                } else if (key == trayIconShowKey_) {
                    emitTrayIconShowChanged();
                } else if (key == animationEnabledKey_) {
                    emit animationAvailabilityChanged();
                } else if (key == defaultThemeKey_) {
                    emitCurrentThemeChanged();
                }
            });

    connect(&scaleFactorMergeTimer_, &QTimer::timeout, this,
            [this]() { emit scaleFactorChanged(); });
}

void VirtualKeyboardSettings::emitFloatButtonAvailabilityChanged() {
    const bool value = gsettings_->get(floatButtonEnabledKey_).toBool();
    if (value) {
        emit requestFloatButtonEnabled();
    } else {
        emit requestFloatButtonDisabled();
    }
}

void VirtualKeyboardSettings::updateFloatButtonAvailability(const bool value) {
    if (gsettings_ == nullptr) {
        KVKBD_WARN("WARNING : Gsettings Objetc is NULL !");
        return;
    }

    gsettings_->set(floatButtonEnabledKey_, QVariant(value));
}

bool VirtualKeyboardSettings::isFloatButtonEnabled() const {
    if (gsettings_ == nullptr) {
        KVKBD_WARN("WARNING : Gsettings Objetc is NULL !");
        return false;
    }

    return gsettings_->get(floatButtonEnabledKey_).toBool();
}

bool VirtualKeyboardSettings::isAnimationEnabled() const {
    if (gsettings_ == nullptr) {
        KVKBD_WARN("WARNING : Gsettings Objetc is NULL !");
        return false;
    }

    return gsettings_->get(animationEnabledKey_).toBool();
}

bool VirtualKeyboardSettings::isPreloadViewEnabled() const {
    if (gsettings_ == nullptr) {
        KVKBD_WARN("WARNING : Gsettings Objetc is NULL !");
        return false;
    }

    return gsettings_->get(preloadViewEnabledKey_).toBool();
}

float VirtualKeyboardSettings::calculateVirtualKeyboardScaleFactor() const {
    return static_cast<float>(
               gsettings_->get(virtualKeyboardScaleFactorKey_).toInt()) /
           100;
}

const QString VirtualKeyboardSettings::trayIconShow() const {
    if (gsettings_ == nullptr) {
        KVKBD_WARN("WARNING : Gsettings Objetc is NULL !");
        return "NeverShow";
    }

    return gsettings_->get(trayIconShowKey_).toString();
}

void VirtualKeyboardSettings::emitTrayIconShowChanged() {
    const QString value = gsettings_->get(trayIconShowKey_).toString();
    if (value == "NeverShow") {
        emit neverShowTrayIcon();
    } else if (value == "AlwaysShow") {
        emit alwaysShowTrayIcon();
    } else if (value == "ShowWhenKeyboardIsConnected") {
        emit showTrayIconWhenKeyboardisConnected();
    } else {
        KVKBD_WARN("WARNING : Gsettings Set trayIconShow Error !");
    }
}

const QString VirtualKeyboardSettings::currentTheme() const {
    if (gsettings_ == nullptr) {
        KVKBD_WARN(
            "WARNING : Gsettings Objetc is NULL, can't get current theme!");
        return "default";
    }

    return gsettings_->get(defaultThemeKey_).toString();
}

void VirtualKeyboardSettings::emitCurrentThemeChanged() {
    const QString value = gsettings_->get(defaultThemeKey_).toString();
    emit currentThemeChanged(value);
}

VirtualKeyboardSettings &VirtualKeyboardSettings::getInstance() {
    static VirtualKeyboardSettings virtualKeyboardSettings;
    return virtualKeyboardSettings;
}
