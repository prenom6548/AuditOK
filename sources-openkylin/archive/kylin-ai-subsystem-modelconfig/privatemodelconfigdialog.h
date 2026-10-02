#ifndef PRIVATEMODELCONFIGDIALOG_H
#define PRIVATEMODELCONFIGDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QIcon>
#include <QComboBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QMessageBox>
#include <QDesktopServices>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDir>

#include "privatemodelconfigwidget.h"
#include "kaglobal.h"

#include "libukcc/widgets/SettingWidget/addbutton.h"
#include "libukcc/widgets/SettingWidget/settinggroup.h"

#include <kysdk/applications/kswitchbutton.h>
#include <kysdk/applications/kpasswordedit.h>
#include <kysdk/applications/kpasswordedit.h>
#include <kysdk/applications/kborderlessbutton.h>
#include <aiconfig/modelconfig.h>
#include <polkit-qt5-1/polkitqt1-authority.h>
#include <ukuistylehelper/ukuistylehelper.h>

#define MAX_NAME_COUNT 32

using namespace std;
using namespace kdk;
using namespace kyai::config::model;


class PrivateModelConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PrivateModelConfigDialog(PrivateModel model,QWidget *parent = nullptr);
    explicit PrivateModelConfigDialog(QWidget *parent = nullptr);

    ~PrivateModelConfigDialog();
    // 添加模型-初始化模型UI
    // 修改函数签名
    void initUIData(AiCapability capability, DeployType deployType, const QString& sourceGroup = "publicCloud");
    // void initUIData(AiCapability capability, const DeployType deployType);
    // 编辑模型-加载模型数据
    void loadModelData(PrivateModel model);

    void setSourceGroup(const QString& sourceGroup) { m_sourceGroup = sourceGroup; }

    // 判断是公有云模型还是私有云模型
    QString m_sourceGroup;

signals:

public slots:
    void moveToCenter();

protected:
    void keyPressEvent(QKeyEvent* event) override;
private slots:
    void onPasswordEditChanged(KPasswordEdit *passwordEdit);
    void changeTheme();
private:
    // 布局
    void initLayout();
    // 信号槽
    void initConnect();
    // 根据所填内容更新按钮状态
    void updateButtonState();
    // 检查输入框中是否存在空格
    bool checkIsHasSpaces();

    void accept() override;

    bool isEdit = false;
    AiCapability m_capability;
    PrivateModel m_model;

    kdk::KLabel *labelCustomName = nullptr;
    QLabel *labelModelName = nullptr;                // 模型名称
    QLabel *labelApiKey = nullptr;
    kdk::KLabel *labelRequestUrl = nullptr;


    QLineEdit *lineeditCustomName = nullptr;
    QLineEdit *lineeditModelName = nullptr;
    kdk::KPasswordEdit  *passwordEditApi = nullptr;
    QLineEdit *lineeditRequestUrl = nullptr;

    QLabel *labelErrorCustomName = nullptr;
    QLabel *labelErrorModelName = nullptr;
    QLabel *labelErrorApiKey = nullptr;
    QLabel *labelErrorRequestUrl = nullptr;

    kdk::KBorderlessButton *btnHelpLink = nullptr;
    QPushButton *buttonCancel = nullptr;
    QPushButton *buttonAccept = nullptr;

    QWidget *contentWidget = nullptr;


    QMap<QString, QVector<AiCapability>> mapAllModel;

};

#endif // PRIVATEMODELCONFIGDIALOG_H
