/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

/**
 * @brief 由于当前仅用于主题变化时，备份还原工具的一些图标跟随变化，此处暂时先简单封装。后续根据使用情况另行优化。
 */

#include "gsettingswrapper.h"
#include <QIcon>
#include <QLabel>
#include "globalsignals.h"
#include "globalbackupinfo.h"
#include "../common/mydefine.h"

GSettingsWrapper::GSettingsWrapper(token)
{
    m_isDarkTheme = false;
    if (QGSettings::isSchemaInstalled(ORG_UKUI_STYLE)) {
        m_pGsettingThemeData = new QGSettings(ORG_UKUI_STYLE);
        QString themeStyleCurr = this->m_pGsettingThemeData->get(STYLE_NAME).toString();
        if (STYLE_NAME_VALUE_DARK == themeStyleCurr || STYLE_NAME_VALUE_BLACK == themeStyleCurr) {
            // 深色主题
             m_isDarkTheme = true;
        } else if (STYLE_NAME_VALUE_DEFAULT == themeStyleCurr || STYLE_NAME_VALUE_LIGHT == themeStyleCurr || STYLE_NAME_VALUE_WHITE == themeStyleCurr) {
            // 浅色主题
            m_isDarkTheme = false;
        }

        QObject::connect(m_pGsettingThemeData, &QGSettings::changed, [=](const QString &key) {
            // 主题变更
            if (key == STYLE_NAME) {
                QString themeStyle = this->m_pGsettingThemeData->get(STYLE_NAME).toString();
                if (STYLE_NAME_VALUE_DARK == themeStyle || STYLE_NAME_VALUE_BLACK == themeStyle) {
                    m_isDarkTheme = true;
                    // 深色主题
                    emit GlobelBackupInfo::instance().getGlobalSignals()->styleNameChanged(true);
                } else if (STYLE_NAME_VALUE_DEFAULT == themeStyle || STYLE_NAME_VALUE_LIGHT == themeStyle || STYLE_NAME_VALUE_WHITE == themeStyle) {
                    m_isDarkTheme = false;
                    // 浅色主题
                    emit GlobelBackupInfo::instance().getGlobalSignals()->styleNameChanged(false);
                }
            } else if (key == ICON_THEME_NAME) {
                // 图标变更
                emit GlobelBackupInfo::instance().getGlobalSignals()->themeIconChanged();
            } else if ("systemFont" == key || "systemFontSize" == key) {
                // 字体大小或字体类型发生变化
                int fontSize = m_pGsettingThemeData->get("system-font-size").toInt();
                emit GlobelBackupInfo::instance().getGlobalSignals()->fontChanged(fontSize);
                emit GlobelBackupInfo::instance().getGlobalSignals()->widthChanged();
            } else if (key == COLOR_QT_KEY) {
                // 强调色变化
                emit GlobelBackupInfo::instance().getGlobalSignals()->themeColorChanged();
            }
        });
    }
}

GSettingsWrapper::~GSettingsWrapper()
{
    delete m_pGsettingThemeData;
}

