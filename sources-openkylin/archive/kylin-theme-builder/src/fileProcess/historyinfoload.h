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
#ifndef HISTORYINFOLOAD_H
#define HISTORYINFOLOAD_H

#include <QObject>
#include <QMetaType>


struct HistoryInfo {
    bool isEdit = false;
    QString filepath = "";
    QString historytime = "";
    QString coverpath = "";
    QString themecnname = "";
    QString themename = "";
    QString themebuilder = "";
    QString thememail = "";
    QString themetype = "";
    QString wallpaperpath = "";
    QString accentcolor = "";
    QString widgetstyle = "";
    int radius = 6;
    int transparency = 65;
    int windowradius = 6;
};

Q_DECLARE_METATYPE(HistoryInfo)
class HistoryInfoLoad : public QObject {
    Q_OBJECT
public:


    explicit HistoryInfoLoad(const QString& date, QObject* parent = nullptr);
    QString getCover();
    QString getThemeCNName();
    QString getThemeName();
    QString getThemeBuilder();
    QString getThemeMail();
    QString getThemeType();
    void getRadius();
    void getWallpaper();
    void getAccentColor();
    void getTransparency();
    void getWidgetStyle();
    void getWindowRadius();
    HistoryInfo getInfoData();
    void updateHistoryInfo();
signals:

private:
    HistoryInfo m_historyInfo;
};

#endif // HISTORYINFOLOAD_H
