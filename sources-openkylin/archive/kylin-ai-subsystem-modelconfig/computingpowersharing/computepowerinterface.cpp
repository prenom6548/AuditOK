#include "computepowerinterface.h"

#include <ukui-log4qt.h>

#include <QApplication>
#include <QEventLoop>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QPointer>
#include <QTemporaryFile>
#include <QThread>
#include <QTimer>
#include <QUrl>

#ifdef TEST_MODE
const QString checkDevice = "&check-device=false";
#else
const QString checkDevice = "";
#endif

namespace {
QMap<int, ComputePowerShare::ErrorCode> errorCodeMap = {
    {0, ComputePowerShare::ErrorCode::NoError},
    {-1, ComputePowerShare::ErrorCode::OtherError},
    {-2, ComputePowerShare::ErrorCode::InputParameterError},
    {-3, ComputePowerShare::ErrorCode::TimeOutError},
    {-4, ComputePowerShare::ErrorCode::NotSupportComputingPowerSharing},
    {-5, ComputePowerShare::ErrorCode::ServiceInternalHandingError},
    {-6, ComputePowerShare::ErrorCode::VerificationCodeError}};
}

ComputePowerInterface::ComputePowerInterface(QObject *parent) : QObject(parent) {
    m_networkAccessManager = new QNetworkAccessManager(this);
}

ComputePowerShare::ErrorCode ComputePowerInterface::openServerService() {
    qDebug() << __func__;

    QUrl url(serverUrl + "open" + checkDevice);
    QJsonObject response = sendGetRequestSyncWithRetry(url);

    bool success = requestSuccess(response);
    if (!success) {
        ComputePowerShare::ErrorCode errorCode = parseResponseCode(response);
        KyWarning() << "Failed to open service: " << errorMessage(response) << errorCode;
        m_localIp = "";
        return errorCode;
    }

    m_localIp = parseResponseData(response);
    if (!isServiceOpenSuccess()) {
        return ComputePowerShare::ErrorCode::TimeOutError;
    }
    return ComputePowerShare::ErrorCode::NoError;
}

ComputePowerShare::ErrorCode ComputePowerInterface::setVerificationCode(const QString &code) {
    qDebug() << __func__ << code;

    QUrl url(setVerificationCodeUrl + code);
    QJsonObject response = sendGetRequestSyncWithRetry(url);

    bool success = requestSuccess(response);
    if (!success) {
        ComputePowerShare::ErrorCode errorCode = parseResponseCode(response);
        KyWarning() << "Failed to set verification code: " << errorMessage(response) << errorCode;
        return errorCode;
    }
    return ComputePowerShare::ErrorCode::NoError;
}

ComputePowerShare::ErrorCode ComputePowerInterface::getVerificationCode() {
    qDebug() << __func__;

    QUrl url(getVerificationCodeUrl);
    QJsonObject response = sendGetRequestSyncWithRetry(url);

    bool success = requestSuccess(response);
    if (!success) {
        ComputePowerShare::ErrorCode errorCode = parseResponseCode(response);
        KyWarning() << "Failed to get verification code: " << errorMessage(response) << errorCode;
        m_verificationCode = "";
        return errorCode;
    }
    m_verificationCode = parseResponseData(response);
    return ComputePowerShare::ErrorCode::NoError;
}

ComputePowerShare::ErrorCode ComputePowerInterface::requestConsumerList() {
    qDebug() << __func__;

    QUrl url(serverListUrl);
    QJsonObject response = sendGetRequestSyncWithRetry(url);

    m_consumerList.clear();
    if (!requestSuccess(response)) {
        ComputePowerShare::ErrorCode errorCode = parseResponseCode(response);
        KyWarning() << "Failed to request consumer list:" << errorMessage(response) << errorCode;
        return errorCode;
    }
    m_consumerList = consumerIpList(response);

    return ComputePowerShare::ErrorCode::NoError;
}

ComputePowerShare::ErrorCode ComputePowerInterface::resetServerToEndSide() {
    qDebug() << __func__;

    QUrl url(serverUrl + "reset" + "&node-type=end-side");
    QJsonObject response = sendGetRequestSync(url);

    bool success = requestSuccess(response);
    if (!success) {
        ComputePowerShare::ErrorCode errorCode = parseResponseCode(response);
        KyWarning() << "Reset server to end-side failed: " << errorMessage(response) << errorCode;
        return errorCode;
    }

    if (!isServiceOpenSuccess()) {
        return ComputePowerShare::ErrorCode::TimeOutError;
    }
    return ComputePowerShare::ErrorCode::NoError;
}

ComputePowerShare::ErrorCode ComputePowerInterface::closeServerService() {
    qDebug() << __func__;

    QUrl url(serverUrl + "close" + checkDevice);
    QJsonObject response = sendGetRequestSyncWithRetry(url);
    if (!requestSuccess(response)) {
        ComputePowerShare::ErrorCode errorCode = parseResponseCode(response);
        KyWarning() << "Failed to close server: " << errorMessage(response) << errorCode;
        return errorCode;
    }
    return ComputePowerShare::ErrorCode::NoError;
}

ComputePowerShare::ErrorCode ComputePowerInterface::openEndSideService() {
    qDebug() << __func__;

    QUrl url(endSideUrl + "open");
    QJsonObject response = sendGetRequestSyncWithRetry(url);

    bool success = requestSuccess(response);
    if (!success) {
        ComputePowerShare::ErrorCode errorCode = parseResponseCode(response);
        KyWarning() << "Failed to open end-side: " << errorMessage(response) << errorCode;
        m_localIp = "";
        return errorCode;
    }

    m_localIp = parseResponseData(response);
    if (!isServiceOpenSuccess()) {
        return ComputePowerShare::ErrorCode::TimeOutError;
    }
    return ComputePowerShare::ErrorCode::NoError;
}

ComputePowerShare::ErrorCode ComputePowerInterface::requestProviderList() {
    qDebug() << __func__;

    QUrl url(endSideListUrl);
    QJsonObject response = sendGetRequestSyncWithRetry(url);

    m_connectedProviderList.clear();
    m_disconnectedProviderList.clear();
    if (!requestSuccess(response)) {
        ComputePowerShare::ErrorCode errorCode = parseResponseCode(response);
        KyWarning() << "Failed to request end-side list: " << errorMessage(response) << errorCode;
        return errorCode;
    }

    QString dataStr = response.value("data").toString();
    if (dataStr.isEmpty()) {
        KyWarning() << __func__ << "Data field is empty in response";
        return ComputePowerShare::ErrorCode::NoError;
    }

    KyDebug() << __func__ << dataStr << dataStr.toUtf8();
    QJsonDocument innerDoc = QJsonDocument::fromJson(dataStr.toUtf8());
    if (innerDoc.isNull()) {
        KyWarning() << "Failed to parse inner JSON data";
        return ComputePowerShare::ErrorCode::NoError;
    }
    QJsonObject innerObj = innerDoc.object();

    m_connectedProviderList = connectedProviderIpList(innerObj);
    m_disconnectedProviderList = disconnectedProviderIpList(innerObj);

    return ComputePowerShare::ErrorCode::NoError;
}

ComputePowerShare::ErrorCode ComputePowerInterface::connectToProvider(const QString &providerIp, const QString &code,
                                                                      const QString &disconnectIp) {
    qDebug() << __func__ << providerIp << code << disconnectIp;

    if (providerIp.isEmpty() || code.isEmpty()) {
        KyWarning() << "Input Error, Please enter a server IP address.";
        return ComputePowerShare::ErrorCode::OtherError;
    }
    QString urlString = endSideConnectUrl + providerIp + "&verification-code=" + code;
    if (!disconnectIp.isEmpty()) {
        urlString += "&disconnect-ip=" + disconnectIp;
    }
    QUrl url(urlString);
    QJsonObject response = sendGetRequestSync(url);

    bool success = requestSuccess(response);
    if (!success) {
        ComputePowerShare::ErrorCode errorCode = parseResponseCode(response);
        KyWarning() << "Connect" << providerIp << "error: " << errorMessage(response) << errorCode;
        return errorCode;
    }

    return ComputePowerShare::ErrorCode::NoError;
}

ComputePowerShare::ErrorCode ComputePowerInterface::resetEndSideToServer() {
    qDebug() << __func__;

    QUrl url(endSideUrl + "reset" + "&node-type=server" + checkDevice);
    QJsonObject response = sendGetRequestSync(url);

    bool success = requestSuccess(response);
    if (!success) {
        ComputePowerShare::ErrorCode errorCode = parseResponseCode(response);
        KyWarning() << "Reset end-side to server failed: " << errorMessage(response) << errorCode;
        return errorCode;
    }

    if (!isServiceOpenSuccess()) {
        return ComputePowerShare::ErrorCode::TimeOutError;
    }
    return ComputePowerShare::ErrorCode::NoError;
}

ComputePowerShare::ErrorCode ComputePowerInterface::closeEndSideService() {
    qDebug() << __func__;

    QUrl url(endSideUrl + "close");
    QJsonObject response = sendGetRequestSyncWithRetry(url);

    bool success = requestSuccess(response);
    if (!success) {
        ComputePowerShare::ErrorCode errorCode = parseResponseCode(response);
        KyWarning() << "Failed to close end-side: " << errorMessage(response) << errorCode;
        return errorCode;
    }

    return ComputePowerShare::ErrorCode::NoError;
}

bool ComputePowerInterface::isServiceEnabled() {
    qDebug() << __func__;

    QUrl url(serverStatusUrl);
    QJsonObject response = sendGetRequestSync(url);

    bool success = requestSuccess(response);
    if (!success) {
        ComputePowerShare::ErrorCode errorCode = parseResponseCode(response);
        KyWarning() << "Failed to get service status: " << errorMessage(response) << errorCode;
        return false;
    }
    if (parseResponseData(response) == "opened") {
        return true;
    }
    return false;
}

bool ComputePowerInterface::enableProviderService() {
    QStringList ops = {"enable", "start", "provider_on", "restart_kytensor"};
    return executeScript(ops);
}

bool ComputePowerInterface::disableProviderService() {
    QStringList ops = {"disable", "stop", "provider_off", "restart_kytensor"};
    return executeScript(ops);
}

bool ComputePowerInterface::enableConsumerService() {
    QStringList ops = {"enable", "start"};
    return executeScript(ops);
}

bool ComputePowerInterface::disableConsumerService() {
    QStringList ops = {"disable", "stop"};
    return executeScript(ops);
}

bool ComputePowerInterface::disableProviderServiceAndRestartInferenceService() {
    QStringList ops = {"provider_off", "restart_kytensor"};
    return executeScript(ops);
}

bool ComputePowerInterface::enableProviderServiceAndRestartInferenceService() {
    QStringList ops = {"provider_on", "restart_kytensor"};
    return executeScript(ops);
}

bool ComputePowerInterface::executeScript(const QStringList &operations) {
    if (operations.isEmpty()) {
        KyWarning() << __func__ << "No operations provided";
        return false;
    }

    QString scriptContent = "#!/bin/bash\n";
    scriptContent += QString("cd %1\n").arg(QFileInfo(scriptPath).absolutePath());
    scriptContent += "set -e\n";

    for (const QString &op : operations) {
        scriptContent += QString("./%1 %2\n").arg(QFileInfo(scriptPath).fileName()).arg(op);
    }

    QTemporaryFile tempScript;
    tempScript.open();
    tempScript.write(scriptContent.toLocal8Bit());
    tempScript.close();

    QFile::setPermissions(tempScript.fileName(), QFile::permissions(tempScript.fileName()) | QFileDevice::ExeOwner);

    QProcess process;
    process.start("pkexec", QStringList() << "/bin/bash" << tempScript.fileName());
    bool success = process.waitForFinished(-1);

    if (process.exitCode() == 0) {
        KyInfo() << __func__ << "Script execution succeeded: " << QString::fromUtf8(process.readAllStandardOutput())
                 << QString::fromUtf8(process.readAllStandardError());
    } else {
        KyWarning() << __func__ << "Script execution failed: " << QString::fromUtf8(process.readAllStandardOutput())
                    << QString::fromUtf8(process.readAllStandardError());
    }

    QFile::remove(tempScript.fileName());
    return success && process.exitCode() == 0;
}

QString ComputePowerInterface::getLocalIp() { return m_localIp; }

void ComputePowerInterface::clearLocalIp() { m_localIp = ""; };

QJsonObject ComputePowerInterface::sendGetRequestSyncWithRetry(const QUrl &url) {
    const int maxRetries = 5;
    const int retryIntervalMs = 100;

    for (int attempt = 0; attempt <= maxRetries; ++attempt) {
        QJsonObject response = sendGetRequestSync(url);

        if (requestSuccess(response)) {
            qDebug() << __func__ << "Request successful on attempt:" << (attempt + 1);
            return response;
        }

        qWarning() << __func__ << "Request failed on attempt:" << (attempt + 1) << "Code:" << response["code"].toInt()
                   << "Message:" << response["message"].toString();

        if (attempt >= maxRetries) {
            qWarning() << __func__ << "Max retries reached, returning last response";
            return response;
        }

        // 等待重试间隔
        QThread::msleep(retryIntervalMs);
    }

    // 这个不应该被到达，但为了编译器安全而保留
    QJsonObject emptyObj;
    return emptyObj;
}

QJsonObject ComputePowerInterface::sendGetRequestSync(const QUrl &url) {
    QEventLoop loop;
    QNetworkRequest request(url);
    QNetworkReply *reply = m_networkAccessManager->get(request);
    QPointer<QNetworkReply> replyPtr(reply);
    QJsonObject responseObj;
    if (replyPtr == nullptr) {
        qWarning() << __func__ << "Failed to send GET request to URL: " << url;
        return responseObj;
    }

    qDebug() << __func__ << "start request";

    connect(replyPtr, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();  // 同步等待，直到请求完成

    if (!replyPtr) {
        return responseObj;
    }
    qDebug() << __func__ << "request finish";
    if (replyPtr->error() == QNetworkReply::NoError) {
        QByteArray response = replyPtr->readAll();
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(response, &error);
        if (error.error == QJsonParseError::NoError && jsonDoc.isObject()) {
            responseObj = jsonDoc.object();
        } else {
            qWarning() << "sendGetRequestSync "
                       << "JSON parse error:" << error.errorString();
        }
    } else {
        qWarning() << "sendGetRequestSync "
                   << "error:" << replyPtr->errorString();
    }

    qDebug() << __func__ << "return result" << responseObj;
    return responseObj;
}

bool ComputePowerInterface::requestSuccess(const QJsonObject &response) {
    if (response.empty()) {
        return false;
    }
    int code = response["code"].toInt();
    bool success = response["success"].toBool();
    KyInfo() << __func__ << code << success;
    if (code != (int)ComputePowerShare::ErrorCode::NoError || !success) {
        return false;
    }
    return true;
}

QString ComputePowerInterface::errorMessage(const QJsonObject &response) { return response["message"].toString(); }

ComputePowerShare::ErrorCode ComputePowerInterface::parseResponseCode(const QJsonObject &response) {
    if (response.empty()) {
        return ComputePowerShare::ErrorCode::TimeOutError;
    }
    int error = response["code"].toInt();
    if (errorCodeMap.keys().contains(error)) {
        return errorCodeMap.value(error);
    }
    return ComputePowerShare::ErrorCode::OtherError;
}

QString ComputePowerInterface::parseResponseData(const QJsonObject &response) { return response["data"].toString(); }

QList<QString> ComputePowerInterface::consumerIpList(const QJsonObject &response) {
    QString dataListStr = response.value("data").toString();
    qDebug() << __func__ << dataListStr << dataListStr.toUtf8();
    QJsonDocument dataDoc = QJsonDocument::fromJson(dataListStr.toUtf8());
    if (dataDoc.isNull()) {
        qWarning() << __func__ << "Failed to parse data array!";
        return {};
    }

    QList<QString> ipList;
    QJsonArray dataArray = dataDoc.array();

    qDebug() << __func__ << "Server List Response";
    for (const QJsonValue &value : dataArray) {
        QJsonObject nodeObj = value.toObject();
        QString nodeIp = nodeObj.value("nodeIp").toString();
        if (nodeObj.contains("reason")) {
            qWarning() << "The node is running with errors: " << nodeObj.value("reason").toString() << nodeIp;
            continue;
        }

        ipList.append(nodeIp);
        qDebug() << "IP:" << nodeIp;
    }
    return ipList;
}

QList<QString> ComputePowerInterface::connectedProviderIpList(const QJsonObject &response) {
    QList<QString> connectedIpList;
    QJsonArray connectedArray = response.value("connected").toArray();

    qDebug() << __func__ << "end side connect List Response" << connectedArray;

    for (const QJsonValue &value : connectedArray) {
        QJsonObject obj = value.toObject();
        QString nodeIp = obj.value("nodeIp").toString();
        connectedIpList.append(nodeIp);
        qDebug() << "IP:" << nodeIp;
    }
    return connectedIpList;
}

QList<QString> ComputePowerInterface::disconnectedProviderIpList(const QJsonObject &response) {
    QList<QString> disconnectedIpList;
    QJsonArray disconnectedArray = response.value("disconnect").toArray();
    qDebug() << __func__ << "end side disconnect List Response" << disconnectedArray;

    for (const QJsonValue &value : disconnectedArray) {
        QJsonObject obj = value.toObject();
        QString nodeIp = obj.value("nodeIp").toString();
        disconnectedIpList.append(nodeIp);
        qDebug() << "IP:" << nodeIp;
    }
    return disconnectedIpList;
}

bool ComputePowerInterface::isServiceOpenSuccess() {
    qDebug() << __func__;

    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    QEventLoop delayLoop;

    QTimer getOpenStatusTimer;
    bool serviceOpened = false;

    connect(&timeoutTimer, &QTimer::timeout, &delayLoop, &QEventLoop::quit);
    connect(&getOpenStatusTimer, &QTimer::timeout, [this, &delayLoop, &serviceOpened]() {
        if (isServiceEnabled()) {
            serviceOpened = true;
            delayLoop.quit();
        }
    });

    timeoutTimer.start(6000);
    getOpenStatusTimer.start(200);

    delayLoop.exec();

    getOpenStatusTimer.stop();
    timeoutTimer.stop();
    return serviceOpened;
}

QString ComputePowerInterface::getVerificationCodeValue() { return m_verificationCode; }

QList<QString> ComputePowerInterface::getConsumerList() { return m_consumerList; }

QList<QString> ComputePowerInterface::getConnectedProviderList() { return m_connectedProviderList; }

QList<QString> ComputePowerInterface::getDisconnectedProviderList() { return m_disconnectedProviderList; }
