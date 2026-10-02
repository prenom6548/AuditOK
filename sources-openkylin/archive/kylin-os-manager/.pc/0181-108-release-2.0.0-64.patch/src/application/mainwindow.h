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
}

class MainWindow : public kdk::KWidget {
    Q_OBJECT
public:
    enum class PluginOperate {
        kDisplayProblemFeedback = 0,
        kDisplayServiceSupport,

        // 故障诊断
        kRepairSpecificError,
        kDiagnosisSpecificType,
        kUserExperiencePopup,

        // 垃圾清理
        kRunJunkClean,

        // 设备管理
        kDisplayDeviceManager,
        kDisplayHardwareInfo,
        kDisplayDriverManager,
    };

    MainWindow(std::vector<KomApplicationProvider *> tabProviders, QWidget *parent = nullptr);
    ~MainWindow();
    bool SwitchTab(const QString &plugin_name);
    void ControlPlugin(PluginOperate operate, QVariant data = QVariant());

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private Q_SLOTS:
    void onNavigationClicked(const QModelIndex index);
    void showUserManual();
    void showAbout();

private:
    enum NavigationItemRole {
        WidgetPointer = Qt::UserRole + 1,
        Name = Qt::UserRole + 2,
        CallbackFunc = Qt::UserRole + 3,
    };

    void initUi(void);
    void moveCenter();
    QIcon loadPluginIcon(std::string name);
    void jumpToSecondaryWidget(QModelIndex jumpIndex);
    void internal(); //内部模式特殊处理

    std::vector<KomApplicationProvider *> m_tabProviders; /* 指针不需要释放，插件管理器析构时会释放相应的资源 */
    kdk::KNavigationBar *m_navigationBar;
    QStackedWidget *m_stackWidget;
    std::map<std::string, std::shared_ptr<KomApplicationInterface>> m_tabInterfaces;
    std::map<std::string, QStandardItem *> m_tabStandardItems;
};

Q_DECLARE_METATYPE(QCloseEvent *)

#endif
