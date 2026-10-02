#ifndef ENCRYPTSETFRAME_H
#define ENCRYPTSETFRAME_H

#include <QObject>
#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QLineEdit>
#include <QWidget>
#include <QAction>
#include "../PluginService/ui_unit/arrowtooltip.h"
#include "../../KUserRegister/clineeditlabel.h"
#include "../../KUserRegister/checkedlineedit.h"
#include <QPainter>
#include <QPainterPath>
#include <QRadioButton>

using namespace KInstaller;
namespace KServer {

enum class LabelState {
    Username,
    Hostname,
    Pwd,
    CPwd,
    unkown
};
class EncryptSetFrame : public QDialog
{
    Q_OBJECT
public:
    explicit EncryptSetFrame(QWidget *parent = nullptr);

    void initUI();                                      // 加载界面ui

    void initTPMEncryptWidget();                        // 初始化硬件加密界面
    void initSoftEncryptWidget();                       // 初始化软件加密界面
    bool getTPMHarddisk();                              // 检测当前硬盘是否支持TPM加密
    void setSoftEncryptStatus(bool checked);            // 设置软件加密相关控件的状态

    void initFocusPolicy();                     //初始化焦点序列

    void initCloseButton();                             // 初始化关闭按钮界面
    void initPasswdWidget();                            // 初始化输入密码界面
    void initConmfirmPasswdWidget();                    // 初始化确认输入密码界面
    void initCheckpwdSafeWidget();                      // 初始化提示保管密码界面
    void initButtonWidget();                            // 初始化button界面
    QString SetFormatBody(QString text, QLabel *label);    // 设置字体...
    int GetFormatWidth(QLabel *label);              // 获取当前语言下“密码”和“确认密码”栏的最大宽度
    void initAllConnect();                              // 信号和槽链接
    void addStyleSheet();                               // 设置样式
    void translateStr();
    void setMessageInfo(QString info);
    void setCancleHidden();
    void markLineEditCorrect(LabelState state, bool flag);
    void addLineditAction();
    int execid;
signals:
    void signalCancel();
    void signalOK();


private:
    void changeEvent(QEvent *event) override;       //改变语言事件
    void resizeEvent(QResizeEvent* event);
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

public:
    // 主窗口布局
    QVBoxLayout* m_pMainLayout = nullptr;

    // 关闭窗口按钮窗口
    QWidget *m_pCloseButtonWidget = nullptr;
    QHBoxLayout* m_pCloseButtonHLayout = nullptr;
    QPushButton *m_pCloseButton;

    // 中间输入密码窗口
    QWidget *m_pPasswdWidget = nullptr;
    QHBoxLayout *m_pPassWdHLayout = nullptr;
    QLabel* m_pPasswdLabel = nullptr;
    QLineEdit *m_pPasswordBox;

    // 中间密码种类不符合系统要求提示字段
    QWidget *m_pPasswordPromptWidget = nullptr;
    QHBoxLayout *m_pPasswordPromptHBoxLayout = nullptr;
    QLabel *m_pPasswordPrompt = nullptr;

    // 中间确认输入密码窗口
    QWidget *m_pConfirmPasswdWidget = nullptr;
    QHBoxLayout *m_pConfirmPasswdLayout = nullptr;
    QLabel* m_pConfirmPasswdLabel = nullptr;
    QLineEdit *m_pPasswordCanfirmBox;

    // 确认密码两次输入不一致提示信息
    QWidget *m_pConfirmTipInfoWidget = nullptr;
    QHBoxLayout *m_pConfirmTipInfoHBoxLayout = nullptr;
    QLabel *m_pConfirmTipInfoLabel = nullptr;

    // 妥善保管密码提示
    QWidget *m_pCheckpwdSafeWidget = nullptr;
    QHBoxLayout *m_pCheckpwdSafeHBoxLayout = nullptr;
    QLabel *m_pCheckpwdSafeLabel;
    QLabel *m_pCheckpwdSafeSvgLabel;

    // 取消和确认按钮界面
    QWidget *m_pButtonWidget = nullptr;
    QHBoxLayout *m_pButtonHBoxLayout = nullptr;
    QPushButton *m_pCancelButton = nullptr;
    QPushButton *m_pOkButton = nullptr;

    QAction *m_pPasswdAction = nullptr;
    QAction *m_ppasswdComfirmAction = nullptr;

    QString errstr;
    CheckState m_state ;
    LabelState m_LabelState ;
    bool blbtn;
    ArrowWidget *m_tTip;
    bool m_flag = false;
    bool m_bPassWdAction = false;
    bool m_bPassWdConfirmAction = false;
    bool pwdIsOK = false;//密码是否有误
    bool cpwdIsOK = false;//确认密码是否有误

    QString m_passwordSting;
    QString m_comfirmPasswdString;


    QRadioButton* m_pSoftEncrypt;//软件加密

    //硬件加密
    bool m_isTPMEncrypt=false;//当前硬盘是否支持TPM加密
    QWidget *m_pTPMEncryptWidget = nullptr;
    QVBoxLayout *m_pTPMEncryptLayout = nullptr;
    QRadioButton *m_pTPMEncrypt;//TPM全盘加密
    QLabel *m_pTPMEncryptLabel;//注意事项




signals:
     void signalClickBtn(int execid);

public slots:
     void checkPasswd(QString pwd);
     void checkComfirmPasswd(QString comfirmpwd);
     void setActionIcon();
     void setComfirmActionIcon();
     void checkTPMEncrypt(bool checked);
     void checkSoftEncrypt(bool checked);

     // QWidget interface
 protected:
     void showEvent(QShowEvent *event);
};

}
#endif // ENCRYPTSETFRAME_H
