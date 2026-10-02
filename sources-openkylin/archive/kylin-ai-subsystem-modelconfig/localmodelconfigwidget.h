#ifndef LOCALMODELCONFIGWIDGET_H
#define LOCALMODELCONFIGWIDGET_H
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
#include <QFileInfo>
#include <klabel.h>
#include <QApplication>
#include <QProcess>

#include "localmodelitem.h"
#include "kaglobal.h"
#include "util.h"

#include "libukcc/widgets/SettingWidget/addbutton.h"
#include "libukcc/widgets/SettingWidget/settinggroup.h"
#include "ukui-log4qt.h"

#include <kysdk/applications/kswitchbutton.h>
#include <kysdk/applications/kpasswordedit.h>
#include <kysdk/applications/kborderlessbutton.h>
#include <kysdk/applications/kmessagebox.h>
#include <aiconfig/modelconfig.h>
#include <polkit-qt5-1/polkitqt1-authority.h>

#include "configmanager.h"

using namespace std;
using namespace kdk;
using namespace kyai::config::model;

class LocalModelConfigWidget : public QDialog
{
    Q_OBJECT
public:

    explicit  LocalModelConfigWidget(QWidget *parent = nullptr);
    void initLayout();
    void initModelTitleLayout();
    void initModellayout();
    void showCentered();
    ModelStatus getModelStatus(QString modelName);

signals:
    void sigSetSelectLocalModel(AiCapability capability, QString modelName);
    void modelUninstall(const QString &modelName);
    void switchModel();

private slots:
    void onUpdateModelItem(AiCapability capability, QString modelName);
    void onUninstallModelItem(AiCapability capability, QString modelName);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    void getCurSystemVersion();
    void initOtherModelLayout();
    void setupThemeListener();  // 添加主题监听设置方法
    void updateSystemModelIconsForTheme(
        const QString& theme);  // 更新系统模型图标以适配主题
    QString getCurrentTheme();  // 获取当前主题
    void updateSystemModelIcon(
        QLabel* label, const QString& defaultIconPath,
        const QString& lightIconPath);  // 更新系统模型图标

    // 加载nlp模型
    void loadNlpModelConfig();
    // 加载除nlp以外的其他模型
    void loadOtherModelConfig();
    // 信号槽
    void initConnect();
    // 更新NLP模型添加按钮的可见性
    void updateNlpAddButtonVisibility();

    void openSoftwareStore();
    // 加载模型
    void loadModelItem(PublicLocalModel model,QString curSelectedName);
    // 删除模型
    void deleteModelItem(AiCapability capability, QString modelName);
    // 切换选中模型
    void switchModelItem(AiCapability capability, QString modelName);
    // 设置选中模型名称
    bool setSelectLocalModelName(AiCapability capability, DeployType deployType, QString modelName);
    // 点击模型
    void clickLocalModel(AiCapability capability, QString modelName);
    // 检查该模型是否已选择
    bool checkModelSelection(AiCapability capability, QString modelName);

    // 检查当前本地模型是否安装
    bool checkCurrentLocalModelInstall(const QString &filePath);

    bool confirmSwitchLocalModel();
    bool confirmUnloadLocalModel();

private:
    SystemVersion m_curSystemVersion;

    QVBoxLayout *m_mainVLayout;
    QHBoxLayout *m_titleHLayout;
    QLabel *m_titleLabel;
    QVBoxLayout *m_vLeftLayout = nullptr;
    QLabel *m_iconLabel = nullptr;
    QVBoxLayout *m_localHLayout = nullptr;
    QLabel *m_localLabel = nullptr;
    kdk::KLabel *m_descripLabel = nullptr;

    QVBoxLayout *m_baseVLayout = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QWidget *m_scrollWidget = nullptr;
    QVBoxLayout *m_scrollVLayout = nullptr;

    //NLP大语言模型
    QWidget *m_nlpWidget = nullptr;
    QVBoxLayout *m_nlpDescVLayout = nullptr;
    QHBoxLayout *m_nlpHLayout = nullptr;
    QVBoxLayout *m_nlpVLayout = nullptr;

    QLabel* m_nlpLabel = nullptr;
    kdk::KLabel* m_nlpDescLabel = nullptr;
    QHBoxLayout* m_nlpLabelHLayout = nullptr;
    QPushButton* m_nlpAddItemButton = nullptr;
    KBorderlessButton* m_nlpAddButton = nullptr;

    KBorderlessButton *m_installButton = nullptr;
    KBorderlessButton* m_updateOrUninstallBtn = nullptr;

    QHBoxLayout *m_tipHLayout = nullptr;
    kdk::KLabel* m_tipLabel = nullptr;

    QHBoxLayout *m_nlpFrameHLayout = nullptr;
    QLabel *m_nlpFrameLabelText = nullptr;
    QLabel *m_nlpFrameSelectedIcon = nullptr;

    // 系统内置模型 图像识别、向量化、语音处理等其他系统运行所需的模型
    QVBoxLayout *m_otherVLayout = nullptr;
    QVBoxLayout *m_otherDescVLayout = nullptr;
    QLabel *m_otherLabel = nullptr;
    kdk::KLabel *m_otherDescLabel = nullptr;
    QLabel *m_otherFrameLabelText = nullptr;

    // VISION视觉模型
    QWidget *m_visionWidget = nullptr;
    QVBoxLayout *m_visionDescVLayout = nullptr;
    QHBoxLayout *m_visionHLayout = nullptr;
    QVBoxLayout *m_visionVLayout = nullptr;
    QLabel *m_visionLabel = nullptr;
    kdk::KLabel *m_visionDescrLabel = nullptr;

    // SPEECH语音模型
    QWidget *m_speechWidget = nullptr;
    QVBoxLayout *m_speechDescVlayout =nullptr;
    QHBoxLayout *m_speechHLayout = nullptr;
    QVBoxLayout *m_speechVLayout = nullptr;
    QLabel * m_speechLabel = nullptr;
    kdk::KLabel * m_speechDescLabel = nullptr;

    // 图像，语音模型组相关组件
    QLabel* m_otherVisionFrameLabelText = nullptr;
    QLabel* m_otherSpeechFrameLabelText = nullptr;
    UkccFrame* m_otherVisionItemFrame = nullptr;
    UkccFrame* m_otherSpeechItemFrame = nullptr;
    QLabel* m_otherVisionLabel = nullptr;
    QLabel* m_otherSpeechLabel = nullptr;

    // 图标标签
    QLabel* otherVisionIconLabel = nullptr;
    QLabel* otherSpeechIconLabel = nullptr;
    QLabel* otherIconLabel = nullptr;
    QLabel* nlpIconLabel = nullptr;
    // 新增系统模型图标标签
    QLabel* m_otherVisionIconLabel = nullptr;
    QLabel* m_otherSpeechIconLabel = nullptr;
    QLabel* m_otherIconLabel = nullptr;

    SettingGroup* m_otherVisionModelGroup = nullptr;
    SettingGroup* m_otherSpeechModelGroup = nullptr;

    //确定按钮
    QPushButton *m_confirmLocalButton = nullptr;
    QHBoxLayout *m_confirmLocalButtoLayout = nullptr;

    ConfigManager m_configManager;

    // 主题设置对象
    QGSettings* m_themeSettings = nullptr;

public:
    QMap<QString, PublicLocalModel> m_nlpLocalModelMap;
    QMap<QString, PublicLocalModel> m_visionLocalModelMap;
    QMap<QString, PublicLocalModel> m_speechLocalModelMap;

    QMap<QString, LocalModelItem*> m_nlpLocalModelItemMap;
    QMap<QString, LocalModelItem*> m_visionLocalModelItemMap;
    QMap<QString, LocalModelItem*> m_speechLocalModelItemMap;

    QMap<QString, UkccFrame*> m_nlpLocalModelMapUI;
    QMap<QString, UkccFrame*> m_visionLocalModelMapUI;
    QMap<QString, UkccFrame*> m_speechLocalModelMapUI;

    SettingGroup *m_nlpLocalModelGroup = nullptr;
    SettingGroup *m_visionLocalModelGroup = nullptr;
    SettingGroup *m_speechLocalModelGroup = nullptr;
    SettingGroup *m_otherModelGroup = nullptr;

    QString nlpSelectedLocalModel;
    QString visionSelectedLocalModel;
    QString speechSelectedLocalModel;
    bool continuousSelected = false;
    bool realtimeSelected = false;
};

#endif // LOCALMODELCONFIGWIDGET_H
