#ifndef GHOSTIMAGEPROXY_H
#define GHOSTIMAGEPROXY_H

#include "workerfactory.h"
#include "myprocess/mksquashfsprocess.h"
#include "myprocess/rsyncpathtodirprocess.h"
#include "parsebackuplist.h"

class GhostImageProxy : public Worker
{
    Q_OBJECT
    DECLARE_DYNCREATE(GhostImageProxy)
#ifdef UNIT_TEST
    friend class TestGhostImageProxy;
#endif
public:
    explicit GhostImageProxy();
    virtual ~GhostImageProxy();

public:
    // 环境检测
    virtual bool checkEnvEx();

    // 任务处理
    virtual void doWorkEx();

    // 任务取消
    virtual void cancelEx();

private:
    void doGhostImage();

    void deleteFailedData();
    qint64 getTotalDiskSpace();
    bool isVFatFilesystem(const QString &mountPath);
    bool unmountDevices();
    bool mountDevice(const QString &device, const QString &mountPoint, const QString &fsType);
    QString getDeviceByUuid(const QString &uuid);
    bool parseFstab();
    bool mountDevices();
    void createTempDirs();
    void createSystemInfoJson(const QString &outputPath);
    QString getKylinVersion();


    // 存放.kyimg文件的目录
    QString m_destPath;
    // .kyimg文件
    QString m_kyimg;

    // 压缩进程
    MkSquashFSProcess *m_mksquashfs;

    // 是否成功
    bool m_bSuccess;
    // 是否完成
    bool m_isFinished;

    struct MountInfo {
        QString device;
        QString mountPoint;
        QString fsType;
    };

    QMap<QString, MountInfo> mountPoints;
};

#endif // GHOSTIMAGEPROXY_H
