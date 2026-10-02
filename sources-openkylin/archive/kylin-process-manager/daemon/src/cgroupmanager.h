#ifndef CGROUPMANAGER_H
#define CGROUPMANAGER_H

#include <QList>
#include <QObject>
#include <QVariant>
#include <atomic>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

#include "cgroupv2releasenotification.h"
#include "processinfohelper.h"

class CGroupManager : public QObject
{
    Q_OBJECT
public:
    CGroupManager();
    ~CGroupManager();

    // 进程限制
    /*
     * @brief 限制进程的CPU使用率
     * @param pid 进程ID
     * @param cpuPercent CPU使用率（0.00 - 100.00）
     * @return 返回索引，后续可以通过索引解除限制， 如果返回-1表示限制失败
     */
    int LimitProcessCPUPercent(int pid, double cpuPercent, QString mode);

    /*
     * @brief 限制进程的CPU权重
     * @param pid 进程ID
     * @param cpuWeight CPU权重（2-262144）
     * @return 返回索引，后续可以通过索引解除限制， 如果返回-1表示限制失败
     */
    int LimitProcessCPUWeight(int pid, int cpuWeight, QString mode);

    /*
     * @brief 限制进程的内存限制
     * @param pid 进程ID
     * @param memoryMax 内存限制（字节）
     * @return 返回索引，后续可以通过索引解除限制， 如果返回-1表示限制失败
     */
    int LimitProcessMemoryMax(int pid, uint64_t memoryMax, QString mode);

    /*
     * @brief 限制进程的IO权重
     * @param pid 进程ID
     * @param ioWeight IO权重（0-1000）
     * @return 返回索引，后续可以通过索引解除限制， 如果返回-1表示限制失败
     */
    int LimitProcessIOWeight(int pid, int ioWeight, QString mode);

    /*
     * @brief 冻结/解冻进程
     * @param pid 进程ID
     * @return 返回索引，后续可以通过索引解除限制， 如果返回-1表示限制失败
     */
    int FreezeProcess(int pid, QString mode);

    // 线程限制
    /*
     * @brief 限制线程的CPU使用率
     * @param pid 进程ID
     * @param tid 线程ID
     * @param cpuPercent CPU使用率（0.00 - 100.00）
     * @return 返回索引，后续可以通过索引解除限制， 如果返回-1表示限制失败
     */
    int LimitThreadCPUPercent(int pid, int tid, double cpuPercent, QString mode);

    /*
     * @brief 限制线程的CPU权重
     * @param pid 进程ID
     * @param tid 线程ID
     * @param cpuWeight CPU权重（2-262144）
     * @return 返回索引，后续可以通过索引解除限制， 如果返回-1表示限制失败
     */
    int LimitThreadCPUWeight(int pid, int tid, int cpuWeight, QString mode);

    /*
     * @brief 限制线程的内存限制
     * @param pid 进程ID
     * @param tid 线程ID
     * @param memoryMax 内存限制（字节）
     * @return 返回索引，后续可以通过索引解除限制， 如果返回-1表示限制失败
     */
    int LimitThreadMemoryMax(int pid, int tid, uint64_t memoryMax, QString mode);

    // 线程限制 IO权重
    /*
     * @brief 限制线程的IO权重
     * @param pid 进程ID
     * @param tid 线程ID
     * @param ioWeight IO权重（0-1000）
     * @return 返回索引，后续可以通过索引解除限制， 如果返回-1表示限制失败
     */
    int LimitThreadIOWeight(int pid, int tid, int ioWeight, QString mode);

    /*
     * @brief 冻结/解冻线程
     * @param pid 进程ID
     * @param tid 线程ID
     * @return 返回索引，后续可以通过索引解除限制， 如果返回-1表示限制失败
     */
    int FreezeThread(int pid, int tid, QString mode);

    // 解除限制
    /*
     * @brief 根据索引解除限制
     * @param index 限制索引
     * @return 如果返回true表示解除限制成功，返回false表示解除限制失败
     */
    bool ReleaseLimitByIndex(int index);

    /*
     * @brief 解除进程限制
     * @param pid 进程ID
     * @param threadSync 是否线程同步解除限制
     * @return 如果返回true表示解除限制成功，返回false表示解除限制失败
     */
    bool ReleaseProcessLimit(int pid, bool threadSync);

    /*
     * @brief 解除线程限制
     * @param pid 进程ID
     * @param tid 线程ID
     * @return 如果返回true表示解除限制成功，返回false表示解除限制失败
     */
    bool ReleaseThreadLimit(int pid, int tid);

    // 获取限制内容
    /*
     * @brief 获取进程限制信息
     * @param pid 进程ID
     * @return 进程限制信息
     */
    QString GetProcessLimitInfo(int pid);

    /*
     * @brief 获取线程限制信息
     * @param pid 进程ID
     * @param tid 线程ID
     * @return 线程限制信息
     */
    QString GetThreadLimitInfo(int pid, int tid);

    /*
     * @brief 根据索引获取限制内容
     * @param index 限制索引
     * @return 限制信息
     */
    QString GetLimitInfoByIndex(int index);

public:
    /*
     * @brief 设置当前用户相关属性，主要用于构造cgroup路径
     * @param currentUserUID 当前用户的UID
     * @param currentUserSlice 当前用户的slice名称
     * @param currentUserService 当前用户的service名称
     * @param currentUserSession 当前用户的session ID
     * 这些属性会影响cgroup路径的构造，确保限制应用到正确的cgroup路径下
     */
    bool SetCurrentUserProperty(QString currentUserUID, QString currentUserSlice, QString currentUserService, QString currentUserSession);

    /*
     * @brief 设置分组下路径
     * @param name 分组名 
     * @param path 路径
     * 根据分组名设置对应的cgroup路径，确保后续限制能够正确应用到该路径下
     */
    void SetGroupPath(QString currentUserUID, QString name, QString path);

    /*
     * @brief 设置分组下的配置信息
     * @param name 分组名
     * @param node 配置项节点
     * @param value 配置项值
     * 根据分组名和配置项设置对应的cgroup限制值，确保限制能够正确应用到该分组下
     */
    void SetGroupProperty(QString currentUserUID, QString name, QString node, QString value);

    /*
     * @brief 创建分组路径
     * @param name 分组名
     * 根据分组名创建对应的cgroup路径，确保后续限制能够正确应用到该路径下
     */
    void CreateGroupPath(QString currentUserUID, QString name);

    /*
     * @brief 删除分组路径
     * @param name 分组名
     * 根据分组名删除对应的cgroup路径，确保不再对该路径应用限制
     */
    void RemoveGroupPath(QString currentUserUID, QString name);

    /*
     * @brief 根据分组限制进程
     * @param name 分组名
     * @param pid 进程ID
     * 根据分组名将进程加入对应的cgroup路径，确保限制能够正确应用到该进程上
     */
    void LimitProcessByGroup(QString name, int pid);

    /*
     * @brief 删除cgroup路径
     * @param path cgroup路径
     * 根据路径删除对应的cgroup路径
     */
    void RemoveCGroupPath(QString path);

    /*
     * @brief 设置分组使能
     * @param enable 使能
     * 是否开启分组控制
     */
    void SetGroupEnable(bool enable);

    /*
     * @brief 设置冻结使能
     * @param enable 使能
     * 冻结控制是否生效
     */
    void SetFreezeEnable(bool enable);

    /*
     * @brief 解冻进程
     * @param pid 进程ID
     * 将冻结的进程解冻，恢复其运行状态
     */
    void ThawedProcess(int pid);

Q_SIGNALS:
    /*
     * @brief 限制改变信号
     * @param pid 进程ID
     * @param tid 线程ID
     * @param limitType 限制类型
     * @param enable 生效还是失效
     * 每当限制生效或失效时发出
     */
    void LimitChanged(int pid, int tid, QString limitType, bool enable);

public:
    bool isUserVaild(std::string uid);
    void thawedProcess(int pid);
    QList<int> listFreezeProcs();
    QList<int> getGroupPids(const QString &groupName);

    QString getPidCGroupPath(int pid) const;

    QList<int> getPidsInSameCGroup(int pid) const;

private:
    // 限制类型枚举
    enum LimitType {
        NONE, // 无限制
        CPU_PERCENT, // cpu使用率限制
        CPU_WEIGHT, // cpu权重限制
        MEMORY_MAX, // 内存限制
        IO_WEIGHT, // IO权重限制
        FREEZE // 冻结限制
    };

    enum TargetType {
        PROCESS, // 进程
        THREAD // 线程
    };

    using LimitValue = QVariant;

    // 限制记录结构
    struct LimitRecord
    {
        int index_; // 索引
        TargetType target_type; // 目标类型
        pid_t pid_; // 进程ID
        pid_t tid_; // 线程ID (如果是进程限制则为0)
        LimitType limit_type; // 限制类型
        std::string group_; // 限制组别
        std::string mode_;
        LimitValue value_; // 限制值
    };

    // 关联数据
    struct LinkedData
    {
        std::unordered_set<int> active_limits; // 当前启用的限制类型集合
        std::unordered_set<int> limit_indices; // 关联的限制记录索引集合
    };

    // 进程信息
    struct ProcInfo
    {
        pid_t pid_; // 进程ID
        uid_t uid_; // 用户ID
        std::unordered_set<pid_t> tids_; // 线程ID集合
        CgroupMap old_cgroups; // 原始cgroup路径集合
        std::string proc_name; // 进程名称
        LinkedData linked_data; // 关联数据
    };

    // 线程信息
    struct TaskInfo
    {
        pid_t tid_; // 线程ID
        pid_t pid_; // 所属进程ID
        LinkedData linked_data; // 关联数据
    };

    // 分组信息
    struct GroupInfo
    {
        std::string group_; // 组别
        std::string path_; // 路径
        std::unordered_map<LimitType, LimitValue> limits_; // 配置信息
    };
    using GroupInfoMap = std::unordered_map<std::string, GroupInfo>;

    // 用户信息
    struct UserInfo
    {
        std::string uid; // 用户UID
        std::string slice; // 用户slice名称
        std::string service; // 用户service名称
        std::string session; // 用户session ID
    };

    std::unordered_map<int, LimitRecord> m_records; // 索引 -> 记录
    std::unordered_map<int, ProcInfo> m_procInfos; // pid -> 进程信息
    std::unordered_map<int, TaskInfo> m_taskInfos; // tid -> 线程信息
    std::unordered_map<int, std::string> m_procGroup; // pid -> 组名
    std::unordered_map<std::string, GroupInfoMap> m_groupInfos; // 用户名 -> 组名 -> 分组信息
    std::unordered_map<std::string, UserInfo> m_userInfos; // 用户名 -> 用户信息
    std::unordered_set<std::string> m_pathCreated; // 已创建的cgroup路径列表
    std::shared_mutex m_pathMutex; // cgroup路径创建读写锁
    std::atomic<int> m_nextIndex{1}; // 下一个可用索引
    int m_cgroupVersion; // cgroup版本
    bool m_groupEnable; // 分组使能
    CGroupV2ReleaseNotification m_cgroupv2ReleaseNotification;

    // 辅助函数
    int cgroupVersion();
    std::string cgroupBasePath(LimitType type);
    std::string cgroupController(LimitType type);
    std::string cgroupControlType(LimitType type);
    std::string cgroupControlPath(pid_t pid, pid_t tid, std::string group, TargetType targetType);
    std::string cgroupOldControlPath(pid_t pid, LimitType limitType);
    void addCreatedPath(std::string path);
    void removeCreatedPath(std::string path);
    bool containCreatedPath(std::string path);
    std::vector<std::string> cgroupRootPaths(const std::string &uid);
    std::vector<int> groupPids(const std::string path);
    bool ensureDirectory(const std::string &path);
    bool writeFile(const std::string &file, const std::string &content);
    bool readFile(const std::string &file, std::string &content);
    bool moveProcToCgroup(pid_t pid, const std::string &cgroupPath);
    bool moveTaskToCgroup(pid_t tid, const std::string &cgroupPath);
    void moveCgroupToCgroup(std::string cgroupPath, const std::string &oldCgroupPath);
    bool enableNotifyOnRelease(const std::string &cgroupPath);
    TargetType getTargetType(const LimitRecord &rec);
    bool setLimitValue(const LimitRecord &rec);
    bool setLimitValueToGroup(LimitType limitType, LimitValue limitValue, std::string file);
    bool resetLimitValue(const LimitRecord &rec);
    bool resetLimitValueToGroup(LimitType limitType, std::string file);
    bool createCgroupRootPath(const std::string &uid);
    void removeCgroupRootPath(const std::string &uid);
    bool createCgroupForTarget(const LimitRecord &rec);
    void removeCgroupIfEmpty(const std::string &path);
    void initReleaseAgent();
    bool enableSubTreeControl(const std::string &path, const std::vector<std::string> &controllers);
    void changeGroupType(const std::string &path, const std::string &type);
    void restoreCgroup(pid_t pid, pid_t tid, LimitType limitType);
    void updateGroupLimit(const std::string &name, const std::string &uid);
    int addLimitRecord(LimitRecord &rec);
    int executeLimitRecord(LimitRecord &rec);
    void limitChangedAfter(const LimitRecord &rec, bool enable);
    void updateThreadLimitRecords(pid_t pid);
    int checkLimitRecord(pid_t pid, pid_t tid, LimitType limitType);
    bool isCgroupActive(const std::unordered_set<int> &activeLimits, LimitType limitType);
    bool isThreadLimit(pid_t pid, LimitType limitType);
    bool cmpLimitRule(const LimitRecord &rec1, const LimitRecord &rec2);
    int cmpLimitValue(const LimitValue &value1, const LimitValue &value2, LimitType limitType);
    int addProcInfo(pid_t pid);
    int addTaskInfo(pid_t pid, pid_t tid);
    LinkedData *getLinkedData(pid_t pid, pid_t tid);
    std::string getGroupName(pid_t pid);
    QString formatLimitInfo(const LimitRecord &rec);
};
#endif // !CGROUPMANAGER_H
