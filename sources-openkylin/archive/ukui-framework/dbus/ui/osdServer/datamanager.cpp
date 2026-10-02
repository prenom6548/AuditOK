/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
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

#include "datamanager.h"

#include <QDebug>
#include <QPalette>
#include <QGuiApplication>

static QString rgbaToHex(const QString& rgbaStr)
{
    QStringList parts = rgbaStr.split(',');
    if (parts.size() < 3 || parts.size() > 4) {
        return "#000000"; // 默认返回黑色
    }

    bool ok = false;
    int r = parts[0].toInt(&ok);
    if (!ok || r < 0 || r > 255) {
        r = 0;
    }

    int g = parts[1].toInt(&ok);
    if (!ok || g < 0 || g > 255) {
        g = 0;
    }

    int b = parts[2].toInt(&ok);
    if (!ok || b < 0 || b > 255) {
        b = 0;
    }

    int a = (parts.size() == 4) ? parts[3].toInt(&ok) : 255;
    if (!ok || a < 0 || a > 255) {
        a = 255;
    }

    QColor color(r, g, b, a);
    return color.name(QColor::HexRgb);
}


DataManager::DataManager(QObject* parent) : QObject(parent)
{
    m_pOpacityGSettings = new UKUI::QGSettings(OPACITY_SCHEMA);
    m_pThemeGSettings = new UKUI::QGSettings(THEME_SCHEMA);

    updateThemeAndColor();

    m_opacity = m_pOpacityGSettings->get(OPACITY_KEY).toDouble();
    //将不透明度信息传递到qml
    emit opacityChanged();

    connect(m_pThemeGSettings, &UKUI::QGSettings::changed, this, &DataManager::onThemeChanged);
    connect(m_pOpacityGSettings, &UKUI::QGSettings::changed, this, &DataManager::onTransparencyChanged);
}

DataManager::~DataManager()
{
    //管理QBuffer内存
    m_pImageBuffer->close();

    delete m_pImageBuffer;

    m_pImageBuffer = nullptr;

    //管理QGSettings内存
    delete m_pOpacityGSettings;

    m_pOpacityGSettings = nullptr;

    delete m_pThemeGSettings;

    m_pThemeGSettings = nullptr;
}

void DataManager::initImage(const QString& source, QString& imageSource)
{
    int adaptiveValue = 255;

    if (m_currentTheme == LIGHT_THEME){

        adaptiveValue = 0;
    }

    QString sourcePath = LOCAL_ICON_PREFIX + source + ".svg";

    QIcon sourceIcon = QIcon::fromTheme(source, QIcon(sourcePath));

    QImage sourceImage = sourceIcon.pixmap(ICON_SIZE, ICON_SIZE).toImage();

    for (int x = 0; x < sourceImage.width(); x++) {
        for (int y = 0; y < sourceImage.height(); y++) {

            QColor color = sourceImage.pixelColor(x, y);

            //图像中的透明像素点不管
            if (color.alpha() > 0) {

                //因为要保留原图片有色像素点的透明度，所以继续用color去设置像素颜色
                color.setRed(adaptiveValue);
                color.setGreen(adaptiveValue);
                color.setBlue(adaptiveValue);
                sourceImage.setPixelColor(x, y, color);
            }
        }
    }

    QByteArray imageData;

    m_pImageBuffer = new QBuffer(&imageData);

    m_pImageBuffer->open(QIODevice::WriteOnly);

    if (!sourceImage.save(m_pImageBuffer, "PNG")) {

        return;
    }

    // 将 QByteArray 转换为 QUrl
    QUrl imageUrl("data:image/png;base64," + imageData.toBase64());

    imageSource = imageUrl.toString();
}


void DataManager::updateThemeAndColor()
{
    //重新获取主题的值
    m_currentTheme = m_pThemeGSettings->get(THEME_KEY).toString();

    m_themeColor = rgbaToHex(m_pThemeGSettings->get(THEME_COLOR_KEY).toString());

    //刷新颜色
    if (m_currentTheme == LIGHT_THEME) {
        m_currentBackgroundColor = QGuiApplication::palette().light().color().name(QColor::HexRgb);
        m_currentContentColor = QGuiApplication::palette().dark().color().name(QColor::HexRgb);
    } else {
        m_currentBackgroundColor = QGuiApplication::palette().dark().color().name(QColor::HexRgb);
        m_currentContentColor = QGuiApplication::palette().light().color().name(QColor::HexRgb);
    }

    //刷新qml值来改变背景颜色
    emit themeChanged();
}

void DataManager::onInitIconData(const QString& sourceName)
{
    //刷新图像名
    m_currentIconImageName = sourceName;
    initImage(m_currentIconImageName, m_iconImageSource);

    //图片改完发出信号刷新qml的数据及显示
    emit iconImageSourceChanged();
}

void DataManager::onInitBarData(const QString& sourceName, double currentData, double maxData)
{
    m_currentBarImageName = sourceName;

    initImage(m_currentBarImageName, m_barImageSource);

    emit barImageSourceChanged();

    m_progressRatio = currentData / maxData;

    emit ratioChanged();
}

void DataManager::onThemeChanged()
{
    updateThemeAndColor();

    initImage(m_currentIconImageName, m_iconImageSource);
    emit iconImageSourceChanged();

    initImage(m_currentBarImageName, m_barImageSource);
    emit barImageSourceChanged();
}

void DataManager::onTransparencyChanged(const QString &key)
{
    if (key != OPACITY_KEY) {
        return;
    }

    m_opacity = m_pOpacityGSettings->get(OPACITY_KEY).toDouble();
    emit opacityChanged();
}

