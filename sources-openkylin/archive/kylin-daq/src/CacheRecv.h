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
#ifndef CACHERECV_H
#define CACHERECV_H

#include <QObject>
#include "CacheBase.h"

class CacheRecv : public CacheBase
{
    Q_OBJECT
public:
    /**
     * @brief Construct a new Cache Recv object
     * 
     * @param parent 
     */
    CacheRecv(QObject *parent=nullptr);
public slots:
    /**
     * @brief 接收埋点信息
     * 
     * @param info 埋点信息
     */
    void recvUploadInfo(const QString &info);
    void recvUploadEventInfo(const QString &info);

    /**
     * @brief 接收公共信息
     * 
     * @param info 公共信息
     */
    void recvCommonInfo(const QString &info);
    /**
     * @brief 接收tid信息
     * 
     * @param info tid信息
     */
    void recvTidInfo(const QString &info);
    /**
     * @brief 检测数据库中是否存储在tid
     * 
     * @param info tid信息
     * @param existState 检测结果
     */
    void checkTidInfo(const QString &info,bool &existState);

    void getUploadStateMessageInfo(const QString &packageInfo, QString &uploadedMessage, QString &tids, QString &createTimeStamp, KylinDaqCommon::UploadStateEnum &uploadState);
    void saveUploadStateInfo(const QString &packageInfo,const QString &uploadedMessage,const QString &tids,const QString &createTimeStamp,const KylinDaqCommon::UploadStateEnum &uploadState);
    void updateUploadStateInfo(const QString &packageInfo,const QString &uploadedMessage,const QString &tids,const QString &createTimeStamp,const KylinDaqCommon::UploadStateEnum &uploadState);
protected:
    /**
     * @brief 在数据库中存储埋点信息
     *
     * @param info 埋点信息
     * @return true 存储成功
     * @return false 存储失败
     */
    bool saveUploadInfo(const QList<QString> &info);
    bool saveUploadEventInfo(const QList<QString> &info);
    /**
     * @brief 在数据库中存储公共信息
     *
     * @param info 公共信息
     * @return true 存储成功
     * @return false 存储失败
     */
    bool saveCommonInfo(const QList<QString> &info);
    /**
     * @brief 在数据库中存储tid信息
     *
     * @param info
     * @return true 存储成功
     * @return false 存储失败
     */
    bool saveTidInfo(const QList<QString> &info);
    /**
     * @brief 在数据库中检测tid是否存在
     * 
     * @param info tid信息
     * @return true 存在
     * @return false 不存在
     */
    bool checkTidExist(const QString &info);

    /**
     * 获取数据库条数是否超过限制
    */

    bool dataLinesGeMaxLines(bool isEvent,qlonglong maxLines);
private:
    Q_PROPERTY(qlonglong numOfQuantitativeUploadLines READ numOfQuantitativeUploadLines WRITE setNumOfQuantitativeUploadLines NOTIFY numOfQuantitativeUploadLinesChanged)
    qlonglong m_numOfQuantitativeUploadLines=100;
public:
    qlonglong numOfQuantitativeUploadLines() const
    {
        return m_numOfQuantitativeUploadLines;
    }
public slots:
    void setNumOfQuantitativeUploadLines(qlonglong numOfQuantitativeUploadLines)
    {
        if (m_numOfQuantitativeUploadLines == numOfQuantitativeUploadLines)
            return;

        m_numOfQuantitativeUploadLines = numOfQuantitativeUploadLines;
        emit numOfQuantitativeUploadLinesChanged(m_numOfQuantitativeUploadLines);
    }

signals:
    void sendUploadInfoNow(qlonglong sendedInfoNum);
    void sendReUploadInfoNow(qlonglong sendedInfoNum);
    void sendEventUploadInfoNow(qlonglong sendedInfoNum);
    void sendEventReUploadInfoNow(qlonglong sendedInfoNum);
    void numOfQuantitativeUploadLinesChanged(qlonglong numOfQuantitativeUploadLines);


};

#endif // CACHERECV_H
