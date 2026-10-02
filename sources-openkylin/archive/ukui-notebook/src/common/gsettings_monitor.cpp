/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
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

#include <QVariant>
#include <QDebug>
#include <QFontDatabase>
#include <QFont>

#include "gsettings_monitor.h"

GsettingsMonitor::GsettingsMonitor()
{
    m_ukuiStyle = new QGSettings(UKUI_STYLE);
    if (gsettingsCheck()) {
        initGsettings();
    }
}

GsettingsMonitor::~GsettingsMonitor()
{

}

bool GsettingsMonitor::gsettingsCheck()
{
    if(!QGSettings::isSchemaInstalled(UKUI_STYLE)) {
        qCritical() << QString("gsettings Error: %1 not found!").arg(UKUI_STYLE);
        return false;
    }
    return true;
}


void GsettingsMonitor::initGsettings()
{
    connect(m_ukuiStyle, &QGSettings::changed, this, [=] (const QString &key) {
        if (key == UKUI_MENU_TRANSPARENCY) {
            getTransparency();
        }
        else if (key == UKUI_STYLE_NAME) {
            getStyleStatus();
        }
        else if (key == UKUI_WIDGET_THEME_NAME) {
            getWidgetThemeStatus();
        }
    });
    getTransparency();
    getStyleStatus();
    getWidgetThemeStatus();
    return ;
}

void GsettingsMonitor::getTransparency()
{
    if (!gsettingsCheck()) {
        return;
    }

    int tmpMenuTrans = m_ukuiStyle->get(UKUI_MENU_TRANSPARENCY).toInt();
    if (tmpMenuTrans != m_menuTrans) {
        if (m_menuTrans != -1) {
            emit transparencyUpdate(tmpMenuTrans);
        }
        m_menuTrans = tmpMenuTrans;
    }

    return ;
}

void GsettingsMonitor::getStyleStatus()
{
    if (!gsettingsCheck()) {
        return;
    }

    StyleStatus tmpStyleStatus = styleNameConvert(m_ukuiStyle->get(UKUI_STYLE_NAME).toString());
    if (tmpStyleStatus != m_styleStatus) {
        if (m_styleStatus != NONE) {
            emit styleStatusUpdate(tmpStyleStatus);
        }
        m_styleStatus = tmpStyleStatus;
    }
    return ;
}

GsettingsMonitor::StyleStatus GsettingsMonitor::styleNameConvert(const QString &styleName)
{
    if (styleName == "ukui-default"
     || styleName == "ukui-white"
     || styleName == "ukui-light"
     || styleName == "ukui") {
                
        return LIGHT;
    }
    
    if (styleName == "ukui-dark" || styleName == "ukui-black") {
        return DARK;
    }

    return NONE;
}

int GsettingsMonitor::transparency() const
{
    return m_menuTrans;
}

GsettingsMonitor::StyleStatus GsettingsMonitor::styleStatus() const
{
    return m_styleStatus;
}

GsettingsMonitor::WidgetThemeStatus GsettingsMonitor::widgetThemeName() const
{
    return m_isClassical;
}


void GsettingsMonitor::getWidgetThemeStatus()
{
    if ("classical" == m_ukuiStyle->get(UKUI_WIDGET_THEME_NAME).toString()) {
        m_isClassical = CLASSICAL;
    }
    else {
        m_isClassical = UNCLASSICAL;
    }

    emit widgetThemeUpdate(m_isClassical);
}
