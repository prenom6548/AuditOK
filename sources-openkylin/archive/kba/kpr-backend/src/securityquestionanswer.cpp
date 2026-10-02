/*
 * Copyright (C) 2024 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "../include/kba/kba.h"
#include <QLocale>
#include <QByteArray>
#include <QDebug>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <locale.h>
#include <libintl.h>

#define _(string) gettext(string)

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

SecurityQuestionAnswer::SecurityQuestionAnswer(QObject *parent) : QObject(parent)
{
    qDBusRegisterMetaType<QuestionInfo>();
    qDBusRegisterMetaType<QList<QuestionInfo>>();
    qDBusRegisterMetaType<AnswerInfo>();
    qDBusRegisterMetaType<QList<AnswerInfo>>();
}

SecurityQuestionAnswer::~SecurityQuestionAnswer() {}

QList<QuestionInfo> SecurityQuestionAnswer::GetPresetSecurityQuestions(QString language)
{
    QList<QuestionInfo> listQuestionInfo;
    int nCount = 0;
    int nIndex = 0;
    updateLanguage(language);
    SecurityQuestionListNode *questionNode = getPresetSecurityQuestions(&nCount);
    for (; questionNode && nIndex < nCount; questionNode = questionNode->next) {
        SecurityQuestionListNode *pThis = questionNode;
        QuestionInfo info;
        info.nQuestionId = pThis->question.questionId;
        info.strQuestionText = _(pThis->question.questionText);
        listQuestionInfo.append(info);
        free(pThis);
        nIndex++;
    }
    return listQuestionInfo;
}

int SecurityQuestionAnswer::BindUserSecurityAnswers(int nUId, QList<AnswerInfo> listAnswerInfo)
{
    SecurityAnswerListNode *nodeHead = NULL;
    for (auto answerInfo : listAnswerInfo) {
        SecurityAnswer answer;
        memset(&answer, 0, sizeof(SecurityAnswer));
        answer.userId = nUId;
        answer.questionId = answerInfo.nQuestionId;
        strcpy(answer.encryptedAnswer, QByteArray::fromBase64(answerInfo.strAnswerText.toUtf8()).data());
        addAnswerNode(&nodeHead, (void *)&answer, sizeof(SecurityAnswer));
    }
    int nResult = setUserSecurityAnswers(nodeHead);
    for (; nodeHead; nodeHead = nodeHead->next) {
        free(nodeHead);
    }
    return nResult;
}

bool SecurityQuestionAnswer::IsUserHasSecurityAnswers(int nUId)
{
    if (IsUserSecurityAnswers(nUId) == 1) {
        return true;
    } else {
        return false;
    }
}

QList<QuestionInfo> SecurityQuestionAnswer::GetUserSecurityQuestions(int nUId, QString language)
{
    QList<QuestionInfo> listQuestionInfo;
    int nCount = 0;
    int nIndex = 0;
    updateLanguage(language);
    SecurityQuestionListNode *questionNode = getUserSecurityQuestions(nUId, &nCount);
    for (; questionNode && nIndex < nCount; questionNode = questionNode->next) {
        SecurityQuestionListNode *pThis = questionNode;
        QuestionInfo info;
        info.nQuestionId = pThis->question.questionId;
        info.strQuestionText = _(pThis->question.questionText);
        listQuestionInfo.append(info);
        free(pThis);
        nIndex++;
    }
    return listQuestionInfo;
}

QString SecurityQuestionAnswer::VerifyUserSecurityAnswers(int nUId, QList<AnswerInfo> listAnswerInfo)
{
    SecurityAnswerListNode *nodeHead = NULL;
    QString strToken = "";
    char *token = nullptr;
    for (auto answerInfo : listAnswerInfo) {
        SecurityAnswer answer;
        memset(&answer, 0, sizeof(SecurityAnswer));
        answer.userId = nUId;
        answer.questionId = answerInfo.nQuestionId;
        strcpy(answer.encryptedAnswer, QByteArray::fromBase64(answerInfo.strAnswerText.toUtf8()).data());
        addAnswerNode(&nodeHead, (void *)&answer, sizeof(SecurityAnswer));
    }
    int nResult = verifyUserSecurityAnswers(nodeHead, &token);
    for (; nodeHead; nodeHead = nodeHead->next) {
        free(nodeHead);
    }
    if (token) {
        strToken = token;
        free(token);
        token = nullptr;
    }
    if (nResult != 1 || strToken.isEmpty()) {
        qInfo() << "verifyUserSecurityAnswers failed:" << nResult;
        strToken.clear();
    }
    return QString(strToken.toUtf8().toBase64());
}

QString SecurityQuestionAnswer::EncodeSecurityInfo(const QString &strSource)
{
    QString strResult = "";
    char *chResult = nullptr;
    int nRet = KPR_PKCS5_PBKDF2_HMAC2_v2(
        reinterpret_cast<unsigned char *>(strSource.toUtf8().data()), strSource.toUtf8().count(), &chResult);
    if (nRet != 0 || !chResult) {
        qWarning() << "KPR_PKCS5_PBKDF2_HMAC2_v2 failed:" << nRet;
        return strResult;
    }
    strResult = QByteArray(chResult).toBase64();
    free(chResult);
    chResult = nullptr;
    return strResult;
}

int SecurityQuestionAnswer::ChangePassword(int nUId, const QString &strPasswd, QString strToken)
{
    struct passwd *pwinfo = getpwuid(nUId);
    if (pwinfo && pwinfo->pw_name) {
        int nRet = changePassword(
            pwinfo->pw_name, strPasswd.toUtf8().data(), QByteArray::fromBase64(strToken.toUtf8()).data());
        return nRet;
    }
    return -1;
}

int SecurityQuestionAnswer::SetUserPasswdHint(int nUId, const QString &strPasswdHint)
{
    int nRet = set_password_hint(nUId, strPasswdHint.toUtf8().data());
    
    return nRet;
}

QString SecurityQuestionAnswer::GetUserPasswdHint(int nUId) 
{
    char password_hint[256] = {0};
    int nRet = get_password_hint(nUId, password_hint);
    if (nRet == 0) {
        return QString::fromUtf8(password_hint);
    } else {
        return QString(); // 返回空字符串表示用户未找到
    }
}

void SecurityQuestionAnswer::updateLanguage(QString languageCode)
{
    if (languageCode.startsWith("zh_CN")) {
        setenv("LANGUAGE", "zh_CN", 1);
        setenv("LANG", "zh_CN", 1);
        setlocale(LC_ALL, "zh_CN.utf8");
        QLocale lang("zh_CN");
        /*这里设置一个QLocale的默认值，用来影响插件的语言。插件加载翻译文件时，不使用QLocale::system().name()来获取语言，
         * 应该使用QLocale local; local.name() 来获取语言,这样可以识别到登录界面设置的的语言环境*/
        QLocale::setDefault(lang);
    } else if (languageCode.startsWith("zh_HK")) {
        setenv("LANGUAGE", "zh_HK", 1);
        setenv("LANG", "zh_HK", 1);
        setlocale(LC_ALL, "zh_HK.utf8");
        QLocale lang("zh_HK");
        QLocale::setDefault(lang);
    }else if (languageCode.startsWith("mn_MN")) {
        setenv("LANGUAGE", "mn_MN", 1);
        setenv("LANG", "mn_MN", 1);
        setlocale(LC_ALL, "mn_MN.utf8");
        QLocale lang("mn_MN");
        QLocale::setDefault(lang);
    }else if (languageCode.startsWith("bo_CN")) {
        setenv("LANGUAGE", "bo_CN", 1);
        setenv("LANG", "bo_CN", 1);
        setlocale(LC_ALL, "bo_CN.utf8");
        QLocale lang("bo_CN");
        QLocale::setDefault(lang);
    }
    else {
        setenv("LANGUAGE", languageCode.toUtf8().data(), 1);
        setenv("LANG", languageCode.toUtf8().data(), 1);
        setlocale(LC_ALL, "");
        /*这里设置一个QLocale的默认值，用来影响插件的语言。插件加载翻译文件时，不使用QLocale::system().name()来获取语言，
         * 应该使用QLocale local; local.name() 来获取语言,这样可以识别到登录界面设置的的语言环境*/
        QLocale::setDefault(languageCode);
    }
    bindtextdomain("kba", "/usr/share/locale");
    textdomain("kba");
}
