/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QString>
class QSettings;

class Preferences
{
public:
    Preferences();
    ~Preferences();

    void initInfo();
    void reset();
    void save();
    void load();
    void getmaxWindowNum();
    bool getAppNumLimitConfig();



    int maxWindowNum;
    bool m_KmreAutoStart = true;
    bool m_AppNumLimit = true;
    QString displayType;
    QString cpuType;
    QString gpuVendor;
    QString gpuModel;

    QSettings *m_settings = nullptr;
};

#endif // PREFERENCES_H
