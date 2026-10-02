#ifndef KYLIN_OS_MANAGER_PLUGINS_TOOL_BOX_MAINWINDOW_H
#define KYLIN_OS_MANAGER_PLUGINS_TOOL_BOX_MAINWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QListWidget>
#include <ktabbar.h>
#include <ksearchlineedit.h>
#include "text_label.h"
#include "blankpage.h"

namespace tool_box {

class MainWindow: public QWidget {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    enum CategoryIndex {
        kAllTools = 0,
        kFeatureTools,
        kDebugTools,
        kTroubleshootingTools,
        kOtherTools
    };

    void CreateTools();
    void CreateToolsFromConfFile();
    void CreateToolsFromDBus();
    void SetUpListWidget(QListWidget *list_widget);
    void SetUpListWidgetItem(QListWidgetItem *item);
    void UpdateInterface();
    void AddChildWidgetToStackedWidget(QStackedWidget *stacked_widget, QListWidget *list_widget, BlankPage *blank_page);
    void SearchCategoryTools(const QString &text);
    void SearchListWidgetTools(QStackedWidget *stacked_widget, QListWidget *list_widget, const QString &text);

    kdk::KTabBar *         category_tab_bar_;
    kdk::KSearchLineEdit * search_line_edit_;
    QStackedWidget *       stacked_widget_;
    QListWidget *          feature_tools_list_widget_;
    QListWidget *          debug_tools_list_widget_;
    QListWidget *          troubleshooting_tools_list_widget_;
    QListWidget *          other_tools_list_widget_;
    BlankPage *            blank_feature_tools_widget_;
    BlankPage *            blank_debug_tools_widget_;
    BlankPage *            blank_troubleshooting_tools_widget_;
    BlankPage *            blank_other_tools_widget_;
    QStackedWidget *       stacked_feature_tools_widget_;
    QStackedWidget *       stacked_debug_tools_widget_;
    QStackedWidget *       stacked_troubleshooting_tools_widget_;
    QStackedWidget *       stacked_other_tools_widget_;

};

}

#endif
