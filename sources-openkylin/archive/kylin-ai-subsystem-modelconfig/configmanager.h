#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QGSettings/QGSettings>
#include <QObject>
#include <QStringList>

class ConfigManager : public QObject {
    Q_OBJECT
public:
    explicit ConfigManager(QObject *parent = nullptr);

    void setEnableComputePowerSharing(bool enable);
    void setComputePowerSharingRole(const QString &role);
    void setLocalIp(const QString &ip);
    void setEnableRequestComputePower(bool enable);
    void setEnableSharedComputePower(bool enable);
    void setVerificationCode(const QString &code);

    bool getEnableComputePowerSharing();
    QString getComputePowerSharingRole();
    QString getLocalIp();
    bool getEnableRequestComputePower();
    bool getEnableSharedComputePower();
    QString getVerificationCode();
    bool getEnableDomainControl();

    QStringList getLocalNlpModelAddOrder();
    void setLocalNlpModelAddOrder(const QStringList& order);
    QStringList getPrivateNlpModelAddOrder();
    void setPrivateNlpModelAddOrder(const QStringList& order);

private:
    void initConfig();

private:
    QGSettings *m_config = nullptr;
};

#endif  // CONFIGMANAGER_H
