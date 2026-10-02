#ifndef LOCALMODELITEM_H
#define LOCALMODELITEM_H

#include <aiconfig/modelconfig.h>

#include <QGSettings>
#include <QGraphicsColorizeEffect>
#include <QGraphicsEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QToolButton>
#include <QVariant>
#include <QWidget>

#include "kaglobal.h"
#include "ktoolbutton.h"
#include "updatenotificationwidget.h"
#include "util.h"

#define THEME_QT_SCHEMA  "org.ukui.style"
#define TEMP_REBOOT_FILE "/tmp/temp_reboot_file.txt"
using namespace std;
using namespace kyai::config::model;

enum MessageTipType{
    InfoMessage =0x0,
    WarningMessage,
    PersistentMessage
};

class LocalModelItem :public QWidget
{
    Q_OBJECT
public:
    explicit LocalModelItem(PublicLocalModel model, SystemVersion curSystemVersion, QWidget *parent = nullptr);
    ~LocalModelItem();

    void showSelectedIcon(bool flag);
    AiCapability getAICapability();
    QString getModelName();
    QString getModelDebName();
    bool getIsStartDownloading();
    void showMessageTips(MessageTipType type,const QString &msg,const int &duration);
    void setGrayedOut(bool available);

private:
    void initModelDownloadLayout();
    void initLayout();
    void initMenu();
    void initInstallSuccessMsgBox();
    void initConnect();
    void initIconLabel();
    void initCertificationLabel();
    QGraphicsEffect* createGrayEffect();
    void setupThemeListener();                      // 添加主题监听设置方法
    void updateIconForTheme(const QString& theme);  // 更新图标以适配主题
    QString getCurrentTheme();                      // 获取当前主题
    void updateVendorIcon(QLabel* label, const QString& defaultIconPath,
                          const QString& lightIconPath);  // 更新供应商图标

signals:
    void sigEdit(AiCapability capability, QString modelName);
    void sigDelete(AiCapability capability, QString modelName);
    void sigUpdate(AiCapability capability, QString modelName);
    void sigUninstall(AiCapability capability, QString modelName);
public slots:
    void checkHasNewVersion();
    bool createRebootTempFile();
    bool checkHasRebootTempFile();
    void sldownModel();
    void startNetErrorTips();
    void slUpdateModelInstallStatus(int value,QString status ,QString details);
    void slUpdateModelInstallFinished(bool isSuccess,QStringList upgradeGroup,QString errorStr,QString errorDesc);
    void slUpdateCancelButton(bool isInstalling);
private:
    QHBoxLayout *m_mainHLayout;
    //模型名称
    QLabel *m_modelNameLabel;

    //模型面板
    QHBoxLayout *m_panelHLayout;
    //信息
    QHBoxLayout *m_messageHLayout;
    QWidget *m_messageWidget;
    QLabel *m_tipsLabel;
    QToolButton *m_moreButton = nullptr;  // 更多
    QToolButton *m_downButton = nullptr;  //下载
    KToolButton *m_loadButton = nullptr;  // 加载
    QMenu *m_menu;
    //下载进度条
    QHBoxLayout *m_progressHLayout;
    UpdateNotificationWidget *updateProgressBar =nullptr;
    //选择框
    QLabel *m_selectedIconLabel = nullptr;// 已选择状态
    // 添加图标标签
    QLabel* m_iconLabel = nullptr;  // 模型图标
    // 认证标签
    kdk::KLabel* m_certificationLabel = nullptr;  // 认证标签
    // 下载成功对话框
    QMessageBox *m_installSuccessMsgBox;

    // 显示后摩卡被拔出状态的标签
    QLabel* m_deviceStatusLabel = nullptr;

    //当前提示信息
    QString m_currentTipsMsg;

    // 主题设置对象
    QGSettings* m_themeSettings = nullptr;

private:
    kyai::config::model::AiCapability m_capability;
    QString m_modelName;
    QString m_vendor;
    PublicLocalModel m_model;
    LocalModelInstallStatus m_modelStatus;
    bool isModelSelected;
    bool isDownLoading =false;
    bool isStartDownloading = false;
    QTimer *m_timer;

    QString m_currentInfoMessage = "";
    QString m_currentWarningMessage = "";

    QTimer* m_infoTimer = nullptr;
    QTimer* m_warningTimer = nullptr;

    SystemVersion m_curSystemVersion;

    // 添加用于存储选中和未选中状态的Pixmap
    QPixmap m_unselectedPixmap;
    QPixmap m_selectedPixmap;
};

#endif // LOCALMODELITEM_H
