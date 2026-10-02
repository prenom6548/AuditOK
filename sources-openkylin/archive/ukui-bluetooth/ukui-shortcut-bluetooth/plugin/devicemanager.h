/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QSharedPointer>
#include <QVariant>

class BtAdapter;
class BtDevice;

typedef QSharedPointer<BtDevice>  BtDevicePtr;
typedef QSharedPointer<BtAdapter> BtAdapterPtr;

class BtDevice
{
protected:
    BtDevice(QString id);

public:
    ~BtDevice();

public:
    QString id() { return m_id; }

    void setAttr(QMap<QString, QVariant>);

    QMap<QString, QVariant> getAttr(void);
    QVariant getAttr(QString);

    int getRssi(void);

    bool isMouse(void);

    bool isKeyboard(void);
protected:
    QString m_id;

    QMap<QString, QVariant> m_attrs;

    friend class BtAdapter;
};


class BtAdapter : public QObject
{
    Q_OBJECT
public:
    explicit BtAdapter(QString id);
    ~BtAdapter();

    QString id(void) {return m_id;}
    void setAttr(QMap<QString, QVariant>);
    QMap<QString, QVariant> getAttr(void);
    QVariant getAttr(QString);

    void addPairedDevice(QString id, QMap<QString, QVariant> attrs);
    void addNotPairedDevice(QString id, QMap<QString, QVariant> attrs);

    void setDeviceAttr(QString id, QMap<QString, QVariant> attrs);

    QList<QMap<QString, QVariant> > getPairedDevices(void);

    QList<QMap<QString, QVariant> > getNotPairedDevices(void);

    void removeDevice(QString id);

    void cleaDevices(void);

    int getPairedDeviceIter(const QString & id);

    int getNotPairedDeviceIter(const QString & id);

    int getKeyboardNum(void);

    int getMouseNum(void);

protected:
    void calcDeviceList(QString id);
    void calcNotPairedDeviceList(QString id, bool rssichange = false);

    void insertDevid(QString id);
protected:
    QString m_id;

    QMap<QString, QVariant> m_attrs;

    QMap<QString, BtDevicePtr> m_pairedDevices;
    QMap<QString, BtDevicePtr> m_notPairedDevices;
    QList<QString> m_connectDevices;
    QList<QString> m_noConnectDevices;
    QList<QString> m_notPairedDevicelist;
};

#endif // DEVICEMANAGER_H
