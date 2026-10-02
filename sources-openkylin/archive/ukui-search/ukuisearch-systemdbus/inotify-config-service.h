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

#ifndef INOTIFY_CONFIG_SERVICE_H
#define INOTIFY_CONFIG_SERVICE_H

#include <QObject>
#include <ukui-service-interface/serviceinterface.h>


class InotifyConfigService : public QObject, public UKUI::ServiceInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUIServicePluginInterface_iid FILE "metadata.json")
    Q_INTERFACES(UKUI::ServiceInterface)

public:
    explicit InotifyConfigService(QObject *parent = nullptr);
    QList<UKUI::ServiceObject *> getServiceObjects() override;

};



#endif //INOTIFY_CONFIG_SERVICE_H
