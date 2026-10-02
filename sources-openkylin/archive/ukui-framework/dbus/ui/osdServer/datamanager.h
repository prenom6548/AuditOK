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

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "osd_string.h"

#include <QObject>
#include <QIcon>
#include <QUrl>
#include <QBuffer>
#include "qgsettings.h"

class DataManager : public QObject{

    Q_OBJECT

    //暴露给qml的属性以及指定它的读函数和数据改变信号
    //这里函数就不是函数而是属性读取器，在qml中直接调用属性而不是函数
    Q_PROPERTY(QString m_iconImageSource READ giveIconImageSource NOTIFY iconImageSourceChanged)
    Q_PROPERTY(QString m_barImageSource READ giveBarImageSource NOTIFY barImageSourceChanged)

    Q_PROPERTY(double m_opacity READ giveOpacity NOTIFY opacityChanged)

    Q_PROPERTY(QString m_currentBackgroundColor READ giveBackgroundColor NOTIFY themeChanged)
    Q_PROPERTY(QString m_currentContentColor READ giveContentColor NOTIFY themeChanged)
    Q_PROPERTY(QString m_themeColor READ giveThemeColor NOTIFY themeChanged)

    Q_PROPERTY(double m_progressRatio READ giveRatio NOTIFY ratioChanged)

public:

    explicit DataManager(QObject* parent = nullptr);

    ~DataManager();

private:

    UKUI::QGSettings* m_pOpacityGSettings = nullptr;
    UKUI::QGSettings* m_pThemeGSettings = nullptr;

    double m_opacity = 0.65;

    QString m_currentTheme = "";

    //暂存QImage
    QBuffer* m_pImageBuffer = nullptr;

    QString m_iconImageSource = "";
    QString m_barImageSource = "";

    //将当前的图片名存储起来，切换主题时使用
    QString m_currentIconImageName = "";
    QString m_currentBarImageName = "";

    //当前的主题背景颜色和相反的内容颜色
    QString m_currentBackgroundColor = "";
    QString m_currentContentColor = "";
    QString m_themeColor = "";

    double m_progressRatio = 0.0;

private:

    void initImage(const QString& source, QString& imageSource);

    void updateThemeAndColor();

public:

    QString giveIconImageSource() { return m_iconImageSource; }
    QString giveBarImageSource() { return m_barImageSource; }

    double giveOpacity() {return m_opacity;}

    QString giveBackgroundColor() { return m_currentBackgroundColor; }
    QString giveContentColor() { return m_currentContentColor; }
    QString giveThemeColor() { return m_themeColor; }

    double giveRatio() { return m_progressRatio; }

signals:

    void iconImageSourceChanged();
    void barImageSourceChanged();
    void opacityChanged();
    void themeChanged();
    void ratioChanged();

public slots:

    void onInitIconData(const QString& sourceName);
    void onInitBarData(const QString& sourceName, double currentData, double maxData);

    void onThemeChanged();

    void onTransparencyChanged(const QString &key);

};

#endif // DATAMANAGER_H
