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
#include "securityquestionanswer.h"
#include <QDebug>
#include <pwd.h>

#define KPRBACKEND_DBUS_SERVICE "org.kylin.KprBackend"
#define KPRBACKEND_DBUS_PATH "/SecurityQuestionAnswer"
#define KPRBACKEND_DBUS_INTERFACE "org.kylin.KprBackend"

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

SecurityQuestionAnswer::SecurityQuestionAnswer(QObject *parent)
    : QDBusAbstractInterface(
        KPRBACKEND_DBUS_SERVICE, KPRBACKEND_DBUS_PATH, KPRBACKEND_DBUS_INTERFACE, QDBusConnection::systemBus(), parent)
{
    setTimeout(2147483647);

    qDBusRegisterMetaType<QuestionInfo>();
    qDBusRegisterMetaType<QList<QuestionInfo>>();
    qDBusRegisterMetaType<AnswerInfo>();
    qDBusRegisterMetaType<QList<AnswerInfo>>();
}

bool SecurityQuestionAnswer::isActivatable()
{

}

QList<QuestionInfo> SecurityQuestionAnswer::GetPresetSecurityQuestions(QString language)
{
    QList<QuestionInfo> listQuestionInfo;
    QDBusReply<QList<QuestionInfo>> reply = call(QStringLiteral("GetPresetSecurityQuestions"), language);
    if (!reply.isValid()) {
        qWarning() << "GetPresetSecurityQuestions error: " << reply.error().message();
    } else {
        listQuestionInfo = reply.value();
    }
    return listQuestionInfo;
}

int SecurityQuestionAnswer::BindUserSecurityAnswers(int nUId, QList<AnswerInfo> listAnswerInfo)
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

bool SecurityQuestionAnswer::IsUserHasSecurityAnswers(int nUId)
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

QList<QuestionInfo> SecurityQuestionAnswer::GetUserSecurityQuestions(int nUId, QString language)
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

QString SecurityQuestionAnswer::VerifyUserSecurityAnswers(int nUId, QList<AnswerInfo> listAnswerInfo)
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

QString SecurityQuestionAnswer::EncodeSecurityInfo(const QString &strSource)
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

int SecurityQuestionAnswer::ChangePassword(int nUId, const QString &strPasswd, QString strToken)
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

int SecurityQuestionAnswer::SetUserPasswdHint(int nUId, QString strPasswdHint)
{
    QDBusMessage result = call(QStringLiteral("SetUserPasswdHint"), nUId, strPasswdHint);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "SetUserPasswdHint error:" << result.errorMessage();
        return -1;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toInt();
    } else {
        return -1;
    }
}

QString SecurityQuestionAnswer::GetUserPasswdHint(int nUId)
{
    QString userPasswdHint;
    QDBusReply<QString> reply = call(QStringLiteral("GetUserPasswdHint"), nUId);
    if (!reply.isValid()) {
        qWarning() << "GetUserPasswdHint error: " << reply.error().message();
    } else {
        userPasswdHint = reply.value();
    }
    return userPasswdHint;
}




