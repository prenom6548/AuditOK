#ifndef AISUBSYSTEM_H
#define AISUBSYSTEM_H

#include <QObject>
#include <QVBoxLayout>
#include <QFrame>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>

#include <kysdk/applications/kbackgroundgroup.h>
#include <aisubsystemplugininterface.h>
#include <kysdk/applications/kswitchbutton.h>
#include <kysdk/applications/klabel.h>

#include "modelwidget.h"
#include "modelcallpriority.h"
#include "cloudmodelconfigwidget.h"
#include "localmodelconfigwidget.h"
#include "privatemodelconfigwidget.h"
#include "computingpowersharing/computepowersharewidget.h"
#include "computingpowersharing/sharedmodelconfigwidget.h"
#include "configmanager.h"

struct PriorityItem {
    int priority;
    DeployType deployType;
    ModelWidget *modelWidget;
};

class TestAiSubSystem : public QObject, KylinAiSubSystemPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kylinaisubsystem.PluginInterface")
    Q_INTERFACES(KylinAiSubSystemPluginInterface)

public:
    explicit TestAiSubSystem(QObject *parent = nullptr);
    //通过这个接口返回模型配置UI
    QWidget* createModelConfigWidget();
    QWidget* createComputePowerSharingWidget();
    static CloudModelConfigWidget* getCloudModelConfigPage();
    static LocalModelConfigWidget* getLocalModelConfigPage();
    static PrivateModelConfigWidget* getPrivateModelConfigPage();

private:
    void initConnect();
    void initModelGroup();
    void checkDomainControllerStatus();

    void saveModelPriority();
    void updateModelPriority(const QMap<DeployType, int> &priorityMap);

    void sortItemsByPriority();

    void moveItemUp(DeployType deployType);
    void updateUIOrder();

    DeployType getDeployTypeByFrame(QFrame *frame);
    bool hasLocalModel(); // 判断是否需要隐藏本地模型
    bool confirmCloseSharedService();
    bool confirmCloseLocalModel();
    void initComputePowerSharingFrame();
    bool isShowComputePowerSharingFrame();
    void initComputePowerSharingMainWidget();
    void setModelDeployEnable(DeployType deployType, bool enable);
    bool isEnableDomainControl();
    void initDomainControlTipWidget();

    void handleRequestConnectionSuccess();
    void handleRequestDisconnected();

public slots:
    void onModelPriorityChanged(DeployType topPriorityModelType);
    void onModelBootDelayChanged(bool checked); // 添加新的槽函数

private:
    QTranslator *m_translator = nullptr;

    QLabel *m_modelConfigLabel = nullptr;

    QWidget *m_domainControlTipWidget = nullptr;

    ModelWidget *m_publicCloudModel = nullptr;
    ModelWidget *m_ondeviceModel = nullptr;
    ModelWidget *m_privateModel = nullptr;
    ModelWidget *m_computePowerSharingModel = nullptr;

    QFrame *m_modelCallPriorityFrame = nullptr;
    ModelCallPriority *m_modelCallPriorityWidget = nullptr;

    KBackgroundGroup *m_modelGroup = nullptr;

    QWidget *m_mainWidget = nullptr;
    QVBoxLayout *m_mainVLayout = nullptr;

    QMap<DeployType, int> m_priorityMap;

    QList<PriorityItem> m_modelPriorityList;

    QLabel *m_perfResourceLabel = nullptr;
    QFrame *m_modelBootDelayLabelFrame = nullptr;
    QLabel *m_modelBootDelayLabel = nullptr;
    kdk::KSwitchButton *modelBootDelaySwitchBtn = nullptr;

    ComputePowerShareWidget *m_computePowerShareMainWidget = nullptr;
    SharedModelConfigWidget *m_sharedModelConfigWidget = nullptr;
    ConfigManager m_configManager;

    // 定义语音转文字模型名称常量
    static constexpr const char* speechToText = "speechToText";
};

#endif // AISUBSYSTEM_H