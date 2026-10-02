/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 * Authors: LiKe <like1@kylinos.cn>
 *
 */
#ifndef GLOBALTHEMEWIDGET_H
#define GLOBALTHEMEWIDGET_H
#include "globalthemefeature.h"
#include "../fileProcess/datacollect.h"
#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QPainter>
#include <QScrollArea>
#include <QLabel>
#include <QMap>
#include <QFileDialog>
#include <QStackedWidget>
#include <QComboBox>
#include "historywidget.h"
#include "basewidget.h"
#include <QCryptographicHash>
#include <QIntValidator>
#include <QPalette>

class GlobalThemeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GlobalThemeWidget(QWidget *parent = nullptr);
    QString getWallpaperPath();

    void eidtInitWidget(const HistoryInfo &InfoData);
    void refresh();
    static bool g_themeChange;
    QMap<QString, bool> *m_themeChangeMap;
    QMap<QString, bool> *m_globalPresenceMap;
    void useMaximumScale();
    void useRestoreScale();
signals:
    void wallpaperupdate(const QString& filePath);
    void newWallpaperFilePath(const QString& path);
    void newCoverFilePath(const QString& path);
    void newRadius(const int radius);
    void newAccentColor(const QColor selectedColor);
    void newExterior(const QColor selectedColor);
    void newTransparency(const int transparency);
    void newWindowRadius(const int radius);
    void globalthemeChange(QMap<QString, bool> *globalthemeChangeMap);
    void build();
private:
    void initPreviewWidget();
    void initRightWidget();

    void initCoverWidget();
    void initWallPaperWidget();
    void initExteriorWidget();
    void initFilletWidget();
    void initAccentColorWidget();
    void initTransparencyWidget();
    void initWindowFilletWidget();
    void initbuildWidget();
    void addspaceritem();

    void setStyle(QString style);
    void setRadiusSetting(int radius);
    void setAccentColor(QString accentcolor);
    void setTransparency(int transparency);
    void setCover(QString coverpath);
    void setWallpaper(QString wallpaperpath);
    void setWindowRadius(int windowradius);
    QWidget *m_globalthemewidget;
    QWidget *m_coverwidget;
    QWidget *m_wallpaperwidget;
    QWidget *m_exteriorwidget;
    QWidget *m_filletwidget;
    QWidget *m_accentcolorwidget;
    QWidget *m_transparencywidget;
    QWidget *m_windowfilletwidget;
    QWidget *m_buildwidget;
    QVBoxLayout *m_previewlayout;
    QString m_wallpaperpath = ":/resource/background/background-light.png";
    QString m_coverpath = ":/resource/background/custom-preview.png";
    QWidget *m_viewportwidget;
    QScrollArea *m_scrollArea;
    QSlider *m_filletslider;
    QSlider *m_transparencyslider;
    QHBoxLayout *m_globalthemelayout;
    QWidget *m_rightwidget;
    HistoryInfo m_info;
    QComboBox *m_combobox;
    QComboBox *m_filletcombobox;
    QComboBox *m_exteriorcombobox;
    EditButton *m_coverbtn;
    EditButton *wallpaperbtn;
    BasePreviewWidget *m_previewwidget;
    GlobalImageWidget *m_preview;
};

#endif // GLOBALTHEMEWIDGET_H
