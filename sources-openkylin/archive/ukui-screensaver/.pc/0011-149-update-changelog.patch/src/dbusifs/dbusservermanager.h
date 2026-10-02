#ifndef DBUSSERVERMANAGER_H
#define DBUSSERVERMANAGER_H

#include <QObject>
#include <QDBusInterface>

class DBusServerManager : public QObject
{
    Q_OBJECT
private:
    explicit DBusServerManager(QObject *parent = nullptr);

public:
    /**
     * @brief 单实例
     *
     * @param parent 父指针
     * @return DBusServerManager 单实例对象指针
     */
    static DBusServerManager *instance(QObject *parent = nullptr);

    bool serviceExists(const QString &strService);

public Q_SLOTS:
    void onDBusNameOwnerChanged(const QString &name,
                                const QString &oldOwner,
                                const QString &newOwner);

Q_SIGNALS:
    void dbusServerActiveChanged(const QString &strServerName, bool isActive);

private:
    void init();

private:
    class MemCollector{
    public:
        ~MemCollector(){
            if (DBusServerManager::instance()) {
                delete DBusServerManager::instance();
                DBusServerManager::m_instance = nullptr;
            }
        }
    };
    static DBusServerManager *m_instance; /** 统一认证配置服务单实例指针 */
    static MemCollector m_memCollector;
    QDBusInterface *m_dbusService = nullptr;
};

#endif // DBUSSERVERMANAGER_H
