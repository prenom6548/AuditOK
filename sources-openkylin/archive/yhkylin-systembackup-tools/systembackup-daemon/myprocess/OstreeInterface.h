#ifndef OSTREEINTERFACE_H
#define OSTREEINTERFACE_H

#include <QString>
#include <QStringList>
#include <QProcess>
#include <QMap>
#include <QThread>
#include <QTimer>

#include "../common/utils.h"


#define DEFAULT_BACKUP_PATH "/data/.backup/"
#define TEMP_SNAPSHOTS_PATH "/data/.backup/snapshots/"
#define TEMP_RESTORE_PATH "/data/.backup/temp/"
#define TEMP_ETC_PATH "/data/.backup/temp/etc/"
#define KARE_BACKUP_PATH "/data/.backup/kare/"

#define DEFAULT_HOME_PATH "/data/home/"
#define TEMP_HOME_PATH "/data/.backup/temp/home/"
#define TEMP_OTHERS_PATH "/data/.backup/temp/others/"

#define HARD_LINK_PATH "/data/.backup/temp/hardlink/"

class OstreeInterface: public QObject
{
    Q_OBJECT
public:
    OstreeInterface(QObject* parent);
    ~OstreeInterface();

    enum TaskType
    {
        BACKUP,
        RESTORE,
        IDLE,
        BACKUPPOINT_MANAGEMENT,
    };

    // 备份类型
    enum BackupType
    {
        SYSTEM_UPDATE,
        SYSTEM,
        APPLICATION,
        USER,
        ALL,
        FACTORY,
    };

    enum RsyncScense
    {
        RESTORE_SYSTEM,
        RESTORE_HOMEDATA,
        RESTORE_HOMEHIDDENDATA,
        RESTORE_SYSTEM_SAMESYSTEM,
    };


    //执行的操作类型
    enum Operation
    {
        INIT,
        COMMIT,
        CHECKOUT,
        STATUS,
        STATUS_TO_NUM,
        STATUS_TO_UNDEPLOY,
        STATUS_TO_PENDING,
        DEPLOY,
        UNDEPLOY,
        ROLLBACK,
        NOROLLBACK,
        RSYNC,
        RM,
        CP,
        FIND,
        REFS,
        PRUNE,
    };

    /*
     * @brief : 检查Repo路径,不存在就初始化
     * @param backupPath : 备份路径,默認空
     * @param type : 备份类型
     * @return : true成功   false不成功
     */
    bool checkOstreeRepo(QString backupPath, BackupType type);

    /*
     * @brief : 初始化Repo路径
     * @param backupPath : 备份路径,默認空
     * @param type : 备份类型
     * @return : true成功   false不成功
     */
    bool initOstreeRepo(QString backupPath, BackupType type);

    /*
     * @brief : 获取repo路径
     * @param backupPath : 备份路径
     * @param BackupType : 备份类型
     * @return : 对于类型的备份repo路径
     */
    QString getRepoPath(QString backupPath, BackupType type);

    /*
     * @brief : 获取系统当前分支哈希
     * @return :
     */
    QStringList getSystemHashArgs();

    /*
     * @brief : 获取新部署的系统哈希
     * @return :
     */
    QStringList getPendingHashArgs();
    /*
     * @brief : 获取系统当前所有部署点哈希
     * @return :
     */
    QStringList getNeedUndeployHashArgs();

    /*
     * @brief : 根据基础部署点deploy出新的部署点,基础部署点不可以带后缀.0或者.1
     * @return :
     */
    QStringList getDeployArgs(QString bashHash);

    /*
     * @brief : undeploy指定部署点,根据查询num来
     * @return :
     */
    QStringList getUndeployArgs(QString num);

    /*
     * @brief : 获取回滚分支哈希序列号
     * @return :
     */
    QStringList getHashNumberArgs(QString hash);

    /*
     * @brief : 切换到系统指定哈希
     * @param hash : 指定哈希在部署点的排序，从0开始
     */
    QStringList getSystemRollBackArgs(QString number);

    /*
     * @brief : 组装commit参数
     * @param repo : repo路径
     * @param branch : branch分支名字
     * @param path : 需要提交保存的路径
     * @param description : 提交描述或者备份名
     * @return : 参数列表
     */
    QStringList getCommitArgs(QString repo, QString branch, QString path, QString description);

    /*
     * @brief : 组装checkout参数
     * @param repo : repo路径
     * @param destPath: 最终导出的路径
     * @param md5 : commit的哈希
     * @return : 参数列表
     */
    QStringList getCheckoutArgs(QString repo,QString destPath, QString hash);

    /*
     * @brief : 组装checkout参数,直接覆盖目标目录
     * */
    QStringList getCheckoutFullSyncArgs(QString repo,QString destPath, QString hash);

    /*
     * @brief : 组装refs参数
     * @param repo : repo路径
     * @param branch : branch分支名字
     * @return : 参数列表
     */
    QStringList getRefsArgs(QString repo, QString branch);

    /*
     * @brief : 组装prune参数
     * @param repo : repo路径
     * @param commit_id : 要删除的commit id
     * @return : 参数列表
     */
    QStringList getPruneArgs(QString repo, QString commit_id);

    /*
     * @brief 获取rsync命令参数
     * @param src : 同步源目录
     * @param desc : 目的目录
     * @return rsync的参数信息
     */
    QStringList getRsyncRepoArgs(QString src, QString desc);

    /*
     * @brief 获取cp命令参数
     * @param src : 同步源目录
     * @param desc : 目的目录
     */
    QStringList getCpArgs(QString src, QString desc);

    /*
     * @brief 获取rm命令参数
     */
    QStringList getRmArgs(QString path);

    /*
     * @brief 通过find删除特殊文件类型
     */
    QStringList getFindArgs(QString path,int type);

    /*
     * @brief : 设置操作路径
     * @param path : 操作路径
     */
    void setOperationPath(QString path);

    /*
     * @brief : 设置当前的备份步骤
     * @param path : 操作路径
     */
    void setBackUpStep(QString step);

    QString getBackUpStep();

    /*
     * @brief : 设置操作类型
     * @param oper : oper类型
     */
    void setOperation(Operation oper);

    /*
     * @brief : 获取操作类型
     * @return  : oper类型
     */
    Operation getOperation();

    /*
     * @brief : 设置备份还原类型
     * @param oper : oper类型
     */
    void setTaskType(TaskType taskType);

    /*
     * @brief : 获取备份还原类型
     * @return  : oper类型
     */
    TaskType getTaskType();

    /*
     * @brief : 设置Rsync场景
     * @return  : RsyncScense类型
     */
    void setRsyncScense(RsyncScense rsyncScense);
    /*
     * @brief : 获取Rsync场景
     * @return  : RsyncScense类型
     */
    OstreeInterface::RsyncScense getRsyncScense();

    /*
     * @brief 执行ostree指令
     * @param args ostree参数列表
     * @param block 是否阻塞计算模式。默认true——阻塞计算模式
     * @return block为true时返回值为待备份数据大小，单位字节；block为false时，默认返回0
     */
    void start(QStringList args, bool block = true);

    /*
     * @brief 执行ostree指令
     * @param args ostree参数列表
     * @param block 是否阻塞计算模式。默认true——阻塞计算模式
     * @return block为true时返回值为待备份数据大小，单位字节；block为false时，默认返回0
     */
    void startByShell(QStringList args, bool block);


    void stop() { m_process->kill(); }

    /*
     * @brief 计算当前备份、还原进度
     * @return 进度数额
     */
    void calcRate();

signals:
    // 计算结束信号
    void finished(bool result);
    // 进度百分比
    void progress(int currentRate);
    // 存储哈希值
    void commitHash(QString path, QString commitHash);
    // 系统哈希
    void systemHash(QString systemHash);
    //新部署哈希
    void pendingHash(QString systemHash);
    //系统哈希所在的序列号
    void hashNumber(QString hashNumber);
    //ostree所有部署点
    void ostreeHashs(QStringList hashs);
    // 开始ostree接口相关操作
    void startOstreeInterfaceOperation();

    // 定时器任务开始信号，用于触发定时器启动和停止
    void progressTimerStart();
    void progressTimerStop();
private slots:
    // process结束槽
    void processFinish(int exitCode, QProcess::ExitStatus);

    // 定时器进度更新
    void updateTimerProgress();

private:
    void parseResult();

    //
    QProcess * m_process;
    //进度信息
    int m_currentRate = 0;
    //任务类型，备份、还原
    TaskType m_taskType;
    //操作类型
    Operation m_oper;
    //rsync对应的执行场景
    RsyncScense m_rScense;
    //操作路径
    QString m_operPath;
    //对应的执行到的备份步骤
    QString m_backUpStep;

    // 定时器进度系数
    double m_progressCoefficient = 1;
    // 当前定时器时间
    double m_currentTimeSinceCommit = 0;
    // 临时进度定时器
    QTimer* m_progressTimer;
};

#endif // OSTREEINTERFACE_H
