#ifndef STACKEDWIDGETMANAGER_H
#define STACKEDWIDGETMANAGER_H

#include <QStackedWidget>

enum StackedPage {
    PAGE_START = 0,     // 引导页
    PAGE_MANIFEST,      // 清单配置页
    PAGE_PERMISSION,    // 权限配置页
    PAGE_BUILDSETTINGS, // 构建配置页
    PAGE_OUTPUT,        // 构建输出页
    PAGE_COMPONENT,     // 组件配置页
    END
};

class StackedWidgetManager : public QStackedWidget
{
    Q_OBJECT
public:
    StackedWidgetManager(QWidget* parent = nullptr);
    ~StackedWidgetManager();

    StackedPage addWidget(StackedPage index, QWidget* w);

public slots:
    void setCurrentPage(StackedPage index);

private:
    StackedPage m_curPageIndex = StackedPage::PAGE_MANIFEST;
};

#endif // STACKEDWIDGETMANAGER_H
