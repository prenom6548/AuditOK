#ifndef MODELWIDGET_H
#define MODELWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QToolButton>

#include <kysdk/applications/kswitchbutton.h>
#include <kysdk/applications/kborderlessbutton.h>
#include <kysdk/applications/klabel.h>
#include <aiconfig/modelconfig.h>
#include <QVBoxLayout>

using namespace kdk;
using namespace kyai::config::model;

// 自定义模型Widget类
class ModelWidget : public QWidget {
    Q_OBJECT

public:
    explicit ModelWidget(const QString &modelName, const QString &modelIntro, DeployType deployType, QWidget *parent = nullptr);
    ~ModelWidget();
    QSize sizeHint() const override;
    void initConnect();
    void setModelSwitchBtnChecked(bool checked);
    void setModelSwitchBtnEnable(bool enable);
    void setConfigButtonText(const QString& text);
    void setTipButtonVisible(bool visible);

public slots:
    void onTransModelDeployStatusChanged(bool enable);
    void onSortButtonClicked();

signals:
    void configClicked(const QString &modelName);
    void sortClicked(); // 添加一个排序点击信号
    void closeModelSwitchBtn();

public:
    QPushButton *m_sortButton; // 排序按钮
    QLabel *modelNameLabel;      // 模型名称标签
    kdk::KLabel *modelIntroLabel;     // 模型简介标签
    kdk::KBorderlessButton *configButton; // 配置按钮
    kdk::KBorderlessButton *viewButton; // 查看按钮
    QPushButton *tipButton = nullptr;

    KSwitchButton *modelSwitchBtn;  // 自定义的开关按钮
    QVBoxLayout *m_modelLayout = nullptr;
    QHBoxLayout *m_layout = nullptr;
    QString modelWidgetName;
    int m_priority;  // 优先级属性
    DeployType m_deployType; // 标识是云端模型还是本地模型
    kdk::KLabel* modelAuthLabel;  // 认证标签，显示"麒麟认证"或"用户自选"
};
#endif  // MODELWIDGET_H
