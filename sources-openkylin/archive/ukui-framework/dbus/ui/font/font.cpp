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

#include "font.h"

#include <QDebug>
#undef signals
#include <gio/gio.h>
#include "qgsettings.h"
#include "qgsettings_define.h"

using namespace UKUI;

Font::Font(QObject *parent)
    : QObject{parent}
{
    m_pGsettingsManager = new GsettingsManager(parent, "org.ukui.style",
                                                       setChangedHandler());
    FontFamily = m_pGsettingsManager->get(G_UKUI_STYLE_SETTINGS_KEY_FONT).toString();
    FontSize = m_pGsettingsManager->get(G_UKUI_STYLE_SETTINGS_KEY_SIZE).toDouble();
}

Font::~Font()
{

}

const QString Font::GetFontFamily() const
{
    PEEK_SCHEMA(G_UKUI_STYLE_SETTINGS_KEY_FONT, "");
    return FontFamily;
}

bool Font::SetFontFamily(const QString &family)
{
    PEEK_SCHEMA(G_UKUI_STYLE_SETTINGS_KEY_FONT, false);

    if (FontFamily == family)
        return false;

    g_autoptr (GSettings) scopedSettings = g_settings_new("org.ukui.style");
    if (g_settings_set_string(scopedSettings, qPrintable(G_UKUI_STYLE_SETTINGS_KEY_FONT), family.toLocal8Bit().constData())) {
        return true;
    }

    return false;
}

double Font::GetFontSize() const
{
    PEEK_SCHEMA(G_UKUI_STYLE_SETTINGS_KEY_FONT, -1);
    return FontSize;
}

bool Font::SetFontSize(double size)
{
    if (size < 10 || size > 15) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidArgs, tr("Font size are InvalidArgs"));
        }
        return false;
    }

    if (FontSize == size)
        return false;

    m_pGsettingsManager->set(G_UKUI_STYLE_SETTINGS_KEY_SIZE, size);
    return true;
}

bool Font::SetFontSizeWithString(QString size)
{
    if (!m_fontSizeMap.contains(size)) {
         if (calledFromDBus()) {
         sendErrorReply(QDBusError::InvalidArgs, tr("size %1 are invalid args").arg(size));
         }
        return false;
    }
    QString sizeString = QString::number(m_fontSizeMap[size]);
    PEEK_AND_SET_AND_RET(G_UKUI_STYLE_SETTINGS_KEY_SIZE, sizeString.toLocal8Bit().constData() ,false);
    return true;
}

bool Font::SetFontSizeWithType(QString size)
{
    if (!m_fontSizeMap.contains(size)) {
         if (calledFromDBus()) {
         sendErrorReply(QDBusError::InvalidArgs, tr("size %1 are invalid args").arg(size));
         }
        return false;
    }

    QString sizeString = QString::number(m_fontSizeMap[size]);
    PEEK_AND_SET_AND_RET(G_UKUI_STYLE_SETTINGS_KEY_SIZE, sizeString.toLocal8Bit().constData(), false);
    return true;
}

QString Font::GetFontSizeWithType()
{
    if (!m_fontSizeMap.values().contains(FontSize)) {
        if (calledFromDBus()) {
        sendErrorReply(QDBusError::InvalidArgs, tr("size %1 are invalid type").arg(FontSize));
        }
        return "";
    }

    qDebug()<<FontSize;
    return m_fontSizeMap.key(FontSize);
}

QHash<QString, OnKeyChanged> Font::setChangedHandler()
{
    m_settingsChangedHandlers[G_UKUI_STYLE_SETTINGS_KEY_FONT] = [this]{
        FontFamily = m_pGsettingsManager->get(G_UKUI_STYLE_SETTINGS_KEY_FONT).toString();
        emit FontFamilyChanged();
    };

    m_settingsChangedHandlers[G_UKUI_STYLE_SETTINGS_KEY_SIZE] = [this]{
        FontSize = m_pGsettingsManager->get(G_UKUI_STYLE_SETTINGS_KEY_SIZE).toDouble();
        emit FontSizeChanged();
    };
    return m_settingsChangedHandlers;
}
