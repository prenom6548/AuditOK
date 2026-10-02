/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UKUIGTKCONFIG_H
#define UKUIGTKCONFIG_H

#include <QObject>
#include <QMap>
#include <QColor>
#include "QGSettings/qgsettings.h"
class UkuiGtkConfig : public QObject
{
    Q_OBJECT
public:
    UkuiGtkConfig(QObject *parent = nullptr);
    ~UkuiGtkConfig();

    /**
     * @brief connectGsettingSignal
     */
    void connectGsettingSignal();

    /**
     * @brief addImportStatementsToGtkCssUserFile
     */
    void addImportStatementsToGtkCssUserFile();

    /**
     * @brief modifyColorsCssFile
     * @param colorsDefinitions
     */
    void modifyColorsCssFile(QString colorsDefinitions);

    /**
     * @brief converRGBToHex
     * @param color
     * @return
     */
    QString converRGBToHex(QColor color);
public Q_SLOTS:
    void doGsettingsChanged(QString key);
    void doMateSettingsChanged(const QString &key);
private:
     QGSettings *m_colorGsettings = nullptr;
     //mate.interface
     QGSettings *m_gtkThemeGsettings = nullptr;
     //gnome.desktop.interface
     QGSettings *m_gnomeGtkGsettings = nullptr;

     const QMap<QString, QColor> m_themeColorConfig = {
         {"jamPurple",    QColor(120, 115, 245)},
         {"magenta",      QColor(235, 48, 150) },
         {"sunRed",       QColor(243, 34, 45)  },
         {"sunsetOrange", QColor(246, 140, 39) },
         {"dustGold",     QColor(249, 197, 61) },
         {"polarGreen",   QColor(82, 196, 41)  },
         {"gridGreen",    QColor(37, 142, 139) }
     };
     const QColor m_defaultColor = QColor(55, 144, 250);
};

#endif // UKUIGTKCONFIG_H
