/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef UKUI_SNI_STATUS_NOTIFIER_HOST_H
#define UKUI_SNI_STATUS_NOTIFIER_HOST_H
#include <QObject>
#include "status-notifier-item.h"
namespace UkuiSni {
class StatusNotifierHostPrivate;
class StatusNotifierHost : public QObject
{
    Q_OBJECT
public:
    static StatusNotifierHost *self();
    ~StatusNotifierHost() override;

    void registerHost();
    QList<StatusNotifierItem *> items();
    const QList<QString> services() const;

    UkuiSni::StatusNotifierItem *itemForService(const QString service);
Q_SIGNALS:
    void itemAdded(const QString &service);
    void itemRemoved(const QString &service);
private:
    explicit StatusNotifierHost(QObject *parent = nullptr);
    StatusNotifierHostPrivate *d = nullptr;

};
}

#endif //UKUI_SNI_STATUS_NOTIFIER_HOST_H
