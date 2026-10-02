/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/


#ifndef Configuration_H
#define Configuration_H

#include <QObject>
#include <QMutex>
#include <QSettings>

class Configuration : public QObject
{
    Q_OBJECT
public:
    static Configuration *instance();
    Configuration(Configuration &)=delete;
    Configuration &operator =(Configuration)=delete;
public:
    void init(){}
public:
    QVariant get(QString, QString, QVariant d=QVariant());
    void set(QString, QString, QVariant);
    QVariantList gets(QString, QString);
    void sets(QString, QString, QVariantList);
private:
    explicit Configuration(QObject *parent = nullptr);
    ~Configuration();
private:
    static Configuration *m_oInstance;
    static QMutex            m_oMutex;
private:
    QSettings               *m_oSetting;
Q_SIGNALS:

};
#define CFG Configuration::instance()
#endif // Configuration_H
