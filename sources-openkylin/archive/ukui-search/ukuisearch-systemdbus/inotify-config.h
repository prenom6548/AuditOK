/*
* Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#ifndef INOTIFY_CONFIG_H
#define INOTIFY_CONFIG_H

#include <ukui-service-interface/serviceobject.h>

class InotifyConfig : public UKUI::ServiceObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.ukui.search.qt.systemdbus")
public:
    explicit InotifyConfig(QObject *parent = nullptr);

public Q_SLOTS:
    QString setInotifyMaxUserWatchesStep2();
    int AddInotifyMaxUserInstance(int addNum);

};



#endif //INOTIFY_CONFIG_H
