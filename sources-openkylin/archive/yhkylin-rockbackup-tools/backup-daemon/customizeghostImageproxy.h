#ifndef CUSTOMIZEGHOSTIMAGEPROXY_H
#define CUSTOMIZEGHOSTIMAGEPROXY_H

#include "workerfactory.h"
#include "myprocess/rsyncpathtodirprocess.h"
#include "parsebackuplist.h"

class CustomizeGhostImageProxy : public Worker
{
    Q_OBJECT
    DECLARE_DYNCREATE(CustomizeGhostImageProxy)
public:
    explicit CustomizeGhostImageProxy();
    virtual ~CustomizeGhostImageProxy();

public:
    // 环境检测
    virtual bool checkEnvEx();

    // 任务处理
    virtual void doWorkEx();

    // 任务取消
    virtual void cancelEx();

private:
    void doGhostImage();
    bool checkDestDirExists();
    void deleteFailedData();

    ParseBackupList::BackupPoint m_backupPoint;

    // 存放.kyimg文件的目录
    QString m_destPath;
    // .kyimg文件
    QString m_kyimg;
    // 源文件dst.img文件所在目录
    QString m_srcPath;
    // dst.img文件
    QString m_imgDst;

    // 备份进程
    RsyncPathToDirProcess *m_p;

    // 是否成功
    bool m_bSuccess;
    // 是否结束
    bool m_isFinished;
    // 强制结束标志(stop后没反应的情况，系统处于睡眠状态)
    bool m_isForce;
};

#endif // CUSTOMIZEGHOSTIMAGEPROXY_H
