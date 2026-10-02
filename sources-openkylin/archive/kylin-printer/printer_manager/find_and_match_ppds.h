/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef FIND_AND_MATCH_PPDS_H
#define FIND_AND_MATCH_PPDS_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QThread>

#include <iostream>
#include <stdexcept>
#include <QMetaType>
#include "cupsconnection4ppds.h"
#include "findppdsthread.h"
#include "matchppdsthread.h"
#include "device_monitor.h"

enum class FindAndMatchStatus : int {
    OPERATION_OK = 0,
    GETTING_PPD_THREAD_IS_RUNNING,
    PPDS_MAP_IS_NOT_READY,
    PPDS_MATCH_FAILED,
};

#define g_findAndMatchWorkThread (FindAndMatchPPDs::getInstance()) //CUPS连接建立实例化
//#define g_cupsConnection (CupsConnection4PPDs::getInstance()) //CUPS连接建立实例化

class FindAndMatchPPDs : public QObject
{
    Q_OBJECT
public:

    static FindAndMatchPPDs* getInstance();//public接口，返回唯一实例化对象的指针

    int updatePPDsMap();//public接口，用于更新PPD的Map
    int matchPPDToPrinter(QString vendor,
                          QString model,
                          QString suggestPpdName,
                          PPDsAndAttr &exactMatchPPD,
                          QList<PPDsAndAttr> &matchResultList);//public接口，用于匹配打印机对应的PPD文件

    int getCurrentPPDsMap(ppdPrinterMap& ppdMap) const;//public接口，用于只读当前的PPD列表容器

    bool isBusy() const;//public接口，用于检测当前是否可以进行PPD的Map更新
    bool hasPPDsMap() const;//public接口，用于检测是否可以进行PPD的匹配


protected:



public slots:
    void getPPDsErr(int);//RequestError errCode);
    void gotPPDsMap(const ppdPrinterMap &temp,const ppdPrinterMap &extra);
    void detectStart();
signals:
    void startFindPPDs();
    void start();
    void updateFinished();
    void updateFailed();
private:

    ~FindAndMatchPPDs();
    FindAndMatchPPDs(QObject *parent = nullptr);
    FindAndMatchPPDs(FindAndMatchPPDs const&);
    FindAndMatchPPDs& operator=(FindAndMatchPPDs const&);

    static QMutex m_mutex;                         //线程锁保证单例线程安全

    static ppdPrinterMap m_currentPPDsMap;           //当前的ppd列表容器
    static ppdPrinterMap m_extraPPDsMap;           //当前的ppd列表容器

    void buildCupsConnection();                    //建立CUPS链接
    void destroyCupsConnection();

    void buildGetPPDsThreadsAndConnections();      //建立获取PPD线程
    void destroyGetPPDsConnection();

    void buildMatchPPDsThreadsAndConnections();    //建立匹配PPD线程
    void destroyMatchPPDsConnections();


    /***********************链接CUPS查找PPDS部分用的**************************/
    CupsConnection4PPDs* m_cupsConnection = nullptr;
    http_t *m_newHttp = nullptr;
    ipp_t *m_ppdRequest = nullptr;
    ipp_t *m_cupsAnswer = nullptr;
    ipp_attribute_t *m_attr = nullptr;

    QMap<QStringList, QString> m_originPPDs = {}; //从CUPS请求到的原始PPDS列表

    FindPPDsThread *m_cmdFindPPDs = nullptr; //从CUPS链接获取PPDS列表的线程
    QThread *m_FindPPDs = nullptr;
    MatchPPDsThread *m_cmdMatchPPDs = nullptr; //从PPDS列表匹配打印机名称的线程
    QThread *m_MatchPPDs = nullptr;

    bool m_ppdsMapisOK = false;                                  //是否有精准匹配的PPDS
    QMap<QString, QMap<QString, PPDsAndAttr>> m_originData = {}; //QMap<厂商名，QMap<型号，PPDS属性数据结构>>的原始数据

    bool m_isPPDsCollecting;
    bool m_canMatchPPD;

    class DEL
    {
    public:
        ~DEL()
        {
            if( g_findAndMatchWorkThread )
            {
                qDebug() << "调用了析构函数！！！";
                delete g_findAndMatchWorkThread;
            }
        }
    };

    static DEL m_Gabor;
};

#endif // FIND_AND_MATCH_PPDS_H
