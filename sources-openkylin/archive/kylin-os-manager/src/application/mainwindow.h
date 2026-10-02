/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef KYLIN_OS_MANAGER_SRC_MAINWINDOW_H
#define KYLIN_OS_MANAGER_SRC_MAINWINDOW_H

#include <memory>
#include <vector>
#include <map>

#include <QModelIndex>
#include <kwidget.h>

class QKeyEvent;
class QStackedWidget;
class QStandardItem;
class KomApplicationProvider;
class KomApplicationInterface;

namespace kdk {
class KNavigationBar;
class KPushButton;
}

class MainWindow : public kdk::KWidget {
    Q_OBJECT
public:
    enum class PluginOperate {
        // 服务支持
        kDisplayProblemFeedback = 0,
        kDisplayServiceSupport,
        kDisplayServiceSupportMainpage,
        kSwitchToFaqSpecificType,
        // 返回按钮
        kBackButtonClicked,
        kChaneBackButtonState,

        // 故障诊断
        kRunFaultDiagnosis,
        kRepairSpecificError,
        kDiagnosisSpecificType,
        kUserExperiencePopup,

        // 垃圾清理
        kRunJunkClean,

        // 设备管理
        kDisplayDeviceManager,
        kDisplayHardwareInfo,
        kDisplayDriverManager,

        // 百宝箱
        kDisplayToolBox,

        // 首页
        kDisplayHomePage
    };

    MainWindow(std::vector<KomApplicationProvider *> tabProviders, QWidget *parent = nullptr);
    ~MainWindow();
    void Init();
    bool SwitchTab(const QString &plugin_name);
    void ControlPlugin(PluginOperate operate, QVariant data = QVariant());

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private Q_SLOTS:
    void onNavigationClicked(const QModelIndex index);
    void showUserManual();
    void showAbout();
    void on_ShowSetWidget();

private:
    enum NavigationItemRole {
        WidgetPointer = Qt::UserRole + 1,
        Name = Qt::UserRole + 2,
        // 使用 Qt::UserRole + 3 不管设置什么类型的数据，在获取数据时，获取到的类型总是 bool 类型，导致数据不能正确获取
        // 不知道是什么原因导致的，此处越过 Qt::UserRole + 3
        CallbackFunc = Qt::UserRole + 4,
    };

    void initUi(void);
    void moveCenter();
    QIcon loadPluginIcon(std::string name);
    void jumpToSecondaryWidget(QModelIndex jumpIndex);
    void internal();    //内部模式特殊处理

    kdk::KNavigationBar *m_navigationBar;
    kdk::KPushButton *   m_backButton; 
    QStackedWidget *     m_stackWidget;
    std::vector<KomApplicationProvider *>  m_tabProviders;    /* 指针不需要释放，插件管理器析构时会释放相应的资源 */
    std::map<std::string, QStandardItem *> m_tabStandardItems;
    std::map<std::string, std::shared_ptr<KomApplicationInterface>> m_tabInterfaces;
    QLabel *m_placeholderLabel;
};

Q_DECLARE_METATYPE(QCloseEvent *)

#endif
