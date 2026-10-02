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

#include "gamma-manager-gtkconfig.h"
#include <QPalette>

#include <QString>
#include <QVariant>
#include <QDebug>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>

#include "clib-syslog.h"

#define THEME_COLOR        "theme-color"
#define GTK_THEME          "gtk-theme"
#define GTK_CURSOR_SIZE    "cursor-size"
#define GTK_CURSOR_BLINK_TIME "cursor-blink-time"
#define GTK_ICON_THEME      "icon-theme"
UkuiGtkConfig::UkuiGtkConfig(QObject *parent)
{
    const QByteArray ukuiId(UKUI_STYLE_SCHEMA);
    const QByteArray mateId(MATE_INTERFACE_SCHEMA);
    const QByteArray gnomeId(GNOME_DESKTOP_INTERFACE);

    if (!m_colorGsettings && QGSettings::isSchemaInstalled(UKUI_STYLE_SCHEMA)) {
        m_colorGsettings = new QGSettings(ukuiId);
    } else {
        USD_LOG(LOG_DEBUG, "%s is not install", UKUI_STYLE_SCHEMA);
    }

    QVariant gtkTheme(QVariant::Invalid);
    if (!m_gtkThemeGsettings && QGSettings::isSchemaInstalled(MATE_INTERFACE_SCHEMA)) {
        m_gtkThemeGsettings = new QGSettings(mateId);
        if (m_gtkThemeGsettings->isOK()) {
            if (m_gtkThemeGsettings->keys().contains(GTK_THEME)) {
                gtkTheme = m_gtkThemeGsettings->get(GTK_THEME);
            }

            if (nullptr !=  m_colorGsettings) {
                doGsettingsChanged(THEME_COLOR);
            }
        }
    } else {
        USD_LOG(LOG_DEBUG, "%s is not install", MATE_INTERFACE_SCHEMA);
    }

    // wayland 通过设置gnome.desktop.interface gtk-theme 实现主题切换
    if (m_gtkThemeGsettings && QGSettings::isSchemaInstalled(GNOME_DESKTOP_INTERFACE)) {
        m_gnomeGtkGsettings = new QGSettings(gnomeId);
        if (m_gnomeGtkGsettings->isOK()) {
            Q_FOREACH(auto & key, m_gtkThemeGsettings->keys()) {
                m_gnomeGtkGsettings->set(key, m_gtkThemeGsettings->get(key));
            }
        }
    } else {
        USD_LOG(LOG_DEBUG, "%s is not install", GNOME_DESKTOP_INTERFACE);
    }
}

UkuiGtkConfig::~UkuiGtkConfig(){
    if (m_colorGsettings) {
        delete m_colorGsettings;
        m_colorGsettings = nullptr;
    }

    if (m_gtkThemeGsettings) {
        delete m_gtkThemeGsettings;
        m_gtkThemeGsettings = nullptr;
    }

    if (m_gnomeGtkGsettings) {
        delete m_gnomeGtkGsettings;
        m_gnomeGtkGsettings = nullptr;
    }
}

void UkuiGtkConfig::connectGsettingSignal(){
    QObject::connect(m_colorGsettings, SIGNAL(changed(QString))/*&QGSettings::changed*/, this, SLOT(doGsettingsChanged(QString)));
    QObject::connect(m_gtkThemeGsettings, SIGNAL(changed(QString))/*&QGSettings::changed*/, this, SLOT(doMateSettingsChanged(QString)));
}

void UkuiGtkConfig::addImportStatementsToGtkCssUserFile(){
    QString gtkStylePath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/gtk-3.0");

    QDir gtkStyle(gtkStylePath);
    if (!gtkStyle.exists(gtkStylePath)) {
        gtkStyle.mkdir(gtkStylePath);
    }

    QString gtkCssPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/gtk-3.0/gtk.css");
    QFile gtkCss(gtkCssPath);

    if (gtkCss.open(QIODevice::ReadWrite)) {
        QByteArray gtkCssContents = gtkCss.readAll().trimmed();

        static const QVector<QByteArray> importStatements{
            QByteArrayLiteral("\n@import 'colors.css';"),
        };

        for (const auto &statement : importStatements) {
            if (!gtkCssContents.contains(statement.trimmed())) {
                gtkCssContents.append(statement);
            }
        }

        gtkCss.remove();
        gtkCss.open(QIODevice::WriteOnly | QIODevice::Text);
        gtkCss.write(gtkCssContents);
    }
}

void UkuiGtkConfig::modifyColorsCssFile(QString colorsDefinitions){
    QString colorsCssPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/gtk-3.0/colors.css");
    QFile colorsCss(colorsCssPath);

    if (colorsCss.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QTextStream colorsCssStream(&colorsCss);
        colorsCssStream << endl << QStringLiteral("@define-color hover_bg_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color hover_bd_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color active_bg_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color active_bd_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color selected_bg_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color selected_borders_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color bt_hover_bg_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color bt_hover_bd_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color bt_active_bg_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color bt_active_bd_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color bt_selected_bg_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color bt_selected_borders_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color crb_active_bg_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color crb_hover_bd_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color crb_active_bd_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color success_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color base_active_bg_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color base_prelight_bg_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color base_selected_bg_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color base_checked_bg_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color base_active_border_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color base_prelight_border_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color base_active_bg_image_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color base_checked_bg_image_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color trough_filled_space_normal_color_ukui ")+colorsDefinitions+QStringLiteral(";");
        colorsCssStream << endl << QStringLiteral("@define-color toolbar_button_bg_active_color_ukui ")+colorsDefinitions+QStringLiteral(";");
    }
}


QString UkuiGtkConfig::converRGBToHex(QColor color){
    USD_LOG(LOG_DEBUG,"%d:%d:%d",color.red(),color.green(), color.blue());
    QString RGBred = QString("%1").arg(color.red(),2,16,QChar('0'));
    QString RGBgreen = QString("%1").arg(color.green(),2,16,QChar('0'));
    QString RGBblue = QString("%1").arg(color.blue(),2,16,QChar('0'));

    QString RGBToHex = "#"+RGBred+RGBgreen+RGBblue;

    return RGBToHex;
}

void UkuiGtkConfig::doGsettingsChanged(QString key)
{
    if (key == THEME_COLOR) {
        qDebug() << endl << m_colorGsettings->get(THEME_COLOR).toString();
        qDebug() << endl << m_gtkThemeGsettings->get(GTK_THEME).toString();
        QString gtkTheme = m_gtkThemeGsettings->get(GTK_THEME).toString();
        QString themeColor = m_colorGsettings->get(THEME_COLOR).toString();
        USD_LOG(LOG_DEBUG,"gtkTheme:%s",gtkTheme.toLatin1().data());
        USD_LOG(LOG_DEBUG,"themeColor:%s",themeColor.toLatin1().data());

        if(gtkTheme == "ukui-white" ||
            gtkTheme == "Menta" ||//make sure accent color can be set when the key are default value.
            gtkTheme == "ukui-light" ||
            gtkTheme == "ukui-black"){

            addImportStatementsToGtkCssUserFile();

            QStringList parts = themeColor.split(",");
            if(parts.size() < 3) {
                return;
            }
            int r = parts[0].toInt();
            int g = parts[1].toInt();
            int b = parts[2].toInt();

            QColor colorRGB(r, g, b);
            // qDebug() << endl << colorRGB << endl << themeColor;
            QString colorHex = converRGBToHex(colorRGB);
            // qDebug() << endl << colorRGB << endl << colorHex;
            USD_LOG(LOG_DEBUG,"colorHex:--->%s",colorHex.toLatin1().data());
            modifyColorsCssFile(colorHex);

        } else {
            qDebug() << gtkTheme;
        }
    }
}

void UkuiGtkConfig::doMateSettingsChanged(const QString& key)
{
    if (m_gnomeGtkGsettings != nullptr) {
        if (m_gnomeGtkGsettings->isOK()){
            if (m_gnomeGtkGsettings->keys().contains(key)) {
                m_gnomeGtkGsettings->set(key, m_gtkThemeGsettings->get(key));
            }
        }
    }

}
