#ifndef PWDLESSSERVICE_H
#define PWDLESSSERVICE_H

#include <QtDBus>
#include <QDBusAbstractInterface>
#include <QList>

class PwdlessService : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    explicit PwdlessService(QObject *parent = nullptr);

public Q_SLOTS:
    /**
     * @brief 获取查询用户支持的认证方法（已绑定对应认证因子）
     * @param 用户名
     * @return 支持的方法列表，目前有otp、ukey
     */
    QStringList GetBoundFactorMethods(QString user);

    /**
     * @brief 查询当前双因子开关状态
     * @param method:如果双因子状态打开，返回当前第二因子认证方法，否则为空
     * @return 0 关闭，1 开启
     */
    int GetDualFactorStatus(QString &method); // int status, QString meth2

    /**
     * @brief mfaEnable 启用双因子认证，并设置第二认证因子类型
     * @param meth2 第二因子认证方法，当前支持otp、ukey
     * @return 设置结果，0成功，非零失败
     */
    int EnableDualFactor(QString meth2);

    /**
     * @brief mfaDisable 停用双因子认证
     * @return 设置结果，0成功，非零失败
     */
    int DisableDualFactor();

    /**
     * @brief sshAuthStatus 获取ssh是否允许当前认证方法
     * @param method method:认证方法，当前支持otp、ukey
     * @return 0表示不允许，1表示允许
     */
    bool GetSshAuthStatus(QString method);

    /**
     * @brief sshAuthEnable 允许或者关闭ssh使用指定认证方法
     * @param method 认证方法，当前支持otp、ukey
     * @param enable 0 停用   1启用
     * @return 0成功，非零失败
     */
    int SetSshAuthStatus(QString method, int enable);

    /**
     * @brief otpBind 绑定OTP
     * @param user 用户名
     * @param flag 0为创建，1为更新
     * @return
     */
   QString GetOtpQrCode(QString user, int flag); //->(int result, String qrcode)

    /**
     * @brief otpVerify 校验OTP密钥，校验不通过则无法完成OTP绑定
     * @param user 用户名
     * @param token OTP密码
     * @return 0为成功，非零失败
     */
    int BindOtpVerify(QString user, QString token, QString &errorMsg);//->(int result, String msg)

    /**
     * @brief ukeyBind 绑定Ukey
     * @param user 用户名
     * @param pin ukey的pin码
     * @return 0为成功，非零失败
     */
   int BindUkeyVerify(QString user, QString pin, QString &errorMsg); //->(int result, String msg)

    /**
     * @brief ukeyUnbind 解绑用户的Ukey
     * @param user 用户名
     * @return 0为成功，非零失败
     */
   int UnBindUkey(QString user, QString &errorMsg);//->(int result, String msg)

    /**
     * @brief ukeyStatus 获取当前Ukey状态，确认Ukey是否已插入
     * @return 0 Ukey未插入， 1 Ukey已插入
     */
    int IsInsertUkey();

    /**
     * @brief getOtpEmergMsg 获取当前OTP绑定结果
     * @return 0 成功
     */
    int getOtpEmergMsg(QString user, QString &msg);
};

#endif // PWDLESSSERVICE_H
