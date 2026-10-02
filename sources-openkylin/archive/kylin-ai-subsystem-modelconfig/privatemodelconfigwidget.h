#ifndef PRIVATEMODELCONFIGWIDGET_H
#define PRIVATEMODELCONFIGWIDGET_H

#include <aiconfig/modelconfig.h>
#include <klabel.h>
#include <kysdk/applications/kbackgroundgroup.h>
#include <kysdk/applications/kborderlessbutton.h>
#include <kysdk/applications/kpasswordedit.h>
#include <kysdk/applications/kswitchbutton.h>

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QFile>
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
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include "expandablemodelgroup.h"
#include "kaglobal.h"
#include "libukcc/widgets/SettingWidget/addbutton.h"
#include "libukcc/widgets/SettingWidget/settinggroup.h"
#include "libukcc/widgets/SettingWidget/ukccframe.h"
#include "modelordermanager.h"
#include "privatemodelconfigdialog.h"
#include "privatemodelitemwidget.h"
#include "ukui-log4qt.h"
#include "util.h"
#include "configmanager.h"

using namespace std;
using namespace kdk;
using namespace kyai::config::model;

class PrivateModelConfigWidget : public QDialog {
    Q_OBJECT
    friend class PrivateModelConfigDialog;

public:
    explicit PrivateModelConfigWidget(QWidget* parent = nullptr);
    void initLayout();
    void initModelTitleLayout();
    void initModelLayout();
    void showCentered();
    ModelStatus getModelStatus(const QString &modelName);

    // 添加模型
    void addModelItem(PrivateModel model, const QString& sourceGroup = "publicCloud");
    // 获取ModelItemMap
    QMap<QString, PrivateModelItemWidget*>& getModelItemMap(AiCapability capability);
    // 编辑选中模型
    bool editSelectPrivateModel(PrivateModel model, bool isContinuousSelected, bool isRealTimeSelected,
                                bool isValueChanged, QDialog* parentDialog);
    // 设置选中模型
    bool setSelectPrivateModelConfig(PrivateModel model);
    bool editSelectCloudModel(PrivateModel model, bool isContinuousSelected, bool isRealTimeSelected,
                              bool isValueChanged, QDialog* parentDialog);
    bool setSelectCloudModelConfig(PrivateModel model, bool continuousSelectedState = true,
                                   bool realtimeSelectedState = true);

    void setSourceGroup(const QString& sourceGroup) { m_sourceGroup = sourceGroup; }
    void loadModelItemWithSourceGroup(PrivateModel model, const QString& sourceGroup, const QString& selectedModelName,
                                      bool isCustomModel);
    bool hasLocalModel(); // 添加判断本地模型支持的函数
    void updateLocalModelGroupVisibility(); // 添加更新本地模型组可见性的函数

signals:
    void sigSetSelectPrivateModel(AiCapability capability, QString modelName);
public slots:
    void onEditModelItem(AiCapability capability, QString modelName);
    void onDeleteModelItem(AiCapability capability, QString modelName);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override; // 添加closeEvent重写声明
private slots:
    // void nlpAddModelItem();
    // void visionAddModelItem();
    // void speechAddModelItem();
private:
    // 从sdk加载数据
    void loadModelConfig();
    // 信号槽
    void initConnect();
    // 加载模型
    void loadModelItem(PrivateModel model, QString curSelectedName);
    // 删除模型
    void deleteModelItem(AiCapability capability, QString modelName);
    // 切换选中模型
    void switchModelItem(AiCapability capability, QString modelName);

    // 设置选中模型名称
    bool setSelectPrivateModelName(AiCapability capability, DeployType deployType, QString modelName);
    bool setSelectCloudModelName(AiCapability capability, DeployType deployType, QString modelName);
    // 点击模型
    void clickPrivateModel(AiCapability capability, QString modelName);

    // 恢复模型key默认值
    bool clearModelMultiAuths(AiCapability capability, QString modelName);

    // 清除自定义模型
    bool clearCustomModel(AiCapability capability, QString modelName);

    // 添加自定义模型
    bool addPrivateModel(PrivateModel model);
    // 更新自定义模型
    bool updatePrivateModel(PrivateModel model);
    // 检查该模型是否已选择
    bool checkModelSelection(AiCapability capability, QString modelName);
    // 授权弹窗
    bool polkit();

    void initExpandableModelGroups();
    void updateNlpPublicCloudModelVisibility();
    void updateVisionPublicCloudModelVisibility();
    void updateSpeechPublicCloudModelVisibility();

    void onNlpPublicCloudAddClicked();
    void onNlpPublicCloudAddItemClicked();
    void onVisionPublicCloudAddClicked();
    void onVisionPublicCloudAddItemClicked();
    void onSpeechPublicCloudAddClicked();
    void onSpeechPublicCloudAddItemClicked();
    void updateLocalNetworkNlpModelVisibility();
    void onLocalNetworkNlpAddClicked();
    void onLocalNetworkNlpAddItemClicked();

    // 模型来源组管理函数
    QString getModelSourceGroup(const QString& modelName);
    void saveModelSourceGroup(const QString& modelName, const QString& sourceGroup);
    QHash<QString, QString> loadModelSourceGroups();
    void initializeModelSourceGroups();
    void processModelInfo(std::vector<ExtendedModel>& models, AiCapability aiCapability);
    bool keyIsEmpty(const QString& modelName);
    void setupAddItemButtonStyle(QPushButton* button);

    // 添加静态状态管理器
    struct GroupExpansionState {
        static bool publicCloudExpanded;
        static bool localNetworkExpanded;
        static bool localExpanded;
    };

    // 添加保存和恢复状态的方法
    void saveExpansionStates();
    void restoreExpansionStates();

private:
    QVBoxLayout* m_mainVLayout = nullptr;
    QHBoxLayout* m_titleHLayout = nullptr;
    QVBoxLayout* m_privateHLayout = nullptr;
    QVBoxLayout *m_vLeftLayout = nullptr;
    QLabel* m_iconLabel = nullptr;
    QLabel *m_privateLabel = nullptr;
    kdk::KLabel *m_descripLabel = nullptr;

    QString m_sourceGroup;

    // 滚动区域相关
    QScrollArea *m_scrollArea = nullptr;
    QWidget *m_scrollWidget = nullptr;
    QVBoxLayout *m_scrollVLayout = nullptr;

    // 公有云模型组
    ExpandableModelGroup* m_publicCloudGroup = nullptr;

    // 新增：公有云文本模型相关
    KBackgroundGroup* m_nlpPublicCloudModelGroup = nullptr;
    kdk::KBorderlessButton* m_nlpPublicCloudAddButton = nullptr;
    QPushButton* m_nlpPublicCloudAddItemButton = nullptr;

    // 新增：公有云图像模型相关
    KBackgroundGroup* m_visionPublicCloudModelGroup = nullptr;
    kdk::KBorderlessButton* m_visionPublicCloudAddButton = nullptr;
    QPushButton* m_visionPublicCloudAddItemButton = nullptr;

    // 新增：公有云语音模型相关
    KBackgroundGroup* m_speechPublicCloudModelGroup = nullptr;
    kdk::KBorderlessButton* m_speechPublicCloudAddButton = nullptr;
    QPushButton* m_speechPublicCloudAddItemButton = nullptr;

    // 新增：局域网模型组
    ExpandableModelGroup* m_localNetworkGroup = nullptr;
    KBackgroundGroup* m_nlpLocalNetworkModelGroup = nullptr;
    kdk::KBorderlessButton* m_nlpLocalNetworkAddButton = nullptr;
    QPushButton* m_nlpLocalNetworkAddItemButton = nullptr;

    // 新增：本地模型组
    ExpandableModelGroup* m_localGroup = nullptr;
    KBackgroundGroup* m_nlpLocalModelGroup = nullptr;
    KBorderlessButton* m_helpLinkButton = nullptr;

    // 确认按钮相关
    QWidget *m_confirmWidget = nullptr;
    QPushButton *m_confirmPrivateButton = nullptr;
    QHBoxLayout *m_confirmPrivateButtoLayout = nullptr;

    // 模型来源组持久化存储
    QHash<QString, QString> m_modelSourceGroups;  // 存储模型名到来源组的映射
    QString m_configFilePath;                     // 配置文件路径

public:
    QMap<QString, PrivateModel> m_nlpPublicCloudModelMap;
    QMap<QString, PrivateModel> m_visionPublicCloudModelMap;
    QMap<QString, PrivateModel> m_speechPublicCloudModelMap;
    QMap<QString, PrivateModel> m_nlpLocalNetworkModelMap;
    QMap<QString, PrivateModel> m_nlpLocalModelMap;

    QMap<QString, PrivateModelItemWidget*> m_nlpPublicCloudModelItemMap;
    QMap<QString, PrivateModelItemWidget*> m_visionPublicCloudModelItemMap;
    QMap<QString, PrivateModelItemWidget*> m_speechPublicCloudModelItemMap;
    QMap<QString, PrivateModelItemWidget*> m_nlpLocalNetworkModelItemMap;
    QMap<QString, PrivateModelItemWidget*> m_nlpLocalModelItemMap;

    QList<PrivateModelItemWidget*> m_nlpPublicCloudModelItemList;
    QList<PrivateModelItemWidget*> m_visionPublicCloudModelItemList;
    QList<PrivateModelItemWidget*> m_speechPublicCloudModelItemList;
    QList<PrivateModelItemWidget*> m_nlpLocalNetworkModelItemList;
    QList<PrivateModelItemWidget*> m_nlpLocalModelItemList;

    QMap<QString, QFrame*> m_nlpPublicCloudMapUI;
    QMap<QString, QFrame*> m_visionPublicCloudMapUI;
    QMap<QString, QFrame*> m_speechPublicCloudMapUI;
    QMap<QString, QFrame*> m_nlpLocalNetworkMapUI;
    QMap<QString, QFrame*> m_nlpLocalMapUI;

    QString nlpSelectedModel;
    QString visionSelectedPublicCloudModel;
    QString speechSelectedPublicCloudModel;

    bool continuousSelected = false;
    bool realtimeSelected = false;
    bool isContinuousSelected = true;
    bool isRealTimeSelected = true;

    ModelOrderManager m_modelOrderManager;
    ConfigManager m_configManager;
};

#endif  // PRIVATEMODELCONFIGWIDGET_H