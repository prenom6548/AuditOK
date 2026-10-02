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

#ifndef PUBLICSTATUS_H
#define PUBLICSTATUS_H

#include <QObject>
namespace KylinRubbishClear
{

class PublicStatus : public QObject
{
    Q_OBJECT
public:
    static PublicStatus *getInstance();
    bool getIsClearTrash()
    {
        return isClearTrash;
    }
    void setIsClearTrash(bool isClear)
    {
        this->isClearTrash = isClear;
    }
    bool getIsSystemCacheNull()
    {
        return isSystemCacheNull;
    }
    void setIsSystemCacheNull(bool flag)
    {
        this->isSystemCacheNull = flag;
    }
    bool getIsCookiesNull()
    {
        return isCookiesNull;
    }
    void setIsCookiesNull(bool flag)
    {
        this->isCookiesNull = flag;
    }
    bool getIsTraceNull()
    {
        return isTraceNull;
    }
    void setIsTraceNull(bool flag)
    {
        this->isTraceNull = flag;
    }

private:
    PublicStatus(QObject *parent = nullptr);
    bool isClearTrash = true;
    bool isSystemCacheNull = false;
    bool isCookiesNull = false;
    bool isTraceNull = false;
};
} // namespace KylinRubbishClear


#endif // PUBLICSTATUS_H
