/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 */

#ifndef SYSTEMDBUSPROXY
#define SYSTEMDBUSPROXY

#include "customdata.h"

#include <QObject>
#include <QScopedPointer>


namespace KylinRubbishClear
{

class SystemDbusHandler;

class SystemDbusProxy : public QObject
{
    Q_OBJECT
public:
    explicit SystemDbusProxy(QObject *parent = 0);
    ~SystemDbusProxy();

    QString demoInfo();
    bool userIsActive(const QString &user, bool active);
    CustomData getCustomData();
    void sendCustomData(const CustomData &message);
    void exitService();

Q_SIGNALS:
    void reportAlert(int ret, const QString &description);

private:
    SystemDbusHandler *m_handler = nullptr;
};

} // namespace KylinRubbishClear
#endif
