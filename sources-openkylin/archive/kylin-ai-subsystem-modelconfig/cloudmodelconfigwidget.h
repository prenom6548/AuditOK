#ifndef CLOUDMODELCONFIGWIDGET_H
#define CLOUDMODELCONFIGWIDGET_H
#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTextEdit>
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
#include <klabel.h>
#include <QApplication>

#include "cloudmodelitem.h"
#include "modelconfigdialog.h"
#include "modelordermanager.h"
#include "kaglobal.h"
#include "libukcc/widgets/SettingWidget/addbutton.h"
#include "libukcc/widgets/SettingWidget/settinggroup.h"

#include <kysdk/applications/kswitchbutton.h>
#include <kysdk/applications/kpasswordedit.h>
#include <kysdk/applications/kborderlessbutton.h>
#include <aiconfig/modelconfig.h>
#include <polkit-qt5-1/polkitqt1-authority.h>

using namespace std;
using namespace kdk;
using namespace kyai::config::model;

class CloudModelConfigWidget : public QDialog
{
    Q_OBJECT
    friend class ModelConfigDialog;
public:
    explicit CloudModelConfigWidget(QWidget *parent = nullptr);
    ~CloudModelConfigWidget();
    void initLayout();
    void initModelTitleLayout();
    void initModellayout();
    ModelStatus getModelStatus(QString modelName);
    void showCentered();
signals:
    void sigSetSelectCloudModel(AiCapability capability, QString modelName);
public slots:
    // void onEditModelItem(AiCapability capability, QString modelName);
    void onDeleteModelItem(AiCapability capability, QString modelName);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
private slots:
    void onUpdateModelItem(AiCapability capability, QString modelName);
    void onUninstallModelItem(AiCapability capability, QString modelName);

private:
    // 从sdk加载数据
    void loadModelConfig();
    // 信号槽
    void initConnect();
    // 添加模型
    void addModelItem(PublicCloudModel model);

    void loadModels();
    void loadNoFreeModels();
    void processModelInfo(std::vector<ExtendedModel>& models, AiCapability aiCapability);
    bool keyIsEmpty(const QString& modelName);
    void loadFreeTrialModels();
    void loadFreeTrialModelItem(PublicCloudModel model, bool isSelected);

    // 加载模型
    void loadModelItem(PublicCloudModel model, QString curSelectedName);
    // 删除模型
    void deleteModelItem(AiCapability capability, QString modelName);
    // 切换选中模型
    void switchModelItem(AiCapability capability, QString modelName);
    // 设置选中模型
    bool setSelectCloudModelConfig(PublicCloudModel model);
    // 设置选中模型名称
    bool setSelectCloudModelName(AiCapability capability, DeployType deployType, QString modelName);
    // 编辑选中模型
    // 返回值表示是否需要关闭当前正在编辑的窗口
    bool editSelectCloudModel(PublicCloudModel model, bool isContinuousSelected, bool isRealTimeSelected,
                              bool isValueChanged, QDialog* parentDialog);
    // 点击模型
    void clickCloudModel(AiCapability capability, QString modelName);
    // 恢复模型key默认值
    bool clearModelMultiAuths(AiCapability capability, QString modelName);
    // 检查该模型是否已选择
    bool checkModelSelection(AiCapability capability, QString modelName);
    // 授权弹窗
    bool polkit();
    // 获取ModelItemMap
    QMap<QString, ModelItem*>& getModelItemMap(AiCapability capability);
    void updateNlpAddButtonVisibility();
    void openSoftwareStore();

private:
    QVBoxLayout *m_mainVLayout = nullptr;
    QHBoxLayout *m_titleHLayout = nullptr;
    QLabel *m_iconLabel = nullptr;
    QLabel *m_cloudLabel = nullptr;
    kdk::KLabel *m_descripLabel = nullptr;
    QVBoxLayout *m_cloudHLayout = nullptr;
    QVBoxLayout *m_vLeftLayout = nullptr;
    QVBoxLayout *m_baseVLayout = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QWidget *m_scrollWidget = nullptr;
    QVBoxLayout *m_scrollVLayout = nullptr;

    //NLP大语言模型
    QWidget *m_nlpWidget = nullptr;
    QVBoxLayout *m_nlpDescVLayout = nullptr;
    QHBoxLayout *m_nlpHLayout = nullptr;
    QVBoxLayout *m_nlpVLayout = nullptr;
    QLabel * m_nlpLabel = nullptr;
    kdk::KLabel * m_nlpDescLabel = nullptr;
    QLabel* m_nlpIconLabel = nullptr;
    KBorderlessButton *m_nlpAddButton = nullptr;
    QPushButton* m_nlpAddItemButton = nullptr;

    // VISION视觉模型
    QWidget *m_visionWidget = nullptr;
    QVBoxLayout *m_visionDescVLayout = nullptr;
    QHBoxLayout *m_visionHLayout = nullptr;
    QVBoxLayout *m_visionVLayout = nullptr;
    QLabel *m_visionLabel = nullptr;
    kdk::KLabel *m_visionDescrLabel = nullptr;
    KBorderlessButton *m_visionAddButton = nullptr;
    QPushButton* m_visionAddItemButton = nullptr;

    // SPEECH语音模型
    QWidget *m_speechWidget = nullptr;
    QVBoxLayout *m_speechDescVlayout =nullptr;
    QHBoxLayout *m_speechHLayout = nullptr;
    QVBoxLayout *m_speechVLayout = nullptr;
    QLabel * m_speechLabel = nullptr;
    kdk::KLabel * m_speechDescLabel = nullptr;
    KBorderlessButton *m_speechAddButton = nullptr;
    QPushButton* m_speechAddItemButton = nullptr;

    // 确定按钮
    QWidget *m_confirmWidget = nullptr;
    QPushButton *m_confirmButton = nullptr;
    QHBoxLayout *m_confirmButtoLayout = nullptr;

public:
    QMap<QString, PublicCloudModel> m_nlpModelMap;
    QMap<QString, PublicCloudModel> m_visionModelMap;
    QMap<QString, PublicCloudModel> m_speechModelMap;

    QMap<QString, ModelItem*> m_nlpModelItemMap;
    QMap<QString, ModelItem*> m_visionModelItemMap;
    QMap<QString, ModelItem*> m_speechModelItemMap;

    QList<ModelItem*> m_nlpModelItemList;
    QList<ModelItem*> m_visionModelItemList;
    QList<ModelItem*> m_speechModelItemList;

    QMap<QString, UkccFrame*> m_nlpModelMapUI;
    QMap<QString, UkccFrame*> m_visionModelMapUI;
    QMap<QString, UkccFrame*> m_speechModelMapUI;

    SettingGroup *m_nlpModelGroup = nullptr;
    SettingGroup *m_visionModelGroup = nullptr;
    SettingGroup *m_speechModelGroup = nullptr;
    SettingGroup *m_freeTrialModelGroup = nullptr;

    QString nlpSelectedModel;
    QString visionSelectedModel;
    QString speechSelectedModel;
    bool continuousSelected = false;
    bool realtimeSelected = false;

    ModelOrderManager m_modelOrderManager;
};

#endif // CLOUDMODELCONFIGWIDGET_H
