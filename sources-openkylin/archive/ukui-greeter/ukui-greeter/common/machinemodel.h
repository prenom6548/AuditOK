/*
 * Copyright (C) 2024 KylinSoftCo., Ltd.
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
#ifndef MACHINEMODEL_H
#define MACHINNEMODEL_H

#include <QObject>
#include <mutex>
#include <memory>

class MachineModel : public QObject
{
    Q_OBJECT
public:
    QString getTheMachineType();
    static std::shared_ptr<MachineModel> getMachineModelInstance();

private:
    MachineModel(QObject *parent = nullptr);
    MachineModel(const MachineModel &);
    const MachineModel &operator =(const MachineModel &) = delete;

private:
    void initMachineType();
    const QString getSysVendor() const;
    const QString getProductName() const;
    const QString getProductFamily() const;
    QString m_machineType;
    static std::shared_ptr<MachineModel> m_machineModel;
    static std::mutex m_mutex;

signals:

};

#endif // MACHINEMODEL_H
