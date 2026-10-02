#ifndef NETWORKMANAGERHELPER_H
#define NETWORKMANAGERHELPER_H

#include <QDBusConnection>
#include <QDBusMessage>
#include <QVariantMap>
#include <QObject>

class NetWorkManagerHelper : public QObject
{
    Q_OBJECT
public:
    explicit NetWorkManagerHelper(QObject *parent = nullptr);

public Q_SLOTS:
    /**
     * @brief 会话属性改变
     *
     * @param QString 接口名
     * @param QVariantMap 变化的属性表
     * @param QStringList 无效的属性
     */
    void onNetworkPropChanged(QString, QVariantMap, QStringList);

    int getNetWorkConnectStatus();

Q_SIGNALS:
    /**
     * @brief 网络状态改变状态改变
     *
     * @param 4 网络可用
     */
    void netWorkConnectStateChanged(int state);
private:
    QDBusInterface *m_networkService = nullptr;
    int m_iNetWorkConnectStatus = -1;
};

#endif // NETWORKMANAGERHELPER_H
