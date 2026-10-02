#ifndef COMPONENTWIDGET_H
#define COMPONENTWIDGET_H

#include <QWidget>
#include <QScopedPointer>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <sourcedialog.h>
#include <QDebug>

class ComponentWidgetPrivate;

/**
 * @brief 构建组件配置界面
 */
class ComponentWidget : public QWidget
{
    Q_OBJECT
public:
    enum Mode {
        AddMode,  // 添加模式
        EditMode  // 编辑模式
    };

    ComponentWidget(QWidget *parent = nullptr);
    virtual ~ComponentWidget();
    void setMode(Mode mode); // 设置模式
    void cleanup();// 暴露给外部的清理函数
    void loadComponentData(const KMBuilderManifest::Component &component);

signals:
    void cancelComponentClicked();
    void okComponentClicked();
    void componentSaved(const KMBuilderManifest::Component &component);

private:
    Q_DECLARE_PRIVATE(ComponentWidget)
    QScopedPointer<ComponentWidgetPrivate> d_ptr;
};

#endif // COMPONENTWIDGET_H
