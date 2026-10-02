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


#ifndef LOGICBURNIMAGE_H
#define LOGICBURNIMAGE_H

#include <QObject>
#include <QMutex>
#include <QFlags>

enum BurnImageMediaStatus {
    BIMAGE_NOTFOUND = 1,
    BIMAGE_NonEmpty = 2,
    BIMAGE_Empty_NOTSUPPORT = 3,
    BIMAGE_Empty_SUPPORT = 4
};
Q_DECLARE_FLAGS( BurnImageMediaStatuss, BurnImageMediaStatus )

class LogicBurnImageTestFriend;
class LogicBurnImageTest;
class LogicBurnImage : public QObject
{
    Q_OBJECT
    friend class LogicBurnImageTestFriend; // 允许测试类访问私有成员
    friend class LogicBurnImageTest;
public:
    static LogicBurnImage *instance();
public:
    LogicBurnImage(const LogicBurnImage*)=delete;
    LogicBurnImage *operator =(const LogicBurnImage*)=delete;
public:
    void init();
    bool check();
    BurnImageMediaStatus checkBurner();
    bool isHasBurner();
    bool isDLDisc();
    QIcon mediumIcon(int);
    QString display(int);
    QString tooltip(int);
    QString  currentDeviceUuid();
public slots:
    void onNewISOPath(QString);
    void setCurrentBurner(QString uuid);
    void slotShowResultMessageBox(bool isSuccess);
private:
    explicit LogicBurnImage(QObject *parent = nullptr);
    ~LogicBurnImage();
    bool checkCapacity();

private slots:
    void slotsNewBurner(void *);
    void slotsDeleteBurner(void *);
    void slotsBurnImage();
    void slotsSetting();
private:
    static LogicBurnImage *m_oInstance;
    static QMutex          m_oMutex;
private:
    QString                m_oISOPath;
    int                    m_iBurner;
    QStringList            m_oISOPaths;
    QList<void *>          m_oBurners;
Q_SIGNALS:
    // logic
    void newISOPath(QString);
    void newDeviceArrieved(QIcon,QString, QString,QString);
    void deleteBurner(QString uuid);
    void enableBurner(bool);
    void enableBusness(bool);
    void isoError(QString);
    void burnerError(QString);
    void ready(QString);
    void setting(void *);
};
#define BURNIMAGE LogicBurnImage::instance()
#endif // LOGICBURNIMAGE_H
