#ifndef EXPANDABLEMODELGROUP_H
#define EXPANDABLEMODELGROUP_H

#include <aiconfig/modelconfig.h>
#include <kysdk/applications/klabel.h>

#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
// 引入UkccFrame
// #include "libukcc/widgets/SettingWidget/ukccframe.h"

using namespace kyai::config::model;

class ExpandableModelGroup : public QWidget {
    Q_OBJECT

public:
    explicit ExpandableModelGroup(const QString& title, const QString& subtitle,
                                  kyai::config::model::DeployType deployType, QWidget* parent = nullptr);

    void setContentLayout(QLayout* contentLayout);
    void updateContentHeight();  // 新增：更新内容高度的方法
    void addContentWidget(QWidget* widget);
    void increaseContentHeight();
    void decreaseContentHeight();
    bool isExpanded() const { return m_isExpanded; }
    void setExpanded(bool expanded);

signals:
    void expanded(bool isExpanded);

private:
    void initLayout();

private slots:
    void toggleContent();

private:
    kyai::config::model::DeployType m_deployType;

    // UI组件
    QWidget* m_mainFrame = nullptr;
    QFrame* m_titleFrame = nullptr;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_descriptionLabel = nullptr;
    QToolButton* m_expandButton = nullptr;
    QWidget* m_contentWidget = nullptr;

    // 布局
    QVBoxLayout* m_mainLayout = nullptr;
    QHBoxLayout* m_titleLayout = nullptr;
    QVBoxLayout* m_contentLayout = nullptr;

    // 状态
    bool m_isExpanded = false;
};

#endif  // EXPANDABLEMODELGROUP_H