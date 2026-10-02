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

#include "themewatcher.h"

#include <QVariant>
#include "log.h"
#include "utils.h"
#include "virtualkeyboardsettings/virtualkeyboardsettings.h"

ThemeWatcher::ThemeWatcher(QObject *parent) : QObject(parent) {
    if (getDesktopEnvironment() == DesktopEnvironment::UKUI) {
        if (QGSettings::isSchemaInstalled("org.ukui.style")) {
            styleSettings_.reset(new QGSettings("org.ukui.style"));
        }
    }
    updateTheme();
    updateThemeColor();
    connectSignals();
}

void ThemeWatcher::updateTheme() {
    QString currentTheme =
        VirtualKeyboardSettings::getInstance().currentTheme();
    if (currentTheme == currentTheme_) {
        return;
    }

    currentTheme_ = currentTheme;
    KVKBD_INFO("current theme:{}", currentTheme_.toStdString());
    emit currentThemeChanged();
}

void ThemeWatcher::updateThemeColor() {
    QString currentThemeColor = "light";
    if (getDesktopEnvironment() == DesktopEnvironment::UKUI) {
        if (styleSettings_ == nullptr) {
            KVKBD_WARN(
                "WARNING : INCORRECT GSETTINGS ID :{}, IS NOT INSTALLED!",
                "org.ukui.style");
            return;
        }

        QString styleName = styleSettings_->get("style-name").toString();
        if (styleName == "ukui-dark") {
            currentThemeColor = "dark";
        } else if (styleName == "ukui-light") {
            currentThemeColor = "light";
        }
    }

    if (currentThemeColor == currentThemeColor_) {
        return;
    }

    currentThemeColor_ = currentThemeColor;
    KVKBD_INFO("current theme color:{}", currentThemeColor_.toStdString());

    emit currentThemeColorChanged();
}

QString ThemeWatcher::currentTheme() const { return currentTheme_; }

QString ThemeWatcher::currentThemeColor() const { return currentThemeColor_; }

void ThemeWatcher::connectSignals() {
    // 根据gsetting配置文件中切换使用不同的主题，在UKUI桌面环境中可以使用默认的主题
    connect(&VirtualKeyboardSettings::getInstance(),
            &VirtualKeyboardSettings::currentThemeChanged, this,
            &ThemeWatcher::updateTheme);

    // 根据麒麟系统主题色的变化来更新当前主题色，目前只支持浅色和深色两种主题色
    // 如果后期想要支持KDE、Ubuntu系统，那需要支持他们的主题色变化信号
    // 监听 org.ukui.style 中 style-name 的值变化
    if (getDesktopEnvironment() == DesktopEnvironment::UKUI &&
        styleSettings_ != nullptr) {
        connect(styleSettings_.get(), &QGSettings::changed, this,
                [this](const QString &key) {
                    if (key == "styleName") {
                        updateThemeColor();
                    }
                });
    }
}
