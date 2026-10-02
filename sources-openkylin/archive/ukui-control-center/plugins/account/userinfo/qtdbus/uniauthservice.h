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
#ifndef UNIAUTH_SERVICE_H
#define UNIAUTH_SERVICE_H

#include <QtDBus>
#include <QDBusAbstractInterface>
#include <QList>

struct QuestionInfo
{
    int nQuestionId;
    QString strQuestionText;
};

struct AnswerInfo
{
    int nQuestionId;
    QString strAnswerText;
};

Q_DECLARE_METATYPE(QuestionInfo)
Q_DECLARE_METATYPE(QList<QuestionInfo>)
Q_DECLARE_METATYPE(AnswerInfo)
Q_DECLARE_METATYPE(QList<AnswerInfo>)

QDBusArgument &operator<<(QDBusArgument &argument, const QuestionInfo &questionInfo);
const QDBusArgument &operator>>(const QDBusArgument &argument, QuestionInfo &questionInfo);

QDBusArgument &operator<<(QDBusArgument &argument, const AnswerInfo &answerInfo);
const QDBusArgument &operator>>(const QDBusArgument &argument, AnswerInfo &answerInfo);


class Groupservice : public QDBusAbstractInterface
{
    Q_OBJECT
public:
   explicit Groupservice(QObject *parent = nullptr);

public Q_SLOTS:
    /**
     * @brief BindUserSecurityAnswers 绑定用户安全问题
     * @param nUId 用户id
     * @param listAnswerInfo 安全问题答案信息
     * @return 0 成功，其他失败
     */
    int BindUserSecurityAnswers(int nUId, QList<AnswerInfo> listAnswerInfo);

private:
    bool m_isActivatable;
};

class UniAuthService : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    explicit UniAuthService(QObject *parent = nullptr);

public Q_SLOTS:
    /**
     * @brief GetPresetSecurityQuestions 获取预设安全问题列表
     * @param language 语言
     * @return 问题列表
     */
    QList<QuestionInfo> GetPresetSecurityQuestions(QString language);

    /**
     * @brief BindUserSecurityAnswers 绑定用户安全问题
     * @param nUId 用户id
     * @param listAnswerInfo 安全问题答案信息
     * @return 0 成功，其他失败
     */
    int BindUserSecurityAnswers(const int nUId, QList<AnswerInfo> listAnswerInfo);

    /**
     * @brief IsUserHasSecurityAnswers 用户是否绑定安全问题
     * @param nUId 用户id
     * @return true 已绑定，其他未绑定
     */
    bool IsUserHasSecurityAnswers(int nUId);

    /**
     * @brief GetUserSecurityQuestions 获取用户安全问题列表
     * @param nUId 用户id
     * @param language 语言
     * @return 安全问题列表
     */
    QList<QuestionInfo> GetUserSecurityQuestions(int nUId, QString language);

    /**
     * @brief VerifyUserSecurityAnswers 校验用户安全问题答案
     * @param nUId 用户id
     * @param listAnswerInfo 安全问题答案列表
     * @return 非空 校验token，空则校验失败
     */
    QString VerifyUserSecurityAnswers(int nUId, QList<AnswerInfo> listAnswerInfo);

    /**
     * @brief EncodeSecurityInfo 编码安全问题答案接口
     * @param strSource 原始安全问题答案
     * @return 编码后的安全问题答案
     */
    QString EncodeSecurityInfo(const QString &strSource);

    /**
     * @brief ChangePassword 修改用户密码
     * @param nUId 用户id
     * @param strPasswd 新密码
     * @param nTimeout 超时
     * @param strToken 校验返回的token
     * @return 0 修改成功，其他失败
     */
    int ChangePassword(int nUId, const QString &strPasswd, QString strToken);

    /**
     * @brief SetUserPasswordHint 修改用户密码提示
     * @param nUId 用户id
     * @param strHint 密码提示
     * @return 0 修改成功，其他失败
     */
    int SetUserPasswordHint(int nUid, const QString &strHint);

    /**
     * @brief GetUserPasswordHint 获取用户密码提示
     * @param nUId 用户id
     * @return 密码提示
     */
    QString GetUserPasswordHint(int nUid);

    int CheckUserHistoryPassword(const QString &strUser, const QString &strPasswd);

public:
    bool isActivatable();
    bool authoriyEdit();

private:
    bool m_isActivatable;
    qint64 _id = 0;
};

#endif // UNIAUTH_SERVICE_H
