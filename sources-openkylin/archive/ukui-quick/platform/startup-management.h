/*
*  * Copyright (C) 2025, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef STARTUP_MANAGEMENT_H
#define STARTUP_MANAGEMENT_H

#include <QObject>
namespace UkuiQuick {
class StartupManagementPrivate;
class StartupManagement : public QObject
{
    Q_OBJECT
public:
    static StartupManagement* self();
    ~StartupManagement() override;

    void setStartupGeometry(uint32_t pid, int32_t x, int32_t y, uint32_t width, uint32_t height);

private:
    explicit StartupManagement(QObject *parent = nullptr);
    StartupManagementPrivate* d = nullptr;

};
}

#endif //STARTUP_MANAGEMENT_H
