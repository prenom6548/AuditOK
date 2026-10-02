#ifndef GLABELBACKUPINFO_H
#define GLABELBACKUPINFO_H

#include "../common/singleton.h"
#include "../common/spinlock_mutex.h"
#include "functypeconverter.h"
#include "globalsignals.h"

class GlobelBackupInfo : public Singleton<GlobelBackupInfo> {
public:
    enum BlackgroundMode {
        Auto,
        Light,
        Dark
    };

    GlobelBackupInfo(token) {}
    ~GlobelBackupInfo() {}

    // 设置是否管理员用户登录
    void setIsManager(bool isManager) {
        m_isManager = isManager;
    }
    // 是否管理员用户登录
    bool isManager() {
        return m_isManager;
    }

    // 设置当前登录用户id
    void setCurUserId(QString curUserId) {
        m_curUserId = curUserId;
    }
    // 当前登录用户id
    QString curUserId() {
        return m_curUserId;
    }

    // 设置是否以--restore参数启动的备份还原工具
    void setHasArgRestore(bool hasArg_restore) {
        m_hasArg_restore = hasArg_restore;
    }
    // 是否以--restore参数启动的备份还原工具
    bool hasArgRestore() {
        return m_hasArg_restore;
    }

    // 设置是否正在进行备份、还原等操作
    void setIsBusy(bool isBusy) {
        std::lock_guard<spinlock_mutex> lock(m_interMutex);
        m_isBusy = isBusy;
        emit m_globalSignals.busy(isBusy);
    }

    // 是否正在进行备份、还原等操作
    bool isBusy() {
        std::lock_guard<spinlock_mutex> lock(m_interMutex);
        return m_isBusy;
    }

    // 是否有备份分区
    void setHasBackupPartition(bool has) {
        m_hasBackupPartition = has;
    }

    // 是否有备份分区
    bool hasBackupPartition() {
        return m_hasBackupPartition;
    }

    // 设置当前功能类型
    void setFuncType(FuncTypeConverter::FunType type) {
        m_funcType = type;
    }

    // 设置是否平板模式
    void setIsTabletMode(bool is) {
        m_isTabletMode = is;
    }

    // 是否平板模式
    bool isTabletMode() {
        return m_isTabletMode;
    }

    // 设置是否试安装小系统中运行
    void setIsCasper(bool isCasper) {
        m_isCasper = isCasper;
    }

    // 是否试安装小系统中运行模式
    bool isCasper() {
        return m_isCasper;
    }

    // 获取当前功能类型
    FuncTypeConverter::FunType getFuncType() {
        return m_funcType;
    }

    void setMainWidget(QWidget * mainWidget) {
        m_mainWidget = mainWidget;
    }

    QWidget * getMainWidget() {
        return m_mainWidget;
    }

    // 设置主题背景模式
    void setBlackgroundMode(BlackgroundMode blackgroundMode) {
        m_BlackgroundMode = blackgroundMode;
    }

    // 主题背景模式
    BlackgroundMode getBlackgroundMode() {
        return m_BlackgroundMode;
    }

    GlobalSignals * getGlobalSignals() {
        return &m_globalSignals;
    }

    void lock() {
        m_outerMutex.lock();
    }

    void unlock() {
        m_outerMutex.unlock();
    }

    bool try_lock() {
        return m_outerMutex.try_lock();
    }

private:
    // 是否试安装小系统中运行
    bool m_isCasper = false;
    // 是否管理员
    bool m_isManager = true;
    // 当前登录用户id
    QString m_curUserId;
    // 是否正在进行备份、还原等操作
    bool m_isBusy = false;
    // 是否有备份分区
    bool m_hasBackupPartition = true;
    // 当前功能类型
    FuncTypeConverter::FunType m_funcType = FuncTypeConverter::FunType::TOTALMODULES;
    // 是否以--restore参数启动的备份还原工具
    bool m_hasArg_restore = false;
    // 是否跟随主题
    BlackgroundMode m_BlackgroundMode = BlackgroundMode::Auto;
    // 是否平板模式
    bool m_isTabletMode = false;

    QWidget * m_mainWidget = nullptr;

    // 全局信号对象
    GlobalSignals m_globalSignals;

    spinlock_mutex m_interMutex;
    spinlock_mutex m_outerMutex;
};

#endif // GLABELBACKUPINFO_H
