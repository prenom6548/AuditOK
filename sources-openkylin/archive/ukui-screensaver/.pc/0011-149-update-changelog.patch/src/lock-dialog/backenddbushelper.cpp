/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "backenddbushelper.h"
#include "../../src/common/definetypes.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDebug>

BackendDbusHelper::BackendDbusHelper(
    const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
    initConnections();
}

BackendDbusHelper::~BackendDbusHelper() {}

void BackendDbusHelper::initConnections()
{
    connect(this, &BackendDbusHelper::UpdateInformation, this, &BackendDbusHelper::onUpdateInformation);
}

AgreementInfoPtr BackendDbusHelper::getAgreementInfo()
{
    AgreementInfoPtr agreementInfo = NULL;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GET_AGREEMENT;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getAgreementInfo error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);

        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getAgreementInfo Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getAgreementInfo Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret") && objRes.contains("Content")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    QJsonObject object = objRes.value("Content").toObject();
                    if (nCmdId != LOCK_CMD_ID_GET_AGREEMENT || nRet != 0 || object.isEmpty()) {
                        qInfo() << "getAgreementInfo Failed!!";
                    } else {
                        agreementInfo = ParseAgreementInfo(objRes);
                    }
                } else {
                    qInfo() << "getAgreementInfo Json is invalid!!";
                }
            }
        }
    }

    if (!agreementInfo) {
        agreementInfo = std::make_shared<AgreementInfo>();
    }

    return agreementInfo;
}

QList<UserInfoPtr> BackendDbusHelper::getUsersInfo()
{
    QList<UserInfoPtr> listUser;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GET_USERINFO_LIST;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getUsersInfo error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getUsersInfo Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getUsersInfo Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret") && objRes.contains("Content")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    QJsonArray jsonArray = objRes.value("Content").toArray();
                    if (nCmdId != LOCK_CMD_ID_GET_USERINFO_LIST || nRet != 0 || jsonArray.isEmpty()) {
                        qInfo() << "getUsersInfo Failed!!";
                    } else {
                        listUser = ParseUsersInfo(objRes);
                    }
                } else {
                    qInfo() << "getUsersInfo Json is invalid!!";
                }
            }
        }
    }
    return listUser;
}

QList<QString> BackendDbusHelper::getSessionsInfo()
{
    QList<QString> listSession;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GET_SESSIONS_LIST;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getSessionsInfo error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getSessionsInfo Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getSessionsInfo Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret") && objRes.contains("Content")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    QJsonArray jsonArray = objRes.value("Content").toArray();
                    if (nCmdId != LOCK_CMD_ID_GET_SESSIONS_LIST || nRet != 0 || jsonArray.isEmpty()) {
                        qInfo() << "getSessionsInfo Failed!!";
                    } else {
                        listSession = ParseSessionsInfo(objRes);
                    }
                } else {
                    qInfo() << "getSessionsInfo Json is invalid!!";
                }
            }
        }
    }
    return listSession;
}

QStringList BackendDbusHelper::getBatteryArgs()
{
    QStringList listArgs;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_UPOWER_BATTERY;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getBatteryArgs error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getBatteryArgs Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getBatteryArgs Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret") && objRes.contains("Content")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    QJsonArray jsonArray = objRes.value("Content").toArray();
                    if (nCmdId != LOCK_CMD_ID_UPOWER_BATTERY || nRet != 0 || jsonArray.isEmpty()) {
                        qInfo() << "getBatteryArgs Failed!!";
                    } else {
                        listArgs = ParseBatteryArgs(objRes);
                    }
                } else {
                    qInfo() << "getBatteryArgs Json is invalid!!";
                }
            }
        }
    }
    return listArgs;
}

QStringList BackendDbusHelper::getSleepLockcheck()
{
    QStringList listArgs;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_SESSION_GET_SLEEP_LOCKCHECK;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getSleepLockcheck error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getSleepLockcheck Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getSleepLockcheck Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret") && objRes.contains("Content")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_SESSION_GET_SLEEP_LOCKCHECK || nRet != 0) {
                        qInfo() << "getSleepLockcheck Failed!!";
                    } else {
                        listArgs = ParseSleepLockcheck(objRes);
                    }
                } else {
                    qInfo() << "getSleepLockcheck Json is invalid!!";
                }
            }
        }
    }
    return listArgs;
}

QString BackendDbusHelper::getBatteryIconName()
{
    QString batteryIconName = "";
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_UPOWER_BATTERY_STATUS;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getBatteryIconName error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getBatteryIconName Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getBatteryIconName Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_UPOWER_BATTERY_STATUS || nRet != 0) {
                        qInfo() << "getBatteryIconName Failed!!";
                    } else {
                        batteryIconName = objRes.value("Content").toString();
                    }
                } else {
                    qInfo() << "getBatteryIconName Json is invalid!!";
                }
            }
        }
    }
    return batteryIconName;
}

bool BackendDbusHelper::getIsBattery()
{
    bool isBattery = false;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_UPOWER_IS_BATTERY;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getIsBattery error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getIsBattery Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getIsBattery Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_UPOWER_IS_BATTERY || nRet != 0) {
                        qInfo() << "getIsBattery Failed!!";
                    } else {
                        isBattery = objRes.value("Content").toBool();
                    }
                } else {
                    qInfo() << "getIsBattery Json is invalid!!";
                }
            }
        }
    }
    return isBattery;
}

bool BackendDbusHelper::getCurTabletMode()
{
    bool tabletMode = false;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_TABLET_MODE;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getCurTabletMode error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getCurTabletMode Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getCurTabletMode Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_TABLET_MODE || nRet != 0) {
                        qInfo() << "getCurTabletMode Failed!!";
                    } else {
                        tabletMode = objRes.value("Content").toBool();
                    }
                } else {
                    qInfo() << "getCurTabletMode Json is invalid!!";
                }
            }
        }
    }
    return tabletMode;
}

int BackendDbusHelper::getNetworkConnectState()
{
    int connectState = false;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_NETWORK_CONNECT_STATE;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getNetworkConnectState error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getNetworkConnectState Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getNetworkConnectState Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_NETWORK_CONNECT_STATE || nRet != 0) {
                        qInfo() << "getNetworkConnectState Failed!!";
                    } else {
                        connectState = objRes.value("Content").toInt();
                    }
                } else {
                    qInfo() << "getNetworkConnectState Json is invalid!!";
                }
            }
        }
    }
    return connectState;
}

QVariant BackendDbusHelper::getWeatherData(QString strKey)
{
    QVariant varValue;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GET_WEATHER_DATA;
    jsonCmd["Key"] = strKey;
    QJsonObject objRes;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getWeatherData error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getWeatherData Parse json failed!!";
        } else {
            objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getWeatherData Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_GET_WEATHER_DATA || nRet != 0) {
                        qInfo() << "getWeatherData Failed!!";
                    } else {
                        varValue = objRes.value(strKey);
                    }
                } else {
                    qInfo() << "getWeatherData Json is invalid!!";
                }
            }
        }
    }
    return varValue;
}

bool BackendDbusHelper::setCurrentSession(QString strSession)
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_SET_SESSION;
    jsonCmd["Content"] = strSession;
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "setCurrentSession error: " << reply.error().message();
        return false;
    }
    return true;
}

bool BackendDbusHelper::lockStateChanged(bool isVisible, bool isSessionTools)
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_LOCK_STATE_CHANGED;
    jsonCmd["Content"] = isVisible;
    jsonCmd["SessionTools"] = isSessionTools;
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    if (reply.isError()) {
        qWarning() << "lockStateChanged error: " << reply.error().message();
        return false;
    }
    return true;
}

void BackendDbusHelper::startSession()
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_START_SESSION;
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    if (reply.isError()) {
        qWarning() << "startSession error: " << reply.error().message();
    }
}

void BackendDbusHelper::pamAuthenticate(QString strUserName)
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_PAMAUTH_AUTHENTICATE;
    jsonCmd["UserName"] = strUserName;
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "pamAuthenticate error: " << reply.error().message();
    }
}

void BackendDbusHelper::bioStartAuth(int uid, int nDevId)
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_BIOAUTH_STARTAUTH;
    jsonCmd["UserId"] = uid;
    jsonCmd["DevId"] = nDevId;
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    if (reply.isError()) {
        qWarning() << "bioStartAuth error: " << reply.error().message();
    }
}

void BackendDbusHelper::bioStopAuth()
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_BIOAUTH_STOPAUTH;
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "bioStopAuth error: " << reply.error().message();
    }
}

void BackendDbusHelper::pamRespond(QString strRespond)
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_PAMAUTH_RESPOND;
    jsonCmd["Content"] = strRespond;
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "pamAuthenticate error: " << reply.error().message();
    }
}

void BackendDbusHelper::pamAuthenticateCancel()
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_PAMAUTH_AUTHENTICATE_CANCEL;
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "pamAuthenticate error: " << reply.error().message();
    }
}

bool BackendDbusHelper::pamIsInAuthentication()
{
    bool isInAuthentication = false;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_PAMAUTH_IS_INAUTHTICATION;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "pamIsInAuthentication error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "pamIsInAuthentication Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "pamIsInAuthentication Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_PAMAUTH_IS_INAUTHTICATION || nRet != 0) {
                        qInfo() << "pamIsInAuthentication Failed!!";
                    } else {
                        isInAuthentication = objRes.value("Content").toBool();
                    }
                } else {
                    qInfo() << "pamIsInAuthentication Json is invalid!!";
                }
            }
        }
    }
    return isInAuthentication;
}

bool BackendDbusHelper::pamIsAuthenticated()
{
    bool isAuthenticated = false;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_PAMAUTH_IS_AUTHENTICATED;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "pamIsAuthenticated error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "pamIsAuthenticated Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "pamIsAuthenticated Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_PAMAUTH_IS_AUTHENTICATED || nRet != 0) {
                        qInfo() << "pamIsAuthenticated Failed!!";
                    } else {
                        isAuthenticated = objRes.value("Content").toBool();
                    }
                } else {
                    qInfo() << "pamIsAuthenticated Json is invalid!!";
                }
            }
        }
    }
    return isAuthenticated;
}

QString BackendDbusHelper::pamAuthenticateUser()
{
    QString strUser;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_PAMAUTH_IS_INAUTHTICATION;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "pamAuthenticateUser error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "pamAuthenticateUser Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "pamAuthenticateUser Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_PAMAUTH_IS_INAUTHTICATION || nRet != 0) {
                        qInfo() << "pamAuthenticateUser Failed!!";
                    } else {
                        strUser = objRes.value("Content").toString();
                    }
                } else {
                    qInfo() << "pamAuthenticateUser Json is invalid!!";
                }
            }
        }
    }
    return strUser;
}

void BackendDbusHelper::ParsePamShowMessage(const QJsonObject &objRes)
{
    QString strMsg = objRes.value("Message").toString();
    int nType = objRes.value("MsgType").toInt();
    Q_EMIT pamShowMessage(strMsg, nType);
}

void BackendDbusHelper::ParsePamShowPrompt(const QJsonObject &objRes)
{
    QString strPrompt = objRes.value("Prompt").toString();
    int nType = objRes.value("PromptType").toInt();
    Q_EMIT pamShowPrompt(strPrompt, nType);
}

void BackendDbusHelper::ParsePamAuthCompleted(const QJsonObject &objRes)
{
    Q_UNUSED(objRes);
    Q_EMIT pamAuthCompleted();
}

void BackendDbusHelper::ParseBioServiceStatus(const QJsonObject &objRes)
{
    Q_EMIT bioServiceStatusChanged(objRes.value("IsValid").toBool());
}

void BackendDbusHelper::ParseBioDeviceChanged(const QJsonObject &objRes)
{
    Q_UNUSED(objRes);
    Q_EMIT bioDeviceChanged();
}

void BackendDbusHelper::ParseBioAuthShowMessage(const QJsonObject &objRes)
{
    Q_EMIT bioAuthShowMessage(objRes.value("Message").toString());
}

void BackendDbusHelper::ParseBioAuthStateChanged(const QJsonObject &objRes)
{
    Q_EMIT bioAuthStateChanged(objRes.value("State").toInt());
}

void BackendDbusHelper::ParseBioAuthFrameData(const QJsonObject &objRes)
{
    Q_EMIT bioAuthFrameData(objRes.value("Data").toString());
}

void BackendDbusHelper::ParseBioAuthCompleted(const QJsonObject &objRes)
{
    int nUid = objRes.value("UId").toInt();
    bool isSuccess = objRes.value("IsSuccess").toBool();
    int nError = objRes.value("ErrorNo").toInt();
    int nMaxFailedTime = objRes.value("MaxFailedTime").toInt();
    int nFailedTime = objRes.value("FailedTime").toInt();
    Q_EMIT bioAuthCompleted(nUid, isSuccess, nError, nMaxFailedTime, nFailedTime);
}

void BackendDbusHelper::ParseTabletModeChanged(const QJsonObject &objRes)
{
    Q_EMIT tabletModeChanged(objRes.value("mode").toBool());
}

void BackendDbusHelper::ParseNetWorkConnectChanged(const QJsonObject &objRes)
{
    Q_EMIT networkConnectStateChanged(objRes.value("state").toInt());
}

QList<DeviceInfo> BackendDbusHelper::bioGetAvailableDevices(int nUid)
{
    QList<DeviceInfo> listDevInfo;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_BIOAUTH_GET_AVAILABLE_DEVICES;
    jsonCmd["UserId"] = nUid;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "bioGetAvailableDevices error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "bioGetAvailableDevices Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "bioGetAvailableDevices Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_BIOAUTH_GET_AVAILABLE_DEVICES || nRet != 0) {
                        qInfo() << "bioGetAvailableDevices Failed!!";
                    } else {
                        QJsonArray arrayDevInfo = objRes.value("Content").toArray();
                        for (auto devInfoObj : arrayDevInfo) {
                            if (devInfoObj.isObject()) {
                                DeviceInfo devInfo;
                                devInfo.id = devInfoObj.toObject().value("id").toInt();
                                devInfo.shortName = devInfoObj.toObject().value("ShortName").toString();
                                devInfo.fullName = devInfoObj.toObject().value("FullName").toString();
                                devInfo.driverEnable = devInfoObj.toObject().value("DriverEnable").toInt();
                                devInfo.deviceNum = devInfoObj.toObject().value("DeviceNum").toInt();
                                devInfo.deviceType = devInfoObj.toObject().value("DeviceType").toInt();
                                devInfo.storageType = devInfoObj.toObject().value("StorageType").toInt();
                                devInfo.eigType = devInfoObj.toObject().value("EigType").toInt();
                                devInfo.verifyType = devInfoObj.toObject().value("VerifyType").toInt();
                                devInfo.identifyType = devInfoObj.toObject().value("IdentifyType").toInt();
                                devInfo.busType = devInfoObj.toObject().value("BusType").toInt();
                                devInfo.deviceStatus = devInfoObj.toObject().value("DeviceStatus").toInt();
                                devInfo.OpsStatus = devInfoObj.toObject().value("OpsStatus").toInt();
                                listDevInfo.append(devInfo);
                            }
                        }
                    }
                } else {
                    qInfo() << "bioGetAvailableDevices Json is invalid!!";
                }
            }
        }
    }
    return listDevInfo;
}

QList<int> BackendDbusHelper::bioGetDisabledDevices(int nUid)
{
    QList<int> listDevDisabled;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_BIOAUTH_GET_DISABLED_DEVICES;
    jsonCmd["UserId"] = nUid;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "bioGetDisabledDevices error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "bioGetDisabledDevices Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "bioGetDisabledDevices Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_BIOAUTH_GET_DISABLED_DEVICES || nRet != 0) {
                        qInfo() << "bioGetDisabledDevices Failed!!";
                    } else {
                        QJsonArray arrayDevInfo = objRes.value("Content").toArray();
                        for (auto devInfoObj : arrayDevInfo) {
                            if (devInfoObj.isDouble()) {
                                listDevDisabled.append(devInfoObj.toInt());
                            }
                        }
                    }
                } else {
                    qInfo() << "bioGetDisabledDevices Json is invalid!!";
                }
            }
        }
    }
    return listDevDisabled;
}

int BackendDbusHelper::bioGetBioAuthState()
{
    int nAuthState = 0;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_BIOAUTH_GET_STATE;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "bioGetBioAuthState error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "bioGetBioAuthState Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "bioGetBioAuthState Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_BIOAUTH_GET_STATE || nRet != 0) {
                        qInfo() << "bioGetBioAuthState Failed!!";
                    } else {
                        nAuthState = objRes.value("Content").toInt();
                    }
                } else {
                    qInfo() << "bioGetBioAuthState Json is invalid!!";
                }
            }
        }
    }
    return nAuthState;
}

DeviceInfo BackendDbusHelper::bioGetCurBioInfo()
{
    DeviceInfo curDevInfo;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_BIOAUTH_GET_CURDEVICE;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "bioGetCurBioInfo error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "bioGetCurBioInfo Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "bioGetCurBioInfo Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_BIOAUTH_GET_CURDEVICE || nRet != 0) {
                        qInfo() << "bioGetCurBioInfo Failed!!";
                    } else {
                        QJsonObject objDevInfo = objRes.value("Content").toObject();
                        if (!objDevInfo.isEmpty()) {
                            curDevInfo.id = objDevInfo.value("id").toInt();
                            curDevInfo.shortName = objDevInfo.value("ShortName").toString();
                            curDevInfo.fullName = objDevInfo.value("FullName").toString();
                            curDevInfo.driverEnable = objDevInfo.value("DriverEnable").toInt();
                            curDevInfo.deviceNum = objDevInfo.value("DeviceNum").toInt();
                            curDevInfo.deviceType = objDevInfo.value("DeviceType").toInt();
                            curDevInfo.storageType = objDevInfo.value("StorageType").toInt();
                            curDevInfo.eigType = objDevInfo.value("EigType").toInt();
                            curDevInfo.verifyType = objDevInfo.value("VerifyType").toInt();
                            curDevInfo.identifyType = objDevInfo.value("IdentifyType").toInt();
                            curDevInfo.busType = objDevInfo.value("BusType").toInt();
                            curDevInfo.deviceStatus = objDevInfo.value("DeviceStatus").toInt();
                            curDevInfo.OpsStatus = objDevInfo.value("OpsStatus").toInt();
                        }
                    }
                } else {
                    qInfo() << "bioGetCurBioInfo Json is invalid!!";
                }
            }
        }
    }
    return curDevInfo;
}

DeviceInfo BackendDbusHelper::bioFindDeviceById(int nUid, int nDevId)
{
    DeviceInfo curDevInfo;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_BIOAUTH_FIND_DEVICE_BY_ID;
    jsonCmd["UserId"] = nUid;
    jsonCmd["DevId"] = nDevId;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "bioFindDeviceById error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "bioFindDeviceById Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "bioFindDeviceById Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_BIOAUTH_FIND_DEVICE_BY_ID || nRet != 0) {
                        qInfo() << "bioFindDeviceById Failed!!";
                    } else {
                        QJsonObject objDevInfo = objRes.value("Content").toObject();
                        if (!objDevInfo.isEmpty()) {
                            curDevInfo.id = objDevInfo.value("id").toInt();
                            curDevInfo.shortName = objDevInfo.value("ShortName").toString();
                            curDevInfo.fullName = objDevInfo.value("FullName").toString();
                            curDevInfo.driverEnable = objDevInfo.value("DriverEnable").toInt();
                            curDevInfo.deviceNum = objDevInfo.value("DeviceNum").toInt();
                            curDevInfo.deviceType = objDevInfo.value("DeviceType").toInt();
                            curDevInfo.storageType = objDevInfo.value("StorageType").toInt();
                            curDevInfo.eigType = objDevInfo.value("EigType").toInt();
                            curDevInfo.verifyType = objDevInfo.value("VerifyType").toInt();
                            curDevInfo.identifyType = objDevInfo.value("IdentifyType").toInt();
                            curDevInfo.busType = objDevInfo.value("BusType").toInt();
                            curDevInfo.deviceStatus = objDevInfo.value("DeviceStatus").toInt();
                            curDevInfo.OpsStatus = objDevInfo.value("OpsStatus").toInt();
                        }
                    }
                } else {
                    qInfo() << "bioFindDeviceById Json is invalid!!";
                }
            }
        }
    }
    return curDevInfo;
}

DeviceInfo BackendDbusHelper::bioFindDeviceByName(int nUid, QString strDevName)
{
    DeviceInfo curDevInfo;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_BIOAUTH_FIND_DEVICE_BY_NAME;
    jsonCmd["UserId"] = nUid;
    jsonCmd["DevName"] = strDevName;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "bioFindDeviceByName error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "bioFindDeviceByName Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "bioFindDeviceByName Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_BIOAUTH_FIND_DEVICE_BY_NAME || nRet != 0) {
                        qInfo() << "bioFindDeviceByName Failed!!";
                    } else {
                        QJsonObject objDevInfo = objRes.value("Content").toObject();
                        if (!objDevInfo.isEmpty()) {
                            curDevInfo.id = objDevInfo.value("id").toInt();
                            curDevInfo.shortName = objDevInfo.value("ShortName").toString();
                            curDevInfo.fullName = objDevInfo.value("FullName").toString();
                            curDevInfo.driverEnable = objDevInfo.value("DriverEnable").toInt();
                            curDevInfo.deviceNum = objDevInfo.value("DeviceNum").toInt();
                            curDevInfo.deviceType = objDevInfo.value("DeviceType").toInt();
                            curDevInfo.storageType = objDevInfo.value("StorageType").toInt();
                            curDevInfo.eigType = objDevInfo.value("EigType").toInt();
                            curDevInfo.verifyType = objDevInfo.value("VerifyType").toInt();
                            curDevInfo.identifyType = objDevInfo.value("IdentifyType").toInt();
                            curDevInfo.busType = objDevInfo.value("BusType").toInt();
                            curDevInfo.deviceStatus = objDevInfo.value("DeviceStatus").toInt();
                            curDevInfo.OpsStatus = objDevInfo.value("OpsStatus").toInt();
                        }
                    }
                } else {
                    qInfo() << "bioFindDeviceByName Json is invalid!!";
                }
            }
        }
    }
    return curDevInfo;
}

QString BackendDbusHelper::bioGetDefaultDevice(int nUid, QString strUserName, int bioType /* = -1 */)
{
    QString strDevName;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_BIOAUTH_GET_DEFAULT_DEVICE;
    jsonCmd["UserId"] = nUid;
    jsonCmd["UserName"] = strUserName;
    if (bioType != -1)
        jsonCmd["DevType"] = bioType;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "bioGetDefaultDevice error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "bioGetDefaultDevice Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "bioGetDefaultDevice Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_BIOAUTH_GET_DEFAULT_DEVICE || nRet != 0) {
                        qInfo() << "bioGetDefaultDevice Failed!!";
                    } else {
                        strDevName = objRes.value("Content").toString();
                    }
                } else {
                    qInfo() << "bioGetDefaultDevice Json is invalid!!";
                }
            }
        }
    }
    return strDevName;
}

QString BackendDbusHelper::getCurrentSession()
{
    QString strCurSession = "";
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GET_CURRENT_SESSION;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getCurrentSession error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getCurrentSession Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getCurrentSession Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_GET_CURRENT_SESSION || nRet != 0) {
                        qInfo() << "getCurrentSession Failed!!";
                    } else {
                        strCurSession = objRes.value("Content").toString();
                    }
                } else {
                    qInfo() << "getCurrentSession Json is invalid!!";
                }
            }
        }
    }
    return strCurSession;
}

void BackendDbusHelper::onUpdateInformation(const QString &strJson)
{
    QJsonParseError jsonParseError;
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(strJson.toUtf8(), &jsonParseError);
    if (jsonParseError.error != QJsonParseError::NoError) {
        qInfo() << "onUpdateInformation Parse json failed!!";
    } else {
        QJsonObject objRes = jsonDoc.object();
        if (objRes.isEmpty()) {
            qInfo() << "onUpdateInformation Json is null!!";
        } else {
            if (objRes.contains("CmdId")) {
                int nCmdId = objRes.value("CmdId").toInt(-1);
                switch (nCmdId) {
                    case LOCK_CMD_ID_USERINFO_CHANGED: {
                        ParseUsersInfo(objRes);
                    } break;
                    case LOCK_CMD_ID_CURRENT_USER_CHANGED: {
                        ParseCurrentUser(objRes);
                    } break;
                    case LOCK_CMD_ID_CURRENT_SESSION_CHANGED: {
                        ParseCurrentSession(objRes);
                    } break;
                    case LOCK_CMD_ID_LOGIN1_REQ_LOCK: {
                        ParseLogin1ReqLock(objRes);
                    } break;
                    case LOCK_CMD_ID_LOGIN1_REQ_UNLOCK: {
                        ParseLogin1ReqUnLock(objRes);
                    } break;
                    case LOCK_CMD_ID_LOGIN1_PREPAREFORSLEEP: {
                        ParseLogin1PrepareForSleep(objRes);
                    } break;
                    case LOCK_CMD_ID_LOGIN1_SESSION_ACTIVE_CHANGED: {
                        ParseLogin1SessionActiveChanged(objRes);
                    } break;
                    case LOCK_CMD_ID_GSETTINGS_LOCKSCREEN_CONF_CHANGED: {
                        ParseLockScreenConf(objRes);
                    } break;
                    case LOCK_CMD_ID_GSETTINGS_SCREENSAVER_CONF_CHANGED: {
                        ParseScreenSaverConf(objRes);
                    } break;
                    case LOCK_CMD_ID_GSETTINGS_POWERMANAGER_CONF_CHANGED: {
                        ParsePowerManagerConf(objRes);
                    } break;
                    case LOCK_CMD_ID_GSETTINGS_MATEBG_CONF_CHANGED: {
                        ParseMateBgConf(objRes);
                    } break;
                    case LOCK_CMD_ID_GSETTINGS_UKCCPLUGINS_CONF_CHANGED: {
                        ParseUkccPluginsConf(objRes);
                    } break;
                    case LOCK_CMD_ID_GSETTINGS_THEMESTYLE_CONF_CHANGED: {
                        ParseThemeStyleConf(objRes);
                    } break;
                    case LOCK_CMD_ID_GSETTINGS_SESSION_CONF_CHANGED: {
                        ParseSessionConf(objRes);
                    } break;
                    case LOCK_CMD_ID_LOGIN1_BLOCKINHIBITED_CHANGED: {
                        ParseBlockInhibitedChanged(objRes);
                    } break;
                    case LOCK_CMD_ID_GSETTINGS_KEYBOARD_CONF_CHANGED: {
                        ParseKeyboardConf(objRes);
                    } break;
                    case LOCK_CMD_ID_PAMAUTH_SHOWMESSAGE: {
                        ParsePamShowMessage(objRes);
                    } break;
                    case LOCK_CMD_ID_PAMAUTH_SHOWPROMPT: {
                        ParsePamShowPrompt(objRes);
                    } break;
                    case LOCK_CMD_ID_PAMAUTH_AUTH_COMPLETED: {
                        ParsePamAuthCompleted(objRes);
                    } break;
                    case LOCK_CMD_ID_GSETTINGS_USD_MEDIAKEYS_CONF_CHANGED: {
                        ParseUsdMediaKeysConf(objRes);
                    } break;
                    case LOCK_CMD_ID_GSETTINGS_USD_MEDIA_STATE_KEYS_CONF_CHANGED: {
                        ParseUsdMediaStateKeysConf(objRes);
                    } break;
                    case LOCK_CMD_ID_UPOWER_BATTERY_STATUS: {
                        ParseBatteryStatusChanged(objRes);
                    } break;
                    case LOCK_CMD_ID_UPOWER_BATTERY: {
                        ParseBatteryChanged(objRes);
                    } break;
                    case LOCK_CMD_ID_BIOAUTH_SERVICE_STATUS_CHANGED: {
                        ParseBioServiceStatus(objRes);
                    } break;
                    case LOCK_CMD_ID_BIOAUTH_DEVICE_CHANGED: {
                        ParseBioDeviceChanged(objRes);
                    } break;
                    case LOCK_CMD_ID_BIOAUTH_SHOW_MESSAGE: {
                        ParseBioAuthShowMessage(objRes);
                    } break;
                    case LOCK_CMD_ID_BIOAUTH_AUTHSTATE_CHANGED: {
                        ParseBioAuthStateChanged(objRes);
                    } break;
                    case LOCK_CMD_ID_BIOAUTH_FRAME_DATA: {
                        ParseBioAuthFrameData(objRes);
                    } break;
                    case LOCK_CMD_ID_BIOAUTH_COMPLETE: {
                        ParseBioAuthCompleted(objRes);
                    } break;
                    case LOCK_CMD_ID_TABLET_MODE_CHANGED: {
                        ParseTabletModeChanged(objRes);
                    } break;
                    case LOCK_CMD_ID_NETWORK_CONNECT_STATE_CHANGED: {
                        ParseNetWorkConnectChanged(objRes);
                    } break;
                    default:
                        break;
                }
            } else {
                qInfo() << "getUsersInfo Json is invalid!!";
            }
        }
    }
}

AgreementInfoPtr BackendDbusHelper::ParseAgreementInfo(const QJsonObject &objRes)
{
    AgreementInfoPtr agreementInfo;
    QJsonObject jsonObject = objRes.value("Content").toObject();

    if (jsonObject.isEmpty()) {
        qInfo() << "updateAgreementInfo failed!!";
        return agreementInfo;
    } else {
        agreementInfo = std::make_shared<AgreementInfo>();
        agreementInfo->updateShowLoginPrompt(jsonObject.value("showLoginPrompt").toBool());
        agreementInfo->updateHideTitle(jsonObject.value("hideTitle").toBool());
        agreementInfo->updatePromptTitle(jsonObject.value("promptTitle").toString());
        agreementInfo->updatePromptText(jsonObject.value("promptText").toString());
        agreementInfo->updatePromptTextFilePath(jsonObject.value("promptTextFilePath").toString());

        qDebug() << "------------------------------------------------------------";
        qDebug() << jsonObject.value("showLoginPrompt").toBool();
        qDebug() << jsonObject.value("hideTitle").toBool();
        qDebug() << jsonObject.value("promptTitle").toString();
        qDebug() << jsonObject.value("promptText").toString();
        qDebug() << jsonObject.value("promptTextFilePath").toString();
        qDebug() << "------------------------------------------------------------";
    }

    return agreementInfo;
}

QList<UserInfoPtr> BackendDbusHelper::ParseUsersInfo(const QJsonObject &objRes)
{
    QList<UserInfoPtr> listUser;
    QJsonArray jsonArray = objRes.value("Content").toArray();
    if (jsonArray.isEmpty()) {
        qInfo() << "updateUsersInfo Failed!!";
        return listUser;
    } else {
        for (auto jsonValue : jsonArray) {
            QJsonObject objUser = jsonValue.toObject();
            UserInfoPtr userPtr = std::make_shared<UserInfo>();
            userPtr->updateUid((uid_t)objUser.value("uid").toDouble(-1));
            userPtr->updateName(objUser.value("name").toString());
            userPtr->updateFullName(objUser.value("fullName").toString());
            userPtr->updateHeadImage(objUser.value("headImg").toString());
            userPtr->updateLoggedIn(objUser.value("loggedIn").toBool(false));
            userPtr->updateBackground(objUser.value("backGround").toString());
            userPtr->updateLang(objUser.value("lang").toString());
            userPtr->updateGreeterBackground(objUser.value("greeterBackGround").toString());
            userPtr->updateBackgroundColor(objUser.value("color").toString());
            userPtr->updateSystemAccount(objUser.value("systemAccount").toBool(false));
            listUser.append(userPtr);
        }
        Q_EMIT usersInfoChanged(listUser);
    }
    return listUser;
}

QList<QString> BackendDbusHelper::ParseSessionsInfo(const QJsonObject &objRes)
{
    QList<QString> listSessions;
    QJsonArray jsonArray = objRes.value("Content").toArray();
    if (jsonArray.isEmpty()) {
        qInfo() << "ParseSessionsInfo Failed!!";
        return listSessions;
    } else {
        for (auto jsonValue : jsonArray) {
            QJsonObject objUser = jsonValue.toObject();
            QString strSession = objUser.value("key").toString();
            listSessions.append(strSession);
        }
    }
    return listSessions;
}

QList<QString> BackendDbusHelper::ParseSaverThemes(const QJsonObject &objRes)
{
    QList<QString> listThemes;
    QJsonArray jsonArray = objRes.value("Content").toArray();
    if (jsonArray.isEmpty()) {
        qInfo() << "ParseSessionsInfo Failed!!";
        return listThemes;
    } else {
        for (auto jsonValue : jsonArray) {
            QJsonObject objUser = jsonValue.toObject();
            QString strSession = objUser.value("key").toString();
            qDebug() << "savertheme = " << strSession;
            listThemes.append(strSession);
        }
    }
    return listThemes;
}

QStringList BackendDbusHelper::ParseBatteryArgs(const QJsonObject &objRes)
{
    QStringList listBattery;
    QJsonArray jsonArray = objRes.value("Content").toArray();
    if (jsonArray.isEmpty()) {
        qInfo() << "ParseBatteryArgs Failed!!";
        return listBattery;
    } else {
        for (auto jsonValue : jsonArray) {
            QJsonObject objUser = jsonValue.toObject();
            QString strBattery = objUser.value("key").toString();
            listBattery.append(strBattery);
        }
    }
    return listBattery;
}

QStringList BackendDbusHelper::ParseSleepLockcheck(const QJsonObject &objRes)
{
    QStringList sleepLockcheck;
    QJsonArray jsonArray = objRes.value("Content").toArray();
    if (jsonArray.isEmpty()) {
        return sleepLockcheck;
    } else {
        for (auto jsonValue : jsonArray) {
            QJsonObject objUser = jsonValue.toObject();
            QString strLockcheck = objUser.value("key").toString();
            sleepLockcheck.append(strLockcheck);
        }
    }
    return sleepLockcheck;
}

QStringList BackendDbusHelper::ParseShutdownLockcheck(const QJsonObject &objRes)
{
    QStringList shutdownLockcheck;
    QJsonArray jsonArray = objRes.value("Content").toArray();
    if (jsonArray.isEmpty()) {
        return shutdownLockcheck;
    } else {
        for (auto jsonValue : jsonArray) {
            QJsonObject objUser = jsonValue.toObject();
            QString strLockcheck = objUser.value("key").toString();
            shutdownLockcheck.append(strLockcheck);
        }
    }
    return shutdownLockcheck;
}

bool BackendDbusHelper::ParseLogin1ReqLock(const QJsonObject &objRes)
{
    Q_UNUSED(objRes);
    Q_EMIT requestLock();
    return true;
}

bool BackendDbusHelper::ParseLogin1ReqUnLock(const QJsonObject &objRes)
{
    Q_UNUSED(objRes);
    Q_EMIT requestUnlock();
    return true;
}

bool BackendDbusHelper::ParseLogin1PrepareForSleep(const QJsonObject &objRes)
{
    bool isSleep = objRes.value("IsSleep").toBool(false);
    Q_EMIT PrepareForSleep(isSleep);
    return true;
}

bool BackendDbusHelper::ParseLogin1SessionActiveChanged(const QJsonObject &objRes)
{
    bool isActive = objRes.value("IsActive").toBool(true);
    Q_EMIT sessionActiveChanged(isActive);
    return true;
}

bool BackendDbusHelper::ParseBlockInhibitedChanged(const QJsonObject &objRes)
{
    QString blockInhibited = objRes.value("BlockInhibited").toString();
    Q_EMIT blockInhibitedChanged(blockInhibited);
    return true;
}

bool BackendDbusHelper::ParseBatteryStatusChanged(const QJsonObject &objRes)
{
    QString iconName = objRes.value("IconName").toString();
    Q_EMIT batteryStatusChanged(iconName);
    return true;
}

bool BackendDbusHelper::ParseBatteryChanged(const QJsonObject &objRes)
{
    QStringList listBattery;
    listBattery = ParseBatteryArgs(objRes);
    Q_EMIT batteryChanged(listBattery);
    return true;
}

bool BackendDbusHelper::isSessionActive()
{
    bool isActive = true;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_LOGIN1_IS_SESSION_ACTIVE;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "isSessionActive error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "isSessionActive Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "isSessionActive Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_LOGIN1_IS_SESSION_ACTIVE || nRet != 0) {
                        qInfo() << "isSessionActive Failed!!";
                    } else {
                        isActive = objRes.value("IsActive").toBool(true);
                    }
                } else {
                    qInfo() << "isSessionActive Json is invalid!!";
                }
            }
        }
    }
    return isActive;
}

bool BackendDbusHelper::setCurrentUser(QString strUserName)
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_SET_USER;
    jsonCmd["Content"] = strUserName;
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    if (reply.isError()) {
        qWarning() << "setCurrentUser error: " << reply.error().message();
        return false;
    }
    return true;
}

int BackendDbusHelper::switchToUser(QString strUserName)
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_SWITCH_TO_USER;
    jsonCmd["Content"] = strUserName;
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "switchToUser error: " << reply.error().message();
        return -1;
    } else {
        return reply.value();
    }
}

bool BackendDbusHelper::usdExternalDoAction(int actionType)
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_USD_MEDIAKEYS;
    jsonCmd["Content"] = actionType;
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    if (reply.isError()) {
        qWarning() << "usdExternalDoAction error: " << reply.error().message();
        return false;
    }
    return true;
}

QString BackendDbusHelper::getDefaultAuthUser()
{
    QString strDefUser = "";
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GET_DEFAULT_AUTH_USER;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getDefaultAuthUser error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getDefaultAuthUser Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getDefaultAuthUser Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_GET_DEFAULT_AUTH_USER || nRet != 0) {
                        qInfo() << "getDefaultAuthUser Failed!!";
                    } else {
                        strDefUser = objRes.value("Content").toString();
                    }
                } else {
                    qInfo() << "getDefaultAuthUser Json is invalid!!";
                }
            }
        }
    }
    return strDefUser;
}

QString BackendDbusHelper::getCurrentUser()
{
    QString strCurUser = "";
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GET_CURRENT_USER;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getCurrentUser error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getCurrentUser Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getCurrentUser Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_GET_CURRENT_USER || nRet != 0) {
                        qInfo() << "getCurrentUser Failed!!";
                    } else {
                        strCurUser = objRes.value("Content").toString();
                    }
                } else {
                    qInfo() << "getCurrentUser Json is invalid!!";
                }
            }
        }
    }
    return strCurUser;
}

bool BackendDbusHelper::ParseCurrentUser(const QJsonObject &objRes)
{
    QString strCurUser = objRes.value("Content").toString();
    Q_EMIT currentUserChanged(strCurUser);
    return true;
}

bool BackendDbusHelper::ParseCurrentSession(const QJsonObject &objRes)
{
    QString strSession = objRes.value("Content").toString();
    Q_EMIT currentSessionChanged(strSession);
    return true;
}

QVariant BackendDbusHelper::getScreenSaverConf(QString strKey)
{
    QVariant varValue;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_SCREENSAVER_CONF;
    jsonCmd["Key"] = strKey;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getScreenSaverConf error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getScreenSaverConf Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getScreenSaverConf Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_GSETTINGS_GET_SCREENSAVER_CONF || nRet != 0) {
                        qInfo() << "getScreenSaverConf Failed!!";
                    } else {
                        varValue = objRes.value(strKey);
                    }
                } else {
                    qInfo() << "getScreenSaverConf Json is invalid!!";
                }
            }
        }
    }
    return varValue;
}

QStringList BackendDbusHelper::getDefaultPhotoPath()
{
    QStringList listArgs;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_DEFAULT_PHOTOPATH;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getDefaultPhotoPath error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getDefaultPhotoPath Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getDefaultPhotoPath Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret") && objRes.contains("Content")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    QJsonArray jsonArray = objRes.value("Content").toArray();
                    if (nCmdId != LOCK_CMD_ID_GSETTINGS_GET_DEFAULT_PHOTOPATH || nRet != 0 || jsonArray.isEmpty()) {
                        qInfo() << "getDefaultPhotoPath Failed!!";
                    } else {
                        listArgs = ParsePhotoPathArgs(objRes);
                    }
                } else {
                    qInfo() << "getDefaultPhotoPath Json is invalid!!";
                }
            }
        }
    }
    return listArgs;
}

QStringList BackendDbusHelper::getCustomPhotoPath()
{
    QStringList listArgs;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_CUSTOM_PHOTOPATH;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getCustomPhotoPath error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getCustomPhotoPath Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getCustomPhotoPath Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret") && objRes.contains("Content")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    QJsonArray jsonArray = objRes.value("Content").toArray();
                    if (nCmdId != LOCK_CMD_ID_GSETTINGS_GET_CUSTOM_PHOTOPATH || nRet != 0 || jsonArray.isEmpty()) {
                        qInfo() << "getCustomPhotoPath Failed!!";
                    } else {
                        listArgs = ParsePhotoPathArgs(objRes);
                    }
                } else {
                    qInfo() << "getCustomPhotoPath Json is invalid!!";
                }
            }
        }
    }
    return listArgs;
}

bool BackendDbusHelper::setScreenSaverConf(QString strKey, QVariant value)
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_SET_SCREENSAVER_CONF;
    jsonCmd["Key"] = strKey;
    switch (value.type()) {
        case QVariant::Bool:
            jsonCmd["Value"] = value.toBool();
            break;
        case QVariant::Int:
            jsonCmd["Value"] = value.toInt();
            break;
        case QVariant::String:
            jsonCmd["Value"] = value.toString();
            break;
        case QVariant::UInt:
            jsonCmd["Value"] = value.toDouble();
            break;
        case QVariant::Double:
            jsonCmd["Value"] = value.toDouble();
            break;
        default:
            return false;
    }
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "setScreenSaverConf error: " << reply.error().message();
        return false;
    }
    return true;
}

bool BackendDbusHelper::ParseScreenSaverConf(const QJsonObject &objRes)
{
    QString strKey = objRes.value("Key").toString();
    Q_EMIT screenSaverConfChanged(strKey, objRes.value("Value").toVariant());
    return true;
}

QStringList BackendDbusHelper::ParsePhotoPathArgs(const QJsonObject &objRes)
{
    QStringList listBattery;
    QJsonArray jsonArray = objRes.value("Content").toArray();
    if (jsonArray.isEmpty()) {
        qInfo() << "ParsePhotoPathArgs Failed!!";
        return listBattery;
    } else {
        for (auto jsonValue : jsonArray) {
            QJsonObject objUser = jsonValue.toObject();
            QString strBattery = objUser.value("key").toString();
            listBattery.append(strBattery);
        }
    }
    return listBattery;
}

bool BackendDbusHelper::ParseLockScreenConf(const QJsonObject &objRes)
{
    QString strKey = objRes.value("Key").toString();
    Q_EMIT lockScreenConfChanged(strKey, objRes.value("Value").toVariant());
    return true;
}

bool BackendDbusHelper::ParsePowerManagerConf(const QJsonObject &objRes)
{
    QString strKey = objRes.value("Key").toString();
    Q_EMIT powerManagerConfChanged(strKey, objRes.value("Value").toVariant());
    return true;
}

bool BackendDbusHelper::ParseMateBgConf(const QJsonObject &objRes)
{
    QString strKey = objRes.value("Key").toString();
    Q_EMIT mateBgConfChanged(strKey, objRes.value("Value").toVariant());
    return true;
}

bool BackendDbusHelper::ParseUkccPluginsConf(const QJsonObject &objRes)
{
    QString strKey = objRes.value("Key").toString();
    Q_EMIT ukccPluginsConfChanged(strKey, objRes.value("Value").toVariant());
    return true;
}

bool BackendDbusHelper::ParseThemeStyleConf(const QJsonObject &objRes)
{
    QString strKey = objRes.value("Key").toString();
    Q_EMIT themeStyleConfChanged(strKey, objRes.value("Value").toVariant());
    return true;
}

bool BackendDbusHelper::ParseSessionConf(const QJsonObject &objRes)
{
    QString strKey = objRes.value("Key").toString();
    Q_EMIT sessionConfChanged(strKey, objRes.value("Value").toVariant());
    return true;
}

bool BackendDbusHelper::ParseKeyboardConf(const QJsonObject &objRes)
{
    QString strKey = objRes.value("Key").toString();
    if (strKey == KEY_CAPSLOCK_STATUS) {
        bool state = objRes.value("Value").toBool();
        Q_EMIT capslockConfChanged(state);
    }
    return true;
}

bool BackendDbusHelper::ParseUsdMediaKeysConf(const QJsonObject &objRes)
{
    QString strKey = objRes.value("Key").toString();
    Q_EMIT usdMediaKeysConfChanged(strKey, objRes.value("Value").toString());
    return true;
}

bool BackendDbusHelper::ParseUsdMediaStateKeysConf(const QJsonObject &objRes)
{
    QString strKey = objRes.value("Key").toString();
    Q_EMIT usdMediaStateKeysConfChanged(strKey, objRes.value("Value").toInt());
    return true;
}

QVariant BackendDbusHelper::getLockScreenConf(QString strKey)
{
    QVariant varValue;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_LOCKSCREEN_CONF;
    jsonCmd["Key"] = strKey;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getLockScreenConf error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getLockScreenConf Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getLockScreenConf Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_GSETTINGS_GET_LOCKSCREEN_CONF || nRet != 0) {
                        qInfo() << "getLockScreenConf Failed!!";
                    } else {
                        varValue = objRes.value(strKey);
                    }
                } else {
                    qInfo() << "getLockScreenConf Json is invalid!!";
                }
            }
        }
    }
    return varValue;
}

QStringList BackendDbusHelper::getShutdownLockcheck()
{
    QStringList listArgs;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_SESSION_GET_SHUTDOWN_LOCKCHECK;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getShutdownLockcheck error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getShutdownLockcheck Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getShutdownLockcheck Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret") && objRes.contains("Content")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_SESSION_GET_SHUTDOWN_LOCKCHECK || nRet != 0) {
                        qInfo() << "getShutdownLockcheck Failed!!";
                    } else {
                        listArgs = ParseShutdownLockcheck(objRes);
                    }
                } else {
                    qInfo() << "getShutdownLockcheck Json is invalid!!";
                }
            }
        }
    }
    return listArgs;
}

QList<QString> BackendDbusHelper::getSaverThemes()
{
    QList<QString> listThemes;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_LOCK_SCREEN_GET_THEMES;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getSaverThemes error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getSaverThemes Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getSaverThemes Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    QJsonArray jsonArray = objRes.value("Content").toArray();
                    if (nCmdId != LOCK_CMD_ID_LOCK_SCREEN_GET_THEMES || nRet != 0 || jsonArray.isEmpty()) {
                        qInfo() << "getSaverThemes Failed!!";
                    } else {
                        listThemes = ParseSaverThemes(objRes);
                    }
                } else {
                    qInfo() << "getSaverThemes Json is invalid!!";
                }
            }
        }
    }
    return listThemes;
}

bool BackendDbusHelper::setLockScreenConf(QString strKey, QVariant value)
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_SET_LOCKSCREEN_CONF;
    jsonCmd["Key"] = strKey;
    switch (value.type()) {
        case QVariant::Bool:
            jsonCmd["Value"] = value.toBool();
            break;
        case QVariant::Int:
            jsonCmd["Value"] = value.toInt();
            break;
        case QVariant::String:
            jsonCmd["Value"] = value.toString();
            break;
        case QVariant::UInt:
            jsonCmd["Value"] = value.toDouble();
            break;
        case QVariant::Double:
            jsonCmd["Value"] = value.toDouble();
            break;
        default:
            return false;
    }
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "setLockScreenConf error: " << reply.error().message();
        return false;
    }
    return true;
}

QVariant BackendDbusHelper::getPowerManagerConf(QString strKey)
{
    QVariant varValue;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_POWERMANAGER_CONF;
    jsonCmd["Key"] = strKey;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getPowerManagerConf error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getPowerManagerConf Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getPowerManagerConf Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_GSETTINGS_GET_POWERMANAGER_CONF || nRet != 0) {
                        qInfo() << "getPowerManagerConf Failed!!";
                    } else {
                        varValue = objRes.value(strKey);
                    }
                } else {
                    qInfo() << "getPowerManagerConf Json is invalid!!";
                }
            }
        }
    }
    return varValue;
}

bool BackendDbusHelper::setPowerManagerConf(QString strKey, QVariant value)
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_SET_POWERMANAGER_CONF;
    jsonCmd["Key"] = strKey;
    switch (value.type()) {
        case QVariant::Bool:
            jsonCmd["Value"] = value.toBool();
            break;
        case QVariant::Int:
            jsonCmd["Value"] = value.toInt();
            break;
        case QVariant::String:
            jsonCmd["Value"] = value.toString();
            break;
        case QVariant::UInt:
            jsonCmd["Value"] = value.toDouble();
            break;
        case QVariant::Double:
            jsonCmd["Value"] = value.toDouble();
            break;
        default:
            return false;
    }
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "setPowerManagerConf error: " << reply.error().message();
        return false;
    }
    return true;
}

QVariant BackendDbusHelper::getMateBgConf(QString strKey)
{
    QVariant varValue;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_MATEBG_CONF;
    jsonCmd["Key"] = strKey;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getMateBgConf error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getMateBgConf Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getMateBgConf Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_GSETTINGS_GET_MATEBG_CONF || nRet != 0) {
                        qInfo() << "getMateBgConf Failed!!";
                    } else {
                        varValue = objRes.value(strKey);
                    }
                } else {
                    qInfo() << "getMateBgConf Json is invalid!!";
                }
            }
        }
    }
    return varValue;
}

bool BackendDbusHelper::setMateBgConf(QString strKey, QVariant value)
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_SET_MATEBG_CONF;
    jsonCmd["Key"] = strKey;
    switch (value.type()) {
        case QVariant::Bool:
            jsonCmd["Value"] = value.toBool();
            break;
        case QVariant::Int:
            jsonCmd["Value"] = value.toInt();
            break;
        case QVariant::String:
            jsonCmd["Value"] = value.toString();
            break;
        case QVariant::UInt:
            jsonCmd["Value"] = value.toDouble();
            break;
        case QVariant::Double:
            jsonCmd["Value"] = value.toDouble();
            break;
        default:
            return false;
    }
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "setMateBgConf error: " << reply.error().message();
        return false;
    }
    return true;
}

QVariant BackendDbusHelper::getUkccPluginsConf(QString strKey)
{
    QVariant varValue;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_UKCCPLUGINS_CONF;
    jsonCmd["Key"] = strKey;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getUkccPluginsConf error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getUkccPluginsConf Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getUkccPluginsConf Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_GSETTINGS_GET_UKCCPLUGINS_CONF || nRet != 0) {
                        qInfo() << "getUkccPluginsConf Failed!!";
                    } else {
                        varValue = objRes.value(strKey);
                    }
                } else {
                    qInfo() << "getUkccPluginsConf Json is invalid!!";
                }
            }
        }
    }
    return varValue;
}

bool BackendDbusHelper::setUkccPluginsConf(QString strKey, QVariant value)
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_SET_UKCCPLUGINS_CONF;
    jsonCmd["Key"] = strKey;
    switch (value.type()) {
        case QVariant::Bool:
            jsonCmd["Value"] = value.toBool();
            break;
        case QVariant::Int:
            jsonCmd["Value"] = value.toInt();
            break;
        case QVariant::String:
            jsonCmd["Value"] = value.toString();
            break;
        case QVariant::UInt:
            jsonCmd["Value"] = value.toDouble();
            break;
        case QVariant::Double:
            jsonCmd["Value"] = value.toDouble();
            break;
        default:
            return false;
    }
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "setUkccPluginsConf error: " << reply.error().message();
        return false;
    }
    return true;
}

QVariant BackendDbusHelper::getThemeStyleConf(QString strKey)
{
    QVariant varValue;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_THEMESTYLE_CONF;
    jsonCmd["Key"] = strKey;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getThemeStyleConf error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getThemeStyleConf Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getThemeStyleConf Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_GSETTINGS_GET_THEMESTYLE_CONF || nRet != 0) {
                        qInfo() << "getThemeStyleConf Failed!!";
                    } else {
                        varValue = objRes.value(strKey);
                    }
                } else {
                    qInfo() << "getThemeStyleConf Json is invalid!!";
                }
            }
        }
    }
    return varValue;
}

bool BackendDbusHelper::setThemeStyleConf(QString strKey, QVariant value)
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_SET_THEMESTYLE_CONF;
    jsonCmd["Key"] = strKey;
    switch (value.type()) {
        case QVariant::Bool:
            jsonCmd["Value"] = value.toBool();
            break;
        case QVariant::Int:
            jsonCmd["Value"] = value.toInt();
            break;
        case QVariant::String:
            jsonCmd["Value"] = value.toString();
            break;
        case QVariant::UInt:
            jsonCmd["Value"] = value.toDouble();
            break;
        case QVariant::Double:
            jsonCmd["Value"] = value.toDouble();
            break;
        default:
            return false;
    }
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "setThemeStyleConf error: " << reply.error().message();
        return false;
    }
    return true;
}

QVariant BackendDbusHelper::getSessionConf(QString strKey)
{
    QVariant varValue;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_SESSION_CONF;
    jsonCmd["Key"] = strKey;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getSessionConf error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getSessionConf Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getSessionConf Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_GSETTINGS_GET_SESSION_CONF || nRet != 0) {
                        qInfo() << "getSessionConf Failed!!";
                    } else {
                        varValue = objRes.value(strKey);
                    }
                } else {
                    qInfo() << "getSessionConf Json is invalid!!";
                }
            }
        }
    }
    return varValue;
}

bool BackendDbusHelper::setSessionConf(QString strKey, QVariant value)
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_SET_SESSION_CONF;
    jsonCmd["Key"] = strKey;
    switch (value.type()) {
        case QVariant::Bool:
            jsonCmd["Value"] = value.toBool();
            break;
        case QVariant::Int:
            jsonCmd["Value"] = value.toInt();
            break;
        case QVariant::String:
            jsonCmd["Value"] = value.toString();
            break;
        case QVariant::UInt:
            jsonCmd["Value"] = value.toDouble();
            break;
        case QVariant::Double:
            jsonCmd["Value"] = value.toDouble();
            break;
        default:
            return false;
    }
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "setSessionConf error: " << reply.error().message();
        return false;
    }
    return true;
}

QVariant BackendDbusHelper::getKeyboardConf(QString strKey)
{
    QVariant varValue;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_KEYBOARD_CONF;
    jsonCmd["Key"] = strKey;
    QJsonObject objRes;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getKeyboardConf error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getKeyboardConf Parse json failed!!";
        } else {
            objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getKeyboardConf Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_GSETTINGS_GET_KEYBOARD_CONF || nRet != 0) {
                        qInfo() << "getKeyboardConf Failed!!";
                    } else {
                        varValue = objRes.value(strKey);
                    }
                } else {
                    qInfo() << "getKeyboardConf Json is invalid!!";
                }
            }
        }
    }
    return varValue;
}

bool BackendDbusHelper::setKeyboardConf(QString strKey, QVariant value)
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_SET_KEYBOARD_CONF;
    jsonCmd["Key"] = strKey;
    switch (value.type()) {
        case QVariant::Bool:
            jsonCmd["Value"] = value.toBool();
            break;
        case QVariant::Int:
            jsonCmd["Value"] = value.toInt();
            break;
        case QVariant::String:
            jsonCmd["Value"] = value.toString();
            break;
        case QVariant::UInt:
            jsonCmd["Value"] = value.toDouble();
            break;
        case QVariant::Double:
            jsonCmd["Value"] = value.toDouble();
            break;
        default:
            return false;
    }
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "setKeyboardConf error: " << reply.error().message();
        return false;
    }
    return true;
}

QVariant BackendDbusHelper::getUsdMediaStateKeys(QString strKey)
{
    QVariant varValue;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_USD_MEDIA_STATE_KEYS_CONF;
    jsonCmd["Key"] = strKey;
    QJsonObject objRes;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getUsdMediaStateKeys error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getUsdMediaStateKeys Parse json failed!!";
        } else {
            objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getUsdMediaStateKeys Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_GSETTINGS_GET_USD_MEDIA_STATE_KEYS_CONF || nRet != 0) {
                        qInfo() << "getUsdMediaStateKeys Failed!!";
                    } else {
                        varValue = objRes.value(strKey);
                    }
                } else {
                    qInfo() << "getUsdMediaStateKeys Json is invalid!!";
                }
            }
        }
    }
    return varValue;
}

bool BackendDbusHelper::setUsdMediaStateKeys(QString strKey, QVariant value)
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_SET_USD_MEDIA_STATE_KEYS_CONF;
    jsonCmd["Key"] = strKey;
    switch (value.type()) {
        case QVariant::Bool:
            jsonCmd["Value"] = value.toBool();
            break;
        case QVariant::Int:
            jsonCmd["Value"] = value.toInt();
            break;
        case QVariant::String:
            jsonCmd["Value"] = value.toString();
            break;
        case QVariant::UInt:
            jsonCmd["Value"] = value.toDouble();
            break;
        case QVariant::Double:
            jsonCmd["Value"] = value.toDouble();
            break;
        default:
            return false;
    }
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "setUsdMediaStateKeys error: " << reply.error().message();
        return false;
    }
    return true;
}

QVariant BackendDbusHelper::getUsdMediaKeys(QString strKey)
{
    QVariant varValue;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GSETTINGS_GET_USD_MEDIAKEYS_CONF;
    jsonCmd["Key"] = strKey;
    QJsonObject objRes;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getUsdMediaKeys error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getUsdMediaKeys Parse json failed!!";
        } else {
            objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getUsdMediaKeys Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_GSETTINGS_GET_USD_MEDIAKEYS_CONF || nRet != 0) {
                        qInfo() << "getUsdMediaKeys Failed!!";
                    } else {
                        varValue = objRes.value(strKey);
                    }
                } else {
                    qInfo() << "getUsdMediaKeys Json is invalid!!";
                }
            }
        }
    }
    return varValue;
}

bool BackendDbusHelper::setPowerManager(QString strFuncName)
{
    qWarning() << __LINE__ << __FUNCTION__ << "====================";
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_LOGIN1_SET_POWER_MANAGER;
    jsonCmd["Content"] = strFuncName;
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << __LINE__ << __FUNCTION__ << reply.error().message();
        return false;
    }
    return true;
}

bool BackendDbusHelper::getPowerManagerCanSuspend()
{
    bool canFlag = true;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANSUSPEND;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getPowerManagerCanSuspend error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getPowerManagerCanSuspend Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getPowerManagerCanSuspend Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANSUSPEND || nRet != 0) {
                        qInfo() << "getPowerManagerCanSuspend Failed!!";
                    } else {
                        canFlag = objRes.value("Content").toBool();
                    }
                } else {
                    qInfo() << "getPowerManagerCanSuspend Json is invalid!!";
                }
            }
        }
    }
    return canFlag;
}

bool BackendDbusHelper::getPowerManagerCanReboot()
{
    bool canFlag = true;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANREBOOT;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getPowerManagerCanReboot error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getPowerManagerCanReboot Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getPowerManagerCanReboot Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANREBOOT || nRet != 0) {
                        qInfo() << "getPowerManagerCanReboot Failed!!";
                    } else {
                        canFlag = objRes.value("Content").toBool();
                    }
                } else {
                    qInfo() << "getPowerManagerCanReboot Json is invalid!!";
                }
            }
        }
    }
    return canFlag;
}

bool BackendDbusHelper::getPowerManagerCanPowerOff()
{
    bool canFlag = true;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANPOWEROFF;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getPowerManagerCanPowerOff error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getPowerManagerCanPowerOff Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getPowerManagerCanPowerOff Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANPOWEROFF || nRet != 0) {
                        qInfo() << "getPowerManagerCanPowerOff Failed!!";
                    } else {
                        canFlag = objRes.value("Content").toBool();
                    }
                } else {
                    qInfo() << "getPowerManagerCanPowerOff Json is invalid!!";
                }
            }
        }
    }
    return canFlag;
}

bool BackendDbusHelper::getPowerManagerCanSwitchUser()
{
    bool canFlag = true;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANSWITCHUSER;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getPowerManagerCanSwitchUser error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getPowerManagerCanSwitchUser Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getPowerManagerCanSwitchUser Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANSWITCHUSER || nRet != 0) {
                        qInfo() << "getPowerManagerCanSwitchUser Failed!!";
                    } else {
                        canFlag = objRes.value("Content").toBool();
                    }
                } else {
                    qInfo() << "getPowerManagerCanSwitchUser Json is invalid!!";
                }
            }
        }
    }
    return canFlag;
}

bool BackendDbusHelper::getPowerManagerCanHibernate()
{
    bool canFlag = true;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANHIBERNATE;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getPowerManagerCanHibernate error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getPowerManagerCanHibernate Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getPowerManagerCanHibernate Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANHIBERNATE || nRet != 0) {
                        qInfo() << "getPowerManagerCanHibernate Failed!!";
                    } else {
                        canFlag = objRes.value("Content").toBool();
                    }
                } else {
                    qInfo() << "getPowerManagerCanHibernate Json is invalid!!";
                }
            }
        }
    }
    return canFlag;
}

bool BackendDbusHelper::getPowerManagerCanLogout()
{
    bool canFlag = true;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANLOGOUT;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getPowerManagerCanLogout error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getPowerManagerCanLogout Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getPowerManagerCanLogout Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANLOGOUT || nRet != 0) {
                        qInfo() << "getPowerManagerCanLogout Failed!!";
                    } else {
                        canFlag = objRes.value("Content").toBool();
                    }
                } else {
                    qInfo() << "getPowerManagerCanLogout Json is invalid!!";
                }
            }
        }
    }
    return canFlag;
}

bool BackendDbusHelper::getPowerManagerCanLockScreen()
{
    bool canFlag = true;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANLOCKSCREEN;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getPowerManagerCanLockScreen error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getPowerManagerCanLockScreen Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getPowerManagerCanLockScreen Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_LOGIN1_GET_POWER_MANAGER_CANLOCKSCREEN || nRet != 0) {
                        qInfo() << "getPowerManagerCanLockScreen Failed!!";
                    } else {
                        canFlag = objRes.value("Content").toBool();
                    }
                } else {
                    qInfo() << "getPowerManagerCanLockScreen Json is invalid!!";
                }
            }
        }
    }
    return canFlag;
}

bool BackendDbusHelper::checkSystemUpgrade()
{
    bool canFlag = true;
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_SYSTEM_UPGRADE_CHECK;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "checkSystemUpgrade error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);
        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "checkSystemUpgrade Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "checkSystemUpgrade Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_SYSTEM_UPGRADE_CHECK || nRet != 0) {
                        qInfo() << "checkSystemUpgrade Failed!!";
                    } else {
                        canFlag = objRes.value("Content").toBool();
                    }
                } else {
                    qInfo() << "checkSystemUpgrade Json is invalid!!";
                }
            }
        }
    }
    return canFlag;
}

QString BackendDbusHelper::getPublicEncrypt()
{
    QString publicEncrypt = "";
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_GET_PUBLIC_KEY;
    QDBusPendingReply<QString> reply = GetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "getPublicEncrypt error: " << reply.error().message();
    } else {
        QString strResponse = reply.value();
        QJsonParseError jsonParseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(strResponse.toUtf8(), &jsonParseError);

        if (jsonParseError.error != QJsonParseError::NoError) {
            qInfo() << "getPublicEncrypt Parse json failed!!";
        } else {
            QJsonObject objRes = jsonDoc.object();
            if (objRes.isEmpty()) {
                qInfo() << "getPublicEncrypt Json is null!!";
            } else {
                if (objRes.contains("CmdId") && objRes.contains("Ret") && objRes.contains("Content")) {
                    int nCmdId = objRes.value("CmdId").toInt(-1);
                    int nRet = objRes.value("Ret").toInt(-1);
                    if (nCmdId != LOCK_CMD_ID_GET_PUBLIC_KEY || nRet != 0) {
                        qInfo() << "getPublicEncrypt Failed!!";
                    } else {
                        publicEncrypt = objRes.value("Content").toString();
                    }
                } else {
                    qInfo() << "getPublicEncrypt Json is invalid!!";
                }
            }
        }
    }

    return publicEncrypt;
}

bool BackendDbusHelper::sendPassword(const QString username, QByteArray password)
{
    QJsonObject jsonCmd;
    jsonCmd["CmdId"] = LOCK_CMD_ID_SEND_USER_PASSWORD;

    QJsonObject objsingleLoginInfo;

    objsingleLoginInfo["username"] = username;
    objsingleLoginInfo["password"] = QJsonValue::fromVariant(password);

    jsonCmd["Content"] = objsingleLoginInfo;
    QDBusPendingReply<int> reply = SetInformation(QString(QJsonDocument(jsonCmd).toJson()));
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "setCurrentSession error: " << reply.error().message();
        return false;
    }
    return true;
}
