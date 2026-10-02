/*
 * UKUI Framework
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "background.h"

#include <QImageReader>

#include "qgsettings.h"
using namespace UKUI;
#undef signals
#include <gio/gio.h>

//决定如何显示墙纸文件名指定的图片。可用的值有：居中、拉伸、填充、平铺、适应、跨区
const QStringList supportOptions = {"centered", "stretched", "scaled", "wallpaper", "zoom", "spanned"};

QString g_bgSettingsSchema = "org.ukui.interface";

bool g_backgroundServiceAvailable = true;

Background::Background(QObject *parent)
    : QObject{parent}
{
    if (!QGSettings::isSchemaInstalled(g_bgSettingsSchema.toLocal8Bit())) {
        if (QGSettings::isSchemaInstalled("org.mate.background")) {
            // 壁纸设置兼容
            // 如果是旧版本的ukui，需要监听org.mate.background
            g_bgSettingsSchema = "org.mate.background";
        } else {
            g_backgroundServiceAvailable = false;
        }
    }
    m_pGsettingsManager = new GsettingsManager(parent, g_bgSettingsSchema, setChangedHandler());
    BackgroundFile = m_pGsettingsManager->get(G_UKUI_BACKGROUND_KEY_PICTURE_NAME).toString();
    BackgroundOption = m_pGsettingsManager->get(G_UKUI_BACKGROUND_KEY_PICTURE_OPTIONS).toString();
}

Background::~Background()
{

}

const QString Background::GetBackgroundFile() const
{
    PEEK_SCHEMA(G_UKUI_BACKGROUND_KEY_PICTURE_NAME, C_ERROR_STRING);
    return BackgroundFile;
}

bool Background::SetBackgroundFile(const QString &filePath)
{
    if (!g_backgroundServiceAvailable) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidService, tr("Background service is not available"));
        }
        return false;
    }

    if (BackgroundFile == filePath) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::Failed, tr("Can not set background repeatly"));
        }
        return false;
    }

    QImageReader imgReader(filePath);
    if (!imgReader.canRead()) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::NotSupported, tr("Can not read image from: %1").arg(filePath));
        }
        return false;
    }

    m_pGsettingsManager->set(G_UKUI_BACKGROUND_KEY_PICTURE_NAME, filePath);
    return true;
}

const QStringList Background::GetSupportedBackgroundOptions()
{
    QStringList errorRet;
    PEEK_SCHEMA(G_UKUI_BACKGROUND_KEY_PICTURE_NAME, errorRet);

    return supportOptions;
}

const QString Background::GetBackgroundOption() const
{
    PEEK_SCHEMA(G_UKUI_BACKGROUND_KEY_PICTURE_NAME, C_ERROR_STRING);
    return BackgroundOption;
}

bool Background::SetBackgroundOption(const QString &option)
{
    if (!g_backgroundServiceAvailable) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidService, tr("Background service is not available"));
        }
        return false;
    }

    if (BackgroundOption == option)
        return false;

    if (supportOptions.contains(option)) {
        m_pGsettingsManager->set(G_UKUI_BACKGROUND_KEY_PICTURE_OPTIONS, option);
        return true;
    } else {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidArgs, tr("Invalid background option: %s, support options: [%1]").arg(supportOptions.join(", ")));
        }
        return false;
    }

    return false;
}

QHash<QString, OnKeyChanged> Background::setChangedHandler()
{
    m_settingsChangedHandlers[G_UKUI_BACKGROUND_KEY_PICTURE_NAME] = [this]{
          BackgroundFile = m_pGsettingsManager->get(G_UKUI_BACKGROUND_KEY_PICTURE_NAME).toString();
        emit BackgroundFileChanged(); };
    m_settingsChangedHandlers[G_UKUI_BACKGROUND_KEY_PICTURE_OPTIONS] = [this]{
         BackgroundOption = m_pGsettingsManager->get(G_UKUI_BACKGROUND_KEY_PICTURE_OPTIONS).toString();
        emit BackgroundOptionChanged(); };
    return m_settingsChangedHandlers;
}
