#ifndef SYSTEMRESTOREWORKER_H
#define SYSTEMRESTOREWORKER_H

#include "workerfactory.h"
#include "myprocess/rsyncpathtodirprocess.h"
#include "parsebackuplist.h"

class SystemRestoreWorker : public Worker
{
    Q_OBJECT
public:
    SystemRestoreWorker();
    virtual ~SystemRestoreWorker();

    void plymountMode();
    void plymountMessage(const QString& msg);

protected:
    // 还原结束
    void rsyncFinished();
    // 还原备份还原工具自身
    void rsyncBackupSelf();

    /**
     * @brief 再一次同步
     * @note 意图通过多次同步查缺补漏，如：
     *       1、因为系统还原过程中~/.config等目录中的部分文件可能会更新，会引起部分bug：
     *          a. 配网，打开奇安信浏览器，触发自动更新（或者软件商店手动更新），且奇安信默认固定在任务栏；
     *          b. 下载安装微信，固定任务栏（新装任意第三方软件均可），会删除desktop文件
     *          c. 上面两种操作，都会触发任务栏配置更新；通过监控任务栏配置文件，在系统还原过程中，可以明显的看到，在还原1%的时候，配置文件还原到历史文件，桌面图标还原，然后下一秒配置文件就又更新成当前最新的了
     */
    void rsyncAgain();

    void restartSystemAfterOneMinute();

protected:
    // .user.txt文件路径
    QString m_userFile;
    // .exclude.user.txt文件路径
    QString m_excludeUserFile;
    // 备份数据所在的data目录
    QString m_backupPath;

    // 当前备份uuid
    QString m_curUuid;
    // 当前还原源目录
    QString m_srcPath;
    // 备份进程
    RsyncPathToDirProcess *m_p;
    // 当前备份节点
    ParseBackupList::BackupPoint m_backupPoint;
    // 是否保留用户数据
    bool m_bRetainUserData;

    //当前系统分支哈希
    QString m_systemHash;
    bool m_bSameSystem;
    QString m_hashNumber;

    //出厂还原相关参数
    QString m_factoryHash;
    QStringList m_allSystemHash;
};

#endif // SYSTEMRESTOREWORKER_H
