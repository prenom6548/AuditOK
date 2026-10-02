#ifndef WORKERFACTORY_H
#define WORKERFACTORY_H

#include <QObject>
#include "../common/mydefine.h"
#include "../common/reflect.h"

/**
 * @brief 备份还原相关任务处理类基类
 */
class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker();
    virtual ~Worker();

signals:
    // 检测结果信号
    void checkResult(int result);
    // 进度百分比
    void progress(int currentRate);
    // 工作结果信号
    void workResult(bool result);
    // 任务取消
    void cancel();
    // 开始ostree接口相关操作
    void startOstreeInterfaceOperation();

public slots:
    // 环境检测
    void checkEnv();

    // 任务处理
    void doWork();

protected:
    // 环境检测，个性化部分派生类去实现
    virtual bool checkEnvEx();

    // 任务处理，个性化部分派生类去实现
    virtual void doWorkEx();

    // 任务取消，个性化部分派生类去实现
    virtual void cancelEx();

public:
    void setParam(const BackupWrapper& backupWrapper) { m_backupWrapper = backupWrapper; }

public:
    // 同一时间只能运行一个备份/还原等操作
    BackupWrapper m_backupWrapper;
    // 是否取消操作
    bool m_bCancel;
};

/**
 * @brief 备份还原相关任务工厂类
 */
class WorkerFactory
{
public:
    /**
     * @brief 根据操作类型和位置创建相应的处理类
     * @param type 操作类型，如：系统备份, 系统还原
     * @param position 本地备份还是U盘备份: 0-本地备份；1-移动设备备份
     * @param user 自动执行还是用户执行:0-自动执行;1-用户手动操作;2-其他
     * @return Worker派生类对象指针，需调用者释放此对象
     */
    static Worker * createWorker(int type, int position, int user = 2);
};

#endif // WORKERFACTORY_H
