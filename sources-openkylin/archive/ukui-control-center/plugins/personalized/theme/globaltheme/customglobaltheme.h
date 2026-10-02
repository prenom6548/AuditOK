/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef CUSTOMGLOBALTHEME_H
#define CUSTOMGLOBALTHEME_H

#include "globaltheme.h"

#include <QPixmap>

class QGSettings;
class QSettings;

class CustomThumbnailer : public QObject
{
    Q_OBJECT
public:
    explicit CustomThumbnailer(QObject *parent = nullptr);

signals:
    void thumbnailUpdated(const QPixmap &pixmap);

public slots:
    void startLoad();

};

class CustomGlobalTheme : public GlobalTheme
{
    Q_OBJECT
public:
    explicit CustomGlobalTheme(QObject *parent = nullptr);
    ~CustomGlobalTheme();

    QString getThemeName() override;
    QString getLocaleThemeName() override;

    bool getSupportLightDarkMode() override;
    QString getDefaultLightDarkMode() override;

    QString getWidgetStyleName() override;
    QString getQtLightThemeName() override;
    QString getQtDarkThemeName() override;
    QString getQtMixedThemeName() override;
    QString getQtAccentRole() override;

    QString getGtkThemeName() override;
    QString getGtkLightThemeName() override;
    QString getGtkDarkThemeName() override;
    QString getGtkAccent() override;

    QString getIconThemeName() override;
    QString getCursorThemeName() override;
    QString getSoundThemeName() override;
    QString getWallPaperPath() override;

    bool getSupportBlur() override;
    bool getBlurEnabled() override;

    bool getSupportTransparency() override;
    int getTransparencyBlur() override;
    int getTransparencyNoBlur() override;

    bool getSupportAnimation() override;
    int getAnimationDuration() override;

    bool getSupportRadius() override;
    QString getRadiusValue() override;

    void loadThumbnail() override;
    QPixmap getThumbnail() override;
    void updateThumbnail(const QPixmap &pixmap) override;

    bool isModified();
    void loadFromOtherGlobalTheme(GlobalTheme *other);
    void updateValue(const QString &functionName, const QVariant &value);

public Q_SLOTS:
    void onGlobalSettingsChanged(const QString &key);

private:
    QGSettings *gsettings = nullptr;

    QSettings *qsettings = nullptr;
    QPixmap pixmap;
    CustomThumbnailer* m_customThumbnailer = nullptr;
};

#endif // CUSTOMGLOBALTHEME_H
