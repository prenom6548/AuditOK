#ifndef MODELCALLPRIORITY_H
#define MODELCALLPRIORITY_H

#include <aiconfig/modelconfig.h>
#include <kysdk/applications/klabel.h>

#include <QComboBox>
#include <QLabel>
#include <QMap>
#include <QObject>
#include <QWidget>

class ModelCallPriority : public QWidget {
    Q_OBJECT
public:
    ModelCallPriority(bool hasLocalModel, bool isShowSharedOption, QWidget *parent = nullptr);
    void setModelCallPriority(kyai::config::model::DeployType deployType);
    void setSharedModelOptionVisible(bool visible);

signals:
    void topModelPriorityChanged(kyai::config::model::DeployType topPriorityModelType);

public Q_SLOTS:
    void modelPriorityChanged(int index);

private:
    void initConnections();

private:
    bool m_hasLocalModel;
    bool m_isShowSharedOption;

    kdk::KLabel *m_modelCallPriorityLabel = nullptr;  // 模型调用优先级排序标签
    QComboBox *m_modelPriorityComboBox = nullptr;     // 模型调用优先级下拉列表
};

#endif  // MODELCALLPRIORITY_H
