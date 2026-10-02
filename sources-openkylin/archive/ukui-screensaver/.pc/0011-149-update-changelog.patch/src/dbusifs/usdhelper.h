/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef USDHELPER_H
#define USDHELPER_H

#include <QDBusConnection>
#include <QVariantMap>
#include <QObject>

class UsdHelper : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造
     *
     * @param parent 父指针
     */
    explicit UsdHelper(QObject *parent = nullptr);
    /**
     * @brief 析构
     *
     */
    virtual ~UsdHelper();

    bool usdExternalDoAction(int actionType);

private:
    void init();

private:
    QDBusInterface *usdInterface = nullptr;
};

#endif // USDHELPER_H
