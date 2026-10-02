#ifndef UDISKDATABACKUPPROXY_H
#define UDISKDATABACKUPPROXY_H

#include "databackupproxy.h"

class UDiskDataBackupProxy : public DataBackupProxy
{
    Q_OBJECT
    DECLARE_DYNCREATE(UDiskDataBackupProxy)
public:
    explicit UDiskDataBackupProxy();
    virtual ~UDiskDataBackupProxy();

public:
    // 环境检测
    virtual bool checkEnvEx();

    // 任务处理
    virtual void doWorkEx();

public slots:
    // 任务取消
    virtual void cancelEx();

private slots:
    // 校验剩余空间是否满足备份
    bool checkFreeCapacityToUdisk(qint64 itotalSize);

    // 备份
    void doBackupToUdisk();

    /**
     * @brief 校验移动盘是否还在
     * @return: bool,存在返回true；不存在返回false
     * @author: zhaominyong
     * @since: 2021/05/24
     * @note:
     *      add by zhaominyong at 2021/05/24 for bug:54377 【备份还原】备份数据到U盘的过程中拔出U盘，备份还原工具仍然一直显示正在备份数据
     */
    bool checkDestDirExists();

protected:
    // 失败则删除相应数据
    virtual void deleteFailedData();
    // 判断是否增量备份
    virtual bool isIncBackup();

private:
    // 计算备份所需空间大小
    void calcSizeForBackupToUdisk();

    /**
     * @brief 记录/backup/snapshots/backuplist.xml文件
     * @return true-成功；false-失败
     */
    bool recordBackupPointToUdisk();

    // 备份准备
    bool doPrepareToUdisk();

    // 备份系统
    bool backupDataToUdisk();

    // 强制结束标志(stop后没反应的情况，系统处于睡眠状态)
    bool m_isForce;
};

#endif // UDISKDATABACKUPPROXY_H
