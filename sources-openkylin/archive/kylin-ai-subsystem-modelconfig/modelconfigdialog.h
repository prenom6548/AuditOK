#ifndef MODELCONFIGDIALOG_H
#define MODELCONFIGDIALOG_H

#include <aiconfig/modelconfig.h>
#include <kysdk/applications/kborderlessbutton.h>
#include <kysdk/applications/kpasswordedit.h>
#include <kysdk/applications/kswitchbutton.h>
#include <polkit-qt5-1/polkitqt1-authority.h>
#include <ukuistylehelper/ukuistylehelper.h>

#include <QComboBox>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QGSettings>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWidget>

#include "kaglobal.h"
#include "libukcc/widgets/SettingWidget/addbutton.h"
#include "libukcc/widgets/SettingWidget/settinggroup.h"
#include "privatemodelconfigwidget.h"

#define THEME_QT_SCHEMA  "org.ukui.style"

using namespace std;
using namespace kdk;
using namespace kyai::config::model;

class ModelConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ModelConfigDialog(PrivateModel model, QWidget* parent = nullptr);
    explicit ModelConfigDialog(QWidget *parent = nullptr);
    ~ModelConfigDialog();

    // 添加模型-初始化模型UI
    void initUIData(AiCapability capability, const DeployType deployType);
    // 编辑模型-加载模型数据
    void loadModelData(PrivateModel model);

protected:
    void keyPressEvent(QKeyEvent* event) override;

signals:

public slots:
    void moveToCenter();

private slots:
    void onPasswordEditChanged(KPasswordEdit *passwordEdit);
    void onSwitchContinuousChanged(bool isVisible);
    void onSwitchRealTimeChanged(bool isVisible);
    void updateLayout(int index);
    void changeTheme();

private:
    // 布局
    void initLayout();
    // 信号槽
    void initConnect();
    // 根据模型需填内容改变尺寸
    void adjustDialogSize();
    // 根据所填内容更新按钮状态
    void updateButtonState();
    // 检查输入框中是否存在空格
    bool checkIsHasSpaces();
    // 添加model到map
    // void addModelToMap(AiCapability capability, const PrivateModel &model);

    void accept() override;

    QString getModelIconPath(const QString& vendor);
    QString getModelIconPathByTheme(const QString& vendor);  // 根据主题返回模型图标路径
    void setupThemeListener();              // 设置主题监听（为保持接口一致性）
    QString getCurrentTheme();              // 获取当前主题

    bool isEdit = false;
    AiCapability m_capability;
    PrivateModel m_model;

    kdk::KLabel *labelModelName = nullptr;                // 模型类型
    QLabel *labelAPPID = nullptr;
    QLabel *labelApiKey = nullptr;
    kdk::KLabel *labelSecretKey = nullptr;
    QLabel *labelErrorInfo = nullptr;

    QComboBox *comboboxModelName = nullptr;
    kdk::KPasswordEdit  *passwordAPPID = nullptr;
    kdk::KPasswordEdit  *passwordEditApi = nullptr;
    kdk::KPasswordEdit *passwordEditSecret = nullptr;

    // 流式
    QFrame *lineSeparator1 = nullptr;
    QLabel *labelSpeechContinuous = nullptr;
    QLabel *labelAPPIDContinuous = nullptr;
    QLabel *labelApiKeyContinuous = nullptr;
    kdk::KLabel *labelSecretKeyContinuous = nullptr;
    kdk::KSwitchButton *switchContinuous = nullptr;         // 开启：必填
    kdk::KPasswordEdit  *pwdAppIdContinuous = nullptr;
    kdk::KPasswordEdit  *pwdEditApiContinuous = nullptr;
    kdk::KPasswordEdit *pwdEditSecretContinuous = nullptr;
    QWidget *continuousWidget = nullptr;
    // 实时
    QFrame *lineSeparator2 = nullptr;
    QLabel *labelSpeechRealTime = nullptr;
    QLabel *labelAPPIDRealTime = nullptr;
    QLabel *labelApiKeyRealTime = nullptr;
    kdk::KSwitchButton *switchRealTime = nullptr;           // 开启：必填
    kdk::KPasswordEdit  *pwdAppIdRealTime = nullptr;
    kdk::KPasswordEdit  *pwdEditApiRealTime = nullptr;
    QWidget *realTimeWidget = nullptr;

    kdk::KBorderlessButton *btnHelpLink= nullptr;
    QPushButton *buttonCancel = nullptr;
    QPushButton *buttonAccept = nullptr;

    QWidget *contentWidget = nullptr;

    bool isContinuousSelected = true;
    bool isRealTimeSelected = true;

    QMap<QString, QVector<AiCapability>> mapAllModel;
    
    // 不再需要m_themeSettings成员，因为ModelConfigDialog只在需要时查询主题
};


#endif // MODELCONFIGDIALOG_H