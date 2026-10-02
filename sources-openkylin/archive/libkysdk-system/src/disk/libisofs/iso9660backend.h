/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#ifndef ISO9660BACKEND_H
#define ISO9660BACKEND_H

class DataCDROMImpl;

class Iso9660Backend
{
public:
    Iso9660Backend() {}
    virtual ~Iso9660Backend() {}

    virtual bool open() = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
    virtual int read( unsigned int sector, char* data, int len ) = 0;
};

class Iso9660DeviceBackend : public Iso9660Backend
{
public:
    explicit Iso9660DeviceBackend( DataCDROMImpl * dev );
    ~Iso9660DeviceBackend() override;

    bool open() override;
    void close() override;
    bool isOpen() const override { return m_isOpen; }
    int read( unsigned int sector, char* data, int len ) override;

private:
    DataCDROMImpl *m_device;
    bool m_isOpen;
};


#endif // ISO9660BACKEND_H
