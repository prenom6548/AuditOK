/*
 *
 * Copyright (C) 2005-2009 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2009 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */


#ifndef ISO9660BACKEND_H
#define ISO9660BACKEND_H

#include <QString>

class DataCDROM;

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
    explicit Iso9660DeviceBackend( DataCDROM * dev );
    ~Iso9660DeviceBackend() override;

    bool open() override;
    void close() override;
    bool isOpen() const override { return m_isOpen; }
    int read( unsigned int sector, char* data, int len ) override;

private:
    DataCDROM *m_device;
    bool m_isOpen;
};


#endif // ISO9660BACKEND_H
