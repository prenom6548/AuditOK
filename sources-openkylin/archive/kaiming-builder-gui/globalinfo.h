#ifndef GLABELBACKUPINFO_H
#define GLABELBACKUPINFO_H

#include <QVector>

#include "common/singleton.h"
#include "common/spinlock_mutex.h"
#include "globalsignals.h"
#include "logic/KMBuilderManifest.h"
#include "logic/KMRef.h"

class GlobelInfo : public Singleton<GlobelInfo> {
public:
    enum BlackgroundMode {
        Auto,
        Light,
        Dark
    };

    GlobelInfo(token) {}
    ~GlobelInfo() {}

    void setMainWidget(QWidget * mainWidget) {
        m_mainWidget = mainWidget;
    }

    QWidget * getMainWidget() {
        return m_mainWidget;
    }

    KMBuilderManifest* getManifest() {
        return &m_manifest;
    }

    // 设置全局项目目录
    void setProjectDir(const QString &dir) {
        projectDir = dir;
    }

    // 获取全局项目目录
    QString getProjectDir() const {
        return projectDir;
    }

    // 设置全局yaml路径
    void setYamlPath(const QString &yamlPath) {
        m_yamlPath = yamlPath;
    }

    // 获取全局yaml路径
    QString getYamlPath() const {
        return m_yamlPath;
    }

    // 准备依赖包列表
    void setDepends(QVector<KMRef>&& depends) {
        m_depends = depends;
    }

    const QVector<KMRef>& getDepends() {
        return m_depends;
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
    // 是否跟随主题
    BlackgroundMode m_BlackgroundMode = BlackgroundMode::Auto;

    QWidget * m_mainWidget = nullptr;
    KMBuilderManifest m_manifest;
    QString projectDir;
    QString m_yamlPath;

    // 预先获取源中kind=depend的开明包列表，设置到全局中，不用频繁查询
    QVector<KMRef> m_depends;

    // 全局信号对象
    GlobalSignals m_globalSignals;

    spinlock_mutex m_interMutex;
    spinlock_mutex m_outerMutex;
};

#endif // GLABELBACKUPINFO_H
