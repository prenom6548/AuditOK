#ifndef SECURITYQUESTIONANSWER_H
#define SECURITYQUESTIONANSWER_H

#include <QObject>
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

class SecurityQuestionAnswer : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    explicit SecurityQuestionAnswer(QObject *parent = nullptr);
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
    int BindUserSecurityAnswers(int nUId, QList<AnswerInfo> listAnswerInfo);

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
     * @brief SetUserPasswdHint 绑定用户密码提示
     * @param nUId 用户id
     * @param strPasswdHint 密码提示信息
     * @return 0 成功，其他失败
     */
    int SetUserPasswdHint(int nUId, QString strPasswdHint);

    /**
     * @brief GetUserSecurityQuestions 获取用户密码提示
     * @param nUId 用户id
     * @return 用户密码提示
     */
    QString GetUserPasswdHint(int nUId);
     
    /**
     * @brief VerifyAccountsInfo 校验用户安全问题答案
     * @param nUId 用户id
     * @param kyidAccounts 麒麟ID账户名
     * @param kyidpasswd 麒麟ID密码
     * @param code 麒麟ID验证码
     * @return 错误码 错误信息
     */
     QStringList VerifyAccountsInfo(int nUId, QString kyidAccounts, QString kyidpasswd, QString code);

     /**
      * @brief ChangePassword2 麒麟ID修改用户密码
      * @param nUId 用户id
      * @param strPasswd 新密码
      * @param strToken 校验返回的token
      * @return 0 修改成功，其他失败
      */
     int ChangePassword2(int nUId, const QString &strPasswd, QString strToken);
public:
    bool isActivatable();
};

#endif // SECURITYQUESTIONANSWER_H
