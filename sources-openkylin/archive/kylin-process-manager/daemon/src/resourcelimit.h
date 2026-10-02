#ifndef RESOURCELIMIT_H
#define RESOURCELIMIT_H

#include "cgroupmanager.h"
#include <QObject>
#include <atomic>
#include <map>

struct LimitInfo
{
    pid_t pid_;
    pid_t tid_;
    std::string res_type;
    std::vector<int> real_limit;
};
class ResourceLimiter;
class ResourceLimit : public QObject
{
    Q_OBJECT
public:
    ResourceLimit(CGroupManager *cgroupManager, QObject *parent = nullptr);
    ~ResourceLimit();

    /*
     * @brief 设置进程（线程）资源限制
     * @param pid 进程ID
     * @param tid 线程ID
     * @param resType 资源类型 (all, cpu, memory, io)
     * @param level 限制等级
     */
    int LimitResource(int pid, int tid, const QString &resType, QString level);

    /*
     * @brief 冻结/解冻进程（线程）
     * @param pid 进程ID
     * @param tid 线程ID
     * @param freeze true表示冻结，false表示解冻
     * @return true 操作成功，false 操作失败
     */
    bool FreezeProcess(int pid, int tid, bool freeze);

    /*
     * @brief 解除资源限制
     * @param index 限制索引
     */
    bool ReleaseLimit(int index);

    /*
     * @brief 获取进程（线程）限制信息
     * @param pid 进程ID
     * @return 进程限制信息字符串
     */
    QString GetLimitInfo(int pid, int tid) const;

    /*
     * @brief 判断进程（线程）是否受资源限制
     * @param pid 进程ID
     * @param tid 线程ID
     * @param resType 资源类型 (all, cpu, memory, io)
     * @return true 受限制，false 不受限制
     */
    bool IsLimited(int pid, int tid, const QString &resType) const;

private:
    CGroupManager *m_cgroupManager;
    std::map<int, LimitInfo> m_indexComparison;
    std::atomic<int> m_nextIndex{1};
    ResourceLimiter *m_resourceLimiter;
};
#endif // !RESOURCELIMIT_H