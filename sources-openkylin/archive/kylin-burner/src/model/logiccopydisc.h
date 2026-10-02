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


#ifndef LOGICCOPYDISC_H
#define LOGICCOPYDISC_H

#include <QObject>
#include <QMutex>

class LogicCopyDisc : public QObject
{
    Q_OBJECT
    friend class LogicCopyDiscTestFriend;
    friend class CopyDiscTest;
    friend class LogicCopyDiscTest;
public:
    static LogicCopyDisc *instance();
public:
    LogicCopyDisc(const LogicCopyDisc*)=delete;
    LogicCopyDisc *operator =(const LogicCopyDisc*)=delete;
public:
    bool isDestinationDiscDL();
    bool capacity();
    void updateISOPath(QString);
private:
    explicit LogicCopyDisc(QObject *parent = nullptr);
public:
    ~LogicCopyDisc();

private:
    void progressWidgetDelayResponeTimer(int ms);
private slots:
    void slotsAddDestination(void *);
    void slotsDestinationChange(int i) { m_iDestination = i; }
    void slotsAddSource(void *);
    void slotsSourceChange(int i) { m_iSource = i; }
    void slotsNoMedium(void *);
    void slotsSetting();
    void slotsGetISO();
    void slotsCopyData();
private:
    static LogicCopyDisc *m_oInstance;
    static QMutex          m_oMutex;
private:
    int                    m_iSource;
    int                    m_iDestination;
    QList<void *>          m_oSources;
    QList<void *>          m_oDestinations;
Q_SIGNALS:
    void sigAddDestination(void *);
    void sigDelDestination(int);
    void sigAddSource(void *);
    void sigDelSource(int);
    void setting(void *);
};
#endif // LOGICCOPYDISC_H
