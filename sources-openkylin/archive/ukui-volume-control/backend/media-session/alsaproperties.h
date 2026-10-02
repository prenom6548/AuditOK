/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#pragma once
//#include <QObject>
//#include <QString>

#include <string>

class PipewireClient;

class AlsaProperties /*: public QObject*/
{
//    Q_OBJECT
//    Q_PROPERTY(bool batchDisabled READ batchDisabled WRITE setBatchDisabled NOTIFY batchDisabledChanged)
//    Q_PROPERTY(int periodSize READ periodSize WRITE setPeriodSize NOTIFY periodSizeChanged)
//signals:
    void batchDisabledChanged(bool);
    void periodSizeChanged(int);

private:
    std::string globalConf;
    std::string userConf;

    bool _batchDisabled = false;
    int _periodSize = 1024;

public:
    explicit AlsaProperties(PipewireClient *client/*, QObject *parent = nullptr*/);
    virtual ~AlsaProperties() = default;

    [[nodiscard]] bool batchDisabled() const { return _batchDisabled; }
    [[nodiscard]] int periodSize() const { return _periodSize; }
    void setBatchDisabled(bool disabled);
    void setPeriodSize(int newPeriod);

private:
    void readGlobalConf();
    void readUserConf();
    void readConf(const std::string& filename);
    void writeUserConf() const;
};
