#ifndef COMPUTEPOWERINTERFACE_H
#define COMPUTEPOWERINTERFACE_H

#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QProcess>
#include <functional>

#include "util.h"

class ComputePowerInterface : public QObject {
    Q_OBJECT
public:
    explicit ComputePowerInterface(QObject *parent = nullptr);

    ComputePowerShare::ErrorCode openServerService();
    ComputePowerShare::ErrorCode setVerificationCode(const QString &code);
    ComputePowerShare::ErrorCode getVerificationCode();
    ComputePowerShare::ErrorCode requestConsumerList();
    ComputePowerShare::ErrorCode resetServerToEndSide();
    ComputePowerShare::ErrorCode closeServerService();

    ComputePowerShare::ErrorCode openEndSideService();
    ComputePowerShare::ErrorCode requestProviderList();
    ComputePowerShare::ErrorCode connectToProvider(const QString &providerIp, const QString &code,
                                                   const QString &disconnectIp = "");
    ComputePowerShare::ErrorCode resetEndSideToServer();
    ComputePowerShare::ErrorCode closeEndSideService();

    bool isServiceEnabled();

    bool enableProviderService();
    bool disableProviderService();
    bool enableConsumerService();
    bool disableConsumerService();
    bool disableProviderServiceAndRestartInferenceService();
    bool enableProviderServiceAndRestartInferenceService();

    QString getLocalIp();
    void clearLocalIp();

    QString getVerificationCodeValue();
    QList<QString> getConsumerList();
    QList<QString> getConnectedProviderList();
    QList<QString> getDisconnectedProviderList();

private:
    bool executeScript(const QStringList &command);
    QJsonObject sendGetRequestSyncWithRetry(const QUrl &url);
    QJsonObject sendGetRequestSync(const QUrl &url);

    bool requestSuccess(const QJsonObject &response);
    QString errorMessage(const QJsonObject &response);
    ComputePowerShare::ErrorCode parseResponseCode(const QJsonObject &response);
    QString parseResponseData(const QJsonObject &response);

    QList<QString> consumerIpList(const QJsonObject &response);
    QList<QString> connectedProviderIpList(const QJsonObject &response);
    QList<QString> disconnectedProviderIpList(const QJsonObject &response);
    bool isServiceOpenSuccess();

private:
    QNetworkAccessManager *m_networkAccessManager;
    QString m_localIp;

    QList<QString> m_consumerList;
    QString m_verificationCode;

    QList<QString> m_connectedProviderList;
    QList<QString> m_disconnectedProviderList;

    QTimer *m_openServiceTimeoutTimer = nullptr;
    QTimer *m_getServiceOpenStatusTimer = nullptr;

private:
    const QString serverUrl = "http://127.0.0.1:18080/share-kytensor/api/external/ui/server/manager-services?action=";
    const QString serverListUrl = "http://127.0.0.1:18080/share-kytensor/api/external/ui/server/list-end-side-nodes";
    const QString setVerificationCodeUrl =
        "http://127.0.0.1:18080/share-kytensor/api/external/ui/server/set-verification-code?verification-code=";
    const QString getVerificationCodeUrl =
        "http://127.0.0.1:18080/share-kytensor/api/external/ui/server/get-verification-code";
    const QString serverStatusUrl = "http://127.0.0.1:18080/share-kytensor/api/external/ui/services-status?";

    const QString endSideUrl =
        "http://127.0.0.1:18080/share-kytensor/api/external/ui/end-side/manager-services?action=";
    const QString endSideListUrl = "http://127.0.0.1:18080/share-kytensor/api/external/ui/end-side/list-server-nodes";
    const QString endSideConnectUrl =
        "http://127.0.0.1:18080/share-kytensor/api/external/ui/end-side/connect-nodes?connect-ip=";

    const QString scriptPath = "/usr/sbin/share-kytensor.sh";
};

#endif  // COMPUTEPOWERINTERFACE_H
