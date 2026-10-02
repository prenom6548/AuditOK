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

#ifndef MAININTERFACE_H
#define MAININTERFACE_H

#include "./module/iconwidgetfeature.h"
#include "./module/iconthemewidget.h"
#include "./module/globalthemewidget.h"
#include "./module/cursorthemewidget.h"
#include "./module/infocreatewidget.h"
#include "./module/plymouththemewidget.h"
#include "./module/grubthemewidget.h"

#include "titlebar.h"
#include "./module/cacheconfirmedwidget.h"
#include "./module/historywidget.h"
#include "./module/basewidget.h"
#include "./fileProcess/datacollect.h"

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QPainter>
#include <QScrollArea>
#include <QLabel>
#include <QMap>
#include <QDebug>
#include <QFileDialog>
#include <QStackedWidget>
#include <knavigationbar.h>

class MainInterface : public QWidget
{
    Q_OBJECT
public:
    explicit MainInterface(QWidget *parent = nullptr);
    void isHistory(bool ishistory);
    void setIconTheme();
    void setGlobalTheme();
    void setCursorTheme();
    void setPlymouthTheme();
    void setGrubTheme();

    void getThemeInfo(const HistoryInfo &InfoData);
    void updateButtonGroup(bool isGlobalTheme);
    void updateThumbnail(QString thumbnailPath);
    //初始化和刷新时使用
    QMap<QString, QString>* getAppIconsMap();
    QMap<QString, QString>* getSystemIconsMap();
    QMap<QString, QString>* getPlaceIconsMap();
    QMap<QString, QString> *getDeviceIconsMap();
    QMap<QString, QString>* getCurosrMap();
    QMap<QString, QString>* getTimeCurosrMap();

    void refresh(QString themeType);
    void startShowPlymouth();

    void useMaximumScale();
    void useRestoreScale();

    void hideNavigation(bool isglobal);

    void setBuildBtnText(bool isglobal);
signals:
    void wallpaperupdate(const QString& filePath);
    void wallpaperPathChanged(QString path);
    void coverPathChanged(QString path);
    void radiusChanged(int radius);
    void transparencyChanged(int transparency);
    void windowRadiusChanged(int windowRadius);
    void accentColorChanged(QColor accentcolor);
    void exteriorChanged(QColor exteriorcolor);
    void appIconsMapChanged(QMap<QString, QString> *appiconsmap);
    void systemIconsMapChanged(QMap<QString, QString> *systemiconsmap);
    void placeIconsMapChanged(QMap<QString, QString> *placeiconsmap);
    void deviceIconsMapChanged(QMap<QString, QString> *deviceiconsmap);
    void cursorMapChanged(QMap<QString, QString> *appiconsmap);
    void timeCursorMapChanged(QMap<QString, QString> *systemiconsmap);
    void plymouthPathChanged(QString path);
    void grubPathChanged(QString path);
    void deleteIcon(QString widgetName);
    void refreshPlymouthDir();
    void startCopy();
    void gohomesignals();
    void globalthemeChange(QMap<QString, bool> *globalthemeChangeMap);
    void iconthemeChange(QMap<QString, bool> *iconthemeChangeMap);
    void cursorthemeChange(QMap<QString, bool> *cursorthemeChangeMap);
    void hasGetThemeName();
private:
    void init();
    void initLeftBar();

    bool m_ishistory = false;
    QMap<QString, QString> *m_iconpathmap;

    IconThemeWidget *m_iconthemewidget;
    GlobalThemeWidget *m_globalthemewidget;
    CursorThemeWidget *m_cursorthemewidget;
    PlymouthThemeWidget *m_plymouththemewidget;
    GrubThemeWidget *m_grubthemewidget;
    QStackedWidget *m_themestackedwidget;
    InfoCreateWidget *m_info;

    QHBoxLayout *m_layout;
    QWidget *m_navigationwidget;
    kdk::KNavigationBar *m_navigation;
    QStandardItem *m_globalbtn;
    QStandardItem *m_iconbtn;
    QStandardItem *m_cursorbtn;
    QStandardItem *m_plymouthbtn;
    QStandardItem *m_grubbtn;

    QStandardItemModel*m_model;
    QListView* m_list;


    QPushButton *m_buildbtn;
    QWidget *m_belowwidget;

};

#endif // MAININTERFACE_H
