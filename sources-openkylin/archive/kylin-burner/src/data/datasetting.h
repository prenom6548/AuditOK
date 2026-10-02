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


#ifndef DATASETTING_H
#define DATASETTING_H

#include <QObject>
#include <QDebug>

class DataSetting : public QObject
{
    Q_OBJECT
public:
    explicit DataSetting(QObject *parent = nullptr);
    DataSetting(DataSetting &other, QObject *parent);
    ~DataSetting();
public:
    void load();
public:
    int getSpeed() const { return m_iSpeed; }
    bool getClean() const { return m_bClean; }
    bool getCreateISO() const { return m_bCreateISO; }
    bool getCloseSession() const { return m_bCloseSession; }
    bool getVerficationData() const { return m_bVerficationData; }
    bool getOpen() const { return m_bOpen; }
    bool getIsShowBeforBurn() const {qDebug() << "(datasetting::updateIsShowBeforBurn) 获取为：" <<m_isShowBeforBurn;return m_isShowBeforBurn; }
    QString getPreparer() const { return m_oPreparer; }
    QString getPublisher() const { return m_oPublisher; }
    QString getVolumnID() const { return m_oVolumnID; }
    QString getCopyright() const { return m_oCopyright; }
    QString getTempPath() const { return m_oTemporaryPth; }
    QString getBurnFormat() const { return m_SBurnFormat; }


public slots:
    void updateSpeed(int speed) { m_iSpeed = speed; }
    void updateClean(bool b) { m_bClean = b; }
    void updateCreateISO(bool b) { m_bCreateISO = b; }
    void updateCloseSession(bool b) { m_bCloseSession = b; }
    void updateVerficationData(bool b) { m_bVerficationData = b; }
    void updateOpen(bool b) { m_bOpen = b; }
    void updateIsShowBeforBurn(bool b) { qDebug() << "(datasetting::updateIsShowBeforBurn) 设置为：" << b;m_isShowBeforBurn = b; }
    void updatePreparer(QString s) { m_oPreparer = s; }
    void updatePublisher(QString s) { m_oPublisher = s; }
    void updateVolumnID(QString s) { m_oVolumnID = s; }
    void updateCopyright(QString s) { m_oCopyright = s; }
    void updateTempPath(QString s) { m_oTemporaryPth = s;}
    void updateBurnFormat(QString s) { m_SBurnFormat = s; }
private:
    int        m_iSpeed;
    bool       m_bClean;
    bool       m_bCreateISO;
    bool       m_bCloseSession;
    bool       m_bVerficationData;
    bool       m_bOpen;
    bool       m_isShowBeforBurn;
    QString    m_oPreparer;
    QString    m_oPublisher;
    QString    m_oVolumnID;
    QString    m_oCopyright;
    QString    m_oTemporaryPth;
    QString    m_SBurnFormat;
Q_SIGNALS:
    void speed(int);
    void clean(bool);
    void createISO(bool);
    void closeSession(bool);
    void verficationData(bool);
    void open(bool);
    void preparer(QString);
    void publisher(QString);
    void volumnID(QString);
    void copyright(QString);
    void tempPath(QString);
    void burnFormat(QString);
    void IsShowBeforBurn(bool);
};

#endif // DATASETTING_H
