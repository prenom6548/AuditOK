/*
 * Copyright (C) 2022 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "uniauthservice.h"
#include <QDebug>
#include <pwd.h>

#define KPR_DBUS_SERVICE "org.kylin.KprBackend"
#define KPR_DBUS_PATH "/SecurityQuestionAnswer"
#define KPR_DBUS_INTERFACE "org.kylin.KprBackend"

#define FD_DBUS_SERVICE "org.freedesktop.DBus"
#define FD_DBUS_PATH "/org/freedesktop/DBus"
#define FD_DBUS_INTERFACE "org.freedesktop.DBus"

QDBusArgument &operator<<(QDBusArgument &argument, const QuestionInfo &questionInfo)
{
    argument.beginStructure();
    argument << questionInfo.nQuestionId << questionInfo.strQuestionText;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QuestionInfo &questionInfo)
{
    argument.beginStructure();
    argument >> questionInfo.nQuestionId >> questionInfo.strQuestionText;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const AnswerInfo &answerInfo)
{
    argument.beginStructure();
    argument << answerInfo.nQuestionId << answerInfo.strAnswerText;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, AnswerInfo &answerInfo)
{
    argument.beginStructure();
    argument >> answerInfo.nQuestionId >> answerInfo.strAnswerText;
    argument.endStructure();
    return argument;
}

UniAuthService::UniAuthService(QObject *parent)
    : QDBusAbstractInterface(
        KPR_DBUS_SERVICE, KPR_DBUS_PATH, KPR_DBUS_INTERFACE, QDBusConnection::systemBus(), parent)
    , m_isActivatable(false)
{
    setTimeout(2147483647);

    qDBusRegisterMetaType<QuestionInfo>();
    qDBusRegisterMetaType<QList<QuestionInfo>>();
    qDBusRegisterMetaType<AnswerInfo>();
    qDBusRegisterMetaType<QList<AnswerInfo>>();

    QDBusInterface *dbusService
        = new QDBusInterface(FD_DBUS_SERVICE, FD_DBUS_PATH, FD_DBUS_INTERFACE, QDBusConnection::systemBus());
    if (dbusService) {
        QDBusReply<QStringList> result = dbusService->call(QStringLiteral("ListActivatableNames"));
        if (!result.isValid()) {
            qWarning() << "ListActivatableNames error:" << result.error().message();
        } else {
            QStringList listNames = result.value();
            if (listNames.contains(KPR_DBUS_INTERFACE)) {
                m_isActivatable = true;
            }
        }
    }
}

bool UniAuthService::isActivatable()
{
    return m_isActivatable;
}

QList<QuestionInfo> UniAuthService::GetPresetSecurityQuestions(QString language)
{   
    //qDebug() << "####GetPresetSecurityQuestions" << language << endl;
    QList<QuestionInfo> listQuestionInfo;
    QDBusReply<QList<QuestionInfo>> reply = call(QStringLiteral("GetPresetSecurityQuestions"), language);
    if (!reply.isValid()) {
        qWarning() << "GetPresetSecurityQuestions error: " << reply.error().message();
    } else {
        listQuestionInfo = reply.value();
    }
    return listQuestionInfo;
}

int UniAuthService::BindUserSecurityAnswers(int nUId, QList<AnswerInfo> listAnswerInfo)
{
    QDBusMessage result = call(QStringLiteral("BindUserSecurityAnswers"), nUId, QVariant::fromValue(listAnswerInfo));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "BindUserSecurityAnswers error:" << result.errorMessage();
        return -1;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toInt();
    } else {
        return -1;
    }
}

bool UniAuthService::IsUserHasSecurityAnswers(int nUId)
{
    QDBusMessage result = call(QStringLiteral("IsUserHasSecurityAnswers"), nUId);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "IsUserHasSecurityAnswers error:" << result.errorMessage();
        return false;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return false;
    }
}

QList<QuestionInfo> UniAuthService::GetUserSecurityQuestions(int nUId, QString language)
{
    QList<QuestionInfo> listQuestionInfo;
    QDBusReply<QList<QuestionInfo>> reply = call(QStringLiteral("GetUserSecurityQuestions"), nUId, language);
    if (!reply.isValid()) {
        qWarning() << "GetUserSecurityQuestions error: " << reply.error().message();
    } else {
        listQuestionInfo = reply.value();
    }
    return listQuestionInfo;
}

QString UniAuthService::VerifyUserSecurityAnswers(int nUId, QList<AnswerInfo> listAnswerInfo)
{
    QDBusMessage result = call(QStringLiteral("VerifyUserSecurityAnswers"), nUId, QVariant::fromValue(listAnswerInfo));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "VerifyUserSecurityAnswers error:" << result.errorMessage();
        return "";
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toString();
    } else {
        return "";
    }
}

QString UniAuthService::EncodeSecurityInfo(const QString &strSource)
{
    QDBusMessage result = call(QStringLiteral("EncodeSecurityInfo"), strSource);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "EncodeSecurityInfo error:" << result.errorMessage();
        return QString("");
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toString();
    } else {
        return QString("");
    }
}

int UniAuthService::ChangePassword(int nUId, const QString &strPasswd, QString strToken)
{
    QDBusMessage result = call(QStringLiteral("ChangePassword"), nUId, strPasswd, strToken);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "ChangePassword error:" << result.errorMessage();
        return false;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toInt();
    } else {
        return -1;
    }
}
